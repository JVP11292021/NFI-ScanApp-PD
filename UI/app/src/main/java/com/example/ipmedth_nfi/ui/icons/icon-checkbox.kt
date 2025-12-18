package com.composables

import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.PathFillType
import androidx.compose.ui.graphics.SolidColor
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.graphics.vector.path
import androidx.compose.ui.unit.dp

val Checkbox: ImageVector
    get() {
        if (_Checkbox != null) return _Checkbox!!

        _Checkbox = ImageVector.Builder(
            name = "Checkbox",
            defaultWidth = 15.dp,
            defaultHeight = 15.dp,
            viewportWidth = 15f,
            viewportHeight = 15f
        ).apply {
            path(
                fill = SolidColor(Color.Black),
                pathFillType = PathFillType.EvenOdd
            ) {
                moveTo(3f, 3f)
                horizontalLineTo(12f)
                verticalLineTo(12f)
                horizontalLineTo(3f)
                lineTo(3f, 3f)
                close()
                moveTo(2f, 3f)
                curveTo(2f, 2.44771f, 2.44772f, 2f, 3f, 2f)
                horizontalLineTo(12f)
                curveTo(12.5523f, 2f, 13f, 2.44772f, 13f, 3f)
                verticalLineTo(12f)
                curveTo(13f, 12.5523f, 12.5523f, 13f, 12f, 13f)
                horizontalLineTo(3f)
                curveTo(2.44771f, 13f, 2f, 12.5523f, 2f, 12f)
                verticalLineTo(3f)
                close()
                moveTo(10.3498f, 5.51105f)
                curveTo(10.506f, 5.28337f, 10.4481f, 4.97212f, 10.2204f, 4.81587f)
                curveTo(9.99275f, 4.65961f, 9.6815f, 4.71751f, 9.52525f, 4.94519f)
                lineTo(6.64048f, 9.14857f)
                lineTo(5.19733f, 7.40889f)
                curveTo(5.02102f, 7.19635f, 4.7058f, 7.16699f, 4.49327f, 7.34329f)
                curveTo(4.28073f, 7.5196f, 4.25137f, 7.83482f, 4.42767f, 8.04735f)
                lineTo(6.2934f, 10.2964f)
                curveTo(6.39348f, 10.4171f, 6.54437f, 10.4838f, 6.70097f, 10.4767f)
                curveTo(6.85757f, 10.4695f, 7.00177f, 10.3894f, 7.09047f, 10.2601f)
                lineTo(10.3498f, 5.51105f)
                close()
            }
        }.build()

        return _Checkbox!!
    }

private var _Checkbox: ImageVector? = null

