package com.example.ipmedth_nfi.navigation

import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.DrawerState
import androidx.compose.material3.Scaffold
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.navigation.NavHostController
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.currentBackStackEntryAsState
import com.example.ipmedth_nfi.bridge.NativeAndroidEngine
import com.example.ipmedth_nfi.data.export.ProjectStorageManager
import com.example.ipmedth_nfi.pages.app.AppPage
import com.example.ipmedth_nfi.pages.model.AnnotationPage
import com.example.ipmedth_nfi.pages.model.ModelPage
import com.example.ipmedth_nfi.pages.scan.ScanPage
import com.example.ipmedth_nfi.ui.components.navbar.AppTopbar
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
    val navBackStackEntry by navController.currentBackStackEntryAsState()
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
                ScanPage(
                    viewModel = viewModel,
                    modifier = Modifier.fillMaxSize()
                )
            }

            composable(MainRoute.MODEL.route) {
                val context = LocalContext.current
                val activeOnderzoek = viewModel.activeOnderzoek.collectAsState().value
                val projectPath = activeOnderzoek?.let { onderzoek ->
                    ProjectStorageManager(context).getProjectDir(onderzoek)
                }

                ModelPage(
                    viewModel = viewModel,
                    modifier = Modifier.fillMaxSize(),
                    engine = NativeAndroidEngine(),
                    projectDirPath = projectPath?.absolutePath
                )
            }

            composable(MainRoute.ANNOTATION.route) {
                val context = LocalContext.current
                val activeOnderzoek = viewModel.activeOnderzoek.collectAsState().value
                val projectPath = activeOnderzoek?.let { onderzoek ->
                    ProjectStorageManager(context).getProjectDir(onderzoek)
                }

                AnnotationPage(
                    viewModel = viewModel,
                    modifier = Modifier.fillMaxSize(),
                    engine = NativeAndroidEngine(),
                    projectDirPath = projectPath?.absolutePath,
                    actionId = viewModel.selectedActionId ?: "01"
                )
            }

            composable(MainRoute.APP.route) {
                AppPage(
                    viewModel = viewModel,
                    onMenuClick = { scope.launch { drawerState.open() } },
                    onPageChanged = { page ->
                        viewModel.setAssessmentPage(page)
                    },
                    onNavigateToAnnotation = { actionId ->
                        viewModel.selectedActionId = actionId
                        navController.navigate(MainRoute.ANNOTATION.route)
                    }
                )
            }
        }
    }
}
