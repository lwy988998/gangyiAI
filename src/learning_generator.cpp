#include "learning_generator.hpp"

#include "json_fix.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace gangyi {
namespace {

using json = nlohmann::json;

std::string trim(const std::string& value) {
    const auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char c) {
        return std::isspace(c);
    });
    if (first == value.end()) return {};
    const auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char c) {
        return std::isspace(c);
    }).base();
    return std::string(first, last);
}

std::string truncateUtf8(const std::string& value, std::size_t maxBytes) {
    if (value.size() <= maxBytes) return value;
    std::size_t cut = maxBytes;
    while (cut > 0 && cut < value.size() &&
           (static_cast<unsigned char>(value[cut]) & 0xC0) == 0x80) {
        --cut;
    }
    return value.substr(0, cut);
}

std::string stringValue(const json& object, const char* key, const std::string& fallback = {}) {
    if (!object.is_object()) return fallback;
    const auto it = object.find(key);
    if (it == object.end() || !it->is_string()) return fallback;
    const std::string value = trim(it->get<std::string>());
    return value.empty() ? fallback : value;
}

std::vector<std::string> stringArrayValue(const json& object,
                                         const char* key,
                                         const std::vector<std::string>& fallback = {}) {
    if (!object.is_object()) return fallback;
    const auto it = object.find(key);
    if (it == object.end() || !it->is_array()) return fallback;

    std::vector<std::string> result;
    for (const auto& item : *it) {
        if (!item.is_string()) continue;
        std::string value = trim(item.get<std::string>());
        if (!value.empty()) result.push_back(std::move(value));
    }
    return result.empty() ? fallback : result;
}

int intValue(const json& object, const char* key, int fallback = -1) {
    if (!object.is_object()) return fallback;
    const auto it = object.find(key);
    if (it == object.end()) return fallback;
    if (it->is_number_integer()) return it->get<int>();
    if (it->is_string()) {
        try {
            std::size_t parsed = 0;
            const int value = std::stoi(trim(it->get<std::string>()), &parsed);
            return parsed == trim(it->get<std::string>()).size() ? value : fallback;
        } catch (const std::exception&) {
            return fallback;
        }
    }
    return fallback;
}

bool isLiteMode(const std::string& mode) {
    return mode == "lite";
}

std::string normalizeMode(const std::string& mode) {
    return isLiteMode(trim(mode)) ? "lite" : "deep";
}

std::vector<SearchResource> limitResources(const std::vector<SearchResource>& resources) {
    const std::size_t count = std::min<std::size_t>(resources.size(), 8);
    return std::vector<SearchResource>(resources.begin(), resources.begin() + static_cast<std::ptrdiff_t>(count));
}

std::vector<Reference> referencesFromResources(const std::vector<SearchResource>& resources) {
    std::vector<Reference> references;
    std::unordered_set<std::string> seen;
    for (const auto& resource : resources) {
        const std::string url = trim(resource.url);
        if (url.empty() || !seen.insert(url).second) continue;
        references.push_back({trim(resource.title), trim(resource.source), url, trim(resource.type)});
    }
    return references;
}

std::vector<Reference> filterReferences(const json& value, const std::vector<SearchResource>& resources) {
    const std::vector<Reference> fallback = referencesFromResources(resources);
    if (!value.is_array()) return fallback;

    std::unordered_map<std::string, SearchResource> allowedByUrl;
    for (const auto& resource : resources) {
        const std::string url = trim(resource.url);
        if (!url.empty()) allowedByUrl.emplace(url, resource);
    }

    std::vector<Reference> references;
    std::unordered_set<std::string> seen;
    for (const auto& item : value) {
        if (!item.is_object()) continue;
        const std::string url = stringValue(item, "url");
        const auto allowed = allowedByUrl.find(url);
        if (allowed == allowedByUrl.end() || !seen.insert(url).second) continue;

        const SearchResource& resource = allowed->second;
        references.push_back({
            stringValue(item, "title", trim(resource.title)),
            stringValue(item, "source", trim(resource.source)),
            url,
            stringValue(item, "type", trim(resource.type)),
        });
    }
    return references.empty() ? fallback : references;
}

std::vector<LessonStep> lessonStepsValue(const json& object, const char* key) {
    if (!object.is_object()) return {};
    const auto it = object.find(key);
    if (it == object.end() || !it->is_array()) return {};

    std::vector<LessonStep> result;
    for (const auto& item : *it) {
        if (!item.is_object()) continue;
        LessonStep step{
            stringValue(item, "title"),
            stringValue(item, "explanation"),
            stringValue(item, "example"),
            stringValue(item, "action"),
            stringValue(item, "check"),
        };
        if (!step.title.empty() && !step.explanation.empty()) result.push_back(std::move(step));
    }
    return result;
}

std::vector<Example> examplesValue(const json& object, const char* key) {
    if (!object.is_object()) return {};
    const auto it = object.find(key);
    if (it == object.end() || !it->is_array()) return {};

    std::vector<Example> result;
    for (const auto& item : *it) {
        if (!item.is_object()) continue;
        Example example{
            stringValue(item, "title"),
            stringValue(item, "content"),
            stringArrayValue(item, "solution"),
        };
        if (!example.title.empty() && !example.content.empty()) result.push_back(std::move(example));
    }
    return result;
}

std::vector<Practice> practiceValue(const json& object, const char* key) {
    if (!object.is_object()) return {};
    const auto it = object.find(key);
    if (it == object.end() || !it->is_array()) return {};

    std::vector<Practice> result;
    for (const auto& item : *it) {
        if (!item.is_object()) continue;
        Practice practice{
            stringValue(item, "title"),
            stringValue(item, "difficulty", u8"入门"),
            stringValue(item, "task"),
            stringValue(item, "check"),
        };
        if (!practice.title.empty() && !practice.task.empty()) result.push_back(std::move(practice));
    }
    return result;
}

std::vector<Quiz> quizValue(const json& object, const char* key) {
    if (!object.is_object()) return {};
    const auto it = object.find(key);
    if (it == object.end() || !it->is_array()) return {};

    std::vector<Quiz> result;
    for (const auto& item : *it) {
        if (!item.is_object()) continue;
        Quiz quiz;
        quiz.question = stringValue(item, "question");
        quiz.options = stringArrayValue(item, "options");
        quiz.answerIndex = intValue(item, "answerIndex");
        quiz.explanation = stringValue(item, "explanation");
        result.push_back(std::move(quiz));
    }
    return result;
}

std::vector<json> normalizeResources(const std::vector<SearchResource>& resources, const std::string& mode) {
    const std::size_t limit = isLiteMode(mode) ? 4 : 6;
    const std::size_t descriptionLength = isLiteMode(mode) ? 260 : 360;
    std::vector<json> briefs;
    const std::size_t count = std::min<std::size_t>(resources.size(), limit);

    briefs.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        const auto& resource = resources[i];
        briefs.push_back({
            {"title", truncateUtf8(trim(resource.title), 120)},
            {"source", truncateUtf8(trim(resource.source), 80)},
            {"description", truncateUtf8(trim(resource.description), descriptionLength)},
            {"url", trim(resource.url)},
            {"type", trim(resource.type)},
        });
    }
    return briefs;
}

json outputSchemaJson() {
    return {
        {"inferredDomain", "string"},
        {"title", "string"},
        {"summary", "string"},
        {"keyConcepts", json::array({"string"})},
        {"lessonSteps", json::array({{{"title", "string"}, {"explanation", "string"}, {"example", "string"}, {"action", "string"}, {"check", "string"}}})},
        {"examples", json::array({{{"title", "string"}, {"content", "string"}, {"solution", json::array({"string"})}}})},
        {"practice", json::array({{{"title", "string"}, {"difficulty", "string"}, {"task", "string"}, {"check", "string"}}})},
        {"quiz", json::array({{{"question", "string"}, {"options", json::array({"A", "B", "C", "D"})}, {"answerIndex", 0}, {"explanation", "string"}}})},
        {"commonMistakes", json::array({"string"})},
        {"checkpoint", json::array({"string"})},
        {"resourceSummary", "string"},
        {"references", json::array({{{"title", "string"}, {"source", "string"}, {"url", "string"}, {"type", "string"}}})},
    };
}

std::vector<ChatMessage> createLearningPromptMessages(const std::string& goal,
                                                       const std::string& phaseName,
                                                       const std::string& topic,
                                                       const std::string& mode,
                                                       const std::vector<SearchResource>& resources,
                                                       bool retry) {
    const std::string stepRange = isLiteMode(mode) ? "3-4" : "5-6";
    const std::string stepLength = isLiteMode(mode) ? "90-140" : "160-260";
    const std::string practiceCount = isLiteMode(mode) ? "2-3" : "3-5";

    json user = {
        {"task", u8"根据高中学习目标、阶段和知识点，生成资料整合后的高质量微课程。"},
        {"teachingStyle", json::array({
             u8"通俗、具体、像老师在讲课",
             u8"只讲高中语文、数学、英语、物理、化学、生物、历史、地理、政治或信息技术",
             u8"围绕 topic，不跑题，紧贴教材知识点、题型或实验",
             u8"适合高中生，兼顾理解、练习和考试检查",
             u8"每段都要能帮助用户真正学会或完成练习",
         })},
        {"avoid", json::array({
             u8"深入学习相关知识", u8"掌握基本概念", u8"多加练习", u8"参考相关资料", u8"提升综合能力",
             u8"只给链接", u8"只列大纲", u8"课程导入", u8"关键抓手", u8"不要只背名词",
             u8"至少完成一次解释和练习", u8"理解阶段目标", u8"用练习把知识变成能力",
             u8"复盘并形成阶段产出", "fallback", "mock", "demo", "debug",
         })},
        {"requirements", {
             {"summary", u8"说明这节课解决什么问题、适合谁、学完能做什么。"},
             {"keyConcepts", u8"3-6 个核心概念；每个概念名称要具体，不要空泛。"},
             {"lessonSteps", stepRange + u8" 步，每步 explanation " + stepLength + u8" 字，必须包含 title/explanation/example/action/check；按“解释-例子-行动-检查”组织。"},
             {"examples", u8"至少 1-2 个强相关高中题目、文本、实验或材料示例；必须有分步分析或解法。"},
             {"practice", practiceCount + u8" 个练习，带难度梯度；task 要写清楚要做什么；check 要写可验证结果，可包含提示或参考答案。"},
             {"quiz", u8"3-5 道选择题；每题 4 个非空选项；answerIndex 必须唯一且和 explanation 一致；干扰项不能过于离谱。"},
             {"commonMistakes", u8"3-5 个常见误区，必须和 topic 强相关。"},
             {"checkpoint", u8"3-5 条可衡量学习目标/完成标准。"},
             {"resourceSummary", u8"用一小段话说明你如何综合了资料摘要，不要罗列搜索结果。"},
             {"references", u8"只能引用输入 resources 里的 title/source/url/type，不得新增链接。"},
         }},
        {"outputSchema", outputSchemaJson()},
        {"goal", goal},
        {"phaseName", phaseName},
        {"topic", topic},
        {"mode", mode},
        {"resources", normalizeResources(resources, mode)},
    };

    std::string userContent = user.dump();
    if (retry) {
        userContent += u8"\n\n请修复上一轮输出：只返回一个可解析的严格 JSON 对象；title 和 summary 必须非空；lessonSteps 数量必须达标；quiz 每题必须恰好 4 个非空 options，answerIndex 必须为 0-3；references.url 只能从本次 resources.url 中选择；不要输出 Markdown、解释文字或代码块。";
    }

    return {
        {"system", u8"你是钢一定制AI高中学习导师，已经拿到联网搜索资料摘要。只讲高中语文、数学、英语、物理、化学、生物、历史、地理、政治或信息技术；围绕教材知识点、题型、实验、阅读与写作，整合资料生成一节完整微课程。正文必须像高中老师讲课：为什么学 -> 是什么 -> 怎样分析或解题 -> 具体例子 -> 可执行练习 -> 小测验 -> 总结。不要生成 Web 开发、网站制作、职业技能或无关内容。不要直接复制资料，不要把搜索结果列表当正文，不要空泛鸡汤，不要只列大纲。references 必须只来自用户提供的 resources.url，且只放在最后参考资料。只输出严格 JSON。"},
        {"user", std::move(userContent)},
    };
}

LearningAnswer adaptLearningAnswer(const json& raw,
                                   const LearningAnswer& fallback,
                                   const std::vector<SearchResource>& resources) {
    if (!raw.is_object()) throw AIClientError("invalid_response", "Learning answer schema invalid");

    LearningAnswer answer;
    answer.inferredDomain = stringValue(raw, "inferredDomain", fallback.inferredDomain);
    answer.title = stringValue(raw, "title", fallback.title);
    answer.summary = stringValue(raw, "summary", fallback.summary);
    answer.keyConcepts = stringArrayValue(raw, "keyConcepts", fallback.keyConcepts);
    answer.lessonSteps = lessonStepsValue(raw, "lessonSteps");
    answer.examples = examplesValue(raw, "examples");
    answer.practice = practiceValue(raw, "practice");
    answer.quiz = quizValue(raw, "quiz");
    answer.commonMistakes = stringArrayValue(raw, "commonMistakes", fallback.commonMistakes);
    answer.checkpoint = stringArrayValue(raw, "checkpoint", fallback.checkpoint);
    answer.resourceSummary = stringValue(raw, "resourceSummary", fallback.resourceSummary);
    answer.references = filterReferences(raw.value("references", json::array()), resources);

    if (answer.lessonSteps.empty()) answer.lessonSteps = fallback.lessonSteps;
    if (answer.examples.empty()) answer.examples = fallback.examples;
    if (answer.practice.empty()) answer.practice = fallback.practice;
    if (answer.quiz.empty()) answer.quiz = fallback.quiz;
    if (raw.contains("notice")) answer.notice = stringValue(raw, "notice");
    return answer;
}

void validateLearningAnswer(const LearningAnswer& answer, const std::string& mode) {
    if (trim(answer.title).empty()) throw AIClientError("quality_rejected", "learning title is empty");
    if (trim(answer.summary).empty()) throw AIClientError("quality_rejected", "learning summary is empty");

    const std::size_t minSteps = isLiteMode(mode) ? 2 : 3;
    if (answer.lessonSteps.size() < minSteps) {
        throw AIClientError("quality_rejected", "learning lessonSteps count is too low");
    }

    for (const auto& quiz : answer.quiz) {
        if (quiz.options.size() != 4 || quiz.answerIndex < 0 || quiz.answerIndex >= 4) {
            throw AIClientError("quality_rejected", "learning quiz schema invalid");
        }
        if (std::any_of(quiz.options.begin(), quiz.options.end(), [](const std::string& option) {
                return trim(option).empty();
            })) {
            throw AIClientError("quality_rejected", "learning quiz option is empty");
        }
    }
}

LearningAnswer fallbackAnswer(const std::string& goal,
                              const std::string& phaseName,
                              const std::string& topic,
                              const std::vector<SearchResource>& resources,
                              const std::string& reason) {
    LearningAnswer answer;
    answer.title = topic + u8"：学习单";
    answer.summary = u8"本节已切换为可直接完成的学习单：先用自己的话说清概念，再用一个例子验证，最后完成练习和小测。";
    if (!topic.empty()) answer.keyConcepts.push_back(topic);
    if (!phaseName.empty()) answer.keyConcepts.push_back(phaseName);
    if (answer.keyConcepts.empty() && !goal.empty()) answer.keyConcepts.push_back(goal);
    answer.keyConcepts.push_back(u8"定义、条件与应用");
    answer.lessonSteps = {
        {u8"先说清研究对象", u8"把“" + topic + u8"”写在纸上，用一句完整的话说明它研究什么、解决什么问题。不要照抄标题，句子中要有对象和结果。", u8"可以使用“当……时，……表示/说明……”的句式。", u8"写出一句不超过 40 字的定义。", u8"定义中同时出现研究对象和作用。"},
        {u8"找出成立条件", u8"任何知识点都有适用范围。回看教材或课堂笔记，列出它成立时必须满足的条件、数据、背景或步骤。", u8"把条件分成“已知信息”和“需要判断的信息”两列。", u8"列出至少 2 条条件，并标出各自来源。", u8"每条条件都能在材料、题干或教材中找到依据。"},
        {u8"用一个例子验证", u8"选择一道教材例题、一道练习题或一个生活/实验材料，按“信息—方法—结论”三步解释它和本主题的关系。", u8"先圈出材料中的关键词，再决定使用哪条定义或条件。", u8"完成一份三步分析。", u8"每一步都写出理由，不只写最后答案。"},
        {u8"做一次自我检查", u8"合上资料后复述本节内容，并检查自己能否说明：它是什么、何时使用、怎样判断是否用对。", u8"如果某一步说不清，就回到对应条件或例子补一句说明。", u8"用 2 分钟口头复述，再写下最容易错的一点。", u8"能独立回答下面小测中的问题。"},
    };
    answer.examples = {{u8"通用分析示例", u8"面对“" + topic + u8"”的材料时，先提取已知条件；再对应本节定义或规律；最后写出由条件推出结论的理由。", {u8"圈出题干中的对象和条件", u8"选择本节最匹配的定义、规律或方法", u8"用完整句写出结论并检查条件是否全部使用"}}};
    answer.practice = {
        {u8"基础复述", u8"基础", u8"不用看资料，写出“" + topic + u8"”的定义、两个条件和一个用途。", u8"定义完整，条件具体，用途能对应一个真实材料。"},
        {u8"材料应用", u8"进阶", u8"找一道与“" + topic + u8"”相关的练习题，按“信息—方法—结论”写出三步分析。", u8"三步都有内容，结论能由前两步推出。"},
    };
    answer.quiz = {
        {u8"判断自己是否真正掌握一个知识点时，最可靠的做法是？", {u8"只记住标题", u8"能说出定义、条件并解释一个例子", u8"只看答案", u8"反复抄写概念"}, 1, u8"真正掌握需要能解释并应用，而不只是记住名称。"},
        {u8"分析与“" + topic + u8"”相关材料的第一步应当是？", {u8"直接写结论", u8"先提取对象和已知条件", u8"先找难题", u8"跳过材料"}, 1, u8"先定位对象和条件，才能选择正确的方法。"},
        {u8"发现自己的结论不稳妥时，优先检查什么？", {u8"是否使用了所有关键条件", u8"字写得是否漂亮", u8"题目是否太长", u8"是否背得更多"}, 0, u8"遗漏或误用条件是分析出错的常见原因。"},
    };
    answer.commonMistakes = {u8"只背结论，不说明适用条件", u8"看到关键词就套方法，没有核对材料", u8"只写答案，缺少推理过程"};
    answer.checkpoint = {u8"能用自己的话说明“" + topic + u8"”", u8"能列出至少两个适用条件", u8"能完成一份三步材料分析", u8"小测正确率达到 70%"};
    answer.resourceSummary = resources.empty() ? u8"本节使用内置学习单，资料恢复后可点击“换一版讲解”获取 AI 讲解。" : u8"参考资料已保留；本节先提供可完成的学习单，之后可重新生成讲解。";
    answer.references = referencesFromResources(resources);
    answer.notice = u8"AI 讲解暂不可用，已切换为可完成的学习单。";
    return answer;
}

LearningAnswer generateOnce(const AIClient& client,
                            const std::string& goal,
                            const std::string& phaseName,
                            const std::string& topic,
                            const std::string& mode,
                            const std::vector<SearchResource>& resources,
                            const LearningAnswer& fallback,
                            bool retry) {
    ChatOptions options;
    options.messages = createLearningPromptMessages(goal, phaseName, topic, mode, resources, retry);
    options.temperature = 0.3;
    options.maxTokens = isLiteMode(mode) ? 3000 : 4000;
    options.timeoutMs = 45000;
    options.responseFormat = "json_object";
    options.maxAttempts = 1;

    const AIResult result = client.chat(options);
    LearningAnswer answer = adaptLearningAnswer(parseAIJson(result.content), fallback, resources);
    validateLearningAnswer(answer, mode);
    return answer;
}

}  // namespace

LearningGenerator::LearningGenerator(AIClient& client) : client_(client) {}

LearningAnswer LearningGenerator::generate(const std::string& goal,
                                           const std::string& phaseName,
                                           const std::string& topic,
                                           const std::string& mode,
                                           const std::vector<SearchResource>& resources) const {
    const std::string safeGoal = trim(goal).empty() ? u8"学习" : trim(goal);
    const std::string safePhaseName = trim(phaseName).empty() ? u8"当前阶段" : trim(phaseName);
    const std::string trimmedTopic = trim(topic);
    const std::string safeTopic = trimmedTopic.empty() ? (trim(goal).empty() ? u8"学习主题" : trim(goal)) : trimmedTopic;
    const std::string safeMode = normalizeMode(mode);
    const std::vector<SearchResource> safeResources = limitResources(resources);

    LearningAnswer fallback;
    fallback.title = safeTopic;
    fallback.keyConcepts = {safeTopic, safePhaseName};
    fallback.references = referencesFromResources(safeResources);

    try {
        return generateOnce(client_, safeGoal, safePhaseName, safeTopic, safeMode, safeResources, fallback, false);
    } catch (const AIClientError& firstError) {
        return fallbackAnswer(safeGoal, safePhaseName, safeTopic, safeResources,
                              firstError.errorType + ": " + firstError.what());
    }
}

}  // namespace gangyi
