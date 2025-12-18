package com.example.ipmedth_nfi.ui.components

import androidx.compose.foundation.pager.PagerState
import androidx.compose.material3.Icon
import androidx.compose.material3.NavigationBar
import androidx.compose.material3.NavigationBarItem
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.rememberCoroutineScope
import com.example.ipmedth_nfi.ui.navigation.AssessmentPage
import kotlinx.coroutines.launch

@Composable
fun BottomNavBar(pagerState: PagerState) {
    val scope = rememberCoroutineScope()

    NavigationBar() {
        AssessmentPage.all.forEachIndexed { index, page ->
            NavigationBarItem(
                selected = pagerState.currentPage == index,
                onClick = {
                    scope.launch {
                        pagerState.animateScrollToPage(index)
                    }
                },
                icon = {
                    Icon(page.icon, contentDescription = page.title)
                },
                label = {
                    Text(page.title, maxLines = 2)
                }
            )
        }
    }
}