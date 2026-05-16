package com.hanzitong.cvfeaturelab.viewmodel

data class ParameterDef(
    val key: String,
    val displayName: String,
    val min: Float,
    val max: Float,
    val default: Float,
    val isInteger: Boolean = true
)

data class PluginInfo(
    val name: String,
    val params: List<ParameterDef>
)

data class UiState(
    val plugins: List<PluginInfo> = emptyList(),
    val selectedPlugin: String = "",
    val parameters: Map<String, Float> = emptyMap(),

    // 出力タイプ: 0=KEYPOINTS, 1=OVERLAY_IMAGE, 2=POINT_VECTORS
    val outputType: Int = 0,

    // KEYPOINTS 出力
    val keypointX: FloatArray = FloatArray(0),
    val keypointY: FloatArray = FloatArray(0),
    val keypointSize: FloatArray = FloatArray(0),
    val keypointCount: Int = 0,

    // OVERLAY_IMAGE 出力（CV_8UC1 グレースケール）
    val overlayPixels: ByteArray = ByteArray(0),
    val overlayWidth: Int = 0,
    val overlayHeight: Int = 0,

    // POINT_VECTORS 出力
    val arrowStartX: FloatArray = FloatArray(0),
    val arrowStartY: FloatArray = FloatArray(0),
    val arrowEndX: FloatArray = FloatArray(0),
    val arrowEndY: FloatArray = FloatArray(0),

    val processingTimeMs: Float = 0f,
    val frameWidth: Int = 480,
    val frameHeight: Int = 640
)
