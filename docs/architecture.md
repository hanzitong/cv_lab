# アーキテクチャ設計書

---

## レイヤー構成図

```
┌─────────────────────────────────────────────────────────────────────┐
│                          Kotlin Layer                               │
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │  Compose UI                                                  │   │
│  │   MainScreen                                                 │   │
│  │   ├── AlgorithmSelector  (TabRow: SIFT / ORB / AKAZE)       │   │
│  │   ├── Box                                                    │   │
│  │   │   ├── CameraPreview  (AndroidView + SurfaceProvider)     │   │
│  │   │   ├── KeypointOverlay (Canvas — coordinate mapping)      │   │
│  │   │   └── StatsBar       (半透明テキスト)                    │   │
│  │   └── ParameterPanel     (LazyColumn + Slider × n)          │   │
│  └───────────────────────────┬──────────────────────────────────┘   │
│                              │ collectAsStateWithLifecycle          │
│  ┌───────────────────────────▼──────────────────────────────────┐   │
│  │  MainViewModel (StateFlow)                                    │   │
│  │   UiState { algorithm, parameters, keypoints, stats }        │   │
│  │   onAlgorithmSelected() / onParameterChanged() / onFrameResult│  │
│  └───────────────────────────┬──────────────────────────────────┘   │
│                              │                                      │
│  ┌───────────────────────────▼──────────────────────────────────┐   │
│  │  FeatureProcessorJni (object)  ← JNIラッパー、外部非公開      │   │
│  └───────────────────────────┬──────────────────────────────────┘   │
│                              │ System.loadLibrary("cv_lab") │
├──────────────────────────────┼──────────────────────────────────────┤
│                    JNI 境界（単一交差点）                           │
│                     jni_bridge.cpp                                  │
├──────────────────────────────┼──────────────────────────────────────┤
│                         C++ Layer                                   │
│                              │                                      │
│  ┌───────────────────────────▼──────────────────────────────────┐   │
│  │  FeatureProcessor                                             │   │
│  │   - std::mutex  (スレッド安全)                               │   │
│  │   - setAlgorithm() / setParameter() / processFrame()         │   │
│  │                                                               │   │
│  │   ┌──────────────────────────────────────────────────────┐   │   │
│  │   │  IFeatureDetector  《Strategy》                       │   │   │
│  │   ├──────────────────────────────────────────────────────┤   │   │
│  │   │  SIFTDetector  │  ORBDetector  │  AKAZEDetector      │   │   │
│  │   └──────────────────────────────────────────────────────┘   │   │
│  └───────────────────────────────────────────────────────────────┘   │
│                          OpenCV (NDK .so)                           │
│              core / imgproc / features2d                            │
└─────────────────────────────────────────────────────────────────────┘
```

---

## データフロー

### カメラフレーム処理（毎フレーム）

```
CameraX (ImageAnalysis)
  │  YUV_420_888 ImageProxy
  │  バックグラウンドスレッド（単一executor）
  ▼
CameraPreview.kt : processFrame()
  │  Y平面のみ抽出（ByteArray）+ rowStride正規化
  ▼
FeatureProcessorJni.processFrame(yuv, w, h, rotation)
  │  JNI crossing
  ▼
FeatureProcessor::processFrame()
  │  cv::Mat wrap (Y平面 = グレースケール)
  │  cv::rotate (rotationDegrees)
  │  detector_->detect()
  │  処理時間計測
  ▼
jni_bridge.cpp : JNI戻り値構築
  │  float[] x, y, size 配列 + ProcessResult Java object
  ▼
MainViewModel.onFrameResult()
  │  StateFlow.update() （スレッドセーフ）
  ▼
Compose recomposition
  │  KeypointOverlay: 座標スケーリング → Canvas.drawCircle()
  └  StatsBar: count / timeMs 更新
```

### パラメーター変更（スライダー操作）

```
Slider.onValueChange (メインスレッド)
  │
  ▼
ParameterPanel → MainScreen → MainViewModel.onParameterChanged(key, value)
  │  StateFlow更新（即座にUI反映）
  │
  ▼
FeatureProcessorJni.setParameter(key, value)  [JNI, メインスレッド]
  │
  ▼
FeatureProcessor::setParameter() → detector_->setParameter()
  └  内部状態更新 → 次フレームに反映
```

---

## ファイル構成

```
cv_lab/
├── app/src/main/
│   ├── cpp/
│   │   ├── CMakeLists.txt
│   │   ├── jni_bridge.cpp          JNI関数定義のみ（ロジックなし）
│   │   ├── feature_processor.h/.cpp  Strategy保持・処理統括
│   │   └── detectors/
│   │       ├── i_feature_detector.h  Strategy interface
│   │       ├── sift_detector.h/.cpp
│   │       ├── orb_detector.h/.cpp
│   │       └── akaze_detector.h/.cpp
│   └── java/com/hanzitong/cvlab/
│       ├── MainActivity.kt          パーミッション + JNI init/destroy
│       ├── jni/
│       │   ├── FeatureProcessorJni.kt  唯一のJNIラッパー
│       │   └── ProcessResult.kt
│       ├── viewmodel/
│       │   ├── MainViewModel.kt
│       │   └── UiState.kt           Algorithm enum + ParameterDef + UiState
│       └── ui/
│           ├── MainScreen.kt        KeypointOverlay + StatsBar もここに
│           ├── CameraPreview.kt
│           ├── AlgorithmSelector.kt
│           ├── ParameterPanel.kt
│           └── theme/Theme.kt
├── docs/
│   ├── specification.md
│   ├── architecture.md  （本文書）
│   ├── design_decisions.md  ← 最重要
│   ├── setup.md
│   └── skeleton/         骨格コードのスナップショット
└── gradle/libs.versions.toml
```

---

## スレッドモデル

| スレッド | 処理内容 |
|---------|---------|
| メインスレッド | UI描画、StateFlow購読、スライダーイベント、`setAlgorithm`/`setParameter` JNI呼び出し |
| CameraX分析スレッド | `processFrame` JNI呼び出し、`onFrameResult` コールバック |
| （内部）OpenCV | SIFT等の並列処理はOpenCV内部スレッドプール任せ |

`std::mutex` が `processFrame` / `setAlgorithm` / `setParameter` の競合を防ぐ。
