package com.hanzitong.cvfeaturelab.viewmodel

// Ordinal values must stay in sync with AlgorithmType enum in feature_processor.h
enum class Algorithm(val displayName: String) {
    SIFT("SIFT"),   // ordinal 0
    ORB("ORB"),     // ordinal 1
    AKAZE("AKAZE"); // ordinal 2
}

data class ParameterDef(
    val key: String,
    val displayName: String,
    val min: Float,
    val max: Float,
    val default: Float,
    val isInteger: Boolean = true
)

val algorithmParameters: Map<Algorithm, List<ParameterDef>> = mapOf(
    Algorithm.SIFT to listOf(
        ParameterDef("nFeatures",         "Max Features",       0f,      2000f,  500f),
        ParameterDef("nOctaveLayers",     "Octave Layers",      1f,      8f,     3f),
        ParameterDef("contrastThreshold", "Contrast Threshold", 0.01f,   0.10f,  0.04f, isInteger = false),
        ParameterDef("edgeThreshold",     "Edge Threshold",     1f,      30f,    10f),
        ParameterDef("sigma",             "Sigma",              0.5f,    3.0f,   1.6f,  isInteger = false),
    ),
    Algorithm.ORB to listOf(
        ParameterDef("nFeatures",     "Max Features",   100f,  5000f, 500f),
        ParameterDef("scaleFactor",   "Scale Factor",   1.1f,  2.0f,  1.2f, isInteger = false),
        ParameterDef("nLevels",       "Levels",         1f,    16f,   8f),
        ParameterDef("edgeThreshold", "Edge Threshold", 1f,    63f,   31f),
        ParameterDef("fastThreshold", "FAST Threshold", 1f,    40f,   20f),
    ),
    Algorithm.AKAZE to listOf(
        ParameterDef("threshold",     "Threshold",     0.0001f, 0.01f, 0.001f, isInteger = false),
        ParameterDef("nOctaves",      "Octaves",       1f,      8f,    4f),
        ParameterDef("nOctaveLayers", "Octave Layers", 1f,      8f,    4f),
    )
)

fun defaultParams(algo: Algorithm): Map<String, Float> =
    algorithmParameters[algo]?.associate { it.key to it.default } ?: emptyMap()

data class UiState(
    val selectedAlgorithm: Algorithm = Algorithm.ORB,
    val parameters: Map<String, Float> = defaultParams(Algorithm.ORB),
    val keypointX: FloatArray = FloatArray(0),
    val keypointY: FloatArray = FloatArray(0),
    val keypointSize: FloatArray = FloatArray(0),
    val keypointCount: Int = 0,
    val processingTimeMs: Float = 0f,
    val frameWidth: Int = 480,
    val frameHeight: Int = 640
)
