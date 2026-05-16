package com.hanzitong.cvfeaturelab.ui

import androidx.compose.foundation.layout.*
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import com.hanzitong.cvfeaturelab.viewmodel.MainViewModel

@Composable
fun MainScreen(viewModel: MainViewModel) {
    val state by viewModel.uiState.collectAsStateWithLifecycle()
    val selectedPluginDefs = state.plugins.find { it.name == state.selectedPlugin }?.params ?: emptyList()

    Column(
        modifier = Modifier
            .fillMaxSize()
            .windowInsetsPadding(WindowInsets.systemBars)
    ) {
        PluginSelector(
            plugins   = state.plugins,
            selected  = state.selectedPlugin,
            onSelect  = viewModel::onPluginSelected,
            modifier  = Modifier.fillMaxWidth()
        )

        Box(modifier = Modifier.weight(1f)) {
            CameraPreview(
                onFrameResult = viewModel::onFrameResult,
                modifier      = Modifier.fillMaxSize()
            )
            ProcessingOverlay(
                state    = state,
                modifier = Modifier.fillMaxSize()
            )
            StatsBar(
                count    = state.keypointCount,
                timeMs   = state.processingTimeMs,
                modifier = Modifier
                    .align(Alignment.BottomCenter)
                    .fillMaxWidth()
            )
        }

        ParameterPanel(
            defs               = selectedPluginDefs,
            parameters         = state.parameters,
            onParameterChanged = viewModel::onParameterChanged,
            modifier           = Modifier.fillMaxWidth()
        )
    }
}

@Composable
private fun StatsBar(
    count: Int,
    timeMs: Float,
    modifier: Modifier = Modifier
) {
    Surface(
        color    = Color.Black.copy(alpha = 0.55f),
        modifier = modifier
    ) {
        Text(
            text     = "Keypoints: $count  |  ${timeMs.toInt()} ms",
            color    = Color.White,
            style    = MaterialTheme.typography.bodySmall,
            modifier = Modifier.padding(horizontal = 16.dp, vertical = 4.dp)
        )
    }
}
