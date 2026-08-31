#pragma once

#include "ai_client.hpp"

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace gangyi {

struct Step {
    std::string title;
    std::string explanation;
    std::string example;
    std::string action;
    std::string check;
};

struct Phase {
    std::string name;
    int durationWeeks = 0;
    std::string duration;
    std::string objective;
    std::string why;
    std::string description;
    std::string overview;
    std::vector<std::string> topics;
    std::vector<std::string> topicDescriptions;
    std::vector<std::string> tasks;
    std::string practice;
    std::string checkpoint;
    std::string output;
    std::vector<std::string> commonMistakes;
    std::vector<Step> steps;
};

struct Slide {
    std::string title;
    std::string subtitle;
    std::string content;
    std::vector<std::string> bullets;
    std::string speakerNote;
    std::string relatedPhase;
};

struct GeneratedPlan {
    std::string inferredDomain;
    std::string learnerGoal;
    std::string courseTitle;
    std::string courseSummary;
    std::string title;
    std::string goal;
    int durationWeeks = 0;
    std::string summary;
    std::string courseIntro;
    std::string overview;
    std::string audience;
    std::string prerequisites;
    std::string outcome;
    std::vector<std::string> learningOutcomes;
    std::vector<Phase> phases;
    std::vector<Slide> slides;
    nlohmann::json mindMap = nullptr;
    nlohmann::json resources = nlohmann::json::array();
    std::vector<std::string> projects;
};

class PlanGenerator {
public:
    explicit PlanGenerator(AIClient& client);

    GeneratedPlan generate(const std::string& goal, const std::string& mode);

private:
    AIClient& client_;
};

}
