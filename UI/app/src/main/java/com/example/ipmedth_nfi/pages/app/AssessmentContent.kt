package com.example.ipmedth_nfi.pages.app

import android.os.Build
import androidx.annotation.RequiresApi
import androidx.compose.runtime.Composable
import com.example.ipmedth_nfi.pages.app.tabs.*
import com.example.ipmedth_nfi.ui.navigation.AssessmentPage
import com.example.ipmedth_nfi.viewmodel.SessionViewModel

@RequiresApi(Build.VERSION_CODES.O)
@Composable
fun AssessmentContent(
    page: AssessmentPage,
    viewModel: SessionViewModel
) {
    when (page) {
        AssessmentPage.Info -> InfoTab(viewModel)
        AssessmentPage.Observations -> ObservationsTab(viewModel)
        AssessmentPage.Themes -> ThemesTab(viewModel)
        AssessmentPage.Attention -> AttentionTab(viewModel)
        AssessmentPage.Plan -> PlanTab(viewModel)
        AssessmentPage.Finish -> FinishTab(viewModel)
    }
}
