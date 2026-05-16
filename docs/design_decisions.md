# 設計判断ドキュメント

> **最重要文書** — このドキュメントはアーキテクチャ上の判断とその根拠を記録する。
> コードを変更する前に必ずこのドキュメントを参照すること。

---

## DD-01: C++（NDK）+ Kotlin のハイブリッド構成

**判断**: OpenCV処理全般をC++ NDK（共有ライブラリ `.so`）で実装し、UIとカメラ制御をKotlinで実装する。

**根拠**:
- OpenCVはC++ネイティブライブラリであり、NDK経由が最も直接的で高速。
- CameraXはJava/KotlinファーストのAPIであり、Kotlin側で制御する方が圧倒的に実装が簡単（Surface管理、ライフサイクル連携など）。
- 処理と表示を異なる言語レイヤーで完全に分離することで、将来的にアルゴリズムのみをC++レイヤーで改良できる。

**却下した代替案**:
- *OpenCV Java SDK のみ*: OpenCV の Javaラッパーを使えばKotlinのみで実装可能だが、JNI呼び出しオーバーヘッドがフレームレベルで発生し、パラメータ変更のたびにオブジェクト再生成が必要になる。また内部的にはJNI経由なので結局同様のコストが発生する。
- *Kotlinだけで完結させる（RenderScript等）*: OpenCV関数（SIFT/ORB/AKAZE）はRenderScriptでは実装不可能。

---

## DD-02: JNI境界を1ファイル（jni_bridge.cpp）に集約する

**判断**: JNI関数の定義を `jni_bridge.cpp` のみに行い、ビジネスロジックは `FeatureProcessor` クラスに分離する。

**根拠**:
- JNI関数名はパッケージ名を含む長い命名規則（`Java_com_hanzitong_...`）に縛られる。ロジックまで同一ファイルに書くと保守性が著しく低下する。
- jni_bridge.cpp はグルー層として薄く保つことで、将来C++側をピュアC++プロジェクトとして単独テストできる。
- Kotlin側から見ると `FeatureProcessorJni` オブジェクトだけがJNIの存在を知る。UI層やViewModelはJNIを直接呼ばない。

---

## DD-03: Strategy パターン（アルゴリズム切替）

**判断**: SIFT/ORB/AKAZEをそれぞれ `IFeatureDetector` を実装したStrategyクラスとして実装する。

**根拠**:
- アルゴリズムは実行時に切り替わるため、if-elseやswitch-caseを `FeatureProcessor` に書くと切替のたびにクラス全体を変更する必要が生じる（Open/Closed原則違反）。
- Strategyにより、新しいアルゴリズム（例：BRISK、KAZE）の追加が `IFeatureDetector` を実装した新クラスを追加するだけで済む。`FeatureProcessor` のコードは変更不要。
- 各Strategyクラスはそのパラメーターセットのみを保持し、他のアルゴリズムのパラメーターを知らない。

**実装の注意**:
- C++の `enum class AlgorithmType` の ordinal 値とKotlinの `enum class Algorithm` の `ordinal` プロパティを常に同じ順序で維持すること。JNI経由でint値として渡すため、ズレると別のアルゴリズムが呼ばれるバグが発生する。
- 現在の正しい対応: SIFT=0, ORB=1, AKAZE=2

---

## DD-04: MVVM + StateFlow（Kotlin側の状態管理）

**判断**: KotlinのUI層はMVVM（ViewModel + StateFlow）で設計する。

**根拠**:
- スライダー値（パラメーター）・選択アルゴリズム・キーポイント座標・統計情報など複数の状態をUIが同時に観察する必要がある。StateFlowは複数コレクターを持てる。
- ViewModelはCamera のライフサイクル（ユーザーが画面回転しても生存）をまたぐ。
- Jetpack Composeは `collectAsStateWithLifecycle()` でStateFlowと自然に統合できる。
- Observerパターンが自然に実現されており、スライダー変更 → ViewModel → JNIコール → 次フレームに反映 という一方向データフローを保てる。

**却下した代替案**:
- *LiveData*: StateFlowよりKotlinコルーチンとの統合が劣る。
- *直接コールバック*: 状態の一元管理ができず、UIと処理が密結合になる。

---

## DD-05: キーポイント座標のみをC++から返す（描画はKotlin/Compose）

**判断**: C++の `processFrame` はキーポイントのx, y, size 配列のみを返す。`cv::drawKeypoints` は使わない。描画はKotlinの `Canvas` が行う。

**根拠**:
- 描画スタイル（色、太さ、形状）の変更がC++の再コンパイルなしに調整できる。
- 処理層と表示層の完全分離という設計方針に合致する。
- Bitmapをやり取りするとピクセルデータのJNI越しコピーが発生し、高コストになる。座標配列は軽量。

**実装の注意**:
- C++のキーポイント座標系（カメラフレーム画素）とCompose Canvasの座標系（dp/px）の変換が必要。`KeypointOverlay` 内でスケーリング係数 `scaleX = canvasWidth / frameWidth` を使って変換すること。
- C++でrotationを適用済みのため、Kotlin側に渡すフレームサイズは回転後のサイズ（90°/270°回転時はwidthとheightが入れ替わる）を使うこと。

---

## DD-06: ミューテックスによるスレッド間排他制御

**判断**: `FeatureProcessor` に `std::mutex` を持たせ、`setAlgorithm`・`setParameter`・`processFrame` すべてでロックを取る。

**根拠**:
- `processFrame` はCameraXの分析スレッド（バックグラウンド）から呼ばれる。
- `setAlgorithm` / `setParameter` はKotlinのメインスレッド（UIイベント）から呼ばれる。
- ミューテックスなしでは、フレーム処理中にアルゴリズム切替が走り、`detector_` がdangling状態になるUBが発生する。

**パフォーマンス上の注意**:
- ロック中はUIスレッドがブロックされる可能性があるが、`setParameter`/`setAlgorithm` の処理は数マイクロ秒で完了するため実用上問題ない。
- `processFrame` のロック時間はアルゴリズムの処理時間（数ms～数十ms）に相当するため、スライダーを素早く動かした場合に短い遅延が発生することがある。

---

## DD-07: YUV420 Y平面 = グレースケール（色変換コスト削減）

**判断**: カメラフレームのYUV420でY平面のみをC++に渡し、そのまま `cv::Mat(height, width, CV_8UC1, yuv)` として使う。`cv::cvtColor` を使った明示的なBGR→グレー変換は行わない。

**根拠**:
- YUV420のY（輝度）成分はグレースケール画像そのものであり、OpenCVの特徴量検出器はすべてグレースケール入力を受け付ける。
- BGRへの変換は不要なメモリコピーとCPUコストを発生させる。
- CameraX の `OUTPUT_IMAGE_FORMAT_YUV_420_888` モードではY平面のピクセルストライドは常に1（1バイト/ピクセル）が保証される。ただし行ストライド（rowStride）は width より大きいことがあるため、Kotlin側でパディングを除去して渡している。

---

## DD-08: arm64-v8a 専用ビルド

**判断**: `abiFilters` を `arm64-v8a` のみに限定する。

**根拠**:
- Google Pixel 7a（Google Tensor G2）はarm64-v8aのみ。x86/x86_64/armeabi-v7aのライブラリを含める必要がない。
- ABI絞り込みにより APKサイズが削減され、ビルド時間も短縮される。
- `-march=armv8-a+simd` によりNEON SIMDを有効化し、OpenCVの内部最適化パスを活用できる。

---

## DD-09: OpenCV Android SDK のリンク方式

**判断**: OpenCV公式のAndroid SDK（zip）をダウンロードし、`sdk/native/jni/` を CMake の `OpenCV_DIR` として指定する `find_package` 方式を採用する。

**根拠**:
- `org.opencv:opencv` Mavenパッケージ（AAR）は存在するが、NDKのネイティブヘッダーへのアクセス方法がバージョンによって不安定。
- `find_package(OpenCV)` 方式は `OpenCVConfig.cmake` が `.so` のリンクと include パスを自動設定するため、CMakeLists.txt が簡潔になる。
- OpenCV Android SDKのバージョンを `local.properties` で管理することで、SDKをgit管理対象外（`.gitignore`）にしつつ複数バージョンの切り替えが容易になる。

**必要なモジュール**: `core`, `imgproc`, `features2d` のみ。（`xfeatures2d` は不要 — SURFは採用しない）

---

## DD-10: minSdk = 33（Android 13）

**判断**: 最小サポートAPIレベルを33（Android 13）とする。

**根拠**:
- Pixel 7aは Android 13 で発売。これが唯一のターゲット端末。
- API 33以上で使えるAPIを制限なく使用できる（例：新しいメディアパーミッションモデル等）。
- 広い端末互換性は本プロジェクトの要件ではない。

---

## DD-11: テスト戦略と層分離

**判断**: 3層構成のテストを採用する。

| 層 | テスト種別 | ファイル | 実行方法 |
|----|-----------|---------|---------|
| Kotlin ViewModel / UiState | ユニットテスト (JVM) | `app/src/test/` | `./gradlew test` |
| JNI + C++全体統合 | インストゥルメンテッドテスト (端末) | `app/src/androidTest/` | `./gradlew connectedAndroidTest` |
| C++単体 (Detector / FeatureProcessor) | Google Test (ホストLinux) | `tests/cpp/` | `cd build_tests && ctest` |

**根拠**:
- ViewModel は純粋なKotlinロジックのため、端末不要・高速なJVMテストで十分。
- C++ の実際の動作（検出数、境界値、スレッド安全性）はOpenCVが必要なため、ホストまたは端末で実行する。
- JNIブリッジのテストは端末上のインストゥルメンテッドテストが唯一の手段（ネイティブライブラリが必要）。

**IFeatureDetectorApi インターフェースを導入した理由**:
- `FeatureProcessorJni` はKotlin `object` であり、`init` ブロックで `System.loadLibrary` を実行する。
- ユニットテスト（JVM上）でこれを呼ぶとネイティブライブラリが見つからず UnsatisfiedLinkError が発生する。
- `IFeatureDetectorApi` を介して `MainViewModel` に依存性注入することで、テスト時はMockKのモックに差し替えられる。
- `MainViewModel(api = FeatureProcessorJni)` がデフォルト引数のため、プロダクションコードに変更は不要。

---

## DD-12: キーポイント座標マッピング（FIT_CENTER → min スケール）

**判断**: `PreviewView.scaleType = FIT_CENTER`（黒帯あり、全フレーム表示）を採用し、座標変換に `min(canvasW/frameW, canvasH/frameH)` を使う。

```
scale   = min(canvasW / frameW, canvasH / frameH)
offsetX = (canvasW - frameW * scale) / 2
offsetY = (canvasH - frameH * scale) / 2
screenX = keypointX * scale + offsetX
screenY = keypointY * scale + offsetY
```

**根拠**:
- `FILL_CENTER`（デフォルト、クロップあり）を使うと、フレームからはみ出したキーポイントが画面外に描画されてしまう。
- 特徴量を「観察する」アプリとして、全キーポイントが見えることが優先。
- min スケールにより数式が単純になり、バグのリスクが下がる。

**変更する場合**:
`FILL_CENTER` に変更するときは `min` → `max` に変え、`sx < 0 || sx > canvasW` のキーポイントを描画スキップするクリップ処理を加えること。

---

## DD-13: JNI クラスキャッシュ戦略

**判断**: `ProcessResult` の `jclass` と `jmethodID` を `init()` 時にグローバル参照としてキャッシュし、`processFrame` 呼び出しのたびに `FindClass` / `GetMethodID` を呼ばない。

**根拠**:
- `FindClass` / `GetMethodID` は JNI 呼び出しの中でも特にコストが高い（クラスローダー検索を伴う）。
- カメラフレームは 30fps = 33ms 間隔で来るため、1フレームあたりのオーバーヘッドは最小化する必要がある。
- `init()` は一度しか呼ばれないため、キャッシュ初期化コストは無視できる。

**注意**:
- `GlobalRef` は `destroy()` 時に必ず `DeleteGlobalRef` で解放する（メモリリーク防止）。
- `destroy()` → `init()` → `processFrame` の順が常に保証されること。

---

## DD-14: cv::rotate に明示的な出力バッファを使用

**判断**: `cv::rotate(src, src, ...)` のような in-place 呼び出しを避け、`cv::rotate(src, gray, ...)` で別バッファに書き出す。

**根拠**:
- `src` は外部 YUV メモリをラップした `cv::Mat`（メモリを所有しない）。
- 非正方形画像（640×480 → 90° → 480×640）の in-place 回転は OpenCV 内部で一時バッファを作るが、コードの意図が不明瞭になる。
- 明示的な出力バッファにより、`gray = src`（回転なし、ゼロコピー参照）と `gray = rotated`（回転あり、新バッファ）の違いが読み取りやすい。

---

---

## DD-15: プラグインアーキテクチャ（Plan B）

**判断**: OpenCV処理をすべて `IProcessor` を実装した C++ プラグインクラスとして実装する。新機能追加は C++ クラスを `ProcessorRegistry` に登録するだけでよく、Kotlin 側の変更は不要。

**根拠**:
- Plan A（アルゴリズム固定）では、Canny エッジ検出のような「KEYPOINTS でない」出力を追加するたびに Kotlin の `Algorithm` enum・`UiState`・描画コードを修正する必要があった。
- プラグインが自身の `name()`・`outputType()`・`paramDefs()` を宣言することで、Kotlin は実行時にこれらを問い合わせるだけでよい（DD-11の DI パターンを拡張）。
- `ProcessorRegistry::listPlugins()` / `getParamDefs()` をJNI経由で公開し、Kotlin の `MainViewModel.loadPlugins()` が起動時に一度問い合わせて `UiState.plugins` に保存する。以降の UI 表示はすべてこのデータ駆動で動作する。

**出力タイプ**:
| 値 | 型 | 用途例 |
|----|-----|--------|
| 0  | KEYPOINTS     | SIFT, ORB, AKAZE |
| 1  | OVERLAY_IMAGE | Canny エッジ      |
| 2  | POINT_VECTORS | オプティカルフロー（将来） |

**新しいプラグインを追加する手順**:
1. `app/src/main/cpp/plugins/` に `my_plugin.h` / `my_plugin.cpp` を作成し `IProcessor` を実装する。
2. `ProcessorRegistry::ProcessorRegistry()` 内で `registerPlugin(std::make_unique<MyPlugin>())` を呼ぶ。
3. `CMakeLists.txt` にソースファイルを追加する。
4. Kotlin の変更は一切不要。

---

## DD-16: `ProcessOutput` のフラット構造（JNI コンストラクタブル）

**判断**: `ProcessOutput` はすべての出力タイプのフィールドを一つのクラスに持つフラット構造とし、JNI の `NewObject` で直接コンストラクトできる設計にする。

**根拠**:
- JNI からは Kotlin のデータクラスのフィールドに個別にアクセスするより、コンストラクタを一発呼ぶ方が高速かつシンプル。
- `sealed class` や `when` でサブタイプ分岐する場合、JNI 側でサブタイプのクラスを `FindClass` する必要が生じる。フラット構造なら `gOutputClass` 一つのキャッシュで済む。
- `keypointX/Y/Size`・`overlayPixels`・`arrowStartX/Y/EndX/Y` はすべて `FloatArray` または `ByteArray` であり、未使用フィールドは空配列（長さ0）として渡す。これにより null チェックが不要。

**コンストラクタシグネチャ（JNI 文字列）**: `"(I[F[F[FI[BII[F[F[F[FF)V"`

**注意**: このシグネチャと `ProcessOutput` のコンストラクタ引数の順序・型は絶対に変更してはならない。変更する場合は C++ の `kOutputCtorSig` と Kotlin の `ProcessOutput` を必ず同時に更新すること。

---

## DD-17: パラメーター定義の文字列シリアライズ（JNI 境界）

**判断**: `getPluginDefs(name)` は `"key|displayName|min|max|default|isInt"` 形式の文字列配列を返す。専用の JNI クラスは作らない。

**根拠**:
- `ParamDef` を JNI クラスとして公開すると、フィールドアクセスごとに `GetFieldID` が必要になり、キャッシュが複雑になる。
- `getPluginDefs` はフレームごとではなくプラグイン選択時のみ呼ばれるため、文字列パースのコストは無視できる。
- パイプ区切り文字列は Kotlin で `split("|")` するだけで読めるため追加ライブラリ不要。

---

## 変更してはならないこと

| 項目 | 理由 |
|------|------|
| `ProcessOutput` のコンストラクタ引数順序 | C++ の `kOutputCtorSig` JNI シグネチャと完全一致 |
| `OutputType` の int 値（KEYPOINTS=0, OVERLAY_IMAGE=1, POINT_VECTORS=2） | C++ と Kotlin で同じ整数値として扱う |
| `ProcessorJni` の外部からの直接呼び出し | ViewModel 経由のみ許可（UI 層が JNI を直接知ることを禁止） |
| `mutex_` の削除（ProcessorRegistry） | マルチスレッド競合が発生する |
| `IProcessor::process()` の戻り値に `outputType` を設定すること | `processFrame()` で上書きするが、プラグインが outputType を返さない場合は KEYPOINTS にデフォルトされる |
