#include "model_evidence_log.hpp"

#include <cassert>
#include <iostream>
#include <vector>

namespace gangyi {
AIClientError::AIClientError(const std::string& type, const std::string& message, int status)
    : std::runtime_error(message), errorType(type), httpStatus(status) {}
}

int main() {
    using Json = nlohmann::json;
    const auto folder = std::filesystem::temp_directory_path() / ("gangyi-model-evidence-" + std::to_string(
        std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(folder);
    gangyi::ChatOptions options; options.activity.taskId = "actual-task";
    options.messages = {{"user","学生真实操作：a=1.5 后调整到 2。",""}};
    // 未启用时没有任何文件写入；图片只留存在标记，不记录图片数据或环境凭据。
    { gangyi::ModelEvidenceLog disabled(options, {}); disabled.chunk("无记录"); }
    assert(std::filesystem::is_empty(folder));
    options.messages.push_back({"user","图片请求", "data:image/png;base64,PRIVATE_IMAGE"});
    gangyi::ModelEvidenceLog evidence(options, folder);
    evidence.chunk("针对真实"); evidence.chunk("操作反馈");
    gangyi::AIResult result; result.content = "针对真实操作反馈"; result.model = "recorded-model";
    result.tokenUsage = {{"prompt_tokens",20},{"completion_tokens",5},{"total_tokens",25}};
    evidence.completed(result);
    std::vector<Json> records;
    for (const auto& file : std::filesystem::directory_iterator(folder)) {
        std::ifstream stream(file.path()); std::string line;
        while (std::getline(stream,line)) { assert(line.find("PRIVATE_IMAGE") == std::string::npos); records.push_back(Json::parse(line)); }
    }
    assert(records.size() == 4 && records.front().at("kind") == "request");
    assert(records.front().at("taskId") == "actual-task" && records.front().at("textOnly") == false);
    assert(records.front().at("messages").at(1).at("hasImage") == true);
    assert(records[1].at("text").get<std::string>() + records[2].at("text").get<std::string>() == records[3].at("rawResponse"));
    assert(records[3].at("usage") == result.tokenUsage && records[3].at("model") == result.model);
    for (const auto& record : records) assert(record.at("modelRequestId") == records.front().at("modelRequestId"));
    // 记录路径不可写时必须在调用模型前拒绝；即使路径恢复，也不能悄悄接续缺失的记录。
    const auto blocked = folder / "blocked"; { std::ofstream stream(blocked); stream << "regular file"; }
    bool denied = false; try { gangyi::ModelEvidenceLog invalid(options, blocked); } catch (const gangyi::AIClientError&) { denied = true; }
    assert(denied); std::filesystem::remove(blocked); std::filesystem::create_directory(blocked);
    denied = false; try { gangyi::ModelEvidenceLog invalid(options, blocked); } catch (const gangyi::AIClientError&) { denied = true; }
    assert(denied); std::filesystem::remove_all(folder);
    std::cout << "模型请求、原始流、正式回复及记录失败停止验证通过\n";
}
