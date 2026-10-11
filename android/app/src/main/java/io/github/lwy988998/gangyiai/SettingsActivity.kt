package io.github.lwy988998.gangyiai

import android.app.Activity
import android.content.Intent
import android.graphics.Color
import android.graphics.Typeface
import android.graphics.drawable.GradientDrawable
import android.os.Bundle
import android.text.InputType
import android.view.View
import android.widget.*
import org.json.JSONObject
import java.net.URI
import java.util.concurrent.Executors

class SettingsActivity : Activity() {
    private val worker = Executors.newSingleThreadExecutor()
    private lateinit var baseUrl: EditText
    private lateinit var apiKey: EditText
    private lateinit var model: EditText
    private lateinit var searchKey: EditText
    private lateinit var state: TextView
    private lateinit var container: LinearLayout
    private val buttons = mutableListOf<Button>()
    private var providerName = "DeepSeek"
    private fun dp(value: Int) = (value * resources.displayMetrics.density).toInt()
    private fun label(text: String, size: Float = 15f) = TextView(this).apply {
        this.text = text; textSize = size; setTextColor(Color.parseColor("#e5eeee")); setPadding(0, dp(12), 0, dp(6))
    }
    private fun input(name: String, value: String, secret: Boolean = false): EditText {
        container.addView(label(name))
        return EditText(this).apply {
            setText(value); setTextColor(Color.parseColor("#e5eeee")); textSize = 16f
            setSingleLine(true); setPadding(dp(12), dp(12), dp(12), dp(12))
            inputType = if (secret) InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_VARIATION_PASSWORD else InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_VARIATION_URI
            importantForAutofill = View.IMPORTANT_FOR_AUTOFILL_NO
            background = GradientDrawable().apply { setColor(Color.parseColor("#11232b")); cornerRadius = dp(9).toFloat(); setStroke(dp(1), Color.parseColor("#3d6059")) }
            container.addView(this, LinearLayout.LayoutParams(-1, dp(54)))
        }
    }
    private fun button(text: String, action: () -> Unit) {
        val value = Button(this).apply { this.text = text; setTextColor(Color.parseColor("#a4e9de")); isAllCaps = false; setOnClickListener { action() } }
        buttons.add(value); container.addView(value, LinearLayout.LayoutParams(-1, dp(52)).apply { topMargin = dp(10) })
    }
    private fun configuration(skip: Boolean = false) = JSONObject().put("provider", providerName)
        .put("baseUrl", baseUrl.text.toString().trim()).put("apiKey", apiKey.text.toString().trim())
        .put("model", model.text.toString().trim()).put("searchKey", searchKey.text.toString().trim()).put("skip", skip)
    private fun valid(config: JSONObject, requireModel: Boolean = true): Boolean {
        val uri = runCatching { URI(config.getString("baseUrl")) }.getOrNull()
        return uri != null && uri.scheme in listOf("http", "https") && !uri.host.isNullOrBlank() && uri.userInfo == null &&
            config.getString("apiKey").isNotBlank() && (!requireModel || config.getString("model").isNotBlank())
    }
    private fun perform(message: String, action: (JSONObject) -> String, requireModel: Boolean = true, result: (JSONObject) -> Unit = {}) {
        val config = configuration()
        if (!valid(config, requireModel)) { state.text = "请填写有效的 API 地址、API Key 和模型名称。"; return }
        buttons.forEach { it.isEnabled = false }; state.text = message
        worker.execute {
            val value = runCatching {
                val paths = NativeEngine.prepare(this)
                paths.keys().forEach { key -> config.put(key, paths.get(key)) }
                JSONObject(action(config))
            }.getOrElse { JSONObject().put("ok", false).put("message", "连接失败，请检查接口和网络。") }
            runOnUiThread { if (!isFinishing) { buttons.forEach { it.isEnabled = true }; state.text = value.optString("message"); result(value) } }
        }
    }
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val saved = runCatching { AppSettings(this).load() }.getOrElse { JSONObject() }
        container = LinearLayout(this).apply { orientation = LinearLayout.VERTICAL; setPadding(dp(22), dp(20), dp(22), dp(36)); setBackgroundColor(Color.parseColor("#09161d")) }
        setContentView(ScrollView(this).apply { addView(container); applySafeInsets(this) })
        container.addView(label("钢一定制AI · 安卓设置", 24f).apply { setTypeface(typeface, Typeface.BOLD) })
        container.addView(label("课程与记录只保存在这台设备；AI 调用使用你的配置。", 13f))
        val provider = Spinner(this).apply { adapter = ArrayAdapter(this@SettingsActivity, android.R.layout.simple_spinner_dropdown_item, arrayOf("DeepSeek", "OpenAI", "自定义")) }
        providerName = saved.optString("provider", "DeepSeek")
        provider.setSelection(listOf("DeepSeek", "OpenAI", "自定义").indexOf(providerName).coerceAtLeast(0))
        container.addView(label("AI 服务商")); container.addView(provider)
        baseUrl = input("AI API 地址", saved.optString("baseUrl", "https://api.deepseek.com/v1"))
        apiKey = input("API Key", saved.optString("apiKey"), true)
        model = input("模型名称", saved.optString("model", "deepseek-v4-flash"))
        searchKey = input("博查搜索 API Key（可选）", saved.optString("searchKey"), true)
        provider.onItemSelectedListener = object : AdapterView.OnItemSelectedListener {
            override fun onNothingSelected(parent: AdapterView<*>?) {}
            override fun onItemSelected(parent: AdapterView<*>?, view: View?, position: Int, id: Long) {
                val selected = listOf("DeepSeek", "OpenAI", "自定义")[position]
                if (providerName != selected) {
                    providerName = selected
                    if (selected == "DeepSeek") { baseUrl.setText("https://api.deepseek.com/v1"); model.setText("deepseek-v4-flash") }
                    if (selected == "OpenAI") { baseUrl.setText("https://api.openai.com/v1"); model.setText("") }
                    apiKey.setText("")
                }
            }
        }
        state = label("密钥由 Android Keystore 加密保存。", 13f); container.addView(state)
        button("获取模型") {
            perform("正在获取模型…", { NativeEngine.models(it.toString()) }, false) { response ->
                if (response.optBoolean("ok")) {
                    val list = response.optJSONArray("models")
                    val values = Array(list?.length() ?: 0) { list!!.getString(it) }
                    if (values.isNotEmpty()) android.app.AlertDialog.Builder(this).setTitle("选择模型").setItems(values) { _, index -> model.setText(values[index]) }.show()
                }
            }
        }
        button("测试连接") { perform("正在测试连接，这会发出一次模型请求…", { NativeEngine.connectionTest(it.toString()) }) }
        button("保存并启动") { save(false) }
        button("暂不配置，直接使用") { save(true) }
        button("返回课堂") { finish() }
        container.addView(label("安卓试用版 1.0.0 · 仅完成已启动的后台任务，重新打开后由你点击恢复。", 12f))
    }
    private fun save(skip: Boolean) {
        val config = configuration(skip)
        if (!skip && !valid(config)) { state.text = "请先填写有效配置。"; return }
        buttons.forEach { it.isEnabled = false }; state.text = "正在保存配置并恢复课堂…"
        worker.execute {
            val outcome = runCatching { AppSettings(this).save(config); NativeEngine.stopAndWait() }
            runOnUiThread {
                if (outcome.isFailure) { state.text = "配置已保存，服务尚未就绪，请稍后重试。"; buttons.forEach { it.isEnabled = true } }
                else { startActivity(Intent(this, MainActivity::class.java).addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP or Intent.FLAG_ACTIVITY_SINGLE_TOP)); finish() }
            }
        }
    }
    override fun onDestroy() { worker.shutdown(); super.onDestroy() }
}
