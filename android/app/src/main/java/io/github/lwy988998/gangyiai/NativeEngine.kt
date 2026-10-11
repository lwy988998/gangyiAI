package io.github.lwy988998.gangyiai

import android.content.Context
import android.content.res.AssetManager
import org.json.JSONObject
import java.io.File
import java.net.ServerSocket
import java.security.SecureRandom

internal object NativeEngine {
    init { System.loadLibrary("gangyi_android") }
    external fun run(config: String): Int
    external fun stop()
    external fun status(): String
    external fun foreground(visible: Boolean)
    external fun pause()
    external fun models(config: String): String
    external fun connectionTest(config: String): String
    @Volatile var origin = ""
    @Volatile var token = ""
    @Volatile var ready = false
    @Volatile var starting = false
    @Volatile var failure = ""
    @Volatile var thread: Thread? = null
    private var owner = 0L

    private fun unpack(assets: AssetManager, path: String, destination: File) {
        val children = assets.list(path).orEmpty()
        if (children.isEmpty()) {
            destination.parentFile?.mkdirs()
            assets.open(path).use { input -> destination.outputStream().use { input.copyTo(it) } }
        } else {
            destination.mkdirs()
            children.forEach { unpack(assets, if (path.isEmpty()) it else "$path/$it", File(destination, it)) }
        }
    }
    @Synchronized fun prepare(context: Context): JSONObject {
        val application = context.applicationContext
        val web = File(application.filesDir, "web")
        val marker = File(web, ".ready")
        val revision = application.packageManager.getPackageInfo(application.packageName, 0).lastUpdateTime.toString()
        // 升级只覆盖随包网页，学习数据与草稿在独立目录中保留。
        if (!marker.exists() || marker.readText() != revision) {
            unpack(application.assets, "", web); marker.writeText(revision)
        }
        File(application.filesDir, "data").mkdirs()
        File(application.cacheDir, "resources").mkdirs()
        return JSONObject().put("database", File(application.filesDir, "data/gangyiAI.db").absolutePath)
            .put("publicRoot", web.absolutePath).put("cacheRoot", File(application.cacheDir, "resources").absolutePath)
            .put("caBundle", File(web, "curl-ca-bundle.crt").absolutePath)
    }
    @Synchronized fun start(context: Context, lease: Long) {
        owner = lease
        if (thread?.isAlive == true || starting) return
        starting = true; ready = false; failure = ""
        val application = context.applicationContext
        val prepared = prepare(application)
        thread = Thread({
            try {
                val configuration = prepared
                val settings = AppSettings(application).load()
                settings.keys().forEach { key -> configuration.put(key, settings.get(key)) }
                val port = ServerSocket(0, 0, java.net.InetAddress.getByName("127.0.0.1")).use { it.localPort }
                token = ByteArray(32).also { SecureRandom().nextBytes(it) }.joinToString("") { "%02x".format(it) }
                origin = "http://127.0.0.1:$port"
                configuration.put("port", port).put("token", token)
                if (run(configuration.toString()) != 0) failure = "本机学习服务未能启动，请在设置中重试。"
            } catch (_: Exception) {
                failure = "本机学习服务启动失败，请检查存储空间或在设置中重新保存配置。"
            } finally { starting = false; ready = false }
        }, "GangyiLearningCore").apply { start() }
    }
    // 新服务接管核心后，旧服务的销毁回调不能停止它。
    @Synchronized fun stopOwned(lease: Long) { if (owner == lease) stopAndWait() }
    @Synchronized fun stopAndWait() {
        stop()
        thread?.join(15000)
        check(thread?.isAlive != true) { "学习服务尚未停止，请稍后重试" }
        thread = null; ready = false; starting = false
    }
}
