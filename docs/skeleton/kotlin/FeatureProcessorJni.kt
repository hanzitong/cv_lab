package com.hanzitong.cvlab.jni

object FeatureProcessorJni {
    external fun init()
    external fun destroy()
    external fun setAlgorithm(type: Int)
    external fun setParameter(key: String, value: Float)
    external fun processFrame(
        yuvBytes: ByteArray,
        width: Int,
        height: Int,
        rotationDegrees: Int
    ): ProcessResult

    init {
        System.loadLibrary("cv_lab")
    }
}
