#pragma once

#include "ai_client.hpp"

#include <string>
#include <vector>

namespace gangyi {

struct AskAnswer {
    std::string title;
    std::vector<std::string> steps;
    std::vector<std::string> commands;
    std::vector<std::string> tips;
};

class AskGenerator {
public:
    explicit AskGenerator(AIClient& client);
    AskAnswer generate(const std::string& goal, const std::string& question, const std::string& mode) const;

private:
    AIClient& client_;
};

}  // namespace gangyi
