package com.example.ipmedth_nfi.ui.components.navbar

import androidx.compose.foundation.pager.PagerState
import androidx.compose.material3.Icon
import androidx.compose.material3.NavigationBar
import androidx.compose.material3.NavigationBarItem
import androidx.compose.material3.NavigationBarItemDefaults
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.graphics.Color
import com.example.ipmedth_nfi.ui.navigation.AssessmentPage
import com.example.ipmedth_nfi.ui.theme.SecondaryOrange
import com.example.ipmedth_nfi.ui.theme.White
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
                },
                colors = NavigationBarItemDefaults.colors(
                    selectedIconColor = White,
                    unselectedIconColor = Color.Gray,
                    selectedTextColor = SecondaryOrange,
                    unselectedTextColor = Color.Gray,
                    indicatorColor = SecondaryOrange
                )
            )
        }
    }
}
