#pragma once

#include "ai_client.hpp"
#include "search_client.hpp"
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#endif

namespace gangyi {

// 仅显式启用的付费实验使用。以微元计价：一元为一百万微元，避免浮点误差。
// 配置和账本路径必须固定；换案例/进程/构建不会建立新预算。
class AIMoneyBudget {
    using Json = nlohmann::json;
    using Amount = std::uint64_t;
    static constexpr Amount maximum_ = 9007199254740991ULL;
    inline static std::mutex processMutex_;
    inline static std::map<std::string, std::pair<Amount, std::string>> observed_;
    inline static std::set<std::string> failed_;
    std::filesystem::path file_;
    Json config_;
    std::string identity_, reservationId_, revalidationIdentity_;
    Amount promptLimit_ = 0, outputLimit_ = 0, validFrom_ = 0, validUntil_ = 0;

    [[noreturn]] static void denied(const std::string& message) {
        throw AIClientError("money_budget", message);
    }
    static Amount number(const Json& value) {
        if (!value.is_number_integer() || (value.is_number_unsigned() ? value.get<Amount>() > maximum_ : value.get<long long>() < 0))
            denied("金额或用量必须为安全范围内的非负整数");
        return value.get<Amount>();
    }
    static Amount add(Amount left, Amount right) {
        if (right > maximum_ || left > maximum_ - right) denied("预算整数溢出");
        return left + right;
    }
    static Amount cost(Amount tokens, Amount price) {
        if (tokens && price > (maximum_ - 999999) / tokens) denied("费用乘积超出安全范围");
        return (tokens * price + 999999) / 1000000;
    }
    static std::string readText(const std::filesystem::path& path) {
        std::ifstream input(path, std::ios::binary);
        if (!input) denied("费用配置或账本不可读，未发送请求");
        return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    }
    static std::string environment(const char* name) {
#ifdef _WIN32
        const std::wstring key(name, name + std::string(name).size());
        const auto size = GetEnvironmentVariableW(key.c_str(), nullptr, 0);
        if (!size) return {};
        std::wstring value(size, L'\0');
        const auto length = GetEnvironmentVariableW(key.c_str(), value.data(), size);
        if (!length || length >= size) denied("预算环境配置读取失败");
        value.resize(length);
        const int bytes = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
        if (bytes <= 0) denied("预算路径无法转换为UTF-8");
        std::string result(bytes, '\0');
        WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), result.data(), bytes, nullptr, nullptr);
        return result;
#else
        const auto* value = std::getenv(name); return value ? value : "";
#endif
    }
    struct FileLock {
#ifdef _WIN32
        HANDLE handle = INVALID_HANDLE_VALUE;
        explicit FileLock(const std::filesystem::path& file) {
            const auto path = file.wstring() + L".lock";
            for (int attempt = 0; attempt < 200; ++attempt) {
                handle = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
                if (handle != INVALID_HANDLE_VALUE) return;
                if (GetLastError() != ERROR_SHARING_VIOLATION) break;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            denied("费用账本正被使用或无法锁定，未发送请求");
        }
        ~FileLock() { if (handle != INVALID_HANDLE_VALUE) CloseHandle(handle); }
#else
        int descriptor = -1;
        explicit FileLock(const std::filesystem::path& file) {
            descriptor = ::open((file.string() + ".lock").c_str(), O_CREAT | O_RDWR, 0600);
            if (descriptor < 0 || flock(descriptor, LOCK_EX | LOCK_NB) != 0) {
                if (descriptor >= 0) ::close(descriptor);
                denied("费用账本无法锁定，未发送请求");
            }
        }
        ~FileLock() { if (descriptor >= 0) { flock(descriptor, LOCK_UN); ::close(descriptor); } }
#endif
    };
    // 发送闸门与短时账本锁分开。跨进程串行网络，崩溃后未结算预留保持不确定。
    std::unique_ptr<FileLock> sendingGate_;
    struct State {
        Amount revision = 0, spent = 0, held = 0;
        bool blocked = false;
        std::string hash;
        std::map<std::string, Json> pending;
    };
    State readState() const {
        if (failed_.count(file_.u8string())) denied("费用账本已发生故障，禁止继续发送");
        const auto text = readText(file_);
        if (text.empty() || text.back() != '\n') denied("费用账本为空或存在未完整写入的记录");
        State state;
        std::istringstream stream(text); std::string line;
        while (std::getline(stream, line)) {
            const auto row = Json::parse(line);
            if (row.at("sessionId") != config_.at("sessionId") || row.at("identity") != identity_ ||
                number(row.at("revision")) != state.revision || row.value("previousHash", "") != state.hash)
                denied("费用账本身份、顺序或历史链不一致");
            auto content = row; const auto savedHash = row.at("hash"); content.erase("hash");
            if (savedHash != resourceDigest(content.dump())) denied("费用账本记录损坏");
            const auto kind = row.at("kind").get<std::string>();
            const auto id = row.value("requestId", "");
            if (state.revision == 0) {
                if (kind != "genesis" || number(row.at("limitMicros")) != number(config_.at("limitMicros"))) denied("费用账本初始额度错误");
            } else if (kind == "reserve") {
                if (id.empty() || state.pending.count(id)) denied("请求预留身份重复");
                state.pending[id] = row; state.held = add(state.held, number(row.at("amountMicros")));
            } else if (kind == "settle") {
                if (!state.pending.count(id)) denied("结算没有对应预留");
                const auto bound = number(state.pending.at(id).at("amountMicros"));
                const auto charge = number(row.at("amountMicros"));
                if (charge > bound) denied("结算金额超过预留");
                state.held -= bound; state.spent = add(state.spent, charge); state.pending.erase(id);
            } else if (kind == "hold" || kind == "block") {
                if (!state.pending.count(id)) denied("保留费用没有对应请求");
                // 已完成但缺usage的请求按最坏预留占用；不确定传输则阻止继续调用。
                if (kind == "block") state.blocked = true;
                state.pending.at(id)["observedComplete"] = kind == "hold";
            } else denied("未知费用记录类型");
            if (add(state.spent, state.held) > number(config_.at("limitMicros"))) denied("费用超过授权上限");
            state.hash = savedHash.get<std::string>(); ++state.revision;
        }
        const auto observed = observed_.find(file_.u8string());
        if (observed != observed_.end() && (state.revision < observed->second.first ||
            (state.revision == observed->second.first && state.hash != observed->second.second))) denied("费用账本被回滚");
        observed_[file_.u8string()] = {state.revision, state.hash};
        return state;
    }
    void append(const State& state, Json row) const {
        row["sessionId"] = config_.at("sessionId"); row["identity"] = identity_;
        row["revision"] = state.revision; row["previousHash"] = state.hash;
        row["atUnix"] = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        row["hash"] = resourceDigest(row.dump());
        const auto bytes = row.dump() + "\n";
        // 日志为唯一事实来源；部分写入或持久化失败后不允许再发送。
#ifdef _WIN32
        const auto handle = CreateFileW(file_.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (handle == INVALID_HANDLE_VALUE) { failed_.insert(file_.u8string()); denied("费用账本无法追加"); }
        DWORD written = 0;
        const bool ok = WriteFile(handle, bytes.data(), static_cast<DWORD>(bytes.size()), &written, nullptr) && written == bytes.size() && FlushFileBuffers(handle);
        CloseHandle(handle);
#else
        const auto descriptor = ::open(file_.c_str(), O_WRONLY | O_APPEND);
        const bool ok = descriptor >= 0 && ::write(descriptor, bytes.data(), bytes.size()) == static_cast<ssize_t>(bytes.size()) && ::fsync(descriptor) == 0;
        if (descriptor >= 0) ::close(descriptor);
#endif
        if (!ok) { failed_.insert(file_.u8string()); denied("费用账本未可靠保存，停止后续请求"); }
        observed_[file_.u8string()] = {state.revision + 1, row.at("hash").get<std::string>()};
    }
    static Json checkedReview(const Json& binding) {
        const auto raw = readText(std::filesystem::u8path(binding.at("reviewReceiptPath").get<std::string>()));
        if (resourceDigest(raw) != binding.at("reviewReceiptSha256")) denied("H1独立审查回执未冻结或已变化");
        const auto review = Json::parse(raw);
        if (review.value("kind", "") != "hermes_money_budget_review" || review.value("state", "") != "completed" ||
            review.value("allBlockingFindingsClosed", Json()) != true || review.value("requestId", "").empty() ||
            !review.value("reviewedFiles", Json()).is_array() || review.at("reviewedFiles").empty())
            denied("H1独立审查尚未关闭阻断项");
        return review;
    }
    static void checkReviewFiles(const Json& review) {
        for (const auto& item : review.at("reviewedFiles"))
            if (resourceDigest(readText(std::filesystem::u8path(item.at("path").get<std::string>()))) != item.at("sha256"))
                denied("费用发送代码已变化，不能沿用旧H1审查");
    }
    void checkValidity() const {
        const auto now = static_cast<Amount>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
        if (now < validFrom_ || now >= validUntil_) denied("费率或授权尚未生效或已过期");
    }
    void checkPricingAuthorization(bool fixture) {
        const auto path = environment("GANGYI_MONEY_REVALIDATION");
        revalidationIdentity_ = environment("GANGYI_MONEY_REVALIDATION_SHA256");
        if (path.empty() && revalidationIdentity_.empty()) {
            validFrom_ = number(config_.at("validFromUnix"));
            validUntil_ = number(config_.at("expiresUnix"));
            checkValidity();
            if (!fixture) checkReviewFiles(checkedReview(config_));
            return;
        }
        if (path.empty() || revalidationIdentity_.empty()) denied("费率续期缺少冻结身份");
        const auto raw = readText(std::filesystem::u8path(path));
        if (resourceDigest(raw) != revalidationIdentity_) denied("费率续期文件未冻结或已变化");
        const auto renewal = Json::parse(raw);
        if (renewal.value("kind", "") != "money_pricing_revalidation" ||
            number(renewal.at("schemaVersion")) != 1 || renewal.at("configSha256") != identity_)
            denied("费率续期未绑定原预算身份");
        validFrom_ = number(renewal.at("checkedAtUnix"));
        validUntil_ = number(renewal.at("expiresUnix"));
        if (validUntil_ <= validFrom_ || validUntil_ - validFrom_ > 86400) denied("费率续期不得超过24小时");
        checkValidity();
        const auto evidenceRaw = readText(std::filesystem::u8path(renewal.at("pricingEvidencePath").get<std::string>()));
        if (resourceDigest(evidenceRaw) != renewal.at("pricingEvidenceSha256")) denied("费率核验证据已变化");
        const auto evidence = Json::parse(evidenceRaw);
        Json terms = Json::object();
        const auto& original = config_.at("pricing");
        for (const auto* key : {"inputMicrosPerMillion", "outputMicrosPerMillion", "contextWindowTokens",
                                "promptOverheadTokens", "textByteUpperBoundVerified", "outputIncludesReasoning"})
            if (original.contains(key)) terms[key] = original.at(key);
        if (evidence.at("terms").dump() != terms.dump() || number(evidence.at("checkedAtUnix")) != validFrom_)
            denied("费率续期不允许修改冻结费率或上界");
        for (const auto* key : {"currency", "model", "endpoint", "fixtureOnly"})
            if (evidence.at(key) != config_.at(key)) denied("费率续期的币种、模型、端点或模式不匹配");
        if (fixture) {
            if (evidence.value("source", "") != "fixture") denied("模拟续期只能使用本机夹具费率");
            return;
        }
        if (evidence.value("source", "") != "https://api-docs.deepseek.com/zh-cn/quick_start/pricing/")
            denied("真实费率续期须绑定 DeepSeek 官方人民币定价");
        const auto previous = checkedReview(config_);
        const auto reviewed = checkedReview(renewal);
        if (reviewed.at("sessionId") != config_.at("sessionId") ||
            reviewed.at("pricingEvidenceSha256") != renewal.at("pricingEvidenceSha256"))
            denied("新审查未绑定同一轮次及本次费率证据");
        std::set<std::string> reviewedPaths;
        for (const auto& item : reviewed.at("reviewedFiles")) reviewedPaths.insert(item.at("path").get<std::string>());
        for (const auto& item : previous.at("reviewedFiles"))
            if (!reviewedPaths.count(item.at("path").get<std::string>())) denied("新审查未覆盖原审查文件");
        checkReviewFiles(reviewed);
    }
    void load(const std::string& filename, const std::string& expected, const std::string& endpoint, const std::string& model) {
        const auto raw = readText(std::filesystem::u8path(filename));
        if (expected.empty() || resourceDigest(raw) != expected) denied("费用配置未冻结或已变化");
        config_ = Json::parse(raw); identity_ = expected;
        if (config_.at("schemaVersion") != 1 || config_.at("currency") != "CNY" || config_.value("sessionId", "").empty() ||
            config_.value("authorized", false) != true || config_.value("reviewed", false) != true ||
            config_.at("model") != model || config_.at("endpoint") != endpoint)
            denied("费用授权、独立审查或模型身份不匹配");
        if (number(config_.at("limitMicros")) == 0 || number(config_.at("limitMicros")) > 10000000) denied("首轮费用上限只能在10元以内");
        const auto& price = config_.at("pricing");
        const bool fixture = config_.value("fixtureOnly", false);
        if (price.value("source", "").empty() || price.value("checkedAt", "").empty() ||
            price.at("outputIncludesReasoning") != true ||
            number(price.at("inputMicrosPerMillion")) == 0 || number(price.at("outputMicrosPerMillion")) == 0)
            denied("费率来源或输出计费上界未核实");
        if (price.contains("contextWindowTokens")) {
            if (number(price.at("contextWindowTokens")) == 0) denied("上下文计费上界无效");
        } else if (!fixture || price.value("textByteUpperBoundVerified", false) != true ||
                   number(price.at("promptOverheadTokens")) < 4096) {
            denied("输入计费上界未核实");
        }
        if (!fixture && (model != "deepseek-flash" || number(price.at("contextWindowTokens")) != 1048576 ||
            (endpoint != "https://api.deepseek.com/chat/completions" && endpoint != "https://api.deepseek.com/v1/chat/completions")))
            denied("首轮真实预算只接受已核对上下文上限的 DeepSeek Flash 官方端点");
        if (!fixture && config_.value("authorizationSource", "").empty()) denied("缺少原付费授权出处");
        checkPricingAuthorization(fixture);
        if (fixture) {
            if (endpoint.rfind("http://127.0.0.1:", 0) != 0 && endpoint.rfind("http://[::1]:", 0) != 0) denied("模拟预算只能用于本机端点");
            const auto portBegin = endpoint.find(':', 7) + 1;
            const auto actualPort = endpoint.rfind("http://[::1]:", 0) == 0 ? std::string("http://[::1]:").size() : portBegin;
            auto index = actualPort;
            while (index < endpoint.size() && endpoint[index] >= '0' && endpoint[index] <= '9') ++index;
            if (index == actualPort || index >= endpoint.size() || endpoint[index] != '/' || endpoint.find_first_of("@?#\\") != std::string::npos)
                denied("模拟端点必须为无凭据、带端口的本机HTTP地址");
        }
        const auto ledger = std::filesystem::u8path(config_.at("ledgerPath").get<std::string>());
        if (!ledger.is_absolute()) denied("累计费用账本必须使用固定绝对路径");
        file_ = std::filesystem::canonical(ledger);
    }
public:
    AIMoneyBudget() = default;
    AIMoneyBudget(const AIMoneyBudget&) = delete;
    AIMoneyBudget& operator=(const AIMoneyBudget&) = delete;
    void reserve(const std::string& endpoint, const std::string& model, const ChatOptions& options, const std::string& body) {
        const auto filename = environment("GANGYI_MONEY_CONFIG");
        const auto scopedLimit = environment("GANGYI_MONEY_CUMULATIVE_LIMIT_MICROS");
        if (filename.empty()) {
            if (!scopedLimit.empty() || environment("GANGYI_MONEY_REQUIRED") == "1")
                denied("本轮要求共用费用预算，但未绑定配置");
            return;
        }
        load(filename, environment("GANGYI_MONEY_CONFIG_SHA256"), endpoint, model);
        const auto accountLimit = number(config_.at("limitMicros"));
        auto cumulativeLimit = accountLimit;
        if (!scopedLimit.empty()) {
            if (scopedLimit.find_first_not_of("0123456789") != std::string::npos)
                denied("本次实验累计上限必须使用整数微元");
            cumulativeLimit = number(Json::parse(scopedLimit, nullptr, false));
            if (!cumulativeLimit || cumulativeLimit > accountLimit)
                denied("本次实验累计上限必须在共用授权内");
        }
        if (!reservationId_.empty() || options.maxTokens <= 0) denied("预留重复或输出上限无效");
        for (const auto& message : options.messages) if (!message.imageDataUrl.empty()) denied("首轮文字预算不能估算图像计费");
        const auto& price = config_.at("pricing");
        // 用服务端的模型上下文上限作保守预留，避免依赖本地字符或字节估算真实用量。
        promptLimit_ = price.contains("contextWindowTokens") ? number(price.at("contextWindowTokens"))
                                                            : add(body.size(), number(price.at("promptOverheadTokens")));
        outputLimit_ = static_cast<Amount>(options.maxTokens);
        const auto limit = add(cost(promptLimit_, number(price.at("inputMicrosPerMillion"))),
            cost(outputLimit_, number(price.at("outputMicrosPerMillion"))));
        auto gatePath = file_; gatePath += ".request";
        sendingGate_ = std::make_unique<FileLock>(gatePath);
        std::lock_guard<std::mutex> guard(processMutex_); FileLock lock(file_);
        const auto state = readState();
        if (state.blocked) denied("存在不确定或异常计费请求，核实前不能继续发送");
        for (const auto& pending : state.pending)
            if (!pending.second.value("observedComplete", false)) denied("上次请求仅有预留而无完成回执，核实前不能继续发送");
        // 在进程间发送锁内同时核对原支出、未结算预留及本次实验上限。
        const auto used = add(state.spent, state.held);
        if (used >= cumulativeLimit || limit > cumulativeLimit - used)
            denied("剩余额度不足以预留下一次实际请求");
        const auto id = config_.at("sessionId").get<std::string>() + ":" + std::to_string(state.revision);
        checkValidity();
        Json record = {{"kind", "reserve"}, {"requestId", id}, {"amountMicros", limit},
            {"promptLimit", promptLimit_}, {"outputLimit", outputLimit_}, {"bodySha256", resourceDigest(body)}};
        if (!revalidationIdentity_.empty()) record["pricingRevalidationSha256"] = revalidationIdentity_;
        if (!scopedLimit.empty()) record["cumulativeLimitMicros"] = cumulativeLimit;
        append(state, std::move(record));
        reservationId_ = id;
    }
    void settle(const Json& usage, bool completeTransport, const std::string& returnedModel = {}) {
        if (reservationId_.empty()) return;
        std::lock_guard<std::mutex> guard(processMutex_); FileLock lock(file_);
        const auto state = readState();
        if (!state.pending.count(reservationId_)) denied("当前实际请求的费用预留丢失");
        Json row{{"kind", completeTransport ? "hold" : "block"}, {"requestId", reservationId_}};
        bool reliable = completeTransport && (returnedModel.empty() || returnedModel == config_.at("model"));
        Amount prompt = 0, output = 0;
        try {
            prompt = number(usage.at("prompt_tokens")); output = number(usage.at("completion_tokens"));
            if (number(usage.at("total_tokens")) != add(prompt, output)) { reliable = false; row["kind"] = "block"; }
            if (prompt > promptLimit_ || output > outputLimit_) { reliable = false; row["kind"] = "block"; }
        } catch (...) { reliable = false; if (usage.is_object() && !usage.empty()) row["kind"] = "block"; }
        if (!returnedModel.empty() && returnedModel != config_.at("model")) { reliable = false; row["kind"] = "block"; }
        if (reliable) {
            row["kind"] = "settle";
            row["amountMicros"] = add(cost(prompt, number(config_.at("pricing").at("inputMicrosPerMillion"))),
                cost(output, number(config_.at("pricing").at("outputMicrosPerMillion"))));
            row["usage"] = usage;
        } else row["reason"] = "用量未知、身份异常或传输不确定，保留全部费用预留";
        append(state, row); reservationId_.clear();
        sendingGate_.reset();
        if (row.at("kind") == "block") denied("请求计费状态不确定或异常，后续调用已停止");
    }
};

}
