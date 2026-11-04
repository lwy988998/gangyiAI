// 【逐行批注】 引入头文件 "independent_chat.hpp"，为后续实现提供类型或接口声明。
#include "independent_chat.hpp"
// 【逐行批注】 引入头文件 "my_ai.hpp"，为后续实现提供类型或接口声明。
#include "my_ai.hpp"

// 【逐行批注】 引入头文件 <algorithm>，为后续实现提供类型或接口声明。
#include <algorithm>
// 【逐行批注】 引入头文件 <chrono>，为后续实现提供类型或接口声明。
#include <chrono>
// 【逐行批注】 引入头文件 <ctime>，为后续实现提供类型或接口声明。
#include <ctime>
// 【逐行批注】 引入头文件 <iomanip>，为后续实现提供类型或接口声明。
#include <iomanip>
// 【逐行批注】 引入头文件 <sstream>，为后续实现提供类型或接口声明。
#include <sstream>
// 【逐行批注】 引入头文件 <stdexcept>，为后续实现提供类型或接口声明。
#include <stdexcept>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 【逐行批注】 声明或进入命名空间：namespace {。
namespace {
// 【逐行批注】 计算右侧结果并写入 using Json，更新当前运行状态。
using Json = AgentJson;

// 【逐行批注】 定义函数 now，执行该函数声明的职责。
std::string now() {
    // 【逐行批注】 计算右侧结果并写入 auto value，更新当前运行状态。
    const auto value = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    // 【逐行批注】 执行当前声明或表达式：std::tm utc{};。
    std::tm utc{};
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 调用 gmtime_s 完成本行所需操作：gmtime_s(&utc, &value);。
    gmtime_s(&utc, &value);
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 调用 gmtime_r 完成本行所需操作：gmtime_r(&value, &utc);。
    gmtime_r(&value, &utc);
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 执行当前声明或表达式：std::ostringstream output;。
    std::ostringstream output;
    // 【逐行批注】 调用 put_time 完成本行所需操作：output << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");。
    output << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    // 【逐行批注】 结束当前函数并返回结果：return output.str();。
    return output.str();
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 requestText，执行该函数声明的职责。
std::string requestText(const Json& body, const char* field, std::size_t limit) {
    // 【逐行批注】 判断条件 (!body.contains(field) || !body[field].is_string()) throw std::i…，成立时执行对应分支。
    if (!body.contains(field) || !body[field].is_string()) throw std::invalid_argument("请求内容不完整");
    // 【逐行批注】 计算右侧结果并写入 auto value，更新当前运行状态。
    const auto value = body[field].get<std::string>();
    // 【逐行批注】 判断条件 (value.empty() || value.size() > limit) throw std::invalid_argum…，成立时执行对应分支。
    if (value.empty() || value.size() > limit) throw std::invalid_argument("请求内容为空或过长");
    // 【逐行批注】 结束当前函数并返回结果：return value;。
    return value;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 lessonForChat，执行该函数声明的职责。
Json lessonForChat(Database& db, const std::string& lessonId) {
    // 【逐行批注】 计算右侧结果并写入 auto row，更新当前运行状态。
    const auto row = db.getClassroomActivity(lessonId);
    // 【逐行批注】 判断条件 (!row || row->kind != "agent-lesson") throw std::invalid_argumen…，成立时执行对应分支。
    if (!row || row->kind != "agent-lesson") throw std::invalid_argument("课时不存在");
    // 【逐行批注】 计算右侧结果并写入 auto lesson，更新当前运行状态。
    const auto lesson = Json::parse(row->payload, nullptr, false);
    // 【逐行批注】 判断条件 (!lesson.is_object() || lesson.value("status", "") == "deleted")，成立时执行对应分支。
    if (!lesson.is_object() || lesson.value("status", "") == "deleted")
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("课时不存在");。
        throw std::invalid_argument("课时不存在");
    // 【逐行批注】 结束当前函数并返回结果：return lesson;。
    return lesson;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 ensureSession，执行该函数声明的职责。
IndependentChatSession ensureSession(Database& db, const std::string& lessonId) {
    // 【逐行批注】 判断条件 (const auto existing = db.independentChatSession(lessonId)) retu…，成立时执行对应分支。
    if (const auto existing = db.independentChatSession(lessonId)) return *existing;
    // 【逐行批注】 执行当前声明或表达式：IndependentChatSession created;。
    IndependentChatSession created;
    // 【逐行批注】 计算右侧结果并写入 created.lessonId，更新当前运行状态。
    created.lessonId = lessonId;
    // 【逐行批注】 计算右侧结果并写入 created.createdAt，更新当前运行状态。
    created.createdAt = now();
    // 【逐行批注】 计算右侧结果并写入 created.updatedAt，更新当前运行状态。
    created.updatedAt = created.createdAt;
    // 【逐行批注】 判断条件 (!db.saveIndependentChatSession(created)) throw std::runtime_err…，成立时执行对应分支。
    if (!db.saveIndependentChatSession(created)) throw std::runtime_error("独立聊天创建失败");
    // 【逐行批注】 结束当前函数并返回结果：return created;。
    return created;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 messageView，执行该函数声明的职责。
Json messageView(const IndependentChatMessage& value) {
    // 【逐行批注】 计算右侧结果并写入 sources，更新当前运行状态。
    auto sources = Json::parse(value.sources, nullptr, false);
    // 【逐行批注】 判断条件 (!sources.is_array()) sources = Json::array();，成立时执行对应分支。
    if (!sources.is_array()) sources = Json::array();
    // 【逐行批注】 结束当前函数并返回结果：return {{"id", value.id}, {"role", value.role}, {"content", value.con…。
    return {{"id", value.id}, {"role", value.role}, {"content", value.content},
        // 【逐行批注】 执行当前声明或表达式：{"status", value.status}, {"sources", sources}, {"model", value.model},。
        {"status", value.status}, {"sources", sources}, {"model", value.model},
        // 【逐行批注】 执行当前声明或表达式：{"error", value.error}, {"createdAt", value.createdAt}, {"updatedAt", value.updatedA…。
        {"error", value.error}, {"createdAt", value.createdAt}, {"updatedAt", value.updatedAt}};
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 activeTask，执行该函数声明的职责。
bool activeTask(const Json& task) {
    // 【逐行批注】 结束当前函数并返回结果：return task.is_object() && (task.value("status", "") == "pending" || …。
    return task.is_object() && (task.value("status", "") == "pending" || task.value("status", "") == "running");
// 【逐行批注】 结束当前代码块。
}
// 【逐行批注】 执行当前声明或表达式：} // namespace。
} // namespace

// 【逐行批注】 定义函数 independentChatView，执行该函数声明的职责。
Json independentChatView(Database& db, const AgentAccess& access, const std::string& lessonId) {
    // 【逐行批注】 调用 lessonForChat 完成本行所需操作：lessonForChat(db, lessonId);。
    lessonForChat(db, lessonId);
    // 【逐行批注】 计算右侧结果并写入 auto session，更新当前运行状态。
    const auto session = ensureSession(db, lessonId);
    // 【逐行批注】 计算右侧结果并写入 Json messages，更新当前运行状态。
    Json messages = Json::array();
    // 【逐行批注】 按循环规则重复处理：for (const auto& item : db.independentChatMessages(lessonId, 200)) message…。
    for (const auto& item : db.independentChatMessages(lessonId, 200)) messages.push_back(messageView(item));
    // 【逐行批注】 计算右侧结果并写入 Json task，更新当前运行状态。
    Json task = nullptr;
    // 【逐行批注】 判断条件 (!session.activeTaskId.empty()) {，成立时执行对应分支。
    if (!session.activeTaskId.empty()) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { task = agentView(db, access, session.activeTaskId, true); } catch (...) {}
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 结束当前函数并返回结果：return {{"lessonId", lessonId}, {"webEnabled", session.webEnabled},。
    return {{"lessonId", lessonId}, {"webEnabled", session.webEnabled},
        // 【逐行批注】 执行当前声明或表达式：{"messages", messages}, {"activeTask", task}};。
        {"messages", messages}, {"activeTask", task}};
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 independentChatSettings，执行该函数声明的职责。
Json independentChatSettings(Database& db, const Json& body) {
    // 【逐行批注】 计算右侧结果并写入 auto lessonId，更新当前运行状态。
    const auto lessonId = requestText(body, "lessonId", 200);
    // 【逐行批注】 调用 lessonForChat 完成本行所需操作：lessonForChat(db, lessonId);。
    lessonForChat(db, lessonId);
    // 【逐行批注】 计算右侧结果并写入 session，更新当前运行状态。
    auto session = ensureSession(db, lessonId);
    // 【逐行批注】 判断条件 (!body.contains("webEnabled") || !body.at("webEnabled").is_boole…，成立时执行对应分支。
    if (!body.contains("webEnabled") || !body.at("webEnabled").is_boolean())
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::invalid_argument("联网开关状态无效");。
        throw std::invalid_argument("联网开关状态无效");
    // 【逐行批注】 计算右侧结果并写入 session.webEnabled，更新当前运行状态。
    session.webEnabled = body.at("webEnabled").get<bool>();
    // 【逐行批注】 计算右侧结果并写入 session.updatedAt，更新当前运行状态。
    session.updatedAt = now();
    // 【逐行批注】 判断条件 (!db.saveIndependentChatSession(session)) throw std::runtime_err…，成立时执行对应分支。
    if (!db.saveIndependentChatSession(session)) throw std::runtime_error("联网开关保存失败");
    // 【逐行批注】 结束当前函数并返回结果：return {{"lessonId", lessonId}, {"webEnabled", session.webEnabled}, {…。
    return {{"lessonId", lessonId}, {"webEnabled", session.webEnabled}, {"updatedAt", session.updatedAt}};
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 independentChatSend，执行该函数声明的职责。
Json independentChatSend(Database& db, const AgentAccess& access, const Json& body) {
    // 【逐行批注】 计算右侧结果并写入 auto lessonId，更新当前运行状态。
    const auto lessonId = requestText(body, "lessonId", 200);
    // 【逐行批注】 计算右侧结果并写入 auto text，更新当前运行状态。
    const auto text = requestText(body, "text", 16000);
    // 【逐行批注】 计算右侧结果并写入 auto requestId，更新当前运行状态。
    const auto requestId = requestText(body, "requestId", 80);
    // 【逐行批注】 调用 lessonForChat 完成本行所需操作：lessonForChat(db, lessonId);。
    lessonForChat(db, lessonId);
    // 【逐行批注】 计算右侧结果并写入 session，更新当前运行状态。
    auto session = ensureSession(db, lessonId);
    // 【逐行批注】 判断条件 (!session.activeTaskId.empty()) {，成立时执行对应分支。
    if (!session.activeTaskId.empty()) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 计算右侧结果并写入 auto current，更新当前运行状态。
            const auto current = agentView(db, access, session.activeTaskId, true);
            // 【逐行批注】 判断条件 (activeTask(current)) return {{"chat", independentChatView(db, a…，成立时执行对应分支。
            if (activeTask(current)) return {{"chat", independentChatView(db, access, lessonId)},
                // 【逐行批注】 执行当前声明或表达式：{"task", current}, {"duplicate", true}};。
                {"task", current}, {"duplicate", true}};
        // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
        } catch (...) {
            // 【逐行批注】 执行当前声明或表达式：session.activeTaskId.clear();。
            session.activeTaskId.clear();
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (body.contains("webEnabled") && body.at("webEnabled").is_boolean…，成立时执行对应分支。
    if (body.contains("webEnabled") && body.at("webEnabled").is_boolean())
        // 【逐行批注】 计算右侧结果并写入 session.webEnabled，更新当前运行状态。
        session.webEnabled = body.at("webEnabled").get<bool>();

    // 【逐行批注】 计算右侧结果并写入 auto userId，更新当前运行状态。
    const auto userId = "independent-user:" + requestId;
    // 【逐行批注】 计算右侧结果并写入 auto createdAt，更新当前运行状态。
    const auto createdAt = now();
    // 【逐行批注】 判断条件 (!db.upsertIndependentChatMessage({userId, lessonId, requestId, …，成立时执行对应分支。
    if (!db.upsertIndependentChatMessage({userId, lessonId, requestId, "user", text, "ready",
            // 【逐行批注】 执行当前声明或表达式："[]", "", "", createdAt, createdAt}))。
            "[]", "", "", createdAt, createdAt}))
        // 【逐行批注】 检测到错误情形时抛出异常：throw std::runtime_error("消息保存失败");。
        throw std::runtime_error("消息保存失败");

    // 【逐行批注】 计算右侧结果并写入 auto event，更新当前运行状态。
    const auto event = Json{{"type", "independent_chat"}, {"lessonId", lessonId},
        // 【逐行批注】 执行当前声明或表达式：{"text", text}, {"webEnabled", session.webEnabled},。
        {"text", text}, {"webEnabled", session.webEnabled},
        // 【逐行批注】 执行当前声明或表达式：{"requestId", "independent:" + requestId}};。
        {"requestId", "independent:" + requestId}};
    // 【逐行批注】 计算右侧结果并写入 auto task，更新当前运行状态。
    const auto task = agentSubmit(db, access, event);
    // 【逐行批注】 计算右侧结果并写入 session.activeTaskId，更新当前运行状态。
    session.activeTaskId = task.value("id", "");
    // 【逐行批注】 计算右侧结果并写入 session.updatedAt，更新当前运行状态。
    session.updatedAt = createdAt;
    // 【逐行批注】 判断条件 (!db.saveIndependentChatSession(session)) throw std::runtime_err…，成立时执行对应分支。
    if (!db.saveIndependentChatSession(session)) throw std::runtime_error("聊天任务关联保存失败");
    // 【逐行批注】 结束当前函数并返回结果：return {{"chat", independentChatView(db, access, lessonId)}, {"task",…。
    return {{"chat", independentChatView(db, access, lessonId)}, {"task", task}, {"duplicate", false}};
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 independentChatClear，执行该函数声明的职责。
Json independentChatClear(Database& db, const AgentAccess& access, const Json& body) {
    // 【逐行批注】 计算右侧结果并写入 auto lessonId，更新当前运行状态。
    const auto lessonId = requestText(body, "lessonId", 200);
    // 【逐行批注】 调用 lessonForChat 完成本行所需操作：lessonForChat(db, lessonId);。
    lessonForChat(db, lessonId);
    // 【逐行批注】 计算右侧结果并写入 auto session，更新当前运行状态。
    const auto session = ensureSession(db, lessonId);
    // 【逐行批注】 判断条件 (!session.activeTaskId.empty()) {，成立时执行对应分支。
    if (!session.activeTaskId.empty()) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try { agentControl(db, access, {{"command", "cancel"}, {"taskId", session.activeTaskId}}); } catch (...) {}
    // 【逐行批注】 结束当前代码块。
    }
    // 【逐行批注】 判断条件 (!db.clearIndependentChat(lessonId)) throw std::runtime_error("聊…，成立时执行对应分支。
    if (!db.clearIndependentChat(lessonId)) throw std::runtime_error("聊天清空失败");
    // 【逐行批注】 结束当前函数并返回结果：return {{"lessonId", lessonId}, {"messages", Json::array()}, {"webEna…。
    return {{"lessonId", lessonId}, {"messages", Json::array()}, {"webEnabled", session.webEnabled}};
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi。
} // namespace gangyi
