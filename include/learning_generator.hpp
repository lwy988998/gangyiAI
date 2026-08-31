#pragma once

#include "ai_client.hpp"

#include <optional>
#include <string>
#include <vector>

namespace gangyi {

struct SearchResource {
    std::string title;
    std::string url;
    std::string source;
    std::string description;
    std::string type;
};

struct LessonStep {
    std::string title;
    std::string explanation;
    std::string example;
    std::string action;
    std::string check;
};

struct Example {
    std::string title;
    std::string content;
    std::vector<std::string> solution;
};

struct Practice {
    std::string title;
    std::string difficulty;
    std::string task;
    std::string check;
};

struct Quiz {
    std::string question;
    std::vector<std::string> options;
    int answerIndex = -1;
    std::string explanation;
};

struct Reference {
    std::string title;
    std::string source;
    std::string url;
    std::string type;
};

struct LearningAnswer {
    std::string inferredDomain;
    std::string title;
    std::string summary;
    std::vector<std::string> keyConcepts;
    std::vector<LessonStep> lessonSteps;
    std::vector<Example> examples;
    std::vector<Practice> practice;
    std::vector<Quiz> quiz;
    std::vector<std::string> commonMistakes;
    std::vector<std::string> checkpoint;
    std::string resourceSummary;
    std::vector<Reference> references;
    std::optional<std::string> notice;
};

class LearningGenerator {
public:
    explicit LearningGenerator(AIClient& client);

    LearningAnswer generate(const std::string& goal,
                            const std::string& phaseName,
                            const std::string& topic,
                            const std::string& mode,
                            const std::vector<SearchResource>& resources) const;

private:
    AIClient& client_;
};

}  // namespace gangyi
