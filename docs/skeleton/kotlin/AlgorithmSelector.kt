package com.hanzitong.cvlab.ui

import androidx.compose.material3.Tab
import androidx.compose.material3.TabRow
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import com.hanzitong.cvlab.viewmodel.Algorithm

@Composable
fun AlgorithmSelector(
    selected: Algorithm,
    onSelect: (Algorithm) -> Unit,
    modifier: Modifier = Modifier
) {
    val algorithms = Algorithm.entries
    TabRow(
        selectedTabIndex = algorithms.indexOf(selected),
        modifier = modifier
    ) {
        algorithms.forEach { algo ->
            Tab(
                selected = selected == algo,
                onClick = { onSelect(algo) },
                text = { Text(algo.displayName) }
            )
        }
    }
}
