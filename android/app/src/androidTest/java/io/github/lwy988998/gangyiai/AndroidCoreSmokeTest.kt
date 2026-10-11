package io.github.lwy988998.gangyiai

import androidx.test.platform.app.InstrumentationRegistry
import org.json.JSONObject
import org.junit.Assert.*
import org.junit.Test
import java.net.HttpURLConnection
import java.net.URL

/** 使用隔离模拟器和本机模拟 AI，不读取电脑档案或调用收费模型。 */
class AndroidCoreSmokeTest {
    private fun api(path: String, body: JSONObject? = null, authorized: Boolean = true, origin: String? = null): Pair<Int, String> {
        val connection = URL(NativeEngine.origin + path).openConnection() as HttpURLConnection
        connection.connectTimeout = 2000; connection.readTimeout = 15000
        if (authorized) connection.setRequestProperty("Cookie", "gangyi_mobile_session=" + NativeEngine.token)
        if (origin != null) connection.setRequestProperty("Origin", origin)
        if (body != null) {
            connection.requestMethod = "POST"; connection.doOutput = true
            connection.setRequestProperty("Content-Type", "application/json")
            connection.outputStream.use { it.write(body.toString().toByteArray(Charsets.UTF_8)) }
        }
        try {
            val status = connection.responseCode
            return status to (if (status < 400) connection.inputStream else connection.errorStream).bufferedReader().use { it.readText() }
        } finally { connection.disconnect() }
    }
    private fun request(path: String, body: JSONObject? = null): JSONObject {
        val (status, text) = api(path, body)
        assertTrue("接口返回 $status：$path", status in 200..299)
        return JSONObject(text)
    }
    private fun waitTask(id: String): JSONObject {
        val deadline = System.nanoTime() + 90000000000L
        while (System.nanoTime() < deadline) {
            val task = request("/api/learning-agent?taskId=$id")
            if (task.optString("status") !in listOf("pending", "running", "idle")) {
                assertTrue(task.toString(), task.optString("status") in listOf("ready", "waiting_student"))
                return task
            }
            Thread.sleep(100)
        }
        throw AssertionError("模拟 AI 任务未完成")
    }
    @Test fun independentCoreAndEncryptedSettings() {
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        val context = instrumentation.targetContext
        val port = InstrumentationRegistry.getArguments().getString("mockPort") ?: throw AssertionError("请由 scripts/android_acceptance.py 启动隔离验收")
        val settings = JSONObject().put("baseUrl", "http://127.0.0.1:$port/v1").put("apiKey", "fictional-fixture-key")
            .put("model", "fixture-v560").put("provider", "自定义").put("searchKey", "").put("skip", true)
        AppSettings(context).save(settings)
        assertEquals("fictional-fixture-key", AppSettings(context).load().getString("apiKey"))
        assertFalse(context.getSharedPreferences("ai-settings", 0).all.toString().contains("fictional-fixture-key"))
        NativeEngine.start(context, 77L)
        try {
            val deadline = System.nanoTime() + 30000000000L
            while (System.nanoTime() < deadline && runCatching { api("/health").first }.getOrDefault(0) != 200) Thread.sleep(100)
            assertEquals(200, api("/health").first)
            assertEquals(403, api("/api/learning-agent", authorized = false).first)
            assertEquals(403, api("/api/learning-agent", origin = "https://example.com").first)
            assertTrue(api("/").second.contains("/android.js"))
            Thread.sleep(1500)
            assertFalse("安卓启动不能自动提交推荐", request("/api/learning-agent").has("id"))
            val configuration = NativeEngine.prepare(context)
            settings.keys().forEach { key -> configuration.put(key, settings.get(key)) }
            val catalog = JSONObject(NativeEngine.models(configuration.toString()))
            assertTrue(catalog.toString(), catalog.optBoolean("ok"))
            assertEquals("fixture-v560", catalog.getJSONArray("models").getString(0))
            NativeEngine.pause()
            assertEquals("ai_paused", JSONObject(NativeEngine.models(configuration.toString())).getString("type"))
            request("/api/ai-activity/control", JSONObject().put("command", "resume"))
            val connectionResult = JSONObject(NativeEngine.connectionTest(configuration.toString()))
            assertTrue(connectionResult.toString(), connectionResult.optBoolean("ok"))
            val plan = request("/api/generate-plan", JSONObject().put("goal", "虚构学生的化学学习").put("mode", "lite").put("requestId", "android-plan"))
            val course = waitTask(plan.getString("id")).getJSONObject("course").getString("id")
            val prepare = request("/api/learning-agent/events", JSONObject().put("type", "prepare_next").put("courseId", course).put("requestId", "android-prepare").put("navigate", false))
            val lesson = waitTask(prepare.getString("id")).getJSONObject("lesson").getString("id")
            request("/api/learning-agent/control", JSONObject().put("command", "enter_lesson").put("lessonId", lesson))
            val saved = request("/api/learning-agent/lesson?lessonId=$lesson")
            val introduction = saved.optString("initialTeachingTaskId")
            if (introduction.isNotBlank()) waitTask(introduction)
            assertFalse(saved.toString().contains("PRIVATE-EXPECTED"))
            for (page in listOf("learn", "practice", "summary")) assertEquals(200, api("/$page?lessonId=$lesson").first)
            context.getSharedPreferences("app-state", 0).edit().putString("lastRoute", "/learn?lessonId=$lesson").putBoolean("notificationAsked", true).commit()
            context.getSharedPreferences("android-test", 0).edit().putString("lessonId", lesson).putString("courseId", course).commit()
        } finally { NativeEngine.stopOwned(77L) }
    }
}
