package com.hanzitong.cvfeaturelab.jni

import org.junit.Assert.*
import org.junit.Test

class ProcessOutputTest {

    private fun makeKeypoints(n: Int, timeMs: Float = 10f) = ProcessOutput(
        outputType       = 0,
        keypointX        = FloatArray(n) { it.toFloat() },
        keypointY        = FloatArray(n) { it.toFloat() * 2 },
        keypointSize     = FloatArray(n) { 8f },
        keypointCount    = n,
        overlayPixels    = ByteArray(0),
        overlayWidth     = 0,
        overlayHeight    = 0,
        arrowStartX      = FloatArray(0),
        arrowStartY      = FloatArray(0),
        arrowEndX        = FloatArray(0),
        arrowEndY        = FloatArray(0),
        processingTimeMs = timeMs
    )

    private fun makeOverlay(w: Int, h: Int) = ProcessOutput(
        outputType       = 1,
        keypointX        = FloatArray(0),
        keypointY        = FloatArray(0),
        keypointSize     = FloatArray(0),
        keypointCount    = 0,
        overlayPixels    = ByteArray(w * h) { 128.toByte() },
        overlayWidth     = w,
        overlayHeight    = h,
        arrowStartX      = FloatArray(0),
        arrowStartY      = FloatArray(0),
        arrowEndX        = FloatArray(0),
        arrowEndY        = FloatArray(0),
        processingTimeMs = 5f
    )

    // ─── KEYPOINTS 出力 ───────────────────────────────────────────────────────

    @Test
    fun `KEYPOINTS outputType is 0`() {
        assertEquals(0, makeKeypoints(3).outputType)
    }

    @Test
    fun `keypointCount matches array lengths`() {
        val out = makeKeypoints(5)
        assertEquals(out.keypointCount, out.keypointX.size)
        assertEquals(out.keypointCount, out.keypointY.size)
        assertEquals(out.keypointCount, out.keypointSize.size)
    }

    @Test
    fun `empty keypoints has zero count`() {
        val out = makeKeypoints(0)
        assertEquals(0, out.keypointCount)
        assertEquals(0, out.keypointX.size)
    }

    @Test
    fun `keypointX values are stored correctly`() {
        val out = makeKeypoints(3)
        assertEquals(0f, out.keypointX[0], 0f)
        assertEquals(1f, out.keypointX[1], 0f)
        assertEquals(2f, out.keypointX[2], 0f)
    }

    @Test
    fun `processingTimeMs is stored`() {
        assertEquals(33.5f, makeKeypoints(0, 33.5f).processingTimeMs, 0.01f)
    }

    @Test
    fun `KEYPOINTS output has empty overlay fields`() {
        val out = makeKeypoints(5)
        assertEquals(0, out.overlayPixels.size)
        assertEquals(0, out.overlayWidth)
        assertEquals(0, out.overlayHeight)
    }

    @Test
    fun `KEYPOINTS output has empty arrow fields`() {
        val out = makeKeypoints(5)
        assertEquals(0, out.arrowStartX.size)
        assertEquals(0, out.arrowEndY.size)
    }

    // ─── OVERLAY_IMAGE 出力 ───────────────────────────────────────────────────

    @Test
    fun `OVERLAY_IMAGE outputType is 1`() {
        assertEquals(1, makeOverlay(320, 240).outputType)
    }

    @Test
    fun `overlayPixels size equals width times height`() {
        val out = makeOverlay(100, 75)
        assertEquals(100 * 75, out.overlayPixels.size)
    }

    @Test
    fun `overlayWidth and overlayHeight are stored`() {
        val out = makeOverlay(100, 75)
        assertEquals(100, out.overlayWidth)
        assertEquals(75, out.overlayHeight)
    }

    @Test
    fun `OVERLAY_IMAGE output has empty keypoint fields`() {
        val out = makeOverlay(100, 75)
        assertEquals(0, out.keypointCount)
        assertEquals(0, out.keypointX.size)
    }

    // ─── POINT_VECTORS 出力 ───────────────────────────────────────────────────

    @Test
    fun `POINT_VECTORS outputType is 2`() {
        val out = ProcessOutput(
            outputType       = 2,
            keypointX        = FloatArray(0),
            keypointY        = FloatArray(0),
            keypointSize     = FloatArray(0),
            keypointCount    = 0,
            overlayPixels    = ByteArray(0),
            overlayWidth     = 0,
            overlayHeight    = 0,
            arrowStartX      = floatArrayOf(1f, 2f),
            arrowStartY      = floatArrayOf(3f, 4f),
            arrowEndX        = floatArrayOf(5f, 6f),
            arrowEndY        = floatArrayOf(7f, 8f),
            processingTimeMs = 2f
        )
        assertEquals(2, out.outputType)
        assertEquals(2, out.arrowStartX.size)
        assertEquals(1f, out.arrowStartX[0], 0f)
        assertEquals(8f, out.arrowEndY[1], 0f)
    }
}
