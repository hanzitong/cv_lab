# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## プロジェクト概要

OpenCV処理（特徴量検出・エッジ検出等）をAndroid（Pixel 7a）のカメラ映像にリアルタイムで適用し、スライダーでパラメーターを変えながら観察するアプリ。プラグインアーキテクチャにより、C++ クラスを追加するだけで新しい OpenCV 処理を追加できる。

---

## ビルド前の準備

1. `local.properties.template` を `local.properties` にコピー
2. `opencv.sdk.path` に OpenCV Android SDK のパスを設定（`docs/setup.md` 参照）
3. Android Studio で開いて Gradle Sync（`gradlew` は Android Studio が生成）

```bash
# デバッグビルド
./gradlew :app:assembleDebug

# 端末にインストール（Pixel 7a 接続必須）
./gradlew :app:installDebug
```

---

## テスト実行

```bash
# Kotlin ユニットテスト（JVM、端末不要、高速）
./gradlew :app:test

# JNI + C++ 統合テスト（Pixel 7a 接続必須）
./gradlew :app:connectedAndroidTest

# C++ ホストテスト（Linux に OpenCV インストール必要）
mkdir build_tests && cd build_tests
cmake ../tests && cmake --build . && ctest --output-on-failure
```

---

## アーキテクチャ（必読）

```
Compose UI
  └── MainScreen
       ├── PluginSelector             # TabRow（動的 — C++ から取得）
       ├── CameraPreview              # AndroidView（PreviewView FIT_CENTER）
       ├── ProcessingOverlay          # when(outputType) → KeypointCanvas / OverlayImageCanvas / ArrowVectorsCanvas
       ├── StatsBar                   # 検出数・処理時間
       └── ParameterPanel             # LazyColumn + Slider（動的 — プラグインの paramDefs）
                 │ collectAsStateWithLifecycle
                 ▼
         MainViewModel（StateFlow/MVVM）
           loadPlugins() / onPluginSelected() / onParameterChanged() / onFrameResult()
                 │ IProcessorApi（テスト時はモック）
                 ▼
         ProcessorJni（唯一の JNI 窓口）
                 │ JNI 境界
                 ▼
         jni_bridge.cpp → ProcessorRegistry → IProcessor（Plugin）
                                               SIFT / ORB / AKAZE / Canny / ...
```

### 新しいプラグインの追加手順（Kotlin 変更不要）

1. `app/src/main/cpp/plugins/my_plugin.h/.cpp` を作成し `IProcessor` を実装する
2. `processor_registry.cpp` の `ProcessorRegistry()` コンストラクタで `registerPlugin(std::make_unique<MyPlugin>())` を呼ぶ
3. `CMakeLists.txt` にソースファイルを追加する
4. 以上。Kotlin 側は何も変更しなくてよい。

---

## 重要な制約と注意点

### ProcessOutput のコンストラクタ引数順序
`ProcessOutput`（Kotlin）のコンストラクタ引数と C++ の `kOutputCtorSig` JNI シグネチャは **絶対に同期すること**。
現在のシグネチャ: `"(I[F[F[FI[BII[F[F[F[FF)V"`

### OutputType の int 値
C++ の `OutputType` enum と Kotlin の `ProcessOutput.outputType` は同じ int 値を使う。
`KEYPOINTS=0`, `OVERLAY_IMAGE=1`, `POINT_VECTORS=2`

### std::mutex の削除禁止
`processFrame` は CameraX バックグラウンドスレッド、`setParameter/selectPlugin` はメインスレッドから呼ばれる。`mutex_` なしでは競合が発生する。

### キーポイント座標マッピング（FIT_CENTER）
C++ は回転後のフレーム座標でキーポイントを返す。`ProcessingOverlay.kt` では FIT_CENTER の min スケールで変換する：
```
scale = min(canvasW / frameW, canvasH / frameH)
screenX = keypointX * scale + (canvasW - frameW * scale) / 2
```
FILL_CENTER に変更する場合は min → max に変え、フレーム外のキーポイントをクリップすること（DD-12）。

### JNI クラスキャッシュ
`jclass` / `jmethodID` は `init()` 時にキャッシュし `destroy()` で解放する。`processFrame` 内で `FindClass` を呼ばないこと（DD-13）。

### YUV 処理
C++ に渡すのは Y 平面のみ（グレースケール相当）。Y 平面 = グレースケールのため `cv::cvtColor` 不要。`CameraPreview.kt` で rowStride パディングを除去してから渡している（DD-07）。

### テスタビリティ
`MainViewModel` は `IProcessorApi` をコンストラクタ注入で受け取る。ユニットテストでは `mockk<IProcessorApi>()` を渡す。プロダクションコードは `ProcessorJni` がデフォルト引数なので変更不要（DD-11）。

---

## ドキュメント

| ファイル | 内容 |
|---------|------|
| `docs/design_decisions.md` | **最重要** — DD-01〜DD-17 全設計判断と根拠 |
| `docs/specification.md` | 機能仕様・パラメーター一覧 |
| `docs/architecture.md` | アーキテクチャ図・データフロー・スレッドモデル |
| `docs/setup.md` | OpenCV SDK ダウンロード・環境構築手順 |
| `docs/skeleton/` | Plan A 初期骨格コードのスナップショット（参照用） |

---

## OpenCV モジュール

使用するのは `core`, `imgproc`, `features2d` のみ。`xfeatures2d`（SURF）は採用しない（DD-01）。
