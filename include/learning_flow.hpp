#pragma once

#include "ai_client.hpp"
#include "db.hpp"
#include <nlohmann/json.hpp>
#include <atomic>
#include <functional>
#include <string>
#include <thread>

namespace gangyi {

using FlowJson = nlohmann::json;
std::string learningContext(Database& db, const std::string& courseId = {});
FlowJson studyPlanView(Database& db);
FlowJson editStudyPlan(Database& db, const FlowJson& body);
FlowJson studyPlanProposal(Database& db, const FlowJson& body, const std::string& action);
FlowJson studyPlanDraft(Database& db, const FlowJson& body);
FlowJson preparationView(Database& db, const std::string& courseId = {});
FlowJson nextStepView(Database& db, const std::string& courseId = {});
FlowJson coursePreviewView(Database& db, const std::string& courseId);
FlowJson requestCoursePreview(Database& db, const std::string& courseId, bool retry = false);
FlowJson exposeLearningBlocks(Database& db, const FlowJson& body);
FlowJson dialogueView(Database& db, const FlowJson& body);
FlowJson beginDialogue(Database& db, const FlowJson& body);
ChatOptions dialogueOptions(Database& db, const FlowJson& body, const FlowJson& turn);
FlowJson finishDialogue(Database& db, const FlowJson& body, const FlowJson& turn,
                        const std::string& answer, const std::string& model,
                        const std::string& failure = {});

class LearningFlow {
public:
    explicit LearningFlow(std::string databasePath);
    ~LearningFlow();
    void start();
    void stop();
private:
    void run();
    void evaluate(Database& db, const ClassroomActivity& activity);
    void preview(Database& db, const ClassroomActivity& activity);
    void coordinate(Database& db, int revision);
    void prepare(Database& db, const FlowJson& teaching, int revision);
    AIResult call(const std::string& system, const FlowJson& input);
    std::string databasePath_;
    std::atomic_bool stopped_{false};
    std::thread worker_;
};
}
