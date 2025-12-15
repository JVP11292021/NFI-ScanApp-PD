package com.composables

import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.PathFillType
import androidx.compose.ui.graphics.SolidColor
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.graphics.vector.path
import androidx.compose.ui.unit.dp

val EyeOpen: ImageVector
    get() {
        if (_EyeOpen != null) return _EyeOpen!!
        
        _EyeOpen = ImageVector.Builder(
            name = "EyeOpen",
            defaultWidth = 15.dp,
            defaultHeight = 15.dp,
            viewportWidth = 15f,
            viewportHeight = 15f
        ).apply {
            path(
                fill = SolidColor(Color.Black),
                pathFillType = PathFillType.EvenOdd
            ) {
                moveTo(7.5f, 11f)
                curveTo(4.80285f, 11f, 2.52952f, 9.62184f, 1.09622f, 7.50001f)
                curveTo(2.52952f, 5.37816f, 4.80285f, 4f, 7.5f, 4f)
                curveTo(10.1971f, 4f, 12.4705f, 5.37816f, 13.9038f, 7.50001f)
                curveTo(12.4705f, 9.62183f, 10.1971f, 11f, 7.5f, 11f)
                close()
                moveTo(7.5f, 3f)
                curveTo(4.30786f, 3f, 1.65639f, 4.70638f, 0.0760002f, 7.23501f)
                curveTo(-0.0253338f, 7.39715f, -0.0253334f, 7.60288f, 0.0760014f, 7.76501f)
                curveTo(1.65639f, 10.2936f, 4.30786f, 12f, 7.5f, 12f)
                curveTo(10.6921f, 12f, 13.3436f, 10.2936f, 14.924f, 7.76501f)
                curveTo(15.0253f, 7.60288f, 15.0253f, 7.39715f, 14.924f, 7.23501f)
                curveTo(13.3436f, 4.70638f, 10.6921f, 3f, 7.5f, 3f)
                close()
                moveTo(7.5f, 9.5f)
                curveTo(8.60457f, 9.5f, 9.5f, 8.60457f, 9.5f, 7.5f)
                curveTo(9.5f, 6.39543f, 8.60457f, 5.5f, 7.5f, 5.5f)
                curveTo(6.39543f, 5.5f, 5.5f, 6.39543f, 5.5f, 7.5f)
                curveTo(5.5f, 8.60457f, 6.39543f, 9.5f, 7.5f, 9.5f)
                close()
            }
        }.build()
        
        return _EyeOpen!!
    }

private var _EyeOpen: ImageVector? = null

