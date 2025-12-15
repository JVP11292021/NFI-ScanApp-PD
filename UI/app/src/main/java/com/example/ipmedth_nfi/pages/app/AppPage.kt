package com.example.ipmedth_nfi.pages.app

import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.pager.HorizontalPager
import androidx.compose.foundation.pager.rememberPagerState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.ui.Modifier
import com.example.ipmedth_nfi.pages.app.tabs.*
import com.example.ipmedth_nfi.ui.components.BottomNavBar
import com.example.ipmedth_nfi.ui.navigation.AssessmentPage
import com.example.ipmedth_nfi.viewmodel.SessionViewModel

@Composable
fun AppPage(
    viewModel: SessionViewModel,
    onMenuClick: () -> Unit,
    onPageChanged: (AssessmentPage) -> Unit
) {
    val pagerState = rememberPagerState(pageCount = { AssessmentPage.all.size })

    LaunchedEffect(pagerState.currentPage) {
        val page = AssessmentPage.all[pagerState.currentPage]
        onPageChanged(page)
    }

    Column() {
        HorizontalPager(
            state = pagerState,
            modifier = Modifier.weight(1f)
        ) { page ->
            when (AssessmentPage.all[page]) {
                AssessmentPage.Info -> InfoTab(viewModel)
                AssessmentPage.Observations -> ObservationsTab(viewModel)
                AssessmentPage.Themes -> ThemesTab(viewModel)
                AssessmentPage.Attention -> AttentionTab(viewModel)
                AssessmentPage.Plan -> PlanTab(viewModel)
                AssessmentPage.Finish -> FinishTab(viewModel)
            }
        }
        BottomNavBar(pagerState)
    }
}