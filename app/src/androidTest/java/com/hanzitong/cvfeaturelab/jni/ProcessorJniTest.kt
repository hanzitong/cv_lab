package com.hanzitong.cvfeaturelab.jni

import androidx.test.ext.junit.runners.AndroidJUnit4
import org.junit.After
import org.junit.Assert.*
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith

/**
 * ProcessorJni + C++ プラグインアーキテクチャの統合テスト。
 * 実端末 / エミュレーターで `./gradlew connectedAndroidTest` により実行。
 */
@RunWith(AndroidJUnit4::class)
class ProcessorJniTest {

    companion object {
        private const val W = 320
        private const val H = 240

        /** チェッカーボード — コーナー豊富で KEYPOINTS 系検出器が確実に反応する */
        fun makeCheckerboard(w: Int = W, h: Int = H, squareSize: Int = 32): ByteArray =
            ByteArray(w * h) { i ->
                val x = i % w; val y = i / w
                if ((x / squareSize + y / squareSize) % 2 == 0) 0 else -1  // 0 or 255(as -1)
            }

        /** 均一グレー（特徴点なし） */
        fun makeUniform(w: Int = W, h: Int = H, value: Byte = 128.toByte()): ByteArray =
            ByteArray(w * h) { value }

        /** ランダムノイズ */
        fun makeNoise(w: Int = W, h: Int = H): ByteArray =
            ByteArray(w * h) { (Math.random() * 256).toInt().toByte() }
    }

    @Before
    fun setUp() { ProcessorJni.init() }

    @After
    fun tearDown() { ProcessorJni.destroy() }

    // ─── listPlugins ──────────────────────────────────────────────────────────

    @Test
    fun listPlugins_returnsNonEmptyArray() {
        val plugins = ProcessorJni.listPlugins()
        assertTrue("listPlugins must return at least one plugin", plugins.isNotEmpty())
    }

    @Test
    fun listPlugins_containsORB() {
        assertTrue(ProcessorJni.listPlugins().contains("ORB"))
    }

    @Test
    fun listPlugins_containsSIFT() {
        assertTrue(ProcessorJni.listPlugins().contains("SIFT"))
    }

    @Test
    fun listPlugins_containsAKAZE() {
        assertTrue(ProcessorJni.listPlugins().contains("AKAZE"))
    }

    @Test
    fun listPlugins_containsCanny() {
        assertTrue(ProcessorJni.listPlugins().contains("Canny"))
    }

    // ─── getPluginDefs ───────────────────────────────────────────────────────

    @Test
    fun getPluginDefs_ORB_returnsNonEmptyArray() {
        val defs = ProcessorJni.getPluginDefs("ORB")
        assertTrue("ORB must have at least one param def", defs.isNotEmpty())
    }

    @Test
    fun getPluginDefs_ORB_defsHaveCorrectFormat() {
        ProcessorJni.getPluginDefs("ORB").forEach { defStr ->
            val parts = defStr.split("|")
            assertEquals("def should have 6 pipe-separated fields: $defStr", 6, parts.size)
            // key は空でないこと
            assertTrue(parts[0].isNotEmpty())
            // min < max
            val min = parts[2].toFloat()
            val max = parts[3].toFloat()
            assertTrue("min($min) must be < max($max)", min < max)
            // default は min..max 範囲内
            val default = parts[4].toFloat()
            assertTrue(default >= min && default <= max)
            // isInt は 0 か 1
            assertTrue(parts[5] == "0" || parts[5] == "1")
        }
    }

    @Test
    fun getPluginDefs_SIFT_hasNFeatures() {
        val defs = ProcessorJni.getPluginDefs("SIFT")
        assertTrue(defs.any { it.startsWith("nFeatures|") })
    }

    @Test
    fun getPluginDefs_Canny_hasThreshold1AndThreshold2() {
        val defs = ProcessorJni.getPluginDefs("Canny")
        assertTrue(defs.any { it.startsWith("threshold1|") })
        assertTrue(defs.any { it.startsWith("threshold2|") })
    }

    @Test
    fun getPluginDefs_unknownPlugin_returnsEmptyArray() {
        val defs = ProcessorJni.getPluginDefs("NonExistent")
        assertEquals(0, defs.size)
    }

    // ─── selectPlugin + processFrame (KEYPOINTS) ──────────────────────────────

    @Test
    fun ORB_processFrame_returnsNonNullResult() {
        ProcessorJni.selectPlugin("ORB")
        val out = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0)
        assertNotNull(out)
    }

    @Test
    fun ORB_processFrame_outputTypeIsKEYPOINTS() {
        ProcessorJni.selectPlugin("ORB")
        val out = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0)
        assertEquals(0, out.outputType)
    }

    @Test
    fun ORB_processFrame_processingTimeIsPositive() {
        ProcessorJni.selectPlugin("ORB")
        val out = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0)
        assertTrue("processingTimeMs must be > 0", out.processingTimeMs > 0f)
    }

    @Test
    fun ORB_processFrame_arrayLengthsMatchCount() {
        ProcessorJni.selectPlugin("ORB")
        val out = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0)
        assertEquals(out.keypointCount, out.keypointX.size)
        assertEquals(out.keypointCount, out.keypointY.size)
        assertEquals(out.keypointCount, out.keypointSize.size)
    }

    @Test
    fun ORB_detectsKeypointsOnCheckerboard() {
        ProcessorJni.selectPlugin("ORB")
        val out = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0)
        assertTrue("ORB should detect keypoints, got ${out.keypointCount}", out.keypointCount > 0)
    }

    @Test
    fun ORB_uniformImage_returnsZeroOrFewKeypoints() {
        ProcessorJni.selectPlugin("ORB")
        val out = ProcessorJni.processFrame(makeUniform(), W, H, 0)
        assertTrue("Uniform should yield few keypoints, got ${out.keypointCount}", out.keypointCount < 10)
    }

    @Test
    fun ORB_keypointsAreWithinFrameBounds() {
        ProcessorJni.selectPlugin("ORB")
        val out = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0)
        out.keypointX.forEach { x -> assertTrue("x=$x out of [0,$W)", x >= 0f && x < W) }
        out.keypointY.forEach { y -> assertTrue("y=$y out of [0,$H)", y >= 0f && y < H) }
    }

    @Test
    fun ORB_keypointSizesArePositive() {
        ProcessorJni.selectPlugin("ORB")
        val out = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0)
        out.keypointSize.forEach { s -> assertTrue("size=$s must be positive", s > 0f) }
    }

    // ─── SIFT ─────────────────────────────────────────────────────────────────

    @Test
    fun SIFT_detectsKeypointsOnCheckerboard() {
        ProcessorJni.selectPlugin("SIFT")
        val out = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0)
        assertTrue("SIFT should detect keypoints, got ${out.keypointCount}", out.keypointCount > 0)
    }

    @Test
    fun SIFT_keypointsAreWithinFrameBounds() {
        ProcessorJni.selectPlugin("SIFT")
        val out = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0)
        out.keypointX.forEachIndexed { i, x ->
            assertTrue("SIFT keypointX[$i]=$x out of bounds", x >= 0f && x < W)
        }
        out.keypointY.forEachIndexed { i, y ->
            assertTrue("SIFT keypointY[$i]=$y out of bounds", y >= 0f && y < H)
        }
    }

    // ─── AKAZE ────────────────────────────────────────────────────────────────

    @Test
    fun AKAZE_detectsKeypointsOnCheckerboard() {
        ProcessorJni.selectPlugin("AKAZE")
        val out = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0)
        assertTrue("AKAZE should detect keypoints, got ${out.keypointCount}", out.keypointCount > 0)
    }

    @Test
    fun AKAZE_keypointsAreWithinFrameBounds() {
        ProcessorJni.selectPlugin("AKAZE")
        val out = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0)
        out.keypointX.forEach { x -> assertTrue(x >= 0f && x < W) }
        out.keypointY.forEach { y -> assertTrue(y >= 0f && y < H) }
    }

    // ─── Canny (OVERLAY_IMAGE) ────────────────────────────────────────────────

    @Test
    fun Canny_processFrame_outputTypeIsOVERLAY_IMAGE() {
        ProcessorJni.selectPlugin("Canny")
        val out = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0)
        assertEquals("Canny must return OVERLAY_IMAGE (1)", 1, out.outputType)
    }

    @Test
    fun Canny_processFrame_overlayDimensionsMatchFrame() {
        ProcessorJni.selectPlugin("Canny")
        val out = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0)
        assertEquals(W, out.overlayWidth)
        assertEquals(H, out.overlayHeight)
        assertEquals(W * H, out.overlayPixels.size)
    }

    @Test
    fun Canny_processFrame_keypointCountIsZero() {
        ProcessorJni.selectPlugin("Canny")
        val out = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0)
        assertEquals(0, out.keypointCount)
    }

    @Test
    fun Canny_processFrame_checkerboard_hasNonZeroPixels() {
        ProcessorJni.selectPlugin("Canny")
        val out = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0)
        assertTrue("Canny on checkerboard should produce edge pixels",
            out.overlayPixels.any { it != 0.toByte() })
    }

    @Test
    fun Canny_processFrame_uniformImage_hasAllZeroPixels() {
        ProcessorJni.selectPlugin("Canny")
        val out = ProcessorJni.processFrame(makeUniform(), W, H, 0)
        assertTrue("Canny on uniform image should produce no edges",
            out.overlayPixels.all { it == 0.toByte() })
    }

    @Test
    fun Canny_rotation90_overlayDimensionsAreSwapped() {
        ProcessorJni.selectPlugin("Canny")
        // 90°回転後: width=H, height=W
        val out = ProcessorJni.processFrame(makeCheckerboard(W, H), W, H, 90)
        assertEquals(H, out.overlayWidth)
        assertEquals(W, out.overlayHeight)
    }

    // ─── プラグイン切替 ────────────────────────────────────────────────────────

    @Test
    fun selectPlugin_switchingChangesOutputType() {
        ProcessorJni.selectPlugin("ORB")
        val orbOut = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0)
        assertEquals(0, orbOut.outputType)

        ProcessorJni.selectPlugin("Canny")
        val cannyOut = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0)
        assertEquals(1, cannyOut.outputType)
    }

    @Test
    fun selectPlugin_multipleSwitch_doesNotCrash() {
        val plugins = ProcessorJni.listPlugins()
        repeat(3) {
            plugins.forEach { ProcessorJni.selectPlugin(it) }
        }
        val out = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0)
        assertNotNull(out)
    }

    // ─── setParameter ─────────────────────────────────────────────────────────

    @Test
    fun setParameter_nFeatures_10_limitsORBOutput() {
        ProcessorJni.selectPlugin("ORB")
        ProcessorJni.setParameter("nFeatures", 10f)
        val out = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0)
        assertTrue("nFeatures=10 should yield <=10 keypoints, got ${out.keypointCount}",
            out.keypointCount <= 10)
    }

    @Test
    fun setParameter_unknownKey_doesNotCrash() {
        ProcessorJni.selectPlugin("ORB")
        ProcessorJni.setParameter("nonExistentKey", 42f)
        val out = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0)
        assertNotNull(out)
    }

    @Test
    fun setParameter_SIFT_contrastThreshold_low_increasesKeypointCount() {
        ProcessorJni.selectPlugin("SIFT")
        ProcessorJni.setParameter("contrastThreshold", 0.01f)
        val countLow = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0).keypointCount

        ProcessorJni.setParameter("contrastThreshold", 0.09f)
        val countHigh = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0).keypointCount

        assertTrue("Lower threshold should produce >= keypoints: $countLow vs $countHigh",
            countLow >= countHigh)
    }

    @Test
    fun setParameter_Canny_highThreshold2_reducesEdges() {
        ProcessorJni.selectPlugin("Canny")
        ProcessorJni.setParameter("threshold2", 10f)
        val moreEdges = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0).overlayPixels
            .count { it != 0.toByte() }

        ProcessorJni.setParameter("threshold2", 250f)
        val fewerEdges = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0).overlayPixels
            .count { it != 0.toByte() }

        assertTrue("Higher threshold should produce fewer or equal edges: $moreEdges vs $fewerEdges",
            moreEdges >= fewerEdges)
    }

    // ─── 回転処理 ─────────────────────────────────────────────────────────────

    @Test
    fun processFrame_rotation0_keypointsWithinOriginalDimensions() {
        ProcessorJni.selectPlugin("ORB")
        val out = ProcessorJni.processFrame(makeCheckerboard(W, H), W, H, 0)
        out.keypointX.forEach { assertTrue(it >= 0f && it < W) }
        out.keypointY.forEach { assertTrue(it >= 0f && it < H) }
    }

    @Test
    fun processFrame_rotation90_keypointsWithinSwappedDimensions() {
        ProcessorJni.selectPlugin("ORB")
        val out = ProcessorJni.processFrame(makeCheckerboard(W, H), W, H, 90)
        out.keypointX.forEach { assertTrue("x=$it out of [0,$H)", it >= 0f && it < H) }
        out.keypointY.forEach { assertTrue("y=$it out of [0,$W)", it >= 0f && it < W) }
    }

    @Test
    fun processFrame_allRotations_doNotCrash() {
        ProcessorJni.selectPlugin("ORB")
        listOf(0, 90, 180, 270).forEach { rotation ->
            val out = ProcessorJni.processFrame(makeCheckerboard(), W, H, rotation)
            assertNotNull("rotation=$rotation returned null", out)
        }
    }

    // ─── 連続呼び出し・安定性 ──────────────────────────────────────────────────

    @Test
    fun processFrame_repeatedCalls_areConsistent() {
        ProcessorJni.selectPlugin("ORB")
        val counts = (1..5).map {
            ProcessorJni.processFrame(makeCheckerboard(), W, H, 0).keypointCount
        }
        assertTrue("Results should be consistent: $counts", counts.all { it == counts[0] })
    }

    @Test
    fun processFrame_100frames_doesNotCrash() {
        ProcessorJni.selectPlugin("ORB")
        repeat(100) { ProcessorJni.processFrame(makeCheckerboard(), W, H, 0) }
    }

    @Test
    fun processFrame_smallImage_doesNotCrash() {
        ProcessorJni.selectPlugin("ORB")
        val tiny = ByteArray(64 * 48) { 128 }
        val out = ProcessorJni.processFrame(tiny, 64, 48, 0)
        assertNotNull(out)
    }

    @Test
    fun initDestroy_canBeCalledSequentially() {
        ProcessorJni.destroy()
        ProcessorJni.init()
        ProcessorJni.selectPlugin("ORB")
        val out = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0)
        assertNotNull(out)
    }

    // ─── Morphology (OVERLAY_IMAGE) ───────────────────────────────────────────

    @Test
    fun Morphology_listPlugins_containsMorphology() {
        assertTrue(ProcessorJni.listPlugins().contains("Morphology"))
    }

    @Test
    fun Morphology_outputTypeIsOVERLAY_IMAGE() {
        ProcessorJni.selectPlugin("Morphology")
        val out = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0)
        assertEquals("Morphology must return OVERLAY_IMAGE (1)", 1, out.outputType)
    }

    @Test
    fun Morphology_overlayDimensionsMatchFrame() {
        ProcessorJni.selectPlugin("Morphology")
        val out = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0)
        assertEquals(W, out.overlayWidth)
        assertEquals(H, out.overlayHeight)
        assertEquals(W * H, out.overlayPixels.size)
    }

    @Test
    fun Morphology_keypointCountIsZero() {
        ProcessorJni.selectPlugin("Morphology")
        val out = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0)
        assertEquals(0, out.keypointCount)
    }

    @Test
    fun Morphology_getPluginDefs_hasRequiredParams() {
        val defs = ProcessorJni.getPluginDefs("Morphology")
        val keys = defs.map { it.split("|")[0] }.toSet()
        assertTrue(keys.contains("operation"))
        assertTrue(keys.contains("kernelSize"))
        assertTrue(keys.contains("iterations"))
        assertTrue(keys.contains("kernelShape"))
    }

    @Test
    fun Morphology_gradient_checkerboardHasNonZeroPixels() {
        ProcessorJni.selectPlugin("Morphology")
        ProcessorJni.setParameter("operation", 4f)  // Gradient
        val out = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0)
        assertTrue("Gradient on checkerboard should produce non-zero pixels",
            out.overlayPixels.any { it != 0.toByte() })
    }

    @Test
    fun Morphology_gradient_uniformImageIsAllZero() {
        ProcessorJni.selectPlugin("Morphology")
        ProcessorJni.setParameter("operation", 4f)  // Gradient
        val out = ProcessorJni.processFrame(makeUniform(), W, H, 0)
        assertTrue("Gradient on uniform should be all zeros",
            out.overlayPixels.all { it == 0.toByte() })
    }

    @Test
    fun Morphology_allOperations_doNotCrash() {
        ProcessorJni.selectPlugin("Morphology")
        for (op in 0..6) {
            ProcessorJni.setParameter("operation", op.toFloat())
            val out = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0)
            assertNotNull("operation=$op returned null", out)
        }
    }

    @Test
    fun Morphology_allKernelShapes_doNotCrash() {
        ProcessorJni.selectPlugin("Morphology")
        for (shape in 0..2) {
            ProcessorJni.setParameter("kernelShape", shape.toFloat())
            val out = ProcessorJni.processFrame(makeCheckerboard(), W, H, 0)
            assertNotNull("kernelShape=$shape returned null", out)
        }
    }

    @Test
    fun Morphology_rotation90_overlayDimensionsAreSwapped() {
        ProcessorJni.selectPlugin("Morphology")
        val out = ProcessorJni.processFrame(makeCheckerboard(W, H), W, H, 90)
        assertEquals(H, out.overlayWidth)
        assertEquals(W, out.overlayHeight)
    }
}
