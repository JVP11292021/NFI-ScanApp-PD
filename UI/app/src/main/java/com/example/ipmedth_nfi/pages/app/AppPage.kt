package com.example.ipmedth_nfi.pages.app

import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.pager.HorizontalPager
import androidx.compose.foundation.pager.rememberPagerState
import androidx.compose.material3.Scaffold
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.example.ipmedth_nfi.ui.components.navbar.BottomNavBar
import com.example.ipmedth_nfi.ui.navigation.AssessmentPage
import com.example.ipmedth_nfi.viewmodel.SessionViewModel

@Composable
fun AppPage(
    viewModel: SessionViewModel,
    onMenuClick: () -> Unit,
    onPageChanged: (AssessmentPage) -> Unit,
    onNavigateToAnnotation: (String) -> Unit = {}
) {
    val pagerState = rememberPagerState(pageCount = { AssessmentPage.all.size })

    LaunchedEffect(pagerState.currentPage) {
        val page = AssessmentPage.all[pagerState.currentPage]
        onPageChanged(page)
    }

    Scaffold(
        modifier = Modifier.fillMaxSize(),
        bottomBar = { BottomNavBar(pagerState) }
    ) { innerPadding ->
        HorizontalPager(
            state = pagerState,
            modifier = Modifier
                .fillMaxSize()
                .padding(innerPadding)
                .padding(horizontal = 16.dp)
        ) { pageIndex ->
            AssessmentContent(
                page = AssessmentPage.all[pageIndex],
                viewModel = viewModel,
                onNavigateToAnnotation = onNavigateToAnnotation
            )
        }
    }
}
