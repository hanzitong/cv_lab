package com.hanzitong.cvlab.jni

/**
 * JNI から直接コンストラクトされる出力クラス。
 * コンストラクタ引数の順序と型は jni_bridge.cpp の kOutputCtorSig と完全一致させること。
 * シグネチャ: "(I[F[F[FI[BII[F[F[F[FF)V"
 */
class ProcessOutput(
    val outputType: Int,              // 0=KEYPOINTS, 1=OVERLAY_IMAGE, 2=POINT_VECTORS
    val keypointX: FloatArray,
    val keypointY: FloatArray,
    val keypointSize: FloatArray,
    val keypointCount: Int,
    val overlayPixels: ByteArray,     // CV_8UC1 グレースケール（行優先）
    val overlayWidth: Int,
    val overlayHeight: Int,
    val arrowStartX: FloatArray,
    val arrowStartY: FloatArray,
    val arrowEndX: FloatArray,
    val arrowEndY: FloatArray,
    val processingTimeMs: Float
)
