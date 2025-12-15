package com.example.ipmedth_nfi.navigation

import androidx.compose.material3.DrawerValue
import androidx.compose.material3.ModalNavigationDrawer
import androidx.compose.material3.rememberDrawerState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.rememberCoroutineScope
import androidx.navigation.compose.currentBackStackEntryAsState
import androidx.navigation.compose.rememberNavController
import com.example.ipmedth_nfi.ui.components.AppDrawer
import com.example.ipmedth_nfi.viewmodel.SessionViewModel
import kotlinx.coroutines.launch

@Composable
fun  AppEntryPoint(viewModel: SessionViewModel) {
    val navController = rememberNavController()
    val drawerState = rememberDrawerState(DrawerValue.Closed)
    val scope = rememberCoroutineScope()
    val currentRoute = navController.currentBackStackEntryAsState().value?.destination?.route
    val gesturesEnabled = currentRoute == MainRoute.APP.route

    ModalNavigationDrawer(
        drawerState = drawerState,
        gesturesEnabled = gesturesEnabled,
        drawerContent = {
            AppDrawer { route ->
                scope.launch {
                    drawerState.close()
                }
                navController.navigate(route) {
                    launchSingleTop = true
                    popUpTo(MainRoute.SCAN.route) { saveState = true }
                    restoreState = true
                }
            }
        }
    ) {
        AppNavGraph(
            navController = navController,
            drawerState = drawerState,
            viewModel = viewModel
        )
    }
}