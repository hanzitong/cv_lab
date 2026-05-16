package com.hanzitong.cvlab.jni

object ProcessorJni : IProcessorApi {
    override external fun init()
    override external fun destroy()
    override external fun listPlugins(): Array<String>
    override external fun getPluginDefs(name: String): Array<String>
    override external fun selectPlugin(name: String)
    override external fun setParameter(key: String, value: Float)
    override external fun processFrame(
        yuvBytes: ByteArray,
        width: Int,
        height: Int,
        rotationDegrees: Int
    ): ProcessOutput

    init {
        System.loadLibrary("cv_lab")
    }
}
