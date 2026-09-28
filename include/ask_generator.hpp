#pragma once

#include "ai_client.hpp"

#include <string>
#include <vector>

namespace gangyi {

struct AskAnswer {
    std::string content;
    std::string model;
    std::string searchStatus;
    std::vector<std::string> sources;
};

class AskGenerator {
public:
    explicit AskGenerator(AIClient& client);
    AskAnswer generate(const std::string& question, const std::string& profileContext = {}) const;

private:
    AIClient& client_;
};

}  // namespace gangyi
