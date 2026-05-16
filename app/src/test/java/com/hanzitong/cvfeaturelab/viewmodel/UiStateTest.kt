package com.hanzitong.cvfeaturelab.viewmodel

import org.junit.Assert.*
import org.junit.Test

class UiStateTest {

    // ─── PluginInfo ───────────────────────────────────────────────────────────

    @Test
    fun `PluginInfo stores name and params`() {
        val defs = listOf(ParameterDef("k", "Key", 0f, 100f, 50f))
        val info = PluginInfo("ORB", defs)
        assertEquals("ORB", info.name)
        assertEquals(1, info.params.size)
    }

    // ─── ParameterDef ─────────────────────────────────────────────────────────

    @Test
    fun `ParameterDef min must be less than max`() {
        val def = ParameterDef("k", "Key", 0f, 100f, 50f)
        assertTrue(def.min < def.max)
    }

    @Test
    fun `ParameterDef default is within min-max range`() {
        val def = ParameterDef("k", "Key", 0f, 100f, 50f)
        assertTrue(def.default >= def.min && def.default <= def.max)
    }

    @Test
    fun `ParameterDef isInteger defaults to true`() {
        val def = ParameterDef("k", "Key", 0f, 10f, 5f)
        assertTrue(def.isInteger)
    }

    @Test
    fun `ParameterDef float parameter has isInteger false`() {
        val def = ParameterDef("threshold", "Threshold", 0.001f, 0.01f, 0.005f, isInteger = false)
        assertFalse(def.isInteger)
    }

    // ─── UiState デフォルト値 ──────────────────────────────────────────────────

    @Test
    fun `default UiState has empty plugins list`() {
        assertEquals(emptyList<PluginInfo>(), UiState().plugins)
    }

    @Test
    fun `default UiState has empty selected plugin`() {
        assertEquals("", UiState().selectedPlugin)
    }

    @Test
    fun `default UiState has empty parameters`() {
        assertEquals(emptyMap<String, Float>(), UiState().parameters)
    }

    @Test
    fun `default UiState outputType is KEYPOINTS (0)`() {
        assertEquals(0, UiState().outputType)
    }

    @Test
    fun `default UiState has empty keypoints`() {
        val state = UiState()
        assertEquals(0, state.keypointX.size)
        assertEquals(0, state.keypointY.size)
        assertEquals(0, state.keypointSize.size)
        assertEquals(0, state.keypointCount)
    }

    @Test
    fun `default UiState has empty overlay fields`() {
        val state = UiState()
        assertEquals(0, state.overlayPixels.size)
        assertEquals(0, state.overlayWidth)
        assertEquals(0, state.overlayHeight)
    }

    @Test
    fun `default UiState has empty arrow fields`() {
        val state = UiState()
        assertEquals(0, state.arrowStartX.size)
        assertEquals(0, state.arrowStartY.size)
        assertEquals(0, state.arrowEndX.size)
        assertEquals(0, state.arrowEndY.size)
    }

    @Test
    fun `default UiState processing time is zero`() {
        assertEquals(0f, UiState().processingTimeMs, 0f)
    }

    @Test
    fun `default UiState frame dimensions are portrait`() {
        val state = UiState()
        assertEquals(480, state.frameWidth)
        assertEquals(640, state.frameHeight)
    }

    // ─── UiState copy ────────────────────────────────────────────────────────

    @Test
    fun `UiState copy preserves all fields`() {
        val plugins = listOf(PluginInfo("ORB", emptyList()))
        val original = UiState(
            plugins          = plugins,
            selectedPlugin   = "ORB",
            parameters       = mapOf("nFeatures" to 100f),
            outputType       = 0,
            keypointX        = floatArrayOf(1f, 2f),
            keypointCount    = 2,
            processingTimeMs = 12.5f,
            frameWidth       = 320,
            frameHeight      = 240
        )
        val copied = original.copy()
        assertEquals(original.plugins, copied.plugins)
        assertEquals(original.selectedPlugin, copied.selectedPlugin)
        assertEquals(original.parameters, copied.parameters)
        assertEquals(original.outputType, copied.outputType)
        assertEquals(original.keypointCount, copied.keypointCount)
        assertEquals(original.processingTimeMs, copied.processingTimeMs, 0.0001f)
        assertEquals(original.frameWidth, copied.frameWidth)
        assertEquals(original.frameHeight, copied.frameHeight)
    }

    @Test
    fun `UiState copy with plugin change retains other fields`() {
        val state = UiState(
            selectedPlugin   = "ORB",
            processingTimeMs = 15f,
            keypointCount    = 5
        )
        val updated = state.copy(selectedPlugin = "SIFT")
        assertEquals("SIFT", updated.selectedPlugin)
        assertEquals(15f, updated.processingTimeMs, 0f)
        assertEquals(5, updated.keypointCount)
    }

    // ─── outputType 値 ───────────────────────────────────────────────────────

    @Test
    fun `KEYPOINTS outputType value is 0`() {
        assertEquals(0, UiState(outputType = 0).outputType)
    }

    @Test
    fun `OVERLAY_IMAGE outputType value is 1`() {
        assertEquals(1, UiState(outputType = 1).outputType)
    }

    @Test
    fun `POINT_VECTORS outputType value is 2`() {
        assertEquals(2, UiState(outputType = 2).outputType)
    }
}
