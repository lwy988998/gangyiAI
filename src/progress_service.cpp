// 【逐行批注】 引入头文件 "progress_service.hpp"，为后续实现提供类型或接口声明。
#include "progress_service.hpp"

// 【逐行批注】 引入头文件 <algorithm>，为后续实现提供类型或接口声明。
#include <algorithm>
// 【逐行批注】 引入头文件 <chrono>，为后续实现提供类型或接口声明。
#include <chrono>
// 【逐行批注】 引入头文件 <cmath>，为后续实现提供类型或接口声明。
#include <cmath>
// 【逐行批注】 引入头文件 <iomanip>，为后续实现提供类型或接口声明。
#include <iomanip>
// 【逐行批注】 引入头文件 <random>，为后续实现提供类型或接口声明。
#include <random>
// 【逐行批注】 引入头文件 <sstream>，为后续实现提供类型或接口声明。
#include <sstream>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 【逐行批注】 声明或进入命名空间：namespace {。
namespace {

// 【逐行批注】 计算右侧结果并写入 using json，更新当前运行状态。
using json = nlohmann::json;

// 【逐行批注】 定义函数 trim，执行该函数声明的职责。
std::string trim(const std::string& value) {
    // 【逐行批注】 计算右侧结果并写入 auto first，更新当前运行状态。
    const auto first = value.find_first_not_of(" \t\r\n");
    // 【逐行批注】 判断条件 (first == std::string::npos) return {};，成立时执行对应分支。
    if (first == std::string::npos) return {};
    // 【逐行批注】 计算右侧结果并写入 auto last，更新当前运行状态。
    const auto last = value.find_last_not_of(" \t\r\n");
    // 【逐行批注】 结束当前函数并返回结果：return value.substr(first, last - first + 1);。
    return value.substr(first, last - first + 1);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 bounded，执行该函数声明的职责。
std::string bounded(const std::string& value, size_t limit) {
    // 【逐行批注】 计算右侧结果并写入 std::string result，更新当前运行状态。
    const std::string result = trim(value);
    // 【逐行批注】 结束当前函数并返回结果：return result.substr(0, std::min(limit, result.size()));。
    return result.substr(0, std::min(limit, result.size()));
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 textValue，执行该函数声明的职责。
std::string textValue(const json& object, const char* key) {
    // 【逐行批注】 判断条件 (!object.is_object() || !object.contains(key) || object[key].is_…，成立时执行对应分支。
    if (!object.is_object() || !object.contains(key) || object[key].is_null()) return {};
    // 【逐行批注】 结束当前函数并返回结果：return object[key].is_string() ? object[key].get<std::string>() : obj…。
    return object[key].is_string() ? object[key].get<std::string>() : object[key].dump();
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 intValue，执行该函数声明的职责。
int intValue(const json& object, const char* key, int fallback = 0) {
    // 【逐行批注】 判断条件 (!object.is_object() || !object.contains(key)) return fallback;，成立时执行对应分支。
    if (!object.is_object() || !object.contains(key)) return fallback;
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 判断条件 (object[key].is_number_integer()) return object[key].get<int>();，成立时执行对应分支。
        if (object[key].is_number_integer()) return object[key].get<int>();
        // 【逐行批注】 判断条件 (object[key].is_number()) return static_cast<int>(object[key].ge…，成立时执行对应分支。
        if (object[key].is_number()) return static_cast<int>(object[key].get<double>());
        // 【逐行批注】 判断条件 (object[key].is_string()) return std::stoi(object[key].get<std::…，成立时执行对应分支。
        if (object[key].is_string()) return std::stoi(object[key].get<std::string>());
    // 【逐行批注】 执行当前声明或表达式：} catch (...) {}。
    } catch (...) {}
    // 【逐行批注】 结束当前函数并返回结果：return fallback;。
    return fallback;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 optionalText，执行该函数声明的职责。
std::optional<std::string> optionalText(const std::string& value) {
    // 【逐行批注】 结束当前函数并返回结果：return value.empty() ? std::nullopt : std::optional<std::string>(valu…。
    return value.empty() ? std::nullopt : std::optional<std::string>(value);
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 randomId，执行该函数声明的职责。
std::string randomId() {
    // 【逐行批注】 调用 random 完成本行所需操作：static std::mt19937_64 random(std::random_device{}());。
    static std::mt19937_64 random(std::random_device{}());
    // 【逐行批注】 执行当前声明或表达式：std::ostringstream output;。
    std::ostringstream output;
    // 【逐行批注】 调用 setfill、setw、random 完成本行所需操作：output << std::hex << std::setfill('0') << std::setw(16) << random() …。
    output << std::hex << std::setfill('0') << std::setw(16) << random() << std::setw(16) << random();
    // 【逐行批注】 结束当前函数并返回结果：return output.str();。
    return output.str();
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 nowIso8601，执行该函数声明的职责。
std::string nowIso8601() {
    // 【逐行批注】 计算右侧结果并写入 auto now，更新当前运行状态。
    const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    // 【逐行批注】 执行当前声明或表达式：std::tm value{};。
    std::tm value{};
// 【逐行批注】 处理条件编译指令：#ifdef _WIN32。
#ifdef _WIN32
    // 【逐行批注】 调用 gmtime_s 完成本行所需操作：gmtime_s(&value, &now);。
    gmtime_s(&value, &now);
// 【逐行批注】 处理条件编译指令：#else。
#else
    // 【逐行批注】 调用 gmtime_r 完成本行所需操作：gmtime_r(&now, &value);。
    gmtime_r(&now, &value);
// 【逐行批注】 处理条件编译指令：#endif。
#endif
    // 【逐行批注】 执行当前声明或表达式：std::ostringstream output;。
    std::ostringstream output;
    // 【逐行批注】 调用 put_time 完成本行所需操作：output << std::put_time(&value, "%Y-%m-%dT%H:%M:%SZ");。
    output << std::put_time(&value, "%Y-%m-%dT%H:%M:%SZ");
    // 【逐行批注】 结束当前函数并返回结果：return output.str();。
    return output.str();
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 arrayLength，执行该函数声明的职责。
size_t arrayLength(const json& object, const char* key) {
    // 【逐行批注】 结束当前函数并返回结果：return object.is_object() && object.contains(key) && object[key].is_a…。
    return object.is_object() && object.contains(key) && object[key].is_array() ? object[key].size() : 0;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 执行当前声明或表达式：template <typename Callback>。
template <typename Callback>
// 【逐行批注】 定义函数 forEachStage，执行该函数声明的职责。
void forEachStage(const json& value, Callback&& callback) {
    // 【逐行批注】 判断条件 (value.is_array()) {，成立时执行对应分支。
    if (value.is_array()) {
        // 【逐行批注】 按循环规则重复处理：for (const auto& stage : value) callback(stage);。
        for (const auto& stage : value) callback(stage);
    // 【逐行批注】 执行当前声明或表达式：} else if (value.is_object()) {。
    } else if (value.is_object()) {
        // 【逐行批注】 按循环规则重复处理：for (const auto& item : value.items()) callback(item.value());。
        for (const auto& item : value.items()) callback(item.value());
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明对象 stageItemCount，并通过构造函数初始化其状态。
size_t stageItemCount(const json& payload, const char* container,
                      // 【逐行批注】 执行当前声明或表达式：const std::initializer_list<const char*>& keys) {。
                      const std::initializer_list<const char*>& keys) {
    // 【逐行批注】 判断条件 (!payload.is_object() || !payload.contains(container)) return 0;，成立时执行对应分支。
    if (!payload.is_object() || !payload.contains(container)) return 0;
    // 【逐行批注】 计算右侧结果并写入 size_t total，更新当前运行状态。
    size_t total = 0;
    // 【逐行批注】 按循环规则重复处理：forEachStage(payload[container], [&](const json& stage) {。
    forEachStage(payload[container], [&](const json& stage) {
        // 【逐行批注】 按循环规则重复处理：for (const char* key : keys) {。
        for (const char* key : keys) {
            // 【逐行批注】 计算右侧结果并写入 size_t count，更新当前运行状态。
            const size_t count = arrayLength(stage, key);
            // 【逐行批注】 判断条件 (count > 0) {，成立时执行对应分支。
            if (count > 0) {
                // 【逐行批注】 计算右侧结果并写入 total +，更新当前运行状态。
                total += count;
                // 【逐行批注】 跳出当前循环或分支。
                break;
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 结束当前代码块。
        }
    // 【逐行批注】 执行当前声明或表达式：});。
    });
    // 【逐行批注】 结束当前函数并返回结果：return total;。
    return total;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 completedTaskCount，执行该函数声明的职责。
int completedTaskCount(Database& db, const std::string& courseId) {
    // 【逐行批注】 计算右侧结果并写入 int count，更新当前运行状态。
    int count = 0;
    // 【逐行批注】 按循环规则重复处理：for (const auto& item : db.listTaskProgress())。
    for (const auto& item : db.listTaskProgress())
        // 【逐行批注】 判断条件 (item.courseId.value_or("") == courseId && item.status == "compl…，成立时执行对应分支。
        if (item.courseId.value_or("") == courseId && item.status == "completed") ++count;
    // 【逐行批注】 结束当前函数并返回结果：return count;。
    return count;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 understoodStepCount，执行该函数声明的职责。
int understoodStepCount(Database& db, const std::string& courseId) {
    // 【逐行批注】 计算右侧结果并写入 int count，更新当前运行状态。
    int count = 0;
    // 【逐行批注】 按循环规则重复处理：for (const auto& item : db.listLearningStepProgress())。
    for (const auto& item : db.listLearningStepProgress())
        // 【逐行批注】 判断条件 (item.courseId.value_or("") == courseId && item.status == "under…，成立时执行对应分支。
        if (item.courseId.value_or("") == courseId && item.status == "understood") ++count;
    // 【逐行批注】 结束当前函数并返回结果：return count;。
    return count;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 completedCardCount，执行该函数声明的职责。
int completedCardCount(Database& db, const std::string& courseId) {
    // 【逐行批注】 计算右侧结果并写入 int count，更新当前运行状态。
    int count = 0;
    // 【逐行批注】 按循环规则重复处理：for (const auto& item : db.listLearningCardProgress())。
    for (const auto& item : db.listLearningCardProgress())
        // 【逐行批注】 判断条件 (item.courseId.value_or("") == courseId && item.status == "compl…，成立时执行对应分支。
        if (item.courseId.value_or("") == courseId && item.status == "completed") ++count;
    // 【逐行批注】 结束当前函数并返回结果：return count;。
    return count;
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 progressJson，执行该函数声明的职责。
json progressJson(const CourseProgress& value) {
    // 【逐行批注】 结束当前函数并返回结果：return {{"overallPercent", value.overallPercent}, {"completedCount", …。
    return {{"overallPercent", value.overallPercent}, {"completedCount", value.completedCount},
            // 【逐行批注】 执行当前声明或表达式：{"totalCount", value.totalCount}, {"lastVisitedUrl", value.lastVisitedUrl.value_or("…。
            {"totalCount", value.totalCount}, {"lastVisitedUrl", value.lastVisitedUrl.value_or("")},
            // 【逐行批注】 执行当前声明或表达式：{"lastPageType", value.lastPageType.value_or("")},。
            {"lastPageType", value.lastPageType.value_or("")},
            // 【逐行批注】 调用 json 完成本行所需操作：{"lastPhaseIndex", value.lastPhaseIndex ? json(*value.lastPhaseIndex)…。
            {"lastPhaseIndex", value.lastPhaseIndex ? json(*value.lastPhaseIndex) : json(nullptr)},
            // 【逐行批注】 执行当前声明或表达式：{"lastPhaseName", value.lastPhaseName.value_or("")},。
            {"lastPhaseName", value.lastPhaseName.value_or("")},
            // 【逐行批注】 调用 json 完成本行所需操作：{"lastTopicIndex", value.lastTopicIndex ? json(*value.lastTopicIndex)…。
            {"lastTopicIndex", value.lastTopicIndex ? json(*value.lastTopicIndex) : json(nullptr)},
            // 【逐行批注】 执行当前声明或表达式：{"lastTopicTitle", value.lastTopicTitle.value_or("")}, {"updatedAt", value.updatedAt…。
            {"lastTopicTitle", value.lastTopicTitle.value_or("")}, {"updatedAt", value.updatedAt}};
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 执行当前声明或表达式：} // namespace。
}  // namespace

// 【逐行批注】 定义函数 normalizeCardStatus，执行该函数声明的职责。
std::string normalizeCardStatus(const std::string& status) {
    // 【逐行批注】 计算右侧结果并写入 std::string value，更新当前运行状态。
    const std::string value = trim(status);
    // 【逐行批注】 结束当前函数并返回结果：return value == "not_started" || value == "in_progress" || value == "…。
    return value == "not_started" || value == "in_progress" || value == "completed"
        // 【逐行批注】 执行当前声明或表达式：? value : "not_started";。
        ? value : "not_started";
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 normalizeStepStatus，执行该函数声明的职责。
std::string normalizeStepStatus(const std::string& status) {
    // 【逐行批注】 计算右侧结果并写入 std::string value，更新当前运行状态。
    const std::string value = trim(status);
    // 【逐行批注】 结束当前函数并返回结果：return value == "unset" || value == "understood" || value == "review"…。
    return value == "unset" || value == "understood" || value == "review" ? value : "unset";
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 saveTaskProgress，执行该函数声明的职责。
ProgressSaveResult saveTaskProgress(Database& db, const json& body) {
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 计算右侧结果并写入 std::string courseId，更新当前运行状态。
        const std::string courseId = bounded(textValue(body, "courseId"), 500);
        // 【逐行批注】 计算右侧结果并写入 int phaseIndex，更新当前运行状态。
        const int phaseIndex = intValue(body, "phaseIndex");
        // 【逐行批注】 计算右侧结果并写入 int taskIndex，更新当前运行状态。
        const int taskIndex = intValue(body, "taskIndex", -1);
        // 【逐行批注】 判断条件 (courseId.empty() || phaseIndex < 1 || taskIndex < 0) return {};，成立时执行对应分支。
        if (courseId.empty() || phaseIndex < 1 || taskIndex < 0) return {};

        // 【逐行批注】 计算右侧结果并写入 existing，更新当前运行状态。
        auto existing = db.findTaskProgress(courseId, phaseIndex, taskIndex);
        // 【逐行批注】 计算右侧结果并写入 TaskProgress value，更新当前运行状态。
        TaskProgress value = existing.value_or(TaskProgress{});
        // 【逐行批注】 判断条件 (value.id.empty()) value.id = randomId();，成立时执行对应分支。
        if (value.id.empty()) value.id = randomId();
        // 【逐行批注】 计算右侧结果并写入 value.courseId，更新当前运行状态。
        value.courseId = courseId;
        // 【逐行批注】 计算右侧结果并写入 value.anonymousId，更新当前运行状态。
        value.anonymousId = optionalText(bounded(textValue(body, "anonymousId"), 500));
        // 【逐行批注】 计算右侧结果并写入 value.goal，更新当前运行状态。
        value.goal = bounded(textValue(body, "goal"), 500);
        // 【逐行批注】 计算右侧结果并写入 value.mode，更新当前运行状态。
        value.mode = optionalText(bounded(textValue(body, "mode"), 40));
        // 【逐行批注】 计算右侧结果并写入 value.phaseIndex，更新当前运行状态。
        value.phaseIndex = phaseIndex;
        // 【逐行批注】 计算右侧结果并写入 value.phaseName，更新当前运行状态。
        value.phaseName = bounded(textValue(body, "phaseName"), 500);
        // 【逐行批注】 判断条件 (value.phaseName.empty()) value.phaseName = "阶段" + std::to_strin…，成立时执行对应分支。
        if (value.phaseName.empty()) value.phaseName = "阶段" + std::to_string(phaseIndex);
        // 【逐行批注】 计算右侧结果并写入 value.taskIndex，更新当前运行状态。
        value.taskIndex = taskIndex;
        // 【逐行批注】 计算右侧结果并写入 value.taskTitle，更新当前运行状态。
        value.taskTitle = bounded(textValue(body, "taskTitle"), 500);
        // 【逐行批注】 判断条件 (value.taskTitle.empty()) value.taskTitle = "任务" + std::to_strin…，成立时执行对应分支。
        if (value.taskTitle.empty()) value.taskTitle = "任务" + std::to_string(taskIndex + 1);
        // 【逐行批注】 计算右侧结果并写入 value.status，更新当前运行状态。
        value.status = normalizeCardStatus(textValue(body, "status"));
        // 【逐行批注】 计算右侧结果并写入 bool saved，更新当前运行状态。
        const bool saved = existing ? db.update(value) : db.insert(value);
        // 【逐行批注】 判断条件 (!saved) return {};，成立时执行对应分支。
        if (!saved) return {};
        // 【逐行批注】 结束当前函数并返回结果：return {true, {{"id", value.id}, {"courseId", courseId}, {"anonymousI…。
        return {true, {{"id", value.id}, {"courseId", courseId}, {"anonymousId", value.anonymousId.value_or("")},
            // 【逐行批注】 执行当前声明或表达式：{"goal", value.goal}, {"mode", value.mode.value_or("")}, {"phaseIndex", phaseIndex},。
            {"goal", value.goal}, {"mode", value.mode.value_or("")}, {"phaseIndex", phaseIndex},
            // 【逐行批注】 执行当前声明或表达式：{"phaseName", value.phaseName}, {"taskIndex", taskIndex}, {"taskTitle", value.taskTi…。
            {"phaseName", value.phaseName}, {"taskIndex", taskIndex}, {"taskTitle", value.taskTitle},
            // 【逐行批注】 执行当前声明或表达式：{"status", value.status}}};。
            {"status", value.status}}};
    // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
    } catch (...) {
        // 【逐行批注】 结束当前函数并返回结果：return {};。
        return {};
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明对象 recomputeCourseProgress，并通过构造函数初始化其状态。
std::optional<json> recomputeCourseProgress(Database& db, const std::string& courseId,
                                             // 【逐行批注】 执行当前声明或表达式：const std::string& anonymousId,。
                                             const std::string& anonymousId,
                                             // 【逐行批注】 执行当前声明或表达式：const std::string& goal) {。
                                             const std::string& goal) {
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 计算右侧结果并写入 auto course，更新当前运行状态。
        const auto course = db.getCourse(courseId);
        // 【逐行批注】 判断条件 (!course) return std::nullopt;，成立时执行对应分支。
        if (!course) return std::nullopt;
        // 【逐行批注】 计算右侧结果并写入 auto snapshots，更新当前运行状态。
        const auto snapshots = db.findSnapshotsByCourseId(courseId);
        // 【逐行批注】 判断条件 (snapshots.empty()) return std::nullopt;，成立时执行对应分支。
        if (snapshots.empty()) return std::nullopt;
        // 【逐行批注】 计算右侧结果并写入 auto latest，更新当前运行状态。
        const auto latest = std::max_element(snapshots.begin(), snapshots.end(), [](const auto& left, const auto& right) {
            // 【逐行批注】 结束当前函数并返回结果：return left.version != right.version ? left.version < right.version :…。
            return left.version != right.version ? left.version < right.version : left.createdAt < right.createdAt;
        // 【逐行批注】 执行当前声明或表达式：});。
        });
        // 【逐行批注】 计算右侧结果并写入 json payload，更新当前运行状态。
        const json payload = json::parse(latest->payload, nullptr, false);
        // 【逐行批注】 判断条件 (!payload.is_object()) return std::nullopt;，成立时执行对应分支。
        if (!payload.is_object()) return std::nullopt;

        // 【逐行批注】 计算右侧结果并写入 size_t cards，更新当前运行状态。
        size_t cards = stageItemCount(payload, "courseStructure", {"topics"});
        // 【逐行批注】 判断条件 (cards == 0) cards = stageItemCount(payload, "roadmap", {"topics…，成立时执行对应分支。
        if (cards == 0) cards = stageItemCount(payload, "roadmap", {"topics", "learningCards", "cards"});
        // 【逐行批注】 计算右侧结果并写入 size_t tasks，更新当前运行状态。
        const size_t tasks = stageItemCount(payload, "roadmap", {"tasks", "checklist", "practices", "projects"});
        // 【逐行批注】 计算右侧结果并写入 size_t steps，更新当前运行状态。
        const size_t steps = stageItemCount(payload, "roadmap", {"steps"});
        // 【逐行批注】 计算右侧结果并写入 int completed，更新当前运行状态。
        int completed = completedTaskCount(db, courseId) + understoodStepCount(db, courseId) +
            // 【逐行批注】 调用 completedCardCount 完成本行所需操作：completedCardCount(db, courseId);。
            completedCardCount(db, courseId);
        // 【逐行批注】 计算右侧结果并写入 bool agentOutline，更新当前运行状态。
        const bool agentOutline = payload.value("generation", json::object()).value("promptVersion", "") == "ai-agent-v551";
        // 【逐行批注】 判断条件 (agentOutline) completed = completedCardCount(db, courseId);，成立时执行对应分支。
        if (agentOutline) completed = completedCardCount(db, courseId);
        // 【逐行批注】 计算右侧结果并写入 int total，更新当前运行状态。
        const int total = std::max(completed, static_cast<int>(agentOutline ? cards : cards + tasks + steps));
        // 【逐行批注】 计算右侧结果并写入 int percent，更新当前运行状态。
        const int percent = total == 0 ? 0 : std::clamp(static_cast<int>(std::round(completed * 100.0 / total)), 0, 100);

        // 【逐行批注】 计算右侧结果并写入 existing，更新当前运行状态。
        auto existing = db.findProgressByCourseId(courseId);
        // 【逐行批注】 计算右侧结果并写入 CourseProgress value，更新当前运行状态。
        CourseProgress value = existing.value_or(CourseProgress{});
        // 【逐行批注】 判断条件 (value.id.empty()) value.id = randomId();，成立时执行对应分支。
        if (value.id.empty()) value.id = randomId();
        // 【逐行批注】 计算右侧结果并写入 value.courseId，更新当前运行状态。
        value.courseId = courseId;
        // 【逐行批注】 计算右侧结果并写入 value.anonymousId，更新当前运行状态。
        value.anonymousId = optionalText(bounded(anonymousId, 500));
        // 【逐行批注】 计算右侧结果并写入 value.userId，更新当前运行状态。
        value.userId = course->userId;
        // 【逐行批注】 计算右侧结果并写入 value.goal，更新当前运行状态。
        value.goal = bounded(goal, 500).empty() ? course->goal : bounded(goal, 500);
        // 【逐行批注】 计算右侧结果并写入 value.mode，更新当前运行状态。
        value.mode = course->mode;
        // 【逐行批注】 计算右侧结果并写入 value.overallPercent，更新当前运行状态。
        value.overallPercent = percent;
        // 【逐行批注】 计算右侧结果并写入 value.completedCount，更新当前运行状态。
        value.completedCount = completed;
        // 【逐行批注】 计算右侧结果并写入 value.totalCount，更新当前运行状态。
        value.totalCount = total;
        // 【逐行批注】 计算右侧结果并写入 value.updatedAt，更新当前运行状态。
        value.updatedAt = nowIso8601();
        // 【逐行批注】 判断条件 (value.createdAt.empty()) value.createdAt = value.updatedAt;，成立时执行对应分支。
        if (value.createdAt.empty()) value.createdAt = value.updatedAt;
        // 【逐行批注】 计算右侧结果并写入 bool saved，更新当前运行状态。
        const bool saved = existing ? db.update(value) : db.insert(value);
        // 【逐行批注】 判断条件 (!saved) return std::nullopt;，成立时执行对应分支。
        if (!saved) return std::nullopt;
        // 【逐行批注】 结束当前函数并返回结果：return progressJson(value);。
        return progressJson(value);
    // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
    } catch (...) {
        // 【逐行批注】 结束当前函数并返回结果：return std::nullopt;。
        return std::nullopt;
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明对象 resetCourseProgress，并通过构造函数初始化其状态。
std::optional<json> resetCourseProgress(Database& db, const std::string& courseId,
                                        // 【逐行批注】 执行当前声明或表达式：const std::string& anonymousId, const std::string& goal) {。
                                        const std::string& anonymousId, const std::string& goal) {
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 按循环规则重复处理：for (const auto& item : db.listLearningCardProgress())。
        for (const auto& item : db.listLearningCardProgress())
            // 【逐行批注】 判断条件 (item.courseId.value_or("") == courseId) db.deleteLearningCardPr…，成立时执行对应分支。
            if (item.courseId.value_or("") == courseId) db.deleteLearningCardProgress(item.id);
        // 【逐行批注】 按循环规则重复处理：for (const auto& item : db.listLearningStepProgress())。
        for (const auto& item : db.listLearningStepProgress())
            // 【逐行批注】 判断条件 (item.courseId.value_or("") == courseId) db.deleteLearningStepPr…，成立时执行对应分支。
            if (item.courseId.value_or("") == courseId) db.deleteLearningStepProgress(item.id);
        // 【逐行批注】 按循环规则重复处理：for (const auto& item : db.listTaskProgress())。
        for (const auto& item : db.listTaskProgress())
            // 【逐行批注】 判断条件 (item.courseId.value_or("") == courseId) db.deleteTaskProgress(i…，成立时执行对应分支。
            if (item.courseId.value_or("") == courseId) db.deleteTaskProgress(item.id);
        // 【逐行批注】 判断条件 (const auto progress = db.findProgressByCourseId(courseId)) db.d…，成立时执行对应分支。
        if (const auto progress = db.findProgressByCourseId(courseId)) db.deleteCourseProgress(progress->id);
        // 【逐行批注】 结束当前函数并返回结果：return recomputeCourseProgress(db, courseId, anonymousId, goal);。
        return recomputeCourseProgress(db, courseId, anonymousId, goal);
    // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
    } catch (...) {
        // 【逐行批注】 结束当前函数并返回结果：return std::nullopt;。
        return std::nullopt;
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 声明对象 updateLastVisited，并通过构造函数初始化其状态。
bool updateLastVisited(Database& db, const std::string& courseId, const std::string& anonymousId,
                       // 【逐行批注】 执行当前声明或表达式：const std::string& goal, const std::string& mode, const json& body) {。
                       const std::string& goal, const std::string& mode, const json& body) {
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 计算右侧结果并写入 auto course，更新当前运行状态。
        const auto course = db.getCourse(courseId);
        // 【逐行批注】 判断条件 (!course) return false;，成立时执行对应分支。
        if (!course) return false;
        // 【逐行批注】 计算右侧结果并写入 existing，更新当前运行状态。
        auto existing = db.findProgressByCourseId(courseId);
        // 【逐行批注】 计算右侧结果并写入 CourseProgress value，更新当前运行状态。
        CourseProgress value = existing.value_or(CourseProgress{});
        // 【逐行批注】 判断条件 (value.id.empty()) value.id = randomId();，成立时执行对应分支。
        if (value.id.empty()) value.id = randomId();
        // 【逐行批注】 计算右侧结果并写入 value.courseId，更新当前运行状态。
        value.courseId = courseId;
        // 【逐行批注】 计算右侧结果并写入 value.anonymousId，更新当前运行状态。
        value.anonymousId = optionalText(bounded(anonymousId, 500));
        // 【逐行批注】 计算右侧结果并写入 value.userId，更新当前运行状态。
        value.userId = course->userId;
        // 【逐行批注】 计算右侧结果并写入 value.goal，更新当前运行状态。
        value.goal = bounded(goal, 500).empty() ? course->goal : bounded(goal, 500);
        // 【逐行批注】 计算右侧结果并写入 value.mode，更新当前运行状态。
        value.mode = optionalText(bounded(mode, 40));
        // 【逐行批注】 计算右侧结果并写入 std::string url，更新当前运行状态。
        const std::string url = textValue(body, "lastVisitedUrl");
        // 【逐行批注】 判断条件 (!url.empty() && url.front() == '/' && url.find("//") == std::st…，成立时执行对应分支。
        if (!url.empty() && url.front() == '/' && url.find("//") == std::string::npos)
            // 【逐行批注】 计算右侧结果并写入 value.lastVisitedUrl，更新当前运行状态。
            value.lastVisitedUrl = bounded(url, 1200);
        // 【逐行批注】 计算右侧结果并写入 value.lastPageType，更新当前运行状态。
        value.lastPageType = optionalText(bounded(textValue(body, "lastPageType"), 40));
        // 【逐行批注】 判断条件 (body.contains("lastPhaseIndex")) value.lastPhaseIndex = intValu…，成立时执行对应分支。
        if (body.contains("lastPhaseIndex")) value.lastPhaseIndex = intValue(body, "lastPhaseIndex");
        // 【逐行批注】 计算右侧结果并写入 value.lastPhaseName，更新当前运行状态。
        value.lastPhaseName = optionalText(bounded(textValue(body, "lastPhaseName"), 500));
        // 【逐行批注】 判断条件 (body.contains("lastTopicIndex")) value.lastTopicIndex = intValu…，成立时执行对应分支。
        if (body.contains("lastTopicIndex")) value.lastTopicIndex = intValue(body, "lastTopicIndex");
        // 【逐行批注】 计算右侧结果并写入 value.lastTopicTitle，更新当前运行状态。
        value.lastTopicTitle = optionalText(bounded(textValue(body, "lastTopicTitle"), 500));
        // 【逐行批注】 计算右侧结果并写入 value.updatedAt，更新当前运行状态。
        value.updatedAt = nowIso8601();
        // 【逐行批注】 判断条件 (value.createdAt.empty()) value.createdAt = value.updatedAt;，成立时执行对应分支。
        if (value.createdAt.empty()) value.createdAt = value.updatedAt;
        // 【逐行批注】 结束当前函数并返回结果：return existing ? db.update(value) : db.insert(value);。
        return existing ? db.update(value) : db.insert(value);
    // 【逐行批注】 执行当前声明或表达式：} catch (...) {。
    } catch (...) {
        // 【逐行批注】 结束当前函数并返回结果：return false;。
        return false;
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 执行当前声明或表达式：} // namespace gangyi。
}  // namespace gangyi
