package com.example.ipmedth_nfi.pages.app

import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.WindowInsets
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.pager.HorizontalPager
import androidx.compose.foundation.pager.rememberPagerState
import androidx.compose.material3.Scaffold
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.navigation.NavHostController
import com.example.ipmedth_nfi.ui.components.navbar.BottomNavBar
import com.example.ipmedth_nfi.ui.navigation.AssessmentPage
import com.example.ipmedth_nfi.viewmodel.SessionViewModel

@Composable
fun AppPage(
    navController: NavHostController,
    viewModel: SessionViewModel
) {
    val pagerState = rememberPagerState(
        pageCount = { AssessmentPage.pages.size }
    )
    Scaffold(
        bottomBar = {
            BottomNavBar(
                pagerState = pagerState,
                pages = AssessmentPage.pages
            )
        },
        contentWindowInsets = WindowInsets(0,0,0,0)
    ) { padding ->

        HorizontalPager(
            modifier = Modifier.padding(padding),
            state = pagerState,
        ) { pageIndex ->
            Box(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(horizontal = 16.dp, vertical = 12.dp)
            ){
                AssessmentContent(
                    page = AssessmentPage.pages[pageIndex],
                    viewModel = viewModel
                )
            }
        }
    }
}
