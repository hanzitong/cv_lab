package com.hanzitong.cvfeaturelab.ui

import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Slider
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.semantics.contentDescription
import androidx.compose.ui.semantics.semantics
import androidx.compose.ui.unit.dp
import com.hanzitong.cvfeaturelab.viewmodel.Algorithm
import com.hanzitong.cvfeaturelab.viewmodel.ParameterDef
import com.hanzitong.cvfeaturelab.viewmodel.algorithmParameters

/**
 * アルゴリズムのパラメータースライダー一覧パネル。
 * アルゴリズムが変わると対応するパラメーターリストが自動的に切り替わる。
 */
@Composable
fun ParameterPanel(
    algorithm: Algorithm,
    parameters: Map<String, Float>,
    onParameterChanged: (String, Float) -> Unit,
    modifier: Modifier = Modifier
) {
    val defs = algorithmParameters[algorithm] ?: emptyList()

    LazyColumn(
        modifier            = modifier.heightIn(max = 260.dp),
        contentPadding      = PaddingValues(horizontal = 16.dp, vertical = 6.dp),
        verticalArrangement = Arrangement.spacedBy(0.dp)
    ) {
        items(defs, key = { it.key }) { def ->
            ParameterSliderRow(
                def           = def,
                value         = parameters[def.key] ?: def.default,
                onValueChange = { onParameterChanged(def.key, it) }
            )
        }
    }
}

@Composable
private fun ParameterSliderRow(
    def: ParameterDef,
    value: Float,
    onValueChange: (Float) -> Unit
) {
    val displayValue = if (def.isInteger) {
        value.toInt().toString()
    } else {
        "%.4f".format(value)
    }

    // 整数パラメーターは離散ステップ。ただし範囲が広い場合は連続スライダーのまま
    // （steps は中間点の数。範囲が大きいと Material3 の描画が重くなるため 200 上限）
    val steps: Int = if (def.isInteger) {
        val range = (def.max - def.min).toInt()
        if (range in 1..200) range - 1 else 0
    } else {
        0  // 浮動小数は連続スライダー
    }

    Column(modifier = Modifier.padding(vertical = 2.dp)) {
        Row(
            modifier              = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween
        ) {
            Text(
                text  = def.displayName,
                style = MaterialTheme.typography.bodySmall,
                color = MaterialTheme.colorScheme.onSurface
            )
            Text(
                text  = displayValue,
                style = MaterialTheme.typography.bodySmall,
                color = MaterialTheme.colorScheme.primary
            )
        }

        Slider(
            value         = value,
            onValueChange = { raw ->
                // 整数パラメーターは整数値に丸める
                val snapped = if (def.isInteger) raw.toInt().toFloat() else raw
                onValueChange(snapped)
            },
            valueRange = def.min..def.max,
            steps      = steps,
            modifier   = Modifier
                .fillMaxWidth()
                .semantics { contentDescription = "${def.displayName} slider" }
        )
    }
}
