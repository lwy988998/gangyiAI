#include <jni.h>
#include <cstdlib>
#include <mutex>
#include "server_runtime.hpp"
#include "ai_activity.hpp"
#include "ai_client.hpp"
#include "ai_model_catalog.hpp"
#include "mobile_policy.hpp"

namespace {
using Json = nlohmann::json;
std::mutex contextMutex;
std::string databasePath;
Json input(JNIEnv* env, jstring value) {
    const char* raw = env->GetStringUTFChars(value, nullptr);
    if (!raw) throw std::runtime_error("配置不可用");
    const std::string text(raw); env->ReleaseStringUTFChars(value, raw);
    return Json::parse(text);
}
jstring output(JNIEnv* env, const Json& value) {
    // JNI 的 NewStringUTF 使用修改版 UTF-8；通过 Java 的 UTF-8 解码保留中文与补充字符。
    const auto text = value.dump();
    auto bytes = env->NewByteArray(static_cast<jsize>(text.size()));
    env->SetByteArrayRegion(bytes, 0, static_cast<jsize>(text.size()), reinterpret_cast<const jbyte*>(text.data()));
    auto stringClass = env->FindClass("java/lang/String");
    auto constructor = env->GetMethodID(stringClass, "<init>", "([BLjava/lang/String;)V");
    auto encoding = env->NewStringUTF("UTF-8");
    auto result = static_cast<jstring>(env->NewObject(stringClass, constructor, bytes, encoding));
    env->DeleteLocalRef(bytes); env->DeleteLocalRef(encoding); env->DeleteLocalRef(stringClass);
    return result;
}
void paths(const Json& config) {
    const auto ca = config.value("caBundle", "");
    if (!ca.empty()) setenv("GANGYI_CA_BUNDLE", ca.c_str(), 1);
    const auto database = config.value("database", "");
    if (!database.empty()) {
        std::lock_guard<std::mutex> lock(contextMutex); databasePath = database;
        gangyi::AIActivity::configure((std::filesystem::path(database).parent_path() / "ai-activity.json").string());
    }
}
std::string database() { std::lock_guard<std::mutex> lock(contextMutex); return databasePath; }
gangyi::AIClient client(const Json& config) {
    return gangyi::AIClient({config.value("baseUrl", ""), config.value("apiKey", ""), config.value("model", ""), 20000, 1});
}
Json failed(const std::exception& error) {
    // 服务商响应可能回显请求，原始错误不跨过原生边界。
    const auto* ai = dynamic_cast<const gangyi::AIClientError*>(&error);
    const auto message = ai && ai->errorType == "ai_paused" ? "AI 已暂停，请返回课堂恢复后再试。" :
        (ai && ai->httpStatus ? "连接失败，服务商返回 HTTP " + std::to_string(ai->httpStatus) + "，请检查配置。" : "连接失败，请检查网络、接口地址和模型配置。");
    return {{"ok", false}, {"type", ai ? ai->errorType : "connection_error"}, {"message", message}};
}
}

extern "C" JNIEXPORT jint JNICALL Java_io_github_lwy988998_gangyiai_NativeEngine_run(JNIEnv* env, jobject, jstring value) {
    try {
        const auto settings = input(env, value); paths(settings);
        gangyi::Config config;
        config.port = settings.at("port").get<int>(); config.database_path = settings.at("database").get<std::string>();
        config.ai_base_url = settings.value("baseUrl", "https://api.deepseek.com/v1");
        config.ai_api_key = settings.value("apiKey", ""); config.ai_model = settings.value("model", "");
        setenv("AI_BASE_URL", config.ai_base_url.c_str(), 1); setenv("AI_API_KEY", config.ai_api_key.c_str(), 1);
        setenv("AI_MODEL", config.ai_model.c_str(), 1); setenv("BOCHA_API_KEY", settings.value("searchKey", "").c_str(), 1);
        setenv("RESOURCE_SEARCH_CACHE_DIR", settings.at("cacheRoot").get<std::string>().c_str(), 1);
        gangyi::ServerOptions options; options.mobile = true;
        options.publicRoot = settings.at("publicRoot").get<std::string>(); options.sessionToken = settings.at("token").get<std::string>();
        return gangyi::runServer(config, options);
    } catch (...) { return 1; }
}
extern "C" JNIEXPORT void JNICALL Java_io_github_lwy988998_gangyiai_NativeEngine_stop(JNIEnv*, jobject) {
    try { gangyi::stopServer(); } catch (...) { /* 禁止 C++ 异常穿过 JNI 边界。 */ }
}
extern "C" JNIEXPORT void JNICALL Java_io_github_lwy988998_gangyiai_NativeEngine_pause(JNIEnv*, jobject) {
    try { gangyi::AIActivity::setPaused(true); } catch (...) { gangyi::AIActivity::shutdown(); }
}
extern "C" JNIEXPORT void JNICALL Java_io_github_lwy988998_gangyiai_NativeEngine_foreground(JNIEnv*, jobject, jboolean visible) {
    try { const auto path = database(); if (!path.empty()) gangyi::runtimeForeground(path, visible); }
    catch (...) { gangyi::AIActivity::shutdown(); }
}
extern "C" JNIEXPORT jstring JNICALL Java_io_github_lwy988998_gangyiai_NativeEngine_status(JNIEnv* env, jobject) {
    try {
        auto view = gangyi::runtimeActivities(database());
        auto tasks = Json::array();
        for (const auto& row : view.value("tasks", Json::array()))
            if (gangyi::MobilePolicy::requestAllowed(row.value("id", ""))) tasks.push_back(row);
        view["tasks"] = tasks; return output(env, view);
    } catch (...) { return output(env, {{"tasks", Json::array()}}); }
}
extern "C" JNIEXPORT jstring JNICALL Java_io_github_lwy988998_gangyiai_NativeEngine_models(JNIEnv* env, jobject, jstring value) {
    std::string activity;
    try {
        const auto config = input(env, value); paths(config);
        gangyi::AIActivityInfo info; info.source = "安卓设置"; info.purpose = "获取可用模型列表";
        activity = gangyi::AIActivity::begin(info, config.value("model", ""));
        const auto epoch = gangyi::AIActivity::pauseEpoch();
        if (!gangyi::AIActivity::requestStarted(activity)) throw gangyi::AIClientError("ai_paused", "AI paused");
        const auto models = gangyi::loadModelCatalog({config.value("baseUrl", ""), config.value("apiKey", ""), config.value("model", "")},
            [epoch] { return gangyi::AIActivity::stopping() || gangyi::AIActivity::paused() || gangyi::AIActivity::pauseEpoch() != epoch; });
        gangyi::AIActivity::update(activity, "ready");
        return output(env, {{"ok", true}, {"models", models}, {"message", "已获取模型，请选择。"}});
    } catch (const std::exception& error) {
        if (!activity.empty()) { try { gangyi::AIActivity::update(activity, gangyi::AIActivity::paused() ? "cancelled" : "error"); } catch (...) {} }
        if (gangyi::AIActivity::paused()) return output(env, failed(gangyi::AIClientError("ai_paused", "AI paused")));
        return output(env, failed(error));
    }
}
extern "C" JNIEXPORT jstring JNICALL Java_io_github_lwy988998_gangyiai_NativeEngine_connectionTest(JNIEnv* env, jobject, jstring value) {
    try {
        const auto config = input(env, value); paths(config);
        gangyi::ChatOptions options; options.messages = {{"user", "回复 OK"}};
        options.maxTokens = 128; options.maxAttempts = 1; options.timeoutMs = 20000;
        options.activity.source = "安卓设置"; options.activity.purpose = "测试 AI 连接";
        client(config).chat(options);
        return output(env, {{"ok", true}, {"message", "AI 连接成功（已发出一次模型请求）。"}});
    } catch (const std::exception& error) { return output(env, failed(error)); }
}
