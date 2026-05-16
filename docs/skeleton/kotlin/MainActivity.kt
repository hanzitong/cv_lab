package com.hanzitong.cvfeaturelab

import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.activity.result.contract.ActivityResultContracts
import androidx.activity.viewModels
import androidx.core.content.ContextCompat
import com.hanzitong.cvfeaturelab.jni.FeatureProcessorJni
import com.hanzitong.cvfeaturelab.ui.MainScreen
import com.hanzitong.cvfeaturelab.ui.theme.CvFeatureLabTheme
import com.hanzitong.cvfeaturelab.viewmodel.MainViewModel

class MainActivity : ComponentActivity() {

    private val viewModel: MainViewModel by viewModels()

    private val requestCameraPermission = registerForActivityResult(
        ActivityResultContracts.RequestPermission()
    ) { granted ->
        if (granted) showUi() else finish()
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        // Edge-to-edge: コンテンツがステータスバー・ナビゲーションバーの下まで広がる
        enableEdgeToEdge()
        super.onCreate(savedInstanceState)

        FeatureProcessorJni.init()

        if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA)
                == PackageManager.PERMISSION_GRANTED) {
            showUi()
        } else {
            requestCameraPermission.launch(Manifest.permission.CAMERA)
        }
    }

    private fun showUi() {
        setContent {
            CvFeatureLabTheme {
                MainScreen(viewModel)
            }
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        FeatureProcessorJni.destroy()
    }
}
