package com.hanzitong.cvfeaturelab.viewmodel

import com.hanzitong.cvfeaturelab.jni.IProcessorApi
import com.hanzitong.cvfeaturelab.jni.ProcessOutput
import io.mockk.*
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.launch
import kotlinx.coroutines.test.*
import org.junit.Assert.*
import org.junit.Before
import org.junit.Test

@OptIn(ExperimentalCoroutinesApi::class)
class MainViewModelTest {

    private lateinit var mockApi: IProcessorApi
    private lateinit var viewModel: MainViewModel

    // プラグイン定義のヘルパー
    private val orbDefs = arrayOf(
        "nFeatures|Max Features|100|5000|500|1",
        "scaleFactor|Scale Factor|1.1|2.0|1.2|0"
    )
    private val siftDefs = arrayOf(
        "nFeatures|Max Features|0|2000|500|1",
        "contrastThreshold|Contrast Threshold|0.01|0.1|0.04|0"
    )
    private val cannyDefs = arrayOf(
        "threshold1|Low Threshold|0|300|50|0",
        "threshold2|High Threshold|0|300|150|0"
    )

    private fun makeOutput(
        outputType: Int = 0,
        kpCount: Int = 0,
        timeMs: Float = 10f
    ) = ProcessOutput(
        outputType       = outputType,
        keypointX        = FloatArray(kpCount),
        keypointY        = FloatArray(kpCount),
        keypointSize     = FloatArray(kpCount),
        keypointCount    = kpCount,
        overlayPixels    = ByteArray(0),
        overlayWidth     = 0,
        overlayHeight    = 0,
        arrowStartX      = FloatArray(0),
        arrowStartY      = FloatArray(0),
        arrowEndX        = FloatArray(0),
        arrowEndY        = FloatArray(0),
        processingTimeMs = timeMs
    )

    @Before
    fun setUp() {
        mockApi = mockk(relaxed = true)
        every { mockApi.listPlugins() } returns arrayOf("AKAZE", "Canny", "ORB", "SIFT")
        every { mockApi.getPluginDefs("ORB") } returns orbDefs
        every { mockApi.getPluginDefs("SIFT") } returns siftDefs
        every { mockApi.getPluginDefs("Canny") } returns cannyDefs
        every { mockApi.getPluginDefs("AKAZE") } returns arrayOf(
            "threshold|Threshold|0.0001|0.01|0.001|0"
        )
        viewModel = MainViewModel(api = mockApi)
    }

    // ─── 初期状態 ──────────────────────────────────────────────────────────────

    @Test
    fun `initial state has empty plugins before loadPlugins`() {
        assertTrue(viewModel.uiState.value.plugins.isEmpty())
    }

    @Test
    fun `initial state has empty selectedPlugin before loadPlugins`() {
        assertEquals("", viewModel.uiState.value.selectedPlugin)
    }

    @Test
    fun `initial keypoint count is zero`() {
        assertEquals(0, viewModel.uiState.value.keypointCount)
    }

    @Test
    fun `initial processing time is zero`() {
        assertEquals(0f, viewModel.uiState.value.processingTimeMs, 0f)
    }

    // ─── loadPlugins ──────────────────────────────────────────────────────────

    @Test
    fun `loadPlugins populates plugins list from api`() {
        viewModel.loadPlugins()
        assertEquals(4, viewModel.uiState.value.plugins.size)
    }

    @Test
    fun `loadPlugins sets selectedPlugin to first plugin`() {
        viewModel.loadPlugins()
        // listPlugins returns ["AKAZE","Canny","ORB","SIFT"] — first is AKAZE
        assertEquals("AKAZE", viewModel.uiState.value.selectedPlugin)
    }

    @Test
    fun `loadPlugins calls selectPlugin for first plugin`() {
        viewModel.loadPlugins()
        verify(exactly = 1) { mockApi.selectPlugin("AKAZE") }
    }

    @Test
    fun `loadPlugins sends default parameters to JNI`() {
        viewModel.loadPlugins()
        verify { mockApi.setParameter("threshold", 0.001f) }
    }

    @Test
    fun `loadPlugins parses ORB params correctly`() {
        viewModel.loadPlugins()
        val orb = viewModel.uiState.value.plugins.find { it.name == "ORB" }
        assertNotNull(orb)
        assertEquals(2, orb!!.params.size)
        assertEquals("nFeatures", orb.params[0].key)
        assertEquals(100f, orb.params[0].min, 0.001f)
        assertEquals(5000f, orb.params[0].max, 0.001f)
        assertEquals(500f, orb.params[0].default, 0.001f)
        assertTrue(orb.params[0].isInteger)
        assertFalse(orb.params[1].isInteger) // scaleFactor
    }

    @Test
    fun `loadPlugins parses float params with isInteger false`() {
        viewModel.loadPlugins()
        val sift = viewModel.uiState.value.plugins.find { it.name == "SIFT" }!!
        val contrastDef = sift.params.find { it.key == "contrastThreshold" }!!
        assertFalse(contrastDef.isInteger)
    }

    @Test
    fun `loadPlugins initializes parameters with first plugin defaults`() {
        viewModel.loadPlugins()
        // First plugin is AKAZE
        assertEquals(0.001f, viewModel.uiState.value.parameters["threshold"]!!, 0.0001f)
    }

    @Test
    fun `loadPlugins calls listPlugins once`() {
        viewModel.loadPlugins()
        verify(exactly = 1) { mockApi.listPlugins() }
    }

    // ─── onPluginSelected ────────────────────────────────────────────────────

    @Test
    fun `onPluginSelected updates selectedPlugin in state`() {
        viewModel.loadPlugins()
        viewModel.onPluginSelected("ORB")
        assertEquals("ORB", viewModel.uiState.value.selectedPlugin)

        viewModel.onPluginSelected("SIFT")
        assertEquals("SIFT", viewModel.uiState.value.selectedPlugin)
    }

    @Test
    fun `onPluginSelected resets parameters to new plugin defaults`() {
        viewModel.loadPlugins()
        viewModel.onParameterChanged("nFeatures", 9999f)  // ORB のパラメーターを汚染
        viewModel.onPluginSelected("ORB")
        assertEquals(500f, viewModel.uiState.value.parameters["nFeatures"]!!, 0.001f)
    }

    @Test
    fun `onPluginSelected does not carry over previous plugin parameters`() {
        viewModel.loadPlugins()
        viewModel.onPluginSelected("ORB")
        val orbState = viewModel.uiState.value.parameters
        // ORBパラメーターのみ存在し、AKAZEのキーが残っていないこと
        assertFalse(orbState.containsKey("threshold"))
    }

    @Test
    fun `onPluginSelected calls JNI selectPlugin`() {
        viewModel.loadPlugins()
        viewModel.onPluginSelected("ORB")
        verify { mockApi.selectPlugin("ORB") }
    }

    @Test
    fun `onPluginSelected sends all new default parameters to JNI`() {
        viewModel.loadPlugins()
        viewModel.onPluginSelected("ORB")
        verify { mockApi.setParameter("nFeatures", 500f) }
        verify { mockApi.setParameter("scaleFactor", 1.2f) }
    }

    @Test
    fun `onPluginSelected calls selectPlugin before setParameter`() {
        viewModel.loadPlugins()
        val callOrder = mutableListOf<String>()
        every { mockApi.selectPlugin(any()) } answers { callOrder.add("selectPlugin") }
        every { mockApi.setParameter(any(), any()) } answers { callOrder.add("setParameter") }

        viewModel.onPluginSelected("ORB")

        assertEquals("selectPlugin", callOrder.first())
    }

    @Test
    fun `onPluginSelected with unknown name does not crash`() {
        viewModel.loadPlugins()
        viewModel.onPluginSelected("NonExistent")
        // 変化なし
        assertEquals("AKAZE", viewModel.uiState.value.selectedPlugin)
    }

    @Test
    fun `switching plugin twice calls JNI selectPlugin twice`() {
        viewModel.loadPlugins()
        viewModel.onPluginSelected("ORB")
        viewModel.onPluginSelected("SIFT")
        verify(exactly = 1) { mockApi.selectPlugin("ORB") }
        verify(exactly = 1) { mockApi.selectPlugin("SIFT") }
    }

    // ─── onParameterChanged ───────────────────────────────────────────────────

    @Test
    fun `onParameterChanged updates target parameter value`() {
        viewModel.loadPlugins()
        viewModel.onParameterChanged("nFeatures", 1000f)
        assertEquals(1000f, viewModel.uiState.value.parameters["nFeatures"], 0f)
    }

    @Test
    fun `onParameterChanged does not affect other parameters`() {
        viewModel.loadPlugins()
        viewModel.onPluginSelected("ORB")
        val before = viewModel.uiState.value.parameters.toMap()
        viewModel.onParameterChanged("nFeatures", 999f)
        val after = viewModel.uiState.value.parameters

        before.keys.filter { it != "nFeatures" }.forEach { key ->
            assertEquals("$key changed unexpectedly", before[key], after[key])
        }
    }

    @Test
    fun `onParameterChanged calls JNI setParameter with correct key and value`() {
        viewModel.loadPlugins()
        viewModel.onParameterChanged("scaleFactor", 1.5f)
        verify(exactly = 1) { mockApi.setParameter("scaleFactor", 1.5f) }
    }

    @Test
    fun `multiple onParameterChanged accumulates changes`() {
        viewModel.loadPlugins()
        viewModel.onPluginSelected("ORB")
        viewModel.onParameterChanged("nFeatures", 200f)
        viewModel.onParameterChanged("scaleFactor", 1.4f)
        assertEquals(200f, viewModel.uiState.value.parameters["nFeatures"], 0f)
        assertEquals(1.4f, viewModel.uiState.value.parameters["scaleFactor"]!!, 0.0001f)
    }

    @Test
    fun `onParameterChanged with same key overwrites previous value`() {
        viewModel.loadPlugins()
        viewModel.onParameterChanged("nFeatures", 100f)
        viewModel.onParameterChanged("nFeatures", 500f)
        assertEquals(500f, viewModel.uiState.value.parameters["nFeatures"], 0f)
    }

    // ─── onFrameResult ────────────────────────────────────────────────────────

    @Test
    fun `onFrameResult KEYPOINTS updates keypoint arrays`() {
        val output = ProcessOutput(
            outputType       = 0,
            keypointX        = floatArrayOf(10f, 20f, 30f),
            keypointY        = floatArrayOf(40f, 50f, 60f),
            keypointSize     = floatArrayOf(8f, 12f, 6f),
            keypointCount    = 3,
            overlayPixels    = ByteArray(0),
            overlayWidth     = 0,
            overlayHeight    = 0,
            arrowStartX      = FloatArray(0),
            arrowStartY      = FloatArray(0),
            arrowEndX        = FloatArray(0),
            arrowEndY        = FloatArray(0),
            processingTimeMs = 18.5f
        )
        viewModel.onFrameResult(output, 640, 480)
        val state = viewModel.uiState.value

        assertArrayEquals(floatArrayOf(10f, 20f, 30f), state.keypointX, 0.001f)
        assertArrayEquals(floatArrayOf(40f, 50f, 60f), state.keypointY, 0.001f)
        assertArrayEquals(floatArrayOf(8f, 12f, 6f),   state.keypointSize, 0.001f)
        assertEquals(3, state.keypointCount)
        assertEquals(0, state.outputType)
    }

    @Test
    fun `onFrameResult OVERLAY_IMAGE updates overlay fields`() {
        val pixels = ByteArray(100 * 75) { 128.toByte() }
        val output = ProcessOutput(
            outputType       = 1,
            keypointX        = FloatArray(0),
            keypointY        = FloatArray(0),
            keypointSize     = FloatArray(0),
            keypointCount    = 0,
            overlayPixels    = pixels,
            overlayWidth     = 100,
            overlayHeight    = 75,
            arrowStartX      = FloatArray(0),
            arrowStartY      = FloatArray(0),
            arrowEndX        = FloatArray(0),
            arrowEndY        = FloatArray(0),
            processingTimeMs = 5f
        )
        viewModel.onFrameResult(output, 640, 480)
        val state = viewModel.uiState.value

        assertEquals(1, state.outputType)
        assertEquals(100, state.overlayWidth)
        assertEquals(75, state.overlayHeight)
        assertEquals(100 * 75, state.overlayPixels.size)
    }

    @Test
    fun `onFrameResult POINT_VECTORS updates arrow fields`() {
        val output = ProcessOutput(
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
            processingTimeMs = 3f
        )
        viewModel.onFrameResult(output, 640, 480)
        val state = viewModel.uiState.value

        assertEquals(2, state.outputType)
        assertArrayEquals(floatArrayOf(1f, 2f), state.arrowStartX, 0.001f)
        assertArrayEquals(floatArrayOf(7f, 8f), state.arrowEndY, 0.001f)
    }

    @Test
    fun `onFrameResult updates processing time`() {
        viewModel.onFrameResult(makeOutput(timeMs = 22.3f), 640, 480)
        assertEquals(22.3f, viewModel.uiState.value.processingTimeMs, 0.01f)
    }

    @Test
    fun `onFrameResult updates frame dimensions`() {
        viewModel.onFrameResult(makeOutput(), 480, 640)
        assertEquals(480, viewModel.uiState.value.frameWidth)
        assertEquals(640, viewModel.uiState.value.frameHeight)
    }

    @Test
    fun `onFrameResult with empty result sets count to zero`() {
        viewModel.onFrameResult(makeOutput(kpCount = 5), 640, 480)
        viewModel.onFrameResult(makeOutput(kpCount = 0), 640, 480)
        assertEquals(0, viewModel.uiState.value.keypointCount)
    }

    @Test
    fun `successive onFrameResult calls always reflect latest result`() {
        repeat(3) { i ->
            viewModel.onFrameResult(makeOutput(kpCount = i), 640, 480)
        }
        assertEquals(2, viewModel.uiState.value.keypointCount)
    }

    // ─── StateFlow emissions ─────────────────────────────────────────────────

    @Test
    fun `StateFlow emits updated plugin on plugin change`() = runTest {
        viewModel.loadPlugins()
        val collected = mutableListOf<String>()
        val job = launch(UnconfinedTestDispatcher(testScheduler)) {
            viewModel.uiState.collect { collected.add(it.selectedPlugin) }
        }
        viewModel.onPluginSelected("ORB")
        viewModel.onPluginSelected("SIFT")
        job.cancel()

        assertTrue(collected.contains("ORB"))
        assertTrue(collected.contains("SIFT"))
    }

    @Test
    fun `StateFlow emits on each parameter change`() = runTest {
        viewModel.loadPlugins()
        val emissionCount = mutableListOf<Int>()
        val job = launch(UnconfinedTestDispatcher(testScheduler)) {
            viewModel.uiState.collect { emissionCount.add(it.keypointCount) }
        }
        viewModel.onParameterChanged("nFeatures", 100f)
        viewModel.onParameterChanged("scaleFactor", 1.4f)
        job.cancel()

        assertTrue(emissionCount.size >= 3)
    }
}
