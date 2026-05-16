package com.hanzitong.cvfeaturelab.jni

interface IFeatureDetectorApi {
    fun init()
    fun destroy()
    fun setAlgorithm(type: Int)
    fun setParameter(key: String, value: Float)
    fun processFrame(
        yuvBytes: ByteArray,
        width: Int,
        height: Int,
        rotationDegrees: Int
    ): ProcessResult
}
