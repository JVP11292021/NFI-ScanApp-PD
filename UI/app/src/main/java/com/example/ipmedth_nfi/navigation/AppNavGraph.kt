package com.example.ipmedth_nfi.navigation

import androidx.compose.foundation.layout.padding
import androidx.compose.material3.DrawerState
import androidx.compose.material3.Scaffold
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.navigation.NavHostController
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.currentBackStackEntryAsState
import com.example.ipmedth_nfi.pages.app.AppPage
import com.example.ipmedth_nfi.pages.model.ModelPage
import com.example.ipmedth_nfi.pages.scan.ScanPage
import com.example.ipmedth_nfi.ui.components.AppTopbar
import com.example.ipmedth_nfi.ui.navigation.AssessmentAwareTopBar
import com.example.ipmedth_nfi.viewmodel.SessionViewModel
import kotlinx.coroutines.launch

@Composable
fun AppNavGraph(
    navController: NavHostController,
    drawerState: DrawerState,
    viewModel: SessionViewModel
) {
    val scope = rememberCoroutineScope()
    val currentRoute =
        navController.currentBackStackEntryAsState().value?.destination?.route
    val assessmentPage = viewModel.currentAssessmentPage

    Scaffold(
        topBar = {
            when (currentRoute) {
                MainRoute.APP.route -> {
                    AssessmentAwareTopBar(
                        viewModel = viewModel,
                        currentPage = assessmentPage,
                        onMenuClick = { scope.launch { drawerState.open() } }
                    )
                }

                else -> {
                    AppTopbar(
                        title = MainRoute.entries
                            .find { it.route == currentRoute }
                            ?.title ?: "",
                        onMenuClick = { scope.launch { drawerState.open() } }
                    )
                }
            }
        }
    ) { paddingValues ->

        NavHost(
            navController = navController,
            startDestination = MainRoute.SCAN.route,
            modifier = Modifier.padding(paddingValues)
        ) {
            composable(MainRoute.SCAN.route) {
                ScanPage(viewModel)
            }

            composable(MainRoute.MODEL.route) {
                ModelPage(viewModel)
            }

            composable(MainRoute.APP.route) {
                AppPage(
                    viewModel = viewModel,
                    onMenuClick = { scope.launch { drawerState.open() } },
                    onPageChanged = { page ->
                        viewModel.setAssessmentPage(page)
                    }
                )
            }
        }
    }
}
