package com.hanzitong.cvfeaturelab.ui

import android.util.Log
import android.util.Size
import androidx.camera.core.CameraSelector
import androidx.camera.core.ImageAnalysis
import androidx.camera.core.ImageProxy
import androidx.camera.core.Preview
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.camera.view.PreviewView
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.viewinterop.AndroidView
import androidx.core.content.ContextCompat
import androidx.lifecycle.compose.LocalLifecycleOwner
import com.hanzitong.cvfeaturelab.jni.ProcessorJni
import com.hanzitong.cvfeaturelab.jni.ProcessOutput
import java.util.concurrent.Executors
import kotlin.coroutines.resume
import kotlin.coroutines.suspendCoroutine

private const val TAG          = "CameraPreview"
private const val FRAME_WIDTH  = 640
private const val FRAME_HEIGHT = 480

/**
 * CameraX プレビュー + YUV フレーム解析 Composable。
 * PreviewView は FIT_CENTER モード（全フレームが見える）で表示する（DD-12参照）。
 *
 * @param onFrameResult  (ProcessOutput, rotatedFrameWidth, rotatedFrameHeight) のコールバック。
 *                       CameraX の分析スレッドから呼ばれる。
 */
@Composable
fun CameraPreview(
    onFrameResult: (ProcessOutput, Int, Int) -> Unit,
    modifier: Modifier = Modifier
) {
    val context         = LocalContext.current
    val lifecycleOwner  = LocalLifecycleOwner.current
    val previewView     = remember {
        PreviewView(context).apply {
            scaleType = PreviewView.ScaleType.FIT_CENTER
        }
    }
    val analysisExecutor = remember { Executors.newSingleThreadExecutor() }

    DisposableEffect(Unit) {
        onDispose {
            analysisExecutor.shutdown()
            Log.d(TAG, "Analysis executor shut down")
        }
    }

    LaunchedEffect(lifecycleOwner) {
        val cameraProvider = suspendCoroutine<ProcessCameraProvider> { cont ->
            ProcessCameraProvider.getInstance(context).also { future ->
                future.addListener({ cont.resume(future.get()) },
                    ContextCompat.getMainExecutor(context))
            }
        }

        val preview = Preview.Builder()
            .setTargetResolution(Size(FRAME_WIDTH, FRAME_HEIGHT))
            .build()
            .also { it.setSurfaceProvider(previewView.surfaceProvider) }

        val imageAnalysis = ImageAnalysis.Builder()
            .setTargetResolution(Size(FRAME_WIDTH, FRAME_HEIGHT))
            .setBackpressureStrategy(ImageAnalysis.STRATEGY_KEEP_ONLY_LATEST)
            .setOutputImageFormat(ImageAnalysis.OUTPUT_IMAGE_FORMAT_YUV_420_888)
            .build()
            .also { analysis ->
                analysis.setAnalyzer(analysisExecutor) { proxy ->
                    analyzeFrame(proxy, onFrameResult)
                }
            }

        cameraProvider.unbindAll()
        try {
            cameraProvider.bindToLifecycle(
                lifecycleOwner,
                CameraSelector.DEFAULT_BACK_CAMERA,
                preview,
                imageAnalysis
            )
            Log.d(TAG, "Camera bound to lifecycle")
        } catch (e: Exception) {
            Log.e(TAG, "Camera bind failed", e)
        }
    }

    AndroidView(factory = { previewView }, modifier = modifier)
}

/**
 * ImageProxy から Y 平面を抽出して JNI processFrame を呼ぶ（DD-07参照）。
 * rowStride が width より大きい場合は行ごとにパディングを除去する。
 */
private fun analyzeFrame(
    imageProxy: ImageProxy,
    onFrameResult: (ProcessOutput, Int, Int) -> Unit
) {
    try {
        val width     = imageProxy.width
        val height    = imageProxy.height
        val rotation  = imageProxy.imageInfo.rotationDegrees
        val yPlane    = imageProxy.planes[0]
        val rowStride = yPlane.rowStride
        val yBuffer   = yPlane.buffer

        val yData: ByteArray = if (rowStride == width) {
            ByteArray(yBuffer.remaining()).also { yBuffer.get(it) }
        } else {
            ByteArray(width * height).also { dst ->
                val rowBuf = ByteArray(rowStride)
                for (row in 0 until height) {
                    yBuffer.get(rowBuf)
                    rowBuf.copyInto(dst, row * width, 0, width)
                }
            }
        }

        val output = ProcessorJni.processFrame(yData, width, height, rotation)

        // C++ 内で rotation 適用済み → 90°/270° の場合は width と height が入れ替わる
        val (fw, fh) = if (rotation == 90 || rotation == 270) height to width else width to height
        onFrameResult(output, fw, fh)

    } catch (e: Exception) {
        Log.e(TAG, "Frame analysis failed", e)
    } finally {
        imageProxy.close()
    }
}
