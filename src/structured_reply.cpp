#include "structured_reply.hpp"
#include "ai_client.hpp"

namespace gangyi {

nlohmann::json parseAIJson(const std::string& content) {
    try {
        auto result = parseStructuredReply(content);
        if (!result.is_object()) throw StructuredReplyError("AI 回复应为一个 JSON 对象");
        return result;
    } catch (const nlohmann::json::exception&) {
        throw AIClientError("json_parse_error", "AI 回复不是完整有效的 JSON 对象");
    } catch (const StructuredReplyError& error) {
        throw AIClientError("json_parse_error", error.what());
    }
}

} // namespace gangyi
