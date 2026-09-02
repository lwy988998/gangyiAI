#pragma once

#include "ai_client.hpp"

#include <string>
#include <vector>

namespace gangyi {

struct ImageGoalAnalysis {
    std::string goal;
    std::string summary;
    std::vector<std::string> keywords;
    std::string suggestedSearchQuery;
};

class ImageGoalAnalyzer {
public:
    explicit ImageGoalAnalyzer(AIClient& client);
    ImageGoalAnalysis analyze(const std::string& prompt, const std::string& mode,
                              const std::string& mimeType, const std::string& imageBytes) const;

private:
    AIClient& client_;
};

}  // namespace gangyi
