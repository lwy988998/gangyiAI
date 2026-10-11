package io.github.lwy988998.gangyiai

import android.content.Context
import android.security.keystore.KeyGenParameterSpec
import android.security.keystore.KeyProperties
import android.util.Base64
import org.json.JSONObject
import java.security.KeyStore
import javax.crypto.Cipher
import javax.crypto.KeyGenerator
import javax.crypto.SecretKey
import javax.crypto.spec.GCMParameterSpec

internal class AppSettings(private val context: Context) {
    private val preferences = context.getSharedPreferences("ai-settings", Context.MODE_PRIVATE)
    private val alias = "gangyi-ai-settings-v1"
    private fun key(): SecretKey {
        val store = KeyStore.getInstance("AndroidKeyStore").apply { load(null) }
        (store.getKey(alias, null) as? SecretKey)?.let { return it }
        return KeyGenerator.getInstance(KeyProperties.KEY_ALGORITHM_AES, "AndroidKeyStore").apply {
            init(KeyGenParameterSpec.Builder(alias, KeyProperties.PURPOSE_ENCRYPT or KeyProperties.PURPOSE_DECRYPT)
                .setKeySize(256)
                .setBlockModes(KeyProperties.BLOCK_MODE_GCM).setEncryptionPaddings(KeyProperties.ENCRYPTION_PADDING_NONE)
                .setUserAuthenticationRequired(false).build())
        }.generateKey()
    }
    fun load(): JSONObject {
        val value = preferences.getString("sealed", null) ?: return JSONObject()
        val envelope = JSONObject(value)
        val cipher = Cipher.getInstance("AES/GCM/NoPadding")
        cipher.init(Cipher.DECRYPT_MODE, key(), GCMParameterSpec(128, Base64.decode(envelope.getString("iv"), Base64.NO_WRAP)))
        val plaintext = cipher.doFinal(Base64.decode(envelope.getString("data"), Base64.NO_WRAP))
        return try { JSONObject(String(plaintext, Charsets.UTF_8)) } finally { plaintext.fill(0) }
    }
    fun save(value: JSONObject) {
        val cipher = Cipher.getInstance("AES/GCM/NoPadding").apply { init(Cipher.ENCRYPT_MODE, key()) }
        val plaintext = value.toString().toByteArray(Charsets.UTF_8)
        val encrypted = try { cipher.doFinal(plaintext) } finally { plaintext.fill(0) }
        val envelope = JSONObject().put("iv", Base64.encodeToString(cipher.iv, Base64.NO_WRAP))
            .put("data", Base64.encodeToString(encrypted, Base64.NO_WRAP))
        check(preferences.edit().putString("sealed", envelope.toString()).commit()) { "配置保存失败" }
    }
    fun canOpen(): Boolean {
        val settings = load()
        return settings.optBoolean("skip") || (settings.optString("apiKey").isNotBlank() && settings.optString("model").isNotBlank())
    }
}
