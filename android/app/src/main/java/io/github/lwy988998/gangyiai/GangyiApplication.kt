package io.github.lwy988998.gangyiai

import android.Manifest
import android.app.Activity
import android.app.Application
import android.app.NotificationManager
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.os.Looper

class GangyiApplication : Application(), Application.ActivityLifecycleCallbacks {
    private var started = 0
    private val handler = Handler(Looper.getMainLooper())
    private val background = Runnable { if (started == 0) enterBackground(this) }
    override fun onCreate() { super.onCreate(); registerActivityLifecycleCallbacks(this) }
    override fun onActivityStarted(activity: Activity) {
        started++; handler.removeCallbacks(background)
        if (NativeEngine.thread?.isAlive == true || NativeEngine.starting)
            startService(Intent(this, CoreService::class.java).setAction("foreground"))
    }
    override fun onActivityStopped(activity: Activity) {
        started = (started - 1).coerceAtLeast(0)
        if (started == 0) handler.postDelayed(background, 200)
    }
    override fun onActivityCreated(activity: Activity, savedInstanceState: Bundle?) {}
    override fun onActivityResumed(activity: Activity) {}
    override fun onActivityPaused(activity: Activity) {}
    override fun onActivitySaveInstanceState(activity: Activity, outState: Bundle) {}
    override fun onActivityDestroyed(activity: Activity) {}
    companion object {
        fun canNotify(context: Context): Boolean =
            (Build.VERSION.SDK_INT < 33 || context.checkSelfPermission(Manifest.permission.POST_NOTIFICATIONS) == PackageManager.PERMISSION_GRANTED) &&
            context.getSystemService(NotificationManager::class.java).areNotificationsEnabled()
        fun enterBackground(context: Context) {
            if (NativeEngine.thread?.isAlive != true && !NativeEngine.starting) return
            val intent = Intent(context, CoreService::class.java).setAction("background")
            runCatching {
                if (canNotify(context)) context.startForegroundService(intent) else context.startService(intent)
            }.onFailure {
                Thread({ NativeEngine.pause(); runCatching { NativeEngine.foreground(false) } }, "GangyiBackgroundPause").start()
            }
        }
    }
}
