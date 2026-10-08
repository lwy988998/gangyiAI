#include "agent_preferences.hpp"
#include "agent_curriculum.hpp"
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <map>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>

namespace gangyi {
namespace {
using Json = nlohmann::json;
Json parse(const std::string& text, Json fallback = Json::object()) {
    const auto value = Json::parse(text, nullptr, false);
    return value.is_discarded() ? fallback : value;
}
std::string fingerprint(const std::string& text) {
    unsigned long long value = 14695981039346656037ULL;
    for (const unsigned char byte : text) { value ^= byte; value *= 1099511628211ULL; }
    std::ostringstream out; out << std::hex << value; return out.str();
}
std::string now() {
    const auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm value{};
#ifdef _WIN32
    gmtime_s(&value, &time);
#else
    gmtime_r(&time, &value);
#endif
    std::ostringstream out; out << std::put_time(&value, "%Y-%m-%dT%H:%M:%SZ"); return out.str();
}
std::string newId() {
    static thread_local std::mt19937_64 generator(std::random_device{}());
    std::ostringstream out; out << "change-" << std::hex << generator() << generator(); return out.str();
}
AgentAccess accessFor(const Json& task) {
    return {task.at("scopeId"), task.at("courseIds").get<std::vector<std::string>>()};
}
bool allowed(const AgentAccess& access, const std::string& courseId) {
    return std::find(access.courseIds.begin(), access.courseIds.end(), courseId) != access.courseIds.end();
}
Course requireCourse(Database& db, const AgentAccess& access, const std::string& courseId) {
    const auto found = db.getCourse(courseId);
    if (!found || found->status != "active" || !allowed(access, courseId))
        throw std::invalid_argument("课程不存在或无权修改");
    return *found;
}
std::string requireText(const Json& value, const char* key) {
    if (!value.value(key, Json()).is_string() || value[key].get<std::string>().empty())
        throw std::invalid_argument(std::string(key) + " 不能为空");
    return value[key];
}
std::string recommendationKey(const AgentAccess& access) { return "agent-recommendations:" + fingerprint(access.scopeId); }
std::vector<std::string> changeIds(Database& db) {
    const auto ids = parse(db.profileMeta("agent-changes"), Json::array());
    return ids.is_array() ? ids.get<std::vector<std::string>>() : std::vector<std::string>{};
}
Json journal(Database& db, const Json& task, const AIResult& source, const Json& args,
             const std::string& kind, const std::string& target, const Json& before, const Json& after) {
    const auto access = accessFor(task);
    auto semantic = args; semantic.erase("reason"); semantic.erase("version");
    const auto signature = fingerprint(Json{{"kind", kind}, {"target", target}, {"args", semantic}}.dump());
    const auto learner = agentLearnerFingerprint(db, access);
    for (const auto& id : changeIds(db)) {
        const auto row = db.getClassroomActivity(id); if (!row || row->kind != "agent-change") continue;
        const auto old = parse(row->payload);
        if (old.value("scopeId", "") == access.scopeId && old.value("status", "") == "undone" &&
            old.value("signature", "") == signature && old.value("undoFingerprint", "") == learner)
            throw std::invalid_argument("用户已撤回同一改动；等待新的学习反馈后再决定，不能立即重新应用");
    }
    return {{"id", newId()}, {"scopeId", access.scopeId}, {"taskId", task.at("id")},
        {"kind", kind}, {"target", target}, {"before", before}, {"after", after}, {"signature", signature},
        {"reason", requireText(args, "reason")}, {"model", source.model}, {"createdAt", now()},
        {"status", "applied"}, {"sourceLearningVersion", task.at("learningVersion")}};
}
Json publicChange(const Json& change) {
    Json value;
    for (const auto* key : {"id", "kind", "target", "reason", "model", "createdAt", "status", "undoneAt"})
        if (change.contains(key)) value[key] = change[key];
    return value;
}
void saveChange(Database& db, const Json& change, const std::string& courseId = {}) {
    if (!db.upsert(ClassroomActivity{change.at("id"), courseId, "agent-change", change.dump(), now(), 0, 0}))
        throw std::runtime_error("调整记录保存失败");
    auto ids = changeIds(db);
    if (std::find(ids.begin(), ids.end(), change.at("id")) == ids.end()) ids.push_back(change.at("id"));
    db.setProfileMeta("agent-changes", Json(ids).dump());
}
std::tm dateValue(const std::string& date) {
    std::tm value{}; std::istringstream input(date); input >> std::get_time(&value, "%Y-%m-%d");
    if (input.fail() || date.size() != 10) throw std::invalid_argument("学习日期无效");
    value.tm_hour = 12; std::mktime(&value);
    std::ostringstream normalized; normalized << std::put_time(&value, "%Y-%m-%d");
    if (normalized.str() != date) throw std::invalid_argument("学习日期不存在");
    return value;
}
std::string today() {
    const auto clock = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm value{};
#ifdef _WIN32
    localtime_s(&value, &clock);
#else
    localtime_r(&clock, &value);
#endif
    std::ostringstream out; out << std::put_time(&value, "%Y-%m-%d"); return out.str();
}
bool samePosition(const Json& a, const Json& b) {
    return a.value("date", "") == b.value("date", "") && a.value("minutes", 0) == b.value("minutes", 0) &&
        a.value("order", 0) == b.value("order", 0);
}
void validatePlan(Database& db, const AgentAccess& access, const Json& old, const Json& plan) {
    if (!plan.value("availability", Json()).is_array() || !plan.value("entries", Json()).is_array())
        throw std::invalid_argument("学习时间与任务列表无效");
    std::map<int, int> budgets;
    for (const auto& slot : plan["availability"]) {
        if (!slot.value("weekday", Json()).is_number_integer() || !slot.value("minutes", Json()).is_number_integer())
            throw std::invalid_argument("可用时间格式无效");
        const int day = slot["weekday"], minutes = slot["minutes"];
        if (day < 1 || day > 7 || minutes < 1 || minutes > 1440 || budgets.count(day))
            throw std::invalid_argument("学习日应为 1 至 7，时长应在一天以内，且不能重复");
        budgets[day] = minutes;
    }
    std::map<std::string, int> totals;
    std::map<std::string, Json> entries;
    for (const auto& item : plan["entries"]) {
        const auto courseId = requireText(item, "courseId"); requireCourse(db, access, courseId);
        const auto id = requireText(item, "taskId"), date = requireText(item, "date");
        const auto stamp = dateValue(date);
        if (!item.value("minutes", Json()).is_number_integer() || item["minutes"].get<int>() < 1 ||
            item["minutes"].get<int>() > 1440 || entries.count(id)) throw std::invalid_argument("任务时长或身份无效");
        const auto kind = item.value("kind", "lesson");
        bool valid = false;
        if (kind == "review") {
            const auto review = db.getClassroomActivity(item.value("reviewId", ""));
            valid = review && review->courseId == courseId && review->kind == "review" && id == "review:" + review->id;
        } else if (kind == "agent-lesson") {
            const auto lesson = db.getClassroomActivity(item.value("lessonId", ""));
            valid = lesson && lesson->courseId == courseId && lesson->kind == "agent-lesson" && id == "agent-lesson:" + lesson->id;
        } else if (kind == "lesson") {
            const int phase = item.value("phaseIndex", 0), topic = item.value("topicIndex", 0);
            Json outline; int version = 0;
            for (const auto& snapshot : db.findSnapshotsByCourseId(courseId)) if (snapshot.version > version) {
                outline = parse(snapshot.payload); version = snapshot.version;
            }
            const auto stages = outline.value("courseStructure", Json::array());
            valid = phase >= 1 && phase <= static_cast<int>(stages.size()) && topic >= 1 &&
                topic <= static_cast<int>(stages[phase - 1].value("topics", Json::array()).size()) &&
                id == "lesson:" + courseId + ":" + std::to_string(phase) + ":" + std::to_string(topic);
        }
        if (!valid) throw std::invalid_argument("课表中的任务身份不存在；请使用 scheduleTasks 中配套的 taskId、kind、courseId 和课时索引，不把备课任务 ID 当作课时 ID");
        entries[id] = item;
        const auto actualProgress = db.findLearningCardProgress(courseId, item.value("phaseIndex", 0), item.value("topicIndex", 0));
        const bool actuallyCompleted = kind == "lesson" && actualProgress && actualProgress->status == "completed";
        if (item.value("completed", false) && !actuallyCompleted) throw std::invalid_argument("不能把未完成任务标成已完成以绕过预算");
        if (!actuallyCompleted && date >= today()) {
            const auto day = stamp.tm_wday == 0 ? 7 : stamp.tm_wday;
            totals[date] += item["minutes"].get<int>();
            if (totals[date] > budgets[day]) throw std::invalid_argument("当天全部课程的时长超过共享预算");
        }
    }
    for (const auto& previous : old.value("entries", Json::array())) {
        const auto progress = db.findLearningCardProgress(previous.value("courseId", ""),
            previous.value("phaseIndex", 0), previous.value("topicIndex", 0));
        const bool stable = previous.value("completed", false) || previous.value("inProgress", false) ||
            (progress && progress->status != "not_started");
        if (stable && (!entries.count(previous.value("taskId", "")) || !samePosition(previous, entries.at(previous["taskId"]))))
            throw std::invalid_argument("已完成或正在学习的任务必须保留原位置和时长");
    }
    for (const auto& [id, item] : entries) {
        const auto requirement = item.value("requires", "");
        if (!requirement.empty() && (!entries.count(requirement) ||
            entries.at(requirement).value("date", "") > item.value("date", "") ||
            (entries.at(requirement).value("date", "") == item.value("date", "") &&
                entries.at(requirement).value("order", 0) > item.value("order", 0))))
            throw std::invalid_argument("安排顺序不满足明确的前置关系");
        if (item.value("date", "") < today()) {
            bool historical = false;
            for (const auto& previous : old.value("entries", Json::array()))
                if (previous.value("taskId", "") == id && samePosition(previous, item)) historical = true;
            if (!historical) throw std::invalid_argument("新的待学习任务不能安排到过去");
        }
    }
}
}

Json agentChanges(Database& db, const AgentAccess& access) {
    Json result = Json::array();
    for (const auto& id : changeIds(db)) {
        const auto row = db.getClassroomActivity(id); if (!row) continue;
        const auto value = parse(row->payload);
        if (value.value("scopeId", "") == access.scopeId) result.push_back(publicChange(value));
    }
    return result;
}
Json agentChangeRecord(Database& db, const Json& task, const AIResult& source, const Json& args,
    const std::string& kind, const std::string& target, const Json& before, const Json& after) {
    return journal(db, task, source, args, kind, target, before, after);
}
void agentSaveChange(Database& db, const Json& change, const std::string& courseId) { saveChange(db, change, courseId); }
Json agentRecommendationView(Database& db, const AgentAccess& access) {
    return parse(db.profileMeta(recommendationKey(access)));
}

void registerAgentPreferenceTools(LearningAgent& agent) {
    agent.registerTool("adjust_goal", {"自主调整可访问课程目标。参数 courseId、goal、reason；不修改历史课程身份，改动可撤回。", false,
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            const auto access = accessFor(task);
            const auto course = requireCourse(db, access, requireText(args, "courseId"));
            const auto goal = requireText(args, "goal");
            if (goal == course.goal) return PreparedAgentTool{{{"changed", false}}, {}};
            const auto change = journal(db, task, source, args, "goal", course.id,
                Json{{"goal", course.goal}, {"updatedAt", course.updatedAt}}, Json{{"goal", goal}, {"updatedAt", now()}});
            return PreparedAgentTool{{{"changed", true}, {"change", publicChange(change)}},
                [course, change, access](Database& connection) {
                    auto current = requireCourse(connection, access, course.id);
                    if (current.goal != course.goal || current.updatedAt != course.updatedAt)
                        throw std::invalid_argument("课程目标已经变化，旧调整不应用");
                    current.goal = change["after"]["goal"]; current.updatedAt = change["after"]["updatedAt"];
                    if (!connection.update(current)) throw std::runtime_error("目标保存失败");
                    connection.markLearningDirty(); saveChange(connection, change, course.id);
                }};
        }});
    agent.registerTool("adjust_schedule", {"仅在用户手动重新排课的任务中生成统一课表候选。参数 availability:[{weekday:1..7,minutes:1..1440}]、entries:[{taskId,courseId,kind,date,minutes,order,phaseIndex,topicIndex,requires}]、reason。使用上下文 scheduleTasks 中真实存在的任务身份：大纲知识点 kind=lesson，taskId=lesson:<courseId>:<phaseIndex>:<topicIndex>；已备课 kind=agent-lesson，taskId=agent-lesson:<lessonId>，附 lessonId；复习 kind=review，taskId=review:<reviewId>，附 reviewId。不可用 agent 备课任务 ID 或裸课时 ID 替代。calendarDate 是本机今天，新安排不得放在过去。始终保留原课表及编辑草稿，等待用户确认；不得直接应用。保留已完成和在学任务；日总量不可超预算。", false,
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            if (!task.value("manualScheduleReplan", false) || task.at("event").value("type", "") != "schedule_replan")
                throw std::invalid_argument("只有用户手动重新排课的任务可以生成课表候选");
            const auto access = accessFor(task);
            for (const auto& course : db.listCourses()) if (course.status == "active" && !allowed(access, course.id))
                throw std::invalid_argument("无权修改其他用户课程的共享预算");
            const auto raw = db.profileMeta("learning-flow"); auto state = parse(raw);
            auto old = state.value("plan", Json::object());
            Json plan = old; plan["availability"] = args.at("availability"); plan["entries"] = args.at("entries");
            validatePlan(db, access, old, plan);
            // 页面标题来自实际课程与课件，不要求模型重复抄写，也不显示内部标识。
            for (auto& entry : plan["entries"]) {
                const auto courseId = entry.at("courseId").get<std::string>();
                const auto course = requireCourse(db, access, courseId);
                entry["courseTitle"] = course.title;
                std::string title;
                const auto kind = entry.value("kind", "lesson");
                if (kind == "agent-lesson" || kind == "review") {
                    const auto saved = db.getClassroomActivity(entry.at(kind == "review" ? "reviewId" : "lessonId"));
                    if (!saved) throw std::invalid_argument("课时已变化，请重新读取任务目录");
                    title = parse(saved->payload).value("title", "");
                } else {
                    const auto snapshots = db.findSnapshotsByCourseId(courseId);
                    if (snapshots.empty()) throw std::invalid_argument("课程大纲已变化，请重新读取任务目录");
                    const auto latest = std::max_element(snapshots.begin(), snapshots.end(),
                        [](const auto& a, const auto& b) { return a.version < b.version; });
                    const auto topic = parse(latest->payload).at("courseStructure").at(entry.at("phaseIndex").get<int>() - 1)
                        .at("topics").at(entry.at("topicIndex").get<int>() - 1);
                    title = topic.is_string() ? topic.get<std::string>() : topic.is_object() ? topic.value("title", "") : "";
                }
                entry["title"] = title.empty() ? course.title : title;
            }
            // 标题中的日期范围跟随候选任务，避免新安排仍显示旧课表的过期区间。
            if (!plan["entries"].empty()) {
                std::string first, last;
                for (const auto& entry : plan["entries"]) {
                    const auto date = entry.at("date").get<std::string>();
                    if (first.empty() || date < first) first = date;
                    if (last.empty() || date > last) last = date;
                }
                plan["weekStart"] = first; plan["weekEnd"] = last;
            }
            plan["version"] = old.value("version", 0) + 1; plan["source"] = "ai"; plan["updatedAt"] = now();
            auto after = state; after["plan"] = plan; after["status"] = "ready"; after["model"] = source.model; after["agentControlled"] = true;
            after["reason"] = requireText(args, "reason"); after.erase("proposal");
            const auto change = journal(db, task, source, args, "schedule", "learning-flow", raw, after.dump());
            const auto draftRaw = db.profileMeta("study-drafts");
            const auto drafts = parse(draftRaw);
            if (!drafts.is_object()) throw std::invalid_argument("study-drafts 必须为对象，原编辑内容未改动");
            return PreparedAgentTool{{{"changed", false}, {"draftProtected", true}, {"proposalId", change.at("id")},
                {"message", "AI 候选安排已保存，等待用户确认，原课表和编辑内容保留。"}},
                [after, draftRaw, raw, old, change, taskId = task.at("id").get<std::string>()](Database& connection) {
                    const auto taskRow = connection.getClassroomActivity(taskId);
                    if (!taskRow || !parse(taskRow->payload).value("manualScheduleReplan", false))
                        throw std::invalid_argument("本次任务没有手动排课授权");
                    if (connection.profileMeta("study-drafts") != draftRaw) throw std::invalid_argument("编辑状态已经变化");
                    connection.setProfileMeta("agent-schedule-proposal", after.dump());
                    auto candidate = change; candidate["status"] = "candidate";
                    auto state = parse(raw); state["agentControlled"] = true; state["status"] = "ready";
                    state["proposal"] = {{"id", change.at("id")}, {"plan", after.at("plan")}, {"previousPlan", old},
                        {"reason", change.at("reason")}, {"learningVersion", connection.learningRevision()}};
                    state["message"] = "候选已生成，请预览并确认；原课表和编辑内容已保留。";
                    if (!connection.compareProfileMeta("learning-flow", raw, state.dump())) throw std::invalid_argument("课表版本已变化");
                    saveChange(connection, candidate);
                }};
        }});
    agent.registerTool("update_recommendations", {"根据最新真实表现决定更新首页推荐；无需固定补弱与探索比例。参数 lite、deep 两组非重复纯文本目标，以及 reason；每组数量自行决定。", false,
        [](Database& db, const Json& args, const Json& task, const AIResult& source) {
            for (const auto* mode : {"lite", "deep"}) {
                if (!args.value(mode, Json()).is_array() || args[mode].empty()) throw std::invalid_argument("每组推荐须有实际目标");
                std::set<std::string> seen;
                for (const auto& item : args[mode]) if (!item.is_string() || item.get<std::string>().empty() ||
                    !seen.insert(item.get<std::string>()).second) throw std::invalid_argument("推荐文字无效或重复");
            }
            const auto key = recommendationKey(accessFor(task)), raw = db.profileMeta(key);
            const auto value = Json{{"status", "ready"}, {"items", {{"lite", args["lite"]}, {"deep", args["deep"]}}},
                {"reason", requireText(args, "reason")}, {"model", source.model}, {"generatedAt", now()},
                {"learningVersion", task.at("learningVersion")}};
            return PreparedAgentTool{{{"updated", true}}, [raw, value, key](Database& connection) {
                if (!connection.compareProfileMeta(key, raw, value.dump())) throw std::invalid_argument("推荐已被其他任务更新");
            }};
        }});
}

Json agentUndoChange(Database& db, const AgentAccess& access, const Json& body) {
    Json change;
    db.transaction([&] {
        const auto row = db.getClassroomActivity(requireText(body, "changeId"));
        if (!row || row->kind != "agent-change") throw std::invalid_argument("调整记录不存在");
        change = parse(row->payload);
        if (change.value("scopeId", "") != access.scopeId) throw std::invalid_argument("无权撤回该调整");
        if (change.value("status", "") == "undone") return;
        if (change.at("kind") == "goal") {
            auto course = requireCourse(db, access, change.at("target"));
            if (course.goal != change["after"]["goal"] || course.updatedAt != change["after"]["updatedAt"])
                throw std::invalid_argument("目标已有后续修改，撤回不能覆盖新的决定");
            course.goal = change["before"]["goal"]; course.updatedAt = now();
            if (!db.update(course)) throw std::runtime_error("目标撤回失败");
        } else if (change.at("kind") == "schedule") {
            const auto key = change.at("target").get<std::string>(), raw = db.profileMeta(key);
            if (raw != change["after"]) throw std::invalid_argument("课表已有后续修改，撤回不能覆盖新的安排");
            auto before = parse(change["before"]), after = parse(raw);
            before["plan"]["version"] = after["plan"].value("version", 0) + 1;
            before["plan"]["updatedAt"] = now();
            if (!db.compareProfileMeta(key, raw, before.dump())) throw std::invalid_argument("撤回时课表已经变化");
        } else if (change.at("kind") == "outline") {
            const auto courseId = change.at("target").get<std::string>(); requireCourse(db, access, courseId);
            const auto snapshots = db.findSnapshotsByCourseId(courseId);
            if (snapshots.empty()) throw std::invalid_argument("大纲不存在");
            const auto latest = std::max_element(snapshots.begin(), snapshots.end(), [](const auto& a, const auto& b) { return a.version < b.version; });
            if (parse(latest->payload) != change.at("after")) throw std::invalid_argument("大纲已有后续修改，不能覆盖");
            agentValidateOutline(db, courseId, change.at("after"), change.at("before"));
            if (!db.insert(CourseSnapshot{newId(), courseId, latest->version + 1, change.at("before").dump(), now()}))
                throw std::runtime_error("大纲撤回失败");
        } else throw std::invalid_argument("该调整暂不支持撤回");
        db.markLearningDirty(); change["status"] = "undone"; change["undoneAt"] = now();
        change["undoFingerprint"] = agentLearnerFingerprint(db, access);
        saveChange(db, change, row->courseId);
    });
    return agentSubmit(db, access, {{"type", "undo"}, {"requestId", "undo-" + change.at("id").get<std::string>()},
        {"change", publicChange(change)}, {"text", "我撤回了这次调整，请尊重我的反馈，不立即重复相同改动。"}});
}
}
