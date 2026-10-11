package io.github.lwy988998.gangyiai

import android.os.Build
import android.view.View
import android.view.WindowInsets

internal fun applySafeInsets(view: View, keyboard: (Boolean) -> Unit = {}) {
    view.setOnApplyWindowInsetsListener { target, insets ->
        if (Build.VERSION.SDK_INT >= 30) {
            val bars = insets.getInsets(WindowInsets.Type.systemBars() or WindowInsets.Type.displayCutout())
            val ime = insets.getInsets(WindowInsets.Type.ime())
            keyboard(insets.isVisible(WindowInsets.Type.ime()))
            target.setPadding(bars.left, bars.top, bars.right, maxOf(bars.bottom, ime.bottom))
        } else {
            @Suppress("DEPRECATION")
            target.setPadding(insets.systemWindowInsetLeft, insets.systemWindowInsetTop, insets.systemWindowInsetRight, insets.systemWindowInsetBottom)
            @Suppress("DEPRECATION")
            keyboard(insets.systemWindowInsetBottom > 120 * target.resources.displayMetrics.density)
        }
        insets
    }
    view.requestApplyInsets()
}
