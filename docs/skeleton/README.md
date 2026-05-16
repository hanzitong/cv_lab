# 骨格コード スナップショット

作成日: 2026-05-16

このディレクトリは初期骨格コードのスナップショットです。
実際の実装は `app/src/main/` 以下にあります。

## ファイル一覧

```
cpp/
├── CMakeLists.txt
├── jni_bridge.cpp
├── feature_processor.h
├── feature_processor.cpp
└── detectors/
    ├── i_feature_detector.h
    ├── sift_detector.h / .cpp
    ├── orb_detector.h / .cpp
    └── akaze_detector.h / .cpp

kotlin/
├── MainActivity.kt
├── ProcessResult.kt       (jni/)
├── FeatureProcessorJni.kt (jni/)
├── UiState.kt             (viewmodel/)
├── MainViewModel.kt       (viewmodel/)
├── MainScreen.kt          (ui/)
├── CameraPreview.kt       (ui/)
├── AlgorithmSelector.kt   (ui/)
└── ParameterPanel.kt      (ui/)

gradle/
├── app_build.gradle.kts
└── libs.versions.toml
```

重要な設計判断については `../design_decisions.md` を参照してください。
