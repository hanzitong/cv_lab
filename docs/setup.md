# 開発環境セットアップ手順

対象端末: Google Pixel 7a  
開発環境: Android Studio Hedgehog 以降推奨

---

## 1. Android Studio のセットアップ

Android Studio を起動後、SDK Manager から以下をインストール:

```
SDK Tools タブ:
  ☑ NDK (Side by side)  — バージョン 26.x 以上
  ☑ CMake               — バージョン 3.22.1 以上
```

---

## 2. OpenCV Android SDK のダウンロード

1. 最新の OpenCV Android SDK をダウンロード:
   - URL: https://github.com/opencv/opencv/releases
   - ファイル名: `opencv-4.x.x-android-sdk.zip`（4.9.0 または最新版）

2. 任意のディレクトリに展開:
   ```
   /path/to/OpenCV-android-sdk/
   ├── sdk/
   │   ├── native/
   │   │   ├── jni/          ← CMake find_package ターゲット
   │   │   └── libs/arm64-v8a/  ← .so ファイル
   │   └── java/
   ```

3. git管理対象外: OpenCV SDKは `.gitignore` 済みのため、各開発者がローカルに配置する。

---

## 3. local.properties の設定

プロジェクトルートの `local.properties` に以下を追記:

```properties
# Android SDK（Android Studio が自動設定）
sdk.dir=/Users/yourname/Library/Android/sdk

# OpenCV Android SDK のパス（展開先に合わせて変更）
opencv.sdk.path=/path/to/OpenCV-android-sdk
```

> `local.properties` は `.gitignore` に追加することを推奨。

---

## 4. プロジェクトを Android Studio で開く

1. Android Studio で `cv_lab/` ディレクトリを開く
2. Gradle Sync を実行（自動で促される場合はそのまま実行）
3. Gradle Wrapper が存在しない場合: Android Studio が自動的に生成を提案する

コマンドラインからの生成が必要な場合:
```bash
gradle wrapper --gradle-version 8.7
```

---

## 5. ビルドと実行

```bash
# デバッグビルド（Android Studio Run ボタンでも可）
./gradlew :app:assembleDebug

# Pixel 7a に直接インストール
./gradlew :app:installDebug
```

---

## 6. CMake / NDK ビルドの確認

OpenCV SDK パスが正しく設定されていない場合、以下のエラーが出る:
```
CMake Error: OPENCV_SDK_PATH is not set.
Add 'opencv.sdk.path=/path/to/OpenCV-android-sdk' to local.properties
```

→ `local.properties` の `opencv.sdk.path` を確認すること。

---

## 7. .gitignore の確認

以下がgit管理対象外であることを確認:

```
OpenCV-android-sdk/    # SDKはダウンロードが必要
local.properties       # 各開発者の環境差異
build/                 # ビルド成果物
.gradle/
```

---

## 8. 実装フェーズ計画

| フェーズ | 内容 | 状態 |
|---------|------|------|
| Ph.1 | プロジェクト骨格・NDKビルド疎通確認 | 骨格完成 |
| Ph.2 | ORB実装 + processFrame動作確認 | 骨格完成 |
| Ph.3 | CameraX映像 + Canvas描画の疎通確認 | 骨格完成 |
| Ph.4 | SIFT・AKAZE追加、タブ切替 | 骨格完成 |
| Ph.5 | ParameterPanel リアルタイム反映 | 骨格完成 |
| Ph.6 | 座標スケーリング精度検証・パフォーマンスチューニング | 未着手 |
