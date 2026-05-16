package com.hanzitong.cvfeaturelab.viewmodel

import androidx.lifecycle.ViewModel
import com.hanzitong.cvfeaturelab.jni.IProcessorApi
import com.hanzitong.cvfeaturelab.jni.ProcessorJni
import com.hanzitong.cvfeaturelab.jni.ProcessOutput
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update

// api はユニットテストでモックに差し替えられる（DD-11参照）
class MainViewModel(
    internal val api: IProcessorApi = ProcessorJni
) : ViewModel() {

    private val _uiState = MutableStateFlow(UiState())
    val uiState: StateFlow<UiState> = _uiState.asStateFlow()

    /** C++ のプラグイン一覧を取得して UiState を初期化する。onCreate から呼ぶこと */
    fun loadPlugins() {
        val names = api.listPlugins()
        val plugins = names.map { name ->
            val defs = api.getPluginDefs(name).map { defStr ->
                val p = defStr.split("|")
                ParameterDef(
                    key        = p[0],
                    displayName = p[1],
                    min        = p[2].toFloat(),
                    max        = p[3].toFloat(),
                    default    = p[4].toFloat(),
                    isInteger  = p[5] == "1"
                )
            }
            PluginInfo(name, defs)
        }

        val first = plugins.firstOrNull()
        val defaults = first?.params?.associate { it.key to it.default } ?: emptyMap()

        _uiState.update {
            it.copy(plugins = plugins, selectedPlugin = first?.name ?: "", parameters = defaults)
        }
        if (first != null) {
            api.selectPlugin(first.name)
            defaults.forEach { (k, v) -> api.setParameter(k, v) }
        }
    }

    fun onPluginSelected(name: String) {
        val plugin = _uiState.value.plugins.find { it.name == name } ?: return
        val defaults = plugin.params.associate { it.key to it.default }
        _uiState.update { it.copy(selectedPlugin = name, parameters = defaults) }
        api.selectPlugin(name)
        defaults.forEach { (k, v) -> api.setParameter(k, v) }
    }

    fun onParameterChanged(key: String, value: Float) {
        _uiState.update { it.copy(parameters = it.parameters + (key to value)) }
        api.setParameter(key, value)
    }

    // CameraX 分析スレッドから呼ばれる — StateFlow.update はスレッドセーフ
    fun onFrameResult(output: ProcessOutput, frameWidth: Int, frameHeight: Int) {
        _uiState.update {
            it.copy(
                outputType       = output.outputType,
                keypointX        = output.keypointX,
                keypointY        = output.keypointY,
                keypointSize     = output.keypointSize,
                keypointCount    = output.keypointCount,
                overlayPixels    = output.overlayPixels,
                overlayWidth     = output.overlayWidth,
                overlayHeight    = output.overlayHeight,
                arrowStartX      = output.arrowStartX,
                arrowStartY      = output.arrowStartY,
                arrowEndX        = output.arrowEndX,
                arrowEndY        = output.arrowEndY,
                processingTimeMs = output.processingTimeMs,
                frameWidth       = frameWidth,
                frameHeight      = frameHeight
            )
        }
    }
}
