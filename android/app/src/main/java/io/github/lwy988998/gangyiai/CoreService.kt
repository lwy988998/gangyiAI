package io.github.lwy988998.gangyiai

import android.Manifest
import android.app.*
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Build
import android.os.IBinder
import org.json.JSONObject
import java.net.HttpURLConnection
import java.net.URL
import java.util.concurrent.Executors
import java.util.concurrent.TimeUnit

class CoreService : Service() {
    private val lease = System.nanoTime()
    private val monitor = Executors.newSingleThreadScheduledExecutor()
    @Volatile private var visible = true
    private var promoted = false
    private var lastNotification = ""
    private val manager get() = getSystemService(NotificationManager::class.java)
    override fun onBind(intent: Intent?): IBinder? = null
    override fun onCreate() {
        super.onCreate()
        manager.createNotificationChannel(NotificationChannel("ai-work", "AI 任务状态", NotificationManager.IMPORTANCE_LOW))
        monitor.execute { NativeEngine.start(this, lease) }
        monitor.scheduleWithFixedDelay({ inspect() }, 0, 1, TimeUnit.SECONDS)
    }
    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        when (intent?.action) {
            "pause" -> monitor.execute { NativeEngine.pause(); inspect() }
            "background" -> {
                visible = false
                if (canNotify()) {
                    promote("正在完成你已启动的 AI 任务")
                    monitor.execute { runCatching { NativeEngine.foreground(false) }; inspect() }
                } else monitor.execute { NativeEngine.pause(); runCatching { NativeEngine.foreground(false) }; inspect() }
            }
            else -> {
                visible = true
                monitor.execute { NativeEngine.start(this, lease); runCatching { NativeEngine.foreground(true) }; inspect() }
            }
        }
        return START_NOT_STICKY
    }
    private fun canNotify() = GangyiApplication.canNotify(this)
    private fun notification(text: String): Notification {
        val open = PendingIntent.getActivity(this, 1, Intent(this, MainActivity::class.java).putExtra("showAi", true)
            .addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP or Intent.FLAG_ACTIVITY_SINGLE_TOP), PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE)
        val stop = PendingIntent.getService(this, 2, Intent(this, CoreService::class.java).setAction("pause"), PendingIntent.FLAG_UPDATE_CURRENT or PendingIntent.FLAG_IMMUTABLE)
        return Notification.Builder(this, "ai-work").setSmallIcon(R.drawable.ic_ai).setContentTitle("钢一定制AI")
            .setContentText(text).setStyle(Notification.BigTextStyle().bigText(text)).setContentIntent(open)
            .setVisibility(Notification.VISIBILITY_PRIVATE).setOngoing(true).setOnlyAlertOnce(true)
            .addAction(Notification.Action.Builder(null, "停止全部 AI", stop).build()).build()
    }
    private fun promote(text: String) {
        if (!canNotify()) return
        runCatching {
            if (!promoted) { startForeground(42, notification(text)); promoted = true }
            else if (lastNotification != text) manager.notify(42, notification(text))
            lastNotification = text
        }.onFailure { monitor.execute { NativeEngine.pause() } }
    }
    private fun healthReady(): Boolean {
        if (NativeEngine.origin.isEmpty()) return false
        return runCatching {
            val connection = URL(NativeEngine.origin + "/health").openConnection() as HttpURLConnection
            connection.connectTimeout = 500; connection.readTimeout = 500
            connection.setRequestProperty("Cookie", "gangyi_mobile_session=" + NativeEngine.token)
            try { connection.responseCode == 200 } finally { connection.disconnect() }
        }.getOrDefault(false)
    }
    private fun inspect() {
        try {
            if (visible && NativeEngine.thread?.isAlive != true && !NativeEngine.starting && NativeEngine.failure.isEmpty()) NativeEngine.start(this, lease)
            if (!NativeEngine.ready) NativeEngine.ready = healthReady()
            if (!NativeEngine.ready) return
            val view = JSONObject(NativeEngine.status())
            val tasks = view.optJSONArray("tasks")
            val active = (0 until (tasks?.length() ?: 0)).map { tasks!!.getJSONObject(it) }
                .filter { it.optString("status") in listOf("pending", "running", "requesting", "receiving", "queued") }
            if (!visible && active.isNotEmpty()) {
                val current = active.last()
                val step = current.optJSONObject("activity")?.optString("title").orEmpty()
                promote(listOf(current.optString("source"), current.optString("purpose"), step).filter { it.isNotBlank() }.joinToString(" · "))
            } else if (promoted) {
                stopForeground(STOP_FOREGROUND_REMOVE); promoted = false; lastNotification = ""
            }
            if (!visible && active.isEmpty()) stopSelf()
        } catch (_: Exception) { /* 服务初始化或数据库恢复期间继续检查，不发起模型请求。 */ }
    }
    override fun onTimeout(startId: Int, fgsType: Int) {
        monitor.execute { NativeEngine.pause(); NativeEngine.stopOwned(lease); stopSelf() }
    }
    override fun onDestroy() {
        monitor.shutdown()
        Thread({ runCatching { NativeEngine.stopOwned(lease) } }, "GangyiCoreShutdown").start()
        super.onDestroy()
    }
}
