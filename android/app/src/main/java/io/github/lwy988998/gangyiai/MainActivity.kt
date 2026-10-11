package io.github.lwy988998.gangyiai

import android.Manifest
import android.app.Activity
import android.content.Intent
import android.content.pm.PackageManager
import android.graphics.Color
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.view.View
import android.view.WindowInsets
import android.webkit.*
import android.widget.*
import org.json.JSONObject

class MainActivity : Activity() {
    private val handler = Handler(Looper.getMainLooper())
    private lateinit var root: FrameLayout
    private var browser: WebView? = null
    private var loadedOrigin = ""
    private var fileCallback: ValueCallback<Array<Uri>>? = null
    private var choosingFile = false
    private var resumed = false
    private var showAi = false
    @Volatile private var keyboardVisible = false
    private val uiStore by lazy { getSharedPreferences("ui-state", MODE_PRIVATE) }
    private val stateStore by lazy { getSharedPreferences("app-state", MODE_PRIVATE) }
    private fun dp(value: Int) = (value * resources.displayMetrics.density).toInt()
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        root = FrameLayout(this).apply { setBackgroundColor(Color.parseColor("#09161d")) }
        applySafeInsets(root) { keyboardVisible = it }
        setContentView(root)
        showAi = intent.getBooleanExtra("showAi", false)
    }
    override fun onResume() {
        super.onResume(); resumed = true
        browser?.onResume()
        val canOpen = runCatching { AppSettings(this).canOpen() }.getOrDefault(false)
        startService(Intent(this, CoreService::class.java).setAction("foreground"))
        if (!canOpen) { startActivity(Intent(this, SettingsActivity::class.java)); return }
        if (Build.VERSION.SDK_INT >= 33 && checkSelfPermission(Manifest.permission.POST_NOTIFICATIONS) != PackageManager.PERMISSION_GRANTED && !stateStore.getBoolean("notificationAsked", false)) {
            stateStore.edit().putBoolean("notificationAsked", true).apply()
            requestPermissions(arrayOf(Manifest.permission.POST_NOTIFICATIONS), 71)
        }
        awaitCore()
    }
    private fun awaitCore() {
        if (!resumed || isFinishing) return
        if (NativeEngine.failure.isNotEmpty()) { displayStatus(NativeEngine.failure, true); return }
        if (NativeEngine.ready) { if (loadedOrigin != NativeEngine.origin) openBrowser(); return }
        displayStatus("正在恢复本机课堂…", false)
        handler.postDelayed({ awaitCore() }, 250)
    }
    private fun displayStatus(text: String, retry: Boolean) {
        if (browser != null && !retry) return
        if (retry) { browser?.destroy(); browser = null; loadedOrigin = "" }
        root.removeAllViews()
        val column = LinearLayout(this).apply { orientation = LinearLayout.VERTICAL; gravity = android.view.Gravity.CENTER; setPadding(dp(24), dp(24), dp(24), dp(24)) }
        column.addView(TextView(this).apply { this.text = text; textSize = 18f; setTextColor(Color.parseColor("#a4e9de")) })
        if (retry) column.addView(Button(this).apply { this.text = "打开设置"; setOnClickListener { startActivity(Intent(this@MainActivity, SettingsActivity::class.java)) } })
        root.addView(column, FrameLayout.LayoutParams(-1, -1))
    }
    private fun trusted(uri: Uri): Boolean = uri.scheme == "http" && uri.host == "127.0.0.1" && uri.port == Uri.parse(NativeEngine.origin).port
    private fun openBrowser() {
        browser?.destroy(); root.removeAllViews()
        val view = WebView(this)
        browser = view; loadedOrigin = NativeEngine.origin
        view.setBackgroundColor(Color.parseColor("#09161d"))
        view.settings.apply { javaScriptEnabled = true; domStorageEnabled = true; allowFileAccess = false; allowContentAccess = true; mixedContentMode = WebSettings.MIXED_CONTENT_NEVER_ALLOW; mediaPlaybackRequiresUserGesture = true; setSupportMultipleWindows(false) }
        WebView.setWebContentsDebuggingEnabled(BuildConfig.DEBUG)
        view.addJavascriptInterface(object {
            @JavascriptInterface fun snapshot(): String = JSONObject(uiStore.all.filterValues { it is String }).toString()
            @JavascriptInterface fun keyboardActive(): Boolean = keyboardVisible
            @JavascriptInterface fun tablet(): Boolean = resources.configuration.smallestScreenWidthDp >= 600
            @JavascriptInterface fun put(key: String, value: String) {
                if (key.length <= 256 && value.length <= 1048576) check(uiStore.edit().putString(key, value).commit())
            }
            @JavascriptInterface fun remove(key: String) { uiStore.edit().remove(key).commit() }
            @JavascriptInterface fun settings() { runOnUiThread { startActivity(Intent(this@MainActivity, SettingsActivity::class.java).addFlags(Intent.FLAG_ACTIVITY_NO_USER_ACTION)) } }
        }, "GangyiAndroid")
        view.webViewClient = object : WebViewClient() {
            override fun shouldOverrideUrlLoading(browser: WebView, request: WebResourceRequest): Boolean {
                if (trusted(request.url)) return false
                if (request.isForMainFrame && request.url.scheme in listOf("https", "http")) runCatching { startActivity(Intent(Intent.ACTION_VIEW, request.url)) }
                return true
            }
            override fun shouldInterceptRequest(browser: WebView, request: WebResourceRequest): WebResourceResponse? {
                if (request.url.scheme in listOf("http", "https") && !trusted(request.url)) return WebResourceResponse("text/plain", "utf-8", 403, "Blocked", emptyMap(), "外部资料请在浏览器打开".byteInputStream())
                return null
            }
            override fun onPageFinished(browser: WebView, url: String) {
                if (!trusted(Uri.parse(url))) return
                stateStore.edit().putString("lastRoute", url.removePrefix(NativeEngine.origin)).apply()
                if (showAi) { browser.evaluateJavascript("document.getElementById('learning-navigation').open=true", null); showAi = false }
            }
        }
        view.webChromeClient = object : WebChromeClient() {
            override fun onShowFileChooser(browser: WebView, callback: ValueCallback<Array<Uri>>, params: FileChooserParams): Boolean {
                fileCallback?.onReceiveValue(null); fileCallback = callback; choosingFile = true
                return try { startActivityForResult(params.createIntent(), 72); true } catch (_: Exception) { choosingFile = false; fileCallback = null; callback.onReceiveValue(null); false }
            }
        }
        root.addView(view, FrameLayout.LayoutParams(-1, -1))
        val cookie = "gangyi_mobile_session=${NativeEngine.token}; Path=/; HttpOnly; SameSite=Strict"
        CookieManager.getInstance().setCookie(NativeEngine.origin, cookie) {
            runOnUiThread {
                if (!isFinishing) {
                    val saved = Uri.parse(stateStore.getString("lastRoute", "/"))
                    val route = if (saved.path in listOf("/learn", "/practice", "/summary") && saved.getQueryParameter("androidRestore") == null)
                        saved.buildUpon().appendQueryParameter("androidRestore", "1").build().toString() else saved.toString()
                    view.loadUrl(NativeEngine.origin + route)
                }
            }
        }
    }
    override fun onUserLeaveHint() {
        super.onUserLeaveHint()
        if (!choosingFile && NativeEngine.ready) GangyiApplication.enterBackground(this)
    }
    override fun onStop() {
        resumed = false; browser?.onPause(); handler.removeCallbacksAndMessages(null)
        super.onStop()
    }
    override fun onNewIntent(intent: Intent) {
        super.onNewIntent(intent); setIntent(intent); showAi = intent.getBooleanExtra("showAi", false)
        if (showAi && browser != null) { browser!!.evaluateJavascript("document.getElementById('learning-navigation').open=true", null); showAi = false }
    }
    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (requestCode == 72) { fileCallback?.onReceiveValue(WebChromeClient.FileChooserParams.parseResult(resultCode, data)); fileCallback = null; choosingFile = false }
    }
    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == 71 && grantResults.firstOrNull() != PackageManager.PERMISSION_GRANTED)
            Toast.makeText(this, "未允许通知：AI 只能在前台运行，离开 App 后会暂停。", Toast.LENGTH_LONG).show()
    }
    override fun onBackPressed() {
        if (Build.VERSION.SDK_INT >= 30 && root.rootWindowInsets?.isVisible(WindowInsets.Type.ime()) == true) {
            getSystemService(android.view.inputmethod.InputMethodManager::class.java).hideSoftInputFromWindow(root.windowToken, 0); return
        }
        val view = browser
        if (view == null) { super.onBackPressed(); return }
        view.evaluateJavascript("(()=>{const d=document.querySelector('.course-directory-mobile[open]');if(d){d.open=false;return 'closed'}return 'back'})()") { result ->
            if (result != "\"closed\"") { if (view.canGoBack()) view.goBack() else moveTaskToBack(true) }
        }
    }
    override fun onDestroy() { handler.removeCallbacksAndMessages(null); fileCallback?.onReceiveValue(null); browser?.destroy(); super.onDestroy() }
}
