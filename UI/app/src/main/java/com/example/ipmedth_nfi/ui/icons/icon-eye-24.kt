package com.composables

import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.PathFillType
import androidx.compose.ui.graphics.SolidColor
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.graphics.vector.path
import androidx.compose.ui.unit.dp

// Scaled from 15f viewport to 24f viewport (Scale Factor = 1.6)

val EyeOpen24: ImageVector
    get() {
        if (_EyeOpen24 != null) return _EyeOpen24!!

        _EyeOpen24 = ImageVector.Builder(
            name = "EyeOpen24",
            defaultWidth = 24.dp, // Scaled from 15.dp
            defaultHeight = 24.dp, // Scaled from 15.dp
            viewportWidth = 24f, // New viewport
            viewportHeight = 24f // New viewport
        ).apply {
            path(
                fill = SolidColor(Color.Black),
                pathFillType = PathFillType.EvenOdd
            ) {
                // All coordinates (7.5, 11, 4.80285, etc.) are multiplied by 1.6

                // Outer eye shape / Iris (center 7.5 -> 12.0)
                moveTo(12.0f, 17.6f) // 7.5 * 1.6 = 12.0 | 11 * 1.6 = 17.6
                curveTo(7.68456f, 17.6f, 4.04723f, 15.3949f, 1.75395f, 12.000016f) // 4.80285 * 1.6 = 7.68456 | 2.52952 * 1.6 = 4.04723 | 1.09622 * 1.6 = 1.75395 | 9.62184 * 1.6 = 15.3949 | 7.50001 * 1.6 = 12.000016
                curveTo(4.04723f, 8.595055f, 7.68456f, 6.4f, 12.0f, 6.4f) // 5.37816 * 1.6 = 8.595055 | 4 * 1.6 = 6.4
                curveTo(16.3154f, 6.4f, 19.9528f, 8.595055f, 22.2461f, 12.000016f) // 10.1971 * 1.6 = 16.3154 | 12.4705 * 1.6 = 19.9528 | 13.9038 * 1.6 = 22.2461
                curveTo(19.9528f, 15.3949f, 16.3154f, 17.6f, 12.0f, 17.6f)
                close()

                // Eye boundary (center 7.5 -> 12.0)
                moveTo(12.0f, 4.8f) // 3 * 1.6 = 4.8
                curveTo(6.89258f, 4.8f, 2.65022f, 7.53021f, 0.1216003f, 11.576016f) // 4.30786 * 1.6 = 6.89258 | 1.65639 * 1.6 = 2.65022 | 0.0760002 * 1.6 = 0.1216003 | 4.70638 * 1.6 = 7.53021 | 7.23501 * 1.6 = 11.576016
                curveTo(-0.04053408f, 11.83544f, -0.04053344f, 12.164608f, 0.12160224f, 12.424016f) // -0.0253338 * 1.6 = -0.04053408 | 7.39715 * 1.6 = 11.83544 | 7.60288 * 1.6 = 12.164608
                curveTo(2.65022f, 16.469824f, 6.89258f, 19.2f, 12.0f, 19.2f) // 10.2936 * 1.6 = 16.469824 | 12 * 1.6 = 19.2
                curveTo(17.1074f, 19.2f, 21.3498f, 16.469824f, 23.8784f, 12.424016f) // 14.924 * 1.6 = 23.8784
                curveTo(24.0405f, 12.164608f, 24.0405f, 11.83544f, 23.8784f, 11.576016f)
                curveTo(21.3498f, 7.53021f, 17.1074f, 4.8f, 12.0f, 4.8f)
                close()

                // Pupil (center 7.5 -> 12.0)
                moveTo(12.0f, 15.2f) // 9.5 * 1.6 = 15.2
                curveTo(13.7673f, 15.2f, 15.2f, 13.7673f, 15.2f, 12.0f) // 8.60457 * 1.6 = 13.7673
                curveTo(15.2f, 10.2327f, 13.7673f, 8.8f, 12.0f, 8.8f) // 5.5 * 1.6 = 8.8
                curveTo(10.2327f, 8.8f, 8.8f, 10.2327f, 8.8f, 12.0f)
                curveTo(8.8f, 13.7673f, 10.2327f, 15.2f, 12.0f, 15.2f)
                close()
            }
        }.build()

        return _EyeOpen24!!
    }

private var _EyeOpen24: ImageVector? = null