package com.hanzitong.cvlab.jni

/**
 * JNI レイヤーの抽象インターフェース。
 * MainViewModel はこのインターフェース越しにのみ JNI を呼ぶ（DD-02参照）。
 * ユニットテストでは MockK でこのインターフェースをモックする（DD-11参照）。
 */
interface IProcessorApi {
    fun init()
    fun destroy()
    fun listPlugins(): Array<String>
    /** "key|displayName|min|max|default|isInt" 形式の String 配列を返す */
    fun getPluginDefs(name: String): Array<String>
    fun selectPlugin(name: String)
    fun setParameter(key: String, value: Float)
    fun processFrame(
        yuvBytes: ByteArray,
        width: Int,
        height: Int,
        rotationDegrees: Int
    ): ProcessOutput
}
