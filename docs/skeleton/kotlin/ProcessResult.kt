package com.hanzitong.cvlab.jni

data class ProcessResult(
    val keypointX: FloatArray,
    val keypointY: FloatArray,
    val keypointSize: FloatArray,
    val count: Int,
    val processingTimeMs: Float
) {
    override fun equals(other: Any?): Boolean {
        if (this === other) return true
        if (other !is ProcessResult) return false
        return count == other.count && processingTimeMs == other.processingTimeMs
    }
    override fun hashCode(): Int = 31 * count + processingTimeMs.hashCode()
}
