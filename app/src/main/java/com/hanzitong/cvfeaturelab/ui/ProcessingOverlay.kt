package com.hanzitong.cvfeaturelab.ui

import android.graphics.Bitmap
import androidx.compose.foundation.Canvas
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.graphics.drawscope.translate
import androidx.compose.ui.graphics.drawscope.scale
import com.hanzitong.cvfeaturelab.viewmodel.UiState

/**
 * outputType に応じてオーバーレイを描画する。
 * 0=KEYPOINTS: 緑の円、1=OVERLAY_IMAGE: グレースケール画像、2=POINT_VECTORS: 黄色の矢印
 * 座標系はすべて FIT_CENTER の min スケール変換を使う（DD-12参照）。
 */
@Composable
fun ProcessingOverlay(state: UiState, modifier: Modifier = Modifier) {
    when (state.outputType) {
        1    -> OverlayImageCanvas(state, modifier)
        2    -> ArrowVectorsCanvas(state, modifier)
        else -> KeypointCanvas(state, modifier)
    }
}

@Composable
private fun KeypointCanvas(state: UiState, modifier: Modifier) {
    Canvas(modifier = modifier) {
        if (state.frameWidth <= 0 || state.frameHeight <= 0 || state.keypointX.isEmpty()) return@Canvas

        val scale   = minOf(size.width / state.frameWidth, size.height / state.frameHeight)
        val offsetX = (size.width  - state.frameWidth  * scale) / 2f
        val offsetY = (size.height - state.frameHeight * scale) / 2f

        for (i in state.keypointX.indices) {
            val sx = state.keypointX[i]   * scale + offsetX
            val sy = state.keypointY[i]   * scale + offsetY
            val r  = (state.keypointSize[i] / 2f * scale).coerceIn(3f, 30f)
            drawCircle(
                color  = Color.Green,
                radius = r,
                center = Offset(sx, sy),
                style  = Stroke(width = 2f)
            )
        }
    }
}

@Composable
private fun OverlayImageCanvas(state: UiState, modifier: Modifier) {
    // ByteArray キーが変わったときだけ Bitmap を再生成する
    val imageBitmap = remember(state.overlayPixels, state.overlayWidth, state.overlayHeight) {
        val w = state.overlayWidth
        val h = state.overlayHeight
        if (w <= 0 || h <= 0 || state.overlayPixels.size < w * h) return@remember null

        val pixels = IntArray(w * h) { i ->
            val g = state.overlayPixels[i].toInt() and 0xFF
            android.graphics.Color.argb(255, g, g, g)
        }
        val bmp = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888)
        bmp.setPixels(pixels, 0, w, 0, 0, w, h)
        bmp.asImageBitmap()
    }

    Canvas(modifier = modifier) {
        val bm = imageBitmap ?: return@Canvas
        if (state.frameWidth <= 0 || state.frameHeight <= 0) return@Canvas

        val sc   = minOf(size.width / state.frameWidth, size.height / state.frameHeight)
        val offX = (size.width  - state.frameWidth  * sc) / 2f
        val offY = (size.height - state.frameHeight * sc) / 2f

        translate(offX, offY) {
            scale(sc, Offset.Zero) {
                drawImage(bm)
            }
        }
    }
}

@Composable
private fun ArrowVectorsCanvas(state: UiState, modifier: Modifier) {
    Canvas(modifier = modifier) {
        if (state.frameWidth <= 0 || state.frameHeight <= 0) return@Canvas

        val sc   = minOf(size.width / state.frameWidth, size.height / state.frameHeight)
        val offX = (size.width  - state.frameWidth  * sc) / 2f
        val offY = (size.height - state.frameHeight * sc) / 2f

        val n = minOf(state.arrowStartX.size, state.arrowEndX.size)
        for (i in 0 until n) {
            val sx = state.arrowStartX[i] * sc + offX
            val sy = state.arrowStartY[i] * sc + offY
            val ex = state.arrowEndX[i]   * sc + offX
            val ey = state.arrowEndY[i]   * sc + offY
            drawLine(Color.Yellow, Offset(sx, sy), Offset(ex, ey), strokeWidth = 2f)
        }
    }
}
