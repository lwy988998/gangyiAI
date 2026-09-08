#pragma once

#include "ai_client.hpp"

#include <string>

namespace gangyi {

struct AskAnswer {
    std::string content;
    std::string model;
};

class AskGenerator {
public:
    explicit AskGenerator(AIClient& client);
    AskAnswer generate(const std::string& question) const;

private:
    AIClient& client_;
};

}  // namespace gangyi
