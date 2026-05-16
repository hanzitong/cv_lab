package com.hanzitong.cvfeaturelab.viewmodel

import androidx.lifecycle.ViewModel
import com.hanzitong.cvfeaturelab.jni.FeatureProcessorJni
import com.hanzitong.cvfeaturelab.jni.IFeatureDetectorApi
import com.hanzitong.cvfeaturelab.jni.ProcessResult
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update

// api is injected to allow mocking in unit tests (default = real JNI object)
class MainViewModel(
    internal val api: IFeatureDetectorApi = FeatureProcessorJni
) : ViewModel() {

    private val _uiState = MutableStateFlow(UiState())
    val uiState: StateFlow<UiState> = _uiState.asStateFlow()

    fun onAlgorithmSelected(algo: Algorithm) {
        val defaults = defaultParams(algo)
        _uiState.update { it.copy(selectedAlgorithm = algo, parameters = defaults) }
        api.setAlgorithm(algo.ordinal)
        defaults.forEach { (k, v) -> api.setParameter(k, v) }
    }

    fun onParameterChanged(key: String, value: Float) {
        _uiState.update { it.copy(parameters = it.parameters + (key to value)) }
        api.setParameter(key, value)
    }

    // Called from CameraX ImageAnalysis background thread — StateFlow.update is thread-safe
    fun onFrameResult(result: ProcessResult, frameWidth: Int, frameHeight: Int) {
        _uiState.update {
            it.copy(
                keypointX        = result.keypointX,
                keypointY        = result.keypointY,
                keypointSize     = result.keypointSize,
                keypointCount    = result.count,
                processingTimeMs = result.processingTimeMs,
                frameWidth       = frameWidth,
                frameHeight      = frameHeight
            )
        }
    }
}
