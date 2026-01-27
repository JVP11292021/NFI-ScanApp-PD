package com.example.ipmedth_nfi.navigation

import androidx.compose.material3.DrawerValue
import androidx.compose.material3.ModalNavigationDrawer
import androidx.compose.material3.rememberDrawerState
import androidx.compose.runtime.*
import androidx.navigation.compose.currentBackStackEntryAsState
import androidx.navigation.compose.rememberNavController
import com.example.ipmedth_nfi.pages.start.StartScreen
import com.example.ipmedth_nfi.ui.components.navbar.AppDrawer
import com.example.ipmedth_nfi.viewmodel.AppViewModel
import com.example.ipmedth_nfi.viewmodel.SessionViewModel
import kotlinx.coroutines.launch

@Composable
fun AppEntryPoint(
    sessionViewModel: SessionViewModel,
    appViewModel: AppViewModel
) {
    val hasActiveOnderzoek by sessionViewModel
        .hasActiveOnderzoek
        .collectAsState(initial = false)

    // 1️⃣ No active project → start screen

    if (!hasActiveOnderzoek) {
        StartScreen(
            appViewModel = appViewModel,
            onProjectStarted = { onderzoek ->
                sessionViewModel.startOnderzoek(onderzoek)
            }
        )
        return
    }

    // 2️⃣ Project active → normal app
    val navController = rememberNavController()
    val drawerState = rememberDrawerState(DrawerValue.Closed)
    val scope = rememberCoroutineScope()

    val currentRoute =
        navController.currentBackStackEntryAsState().value?.destination?.route

//    val gesturesEnabled = currentRoute == MainRoute.APP.route
    val gesturesEnabled = true

    ModalNavigationDrawer(
        drawerState = drawerState,
        gesturesEnabled = gesturesEnabled,
        drawerContent = {
            AppDrawer(
                onMainRouteClick = { route ->
                    scope.launch { drawerState.close() }
                    navController.navigate(route)
                },
                onExportClick = {
                    scope.launch { drawerState.close() }
                    navController.navigate("export")
                },
                onSaveAndClose = {
                    scope.launch { drawerState.close() }
                    sessionViewModel.closeOnderzoek()
                }
            )
        }
    ) {
        AppNavGraph(
            navController = navController,
            drawerState = drawerState,
            viewModel = sessionViewModel
        )
    }
}
