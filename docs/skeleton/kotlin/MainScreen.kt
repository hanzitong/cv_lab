package com.hanzitong.cvlab.ui

import androidx.compose.foundation.Canvas
import androidx.compose.foundation.layout.*
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import com.hanzitong.cvlab.viewmodel.MainViewModel

@Composable
fun MainScreen(viewModel: MainViewModel) {
    val state by viewModel.uiState.collectAsStateWithLifecycle()

    Column(
        modifier = Modifier
            .fillMaxSize()
            .windowInsetsPadding(WindowInsets.systemBars)
    ) {
        AlgorithmSelector(
            selected  = state.selectedAlgorithm,
            onSelect  = viewModel::onAlgorithmSelected,
            modifier  = Modifier.fillMaxWidth()
        )

        Box(modifier = Modifier.weight(1f)) {
            CameraPreview(
                onFrameResult = viewModel::onFrameResult,
                modifier      = Modifier.fillMaxSize()
            )
            KeypointOverlay(
                keypointX    = state.keypointX,
                keypointY    = state.keypointY,
                keypointSize = state.keypointSize,
                frameWidth   = state.frameWidth,
                frameHeight  = state.frameHeight,
                modifier     = Modifier.fillMaxSize()
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
            algorithm          = state.selectedAlgorithm,
            parameters         = state.parameters,
            onParameterChanged = viewModel::onParameterChanged,
            modifier           = Modifier.fillMaxWidth()
        )
    }
}

/**
 * キーポイントをカメラフレーム座標からCanvas座標に変換して描画する。
 *
 * PreviewView は FIT_CENTER モード（全フレームが見える）を使用するため、
 * 変換式は以下の通り:
 *   scale   = min(canvasW / frameW, canvasH / frameH)   ← FIT = min
 *   offsetX = (canvasW - frameW * scale) / 2            ← 黒帯の幅の半分
 *   offsetY = (canvasH - frameH * scale) / 2
 *   screenX = keypointX * scale + offsetX
 *   screenY = keypointY * scale + offsetY
 *
 * NOTE: FILL_CENTER に変更する場合は min → max に変え、
 *       フレーム外にはみ出るキーポイントをクリップすること。
 */
@Composable
private fun KeypointOverlay(
    keypointX: FloatArray,
    keypointY: FloatArray,
    keypointSize: FloatArray,
    frameWidth: Int,
    frameHeight: Int,
    modifier: Modifier = Modifier
) {
    Canvas(modifier = modifier) {
        if (frameWidth <= 0 || frameHeight <= 0 || keypointX.isEmpty()) return@Canvas

        val canvasW = size.width
        val canvasH = size.height

        // FIT_CENTER: アスペクト比を保ちながら全体が収まるようにスケール
        val scale   = minOf(canvasW / frameWidth, canvasH / frameHeight)
        val offsetX = (canvasW - frameWidth  * scale) / 2f
        val offsetY = (canvasH - frameHeight * scale) / 2f

        for (i in keypointX.indices) {
            val sx = keypointX[i]   * scale + offsetX
            val sy = keypointY[i]   * scale + offsetY

            // OpenCV の keypoint.size は近傍直径 → 半径に変換してスケール適用
            val radius = (keypointSize[i] / 2f * scale).coerceIn(3f, 30f)

            drawCircle(
                color  = Color.Green,
                radius = radius,
                center = Offset(sx, sy),
                style  = Stroke(width = 2f)
            )
        }
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
