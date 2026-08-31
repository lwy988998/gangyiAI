#include "json_fix.hpp"
#include "ai_client.hpp"

#include <cassert>

int main() {
    using gangyi::parseAIJson;
    assert(parseAIJson(R"({"a":1})")["a"] == 1);
    assert(parseAIJson("```json\n{\"a\":1}\n```")["a"] == 1);
    assert(parseAIJson(R"({"a":1,})")["a"] == 1);
    assert(parseAIJson("{\xE2\x80\x9C" "a" "\xE2\x80\x9D" ":1}")["a"] == 1);
    assert(parseAIJson(R"({"a":1)")["a"] == 1);
    bool failed = false;
    try { parseAIJson("not json"); } catch (const gangyi::AIClientError& error) { failed = error.errorType == "json_parse_error"; }
    assert(failed);
    return 0;
}
