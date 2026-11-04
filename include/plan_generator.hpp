// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once

// 【逐行批注】 引入头文件 "ai_client.hpp"，为后续实现提供类型或接口声明。
#include "ai_client.hpp"

// 【逐行批注】 引入头文件 <nlohmann/json.hpp>，为后续实现提供类型或接口声明。
#include <nlohmann/json.hpp>

// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>
// 【逐行批注】 引入头文件 <vector>，为后续实现提供类型或接口声明。
#include <vector>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {

// 【逐行批注】 声明数据结构或类型 Step 及其成员接口。
struct Step {
    // 【逐行批注】 执行当前声明或表达式：std::string title;。
    std::string title;
    // 【逐行批注】 执行当前声明或表达式：std::string explanation;。
    std::string explanation;
    // 【逐行批注】 执行当前声明或表达式：std::string example;。
    std::string example;
    // 【逐行批注】 执行当前声明或表达式：std::string action;。
    std::string action;
    // 【逐行批注】 执行当前声明或表达式：std::string check;。
    std::string check;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 声明数据结构或类型 Phase 及其成员接口。
struct Phase {
    // 【逐行批注】 执行当前声明或表达式：std::string name;。
    std::string name;
    // 【逐行批注】 计算右侧结果并写入 int durationWeeks，更新当前运行状态。
    int durationWeeks = 0;
    // 【逐行批注】 执行当前声明或表达式：std::string duration;。
    std::string duration;
    // 【逐行批注】 执行当前声明或表达式：std::string objective;。
    std::string objective;
    // 【逐行批注】 执行当前声明或表达式：std::string why;。
    std::string why;
    // 【逐行批注】 执行当前声明或表达式：std::string description;。
    std::string description;
    // 【逐行批注】 执行当前声明或表达式：std::string overview;。
    std::string overview;
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> topics;。
    std::vector<std::string> topics;
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> topicDescriptions;。
    std::vector<std::string> topicDescriptions;
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> tasks;。
    std::vector<std::string> tasks;
    // 【逐行批注】 执行当前声明或表达式：std::string practice;。
    std::string practice;
    // 【逐行批注】 执行当前声明或表达式：std::string checkpoint;。
    std::string checkpoint;
    // 【逐行批注】 执行当前声明或表达式：std::string output;。
    std::string output;
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> commonMistakes;。
    std::vector<std::string> commonMistakes;
    // 【逐行批注】 执行当前声明或表达式：std::vector<Step> steps;。
    std::vector<Step> steps;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 声明数据结构或类型 Slide 及其成员接口。
struct Slide {
    // 【逐行批注】 执行当前声明或表达式：std::string title;。
    std::string title;
    // 【逐行批注】 执行当前声明或表达式：std::string subtitle;。
    std::string subtitle;
    // 【逐行批注】 执行当前声明或表达式：std::string content;。
    std::string content;
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> bullets;。
    std::vector<std::string> bullets;
    // 【逐行批注】 执行当前声明或表达式：std::string speakerNote;。
    std::string speakerNote;
    // 【逐行批注】 执行当前声明或表达式：std::string relatedPhase;。
    std::string relatedPhase;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 声明数据结构或类型 GeneratedPlan 及其成员接口。
struct GeneratedPlan {
    // 【逐行批注】 执行当前声明或表达式：std::string generationModel;。
    std::string generationModel;
    // 【逐行批注】 执行当前声明或表达式：std::string searchStatus;。
    std::string searchStatus;
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> searchSources;。
    std::vector<std::string> searchSources;
    // 【逐行批注】 执行当前声明或表达式：std::string inferredDomain;。
    std::string inferredDomain;
    // 【逐行批注】 执行当前声明或表达式：std::string learnerGoal;。
    std::string learnerGoal;
    // 【逐行批注】 执行当前声明或表达式：std::string courseTitle;。
    std::string courseTitle;
    // 【逐行批注】 执行当前声明或表达式：std::string courseSummary;。
    std::string courseSummary;
    // 【逐行批注】 执行当前声明或表达式：std::string title;。
    std::string title;
    // 【逐行批注】 执行当前声明或表达式：std::string goal;。
    std::string goal;
    // 【逐行批注】 计算右侧结果并写入 int durationWeeks，更新当前运行状态。
    int durationWeeks = 0;
    // 【逐行批注】 执行当前声明或表达式：std::string summary;。
    std::string summary;
    // 【逐行批注】 执行当前声明或表达式：std::string courseIntro;。
    std::string courseIntro;
    // 【逐行批注】 执行当前声明或表达式：std::string overview;。
    std::string overview;
    // 【逐行批注】 执行当前声明或表达式：std::string audience;。
    std::string audience;
    // 【逐行批注】 执行当前声明或表达式：std::string prerequisites;。
    std::string prerequisites;
    // 【逐行批注】 执行当前声明或表达式：std::string outcome;。
    std::string outcome;
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> learningOutcomes;。
    std::vector<std::string> learningOutcomes;
    // 【逐行批注】 执行当前声明或表达式：std::vector<Phase> phases;。
    std::vector<Phase> phases;
    // 【逐行批注】 执行当前声明或表达式：std::vector<Slide> slides;。
    std::vector<Slide> slides;
    // 【逐行批注】 计算右侧结果并写入 nlohmann::json mindMap，更新当前运行状态。
    nlohmann::json mindMap = nullptr;
    // 【逐行批注】 计算右侧结果并写入 nlohmann::json resources，更新当前运行状态。
    nlohmann::json resources = nlohmann::json::array();
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::string> projects;。
    std::vector<std::string> projects;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 声明数据结构或类型 PlanGenerator 及其成员接口。
class PlanGenerator {
// 【逐行批注】 执行当前声明或表达式：public:。
public:
    // 【逐行批注】 声明对象 PlanGenerator，并通过构造函数初始化其状态。
    explicit PlanGenerator(AIClient& client);

    // 【逐行批注】 声明对象 generate，并通过构造函数初始化其状态。
    GeneratedPlan generate(const std::string& goal, const std::string& mode,
                           // 【逐行批注】 计算右侧结果并写入 std::string& qualityFeedback，更新当前运行状态。
                           const std::string& qualityFeedback = {},
                           // 【逐行批注】 计算右侧结果并写入 std::string& profileContext，更新当前运行状态。
                           const std::string& profileContext = {});

// 【逐行批注】 执行当前声明或表达式：private:。
private:
    // 【逐行批注】 执行当前声明或表达式：AIClient& client_;。
    AIClient& client_;
// 【逐行批注】 执行当前声明或表达式：};。
};

// 【逐行批注】 结束当前代码块。
}
