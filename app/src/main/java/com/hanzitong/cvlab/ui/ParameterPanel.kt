package com.hanzitong.cvlab.ui

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
import com.hanzitong.cvlab.viewmodel.ParameterDef

@Composable
fun ParameterPanel(
    defs: List<ParameterDef>,
    parameters: Map<String, Float>,
    onParameterChanged: (String, Float) -> Unit,
    modifier: Modifier = Modifier
) {
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
    val displayValue = if (def.isInteger) value.toInt().toString() else "%.4f".format(value)

    val steps: Int = if (def.isInteger) {
        val range = (def.max - def.min).toInt()
        if (range in 1..200) range - 1 else 0
    } else {
        0
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
