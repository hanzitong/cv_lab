package com.hanzitong.cvlab.ui.theme

import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color

// カメラアプリとして視認性の高いダークテーマを採用
// Primary = 緑（キーポイントの描画色と揃える）
private val DarkColors = darkColorScheme(
    primary          = Color(0xFF4CAF50),   // キーポイントと同系色
    onPrimary        = Color(0xFF000000),
    primaryContainer = Color(0xFF1B5E20),
    surface          = Color(0xFF121212),
    onSurface        = Color(0xFFE0E0E0),
    background       = Color(0xFF000000),
    onBackground     = Color(0xFFE0E0E0)
)

@Composable
fun CvLabTheme(content: @Composable () -> Unit) {
    MaterialTheme(
        colorScheme = DarkColors,
        content     = content
    )
}
