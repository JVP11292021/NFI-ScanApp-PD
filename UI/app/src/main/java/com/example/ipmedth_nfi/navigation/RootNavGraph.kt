package com.example.ipmedth_nfi.navigation

import androidx.compose.runtime.Composable
import androidx.navigation.NavHostController
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import com.example.ipmedth_nfi.pages.start.StartScreen
import com.example.ipmedth_nfi.viewmodel.AppViewModel
import com.example.ipmedth_nfi.viewmodel.SessionViewModel
import androidx.compose.material3.DrawerState

object RootRoute {
    const val START = "start"
    const val APP = "app"
}

@Composable
fun RootNavGraph(
    navController: NavHostController,
    drawerState: DrawerState,
    appViewModel: AppViewModel,
    sessionViewModel: SessionViewModel
) {
    NavHost(
        navController = navController,
        startDestination = RootRoute.START
    ) {

        composable(RootRoute.START) {
            StartScreen(
                appViewModel = appViewModel,
                onProjectStarted = {
                    navController.navigate(RootRoute.APP) {
                        popUpTo(RootRoute.START) { inclusive = true }
                    }
                }
            )
        }

        composable(RootRoute.APP) {
            AppNavGraph(
                navController = navController,
                drawerState = drawerState,
                viewModel = sessionViewModel
            )
        }
    }
}