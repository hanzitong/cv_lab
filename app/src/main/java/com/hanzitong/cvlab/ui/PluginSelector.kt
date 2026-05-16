package com.hanzitong.cvlab.ui

import androidx.compose.material3.Tab
import androidx.compose.material3.TabRow
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.semantics.contentDescription
import androidx.compose.ui.semantics.semantics
import com.hanzitong.cvlab.viewmodel.PluginInfo

@Composable
fun PluginSelector(
    plugins: List<PluginInfo>,
    selected: String,
    onSelect: (String) -> Unit,
    modifier: Modifier = Modifier
) {
    if (plugins.isEmpty()) return
    val selectedIndex = plugins.indexOfFirst { it.name == selected }.coerceAtLeast(0)
    TabRow(selectedTabIndex = selectedIndex, modifier = modifier) {
        plugins.forEach { plugin ->
            Tab(
                selected = selected == plugin.name,
                onClick  = { onSelect(plugin.name) },
                text     = { Text(plugin.name) },
                modifier = Modifier.semantics {
                    contentDescription = "${plugin.name} plugin"
                }
            )
        }
    }
}
