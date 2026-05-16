# アプリ仕様書 — cv_feature_lab

バージョン: 1.0  
作成日: 2026-05-16  
対象端末: Google Pixel 7a

---

## 1. プロジェクト概要

OpenCVの特徴量検出アルゴリズムをAndroidスマートフォンのカメラ映像に対してリアルタイムで実行し、各アルゴリズムの動作をスライダーでパラメーターを変えながら観察・比較できるアプリ。

---

## 2. 機能要件

| ID  | 機能 | 詳細 |
|-----|------|------|
| F-1 | カメラプレビュー | バックカメラのリアルタイム映像をプレビュー表示 |
| F-2 | 特徴量検出オーバーレイ | 検出されたキーポイントを緑の円でカメラ映像上にリアルタイム描画 |
| F-3 | アルゴリズム切替 | SIFT / ORB / AKAZE からタブで1つ選択 |
| F-4 | パラメーター調整 | 各アルゴリズムのパラメーターをスライダーでリアルタイム変更 |
| F-5 | 統計情報表示 | 検出キーポイント数・処理時間（ms）を画面下部に表示 |

---

## 3. 対象アルゴリズムとパラメーター

### SIFT

| パラメーター | 範囲 | デフォルト |
|-------------|------|---------|
| nFeatures（最大特徴点数） | 0 – 2000 | 500 |
| nOctaveLayers | 1 – 8 | 3 |
| contrastThreshold | 0.01 – 0.10 | 0.04 |
| edgeThreshold | 1 – 30 | 10 |
| sigma | 0.5 – 3.0 | 1.6 |

### ORB

| パラメーター | 範囲 | デフォルト |
|-------------|------|---------|
| nFeatures | 100 – 5000 | 500 |
| scaleFactor | 1.1 – 2.0 | 1.2 |
| nLevels | 1 – 16 | 8 |
| edgeThreshold | 1 – 63 | 31 |
| fastThreshold | 1 – 40 | 20 |

### AKAZE

| パラメーター | 範囲 | デフォルト |
|-------------|------|---------|
| threshold | 0.0001 – 0.01 | 0.001 |
| nOctaves | 1 – 8 | 4 |
| nOctaveLayers | 1 – 8 | 4 |

---

## 4. 技術スタック

| レイヤー | 技術 |
|---------|------|
| UI / カメラ制御 | Kotlin + CameraX + Jetpack Compose |
| JNIブリッジ | JNI（C++ ↔ Kotlin） |
| 画像処理 | C++17 + OpenCV 4.x (NDK) |
| ビルド | CMake 3.22+ + Android NDK + Gradle 8.7 |
| 状態管理 | ViewModel + StateFlow (MVVM) |
| 最低SDK | API 33（Android 13） |
| ABI | arm64-v8a のみ |

---

## 5. JNI インターフェース

```
// Kotlin → C++
init()
destroy()
setAlgorithm(type: Int)              // 0=SIFT, 1=ORB, 2=AKAZE
setParameter(key: String, value: Float)
processFrame(yuvBytes: ByteArray, width: Int, height: Int, rotationDegrees: Int): ProcessResult

// C++ → Kotlin（戻り値）
ProcessResult:
  keypointX: FloatArray    // カメラフレーム内のx座標（px）
  keypointY: FloatArray    // カメラフレーム内のy座標（px）
  keypointSize: FloatArray // キーポイントの近傍サイズ
  count: Int
  processingTimeMs: Float
```

---

## 6. 画面レイアウト

```
┌──────────────────────────────────────┐
│  [SIFT]  [ORB]  [AKAZE]              │  ← TabRow
├──────────────────────────────────────┤
│                                      │
│   カメラプレビュー                   │
│   ＋ キーポイントオーバーレイ(Canvas)│
│                                      │
│  ┌── Keypoints: 342  |  18 ms ──┐   │  ← StatsBar（半透明）
│  └─────────────────────────────┘   │
├──────────────────────────────────────┤
│  nFeatures        ──●──────  500     │
│  contrastThresh   ────●────  0.04   │  ← ParameterPanel
│  sigma            ──────●──  1.6    │
│  ...（最大5項目）                    │
└──────────────────────────────────────┘
```

---

## 7. 非機能要件

| 要件 | 目標値 |
|------|-------|
| フレーム処理遅延 | 30ms以内（ORB基準） |
| スライダー反映遅延 | 次フレーム（~33ms）以内 |
| カメラ解像度 | 640×480（処理用） |
| 画面向き | 縦固定（portrait） |
