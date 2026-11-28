// 【逐行批注】 引入头文件 "search_client.hpp"，为后续实现提供类型或接口声明。
#include "search_client.hpp"
// 【逐行批注】 引入头文件 <chrono>，为后续实现提供类型或接口声明。
#include <chrono>
// 【逐行批注】 引入头文件 <iostream>，为后续实现提供类型或接口声明。
#include <iostream>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>
// 【逐行批注】 引入头文件 <future>，为后续实现提供类型或接口声明。
#include <future>
// 【逐行批注】 引入头文件 <thread>，为后续实现提供类型或接口声明。
#include <thread>
// 【逐行批注】 引入头文件 <filesystem>，为后续实现提供类型或接口声明。
#include <filesystem>
// 【逐行批注】 引入头文件 <fstream>，为后续实现提供类型或接口声明。
#include <fstream>

// 【逐行批注】 计算右侧结果并写入 using Json，更新当前运行状态。
using Json = nlohmann::json;
// 【逐行批注】 定义函数 runQuery，执行该函数声明的职责。
Json runQuery(const Json& input) {
    // 【逐行批注】 开始可能抛出异常的保护代码块。
    try {
        // 【逐行批注】 判断条件 (input.contains("parallel")) {，成立时执行对应分支。
        if (input.contains("parallel")) {
            // 【逐行批注】 执行当前声明或表达式：std::vector<std::future<Json>> pending;。
            std::vector<std::future<Json>> pending;
            // 【逐行批注】 按循环规则重复处理：for (const auto& item : input.at("parallel"))。
            for (const auto& item : input.at("parallel"))
                // 【逐行批注】 调用 async、runQuery 完成本行所需操作：pending.push_back(std::async(std::launch::async, [item] { return runQ…。
                pending.push_back(std::async(std::launch::async, [item] { return runQuery(item); }));
            // 【逐行批注】 计算右侧结果并写入 Json rows，更新当前运行状态。
            Json rows = Json::array();
            // 【逐行批注】 按循环规则重复处理：for (auto& item : pending) rows.push_back(item.get());。
            for (auto& item : pending) rows.push_back(item.get());
            // 【逐行批注】 结束当前函数并返回结果：return {{"parallel", rows}};。
            return {{"parallel", rows}};
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 判断条件 (input.contains("startAfterFile")) {，成立时执行对应分支。
        if (input.contains("startAfterFile")) {
            // 【逐行批注】 计算右侧结果并写入 auto signal，更新当前运行状态。
            const auto signal = std::filesystem::u8path(input.at("startAfterFile").get<std::string>());
            // 【逐行批注】 计算右侧结果并写入 auto deadline，更新当前运行状态。
            const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
            // 【逐行批注】 在条件成立期间循环处理：while (!std::filesystem::exists(signal)) {。
            while (!std::filesystem::exists(signal)) {
                // 【逐行批注】 判断条件 (std::chrono::steady_clock::now() >= deadline) throw std::runtim…，成立时执行对应分支。
                if (std::chrono::steady_clock::now() >= deadline) throw std::runtime_error("本机检索夹具等待信号超时");
                // 【逐行批注】 调用 sleep_for、milliseconds 完成本行所需操作：std::this_thread::sleep_for(std::chrono::milliseconds(5));。
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            // 【逐行批注】 结束当前代码块。
            }
        // 【逐行批注】 结束当前代码块。
        }
        // 【逐行批注】 调用 sleep_for、milliseconds 完成本行所需操作：std::this_thread::sleep_for(std::chrono::milliseconds(input.value("st…。
        std::this_thread::sleep_for(std::chrono::milliseconds(input.value("startDelayMs", 0)));
        // 【逐行批注】 执行当前声明或表达式：gangyi::SearchClient client;。
        gangyi::SearchClient client;
        // 【逐行批注】 执行当前声明或表达式：gangyi::ResourceQuery request;。
        gangyi::ResourceQuery request;
        // 【逐行批注】 计算右侧结果并写入 request.query，更新当前运行状态。
        request.query = input.at("query"); request.freshness = input.value("freshness", "noLimit");
        // 【逐行批注】 计算右侧结果并写入 request.count，更新当前运行状态。
        request.count = input.value("count", 10); request.refresh = input.value("refresh", false);
        // 【逐行批注】 计算右侧结果并写入 request.scopeId，更新当前运行状态。
        request.scopeId = input.value("scopeId", "");
        // 【逐行批注】 计算右侧结果并写入 auto started，更新当前运行状态。
        const auto started = std::chrono::steady_clock::now();
        // 【逐行批注】 计算右侧结果并写入 auto cancelAfter，更新当前运行状态。
        const auto cancelAfter = input.value("cancelAfterMs", -1);
        // 【逐行批注】 计算右侧结果并写入 auto cancelSignal，更新当前运行状态。
        const auto cancelSignal = input.value("cancelOnFile", "");
        // 【逐行批注】 计算右侧结果并写入 auto cancelObserved，更新当前运行状态。
        const auto cancelObserved = input.value("cancelObservedFile", "");
        // 【逐行批注】 计算右侧结果并写入 auto checksAfter，更新当前运行状态。
        const auto checksAfter = input.value("cancelCheckAfterFile", "");
        // 【逐行批注】 计算右侧结果并写入 auto checksFile，更新当前运行状态。
        const auto checksFile = input.value("cancelChecksFile", "");
        // 【逐行批注】 计算右侧结果并写入 request.cancelled，更新当前运行状态。
        request.cancelled = [started, cancelAfter, cancelSignal, cancelObserved, checksAfter, checksFile] {
            // 【逐行批注】 判断条件 (!checksAfter.empty() && !checksFile.empty() && std::filesystem:…，成立时执行对应分支。
            if (!checksAfter.empty() && !checksFile.empty() && std::filesystem::exists(std::filesystem::u8path(checksAfter))) {
                // 【逐行批注】 声明对象 output，并通过构造函数初始化其状态。
                std::ofstream output(std::filesystem::u8path(checksFile), std::ios::app); output << '\n';
            // 【逐行批注】 结束当前代码块。
            }
            // 【逐行批注】 计算右侧结果并写入 bool cancelled，更新当前运行状态。
            const bool cancelled = (!cancelSignal.empty() && std::filesystem::exists(std::filesystem::u8path(cancelSignal))) ||
                // 【逐行批注】 执行当前声明或表达式：(cancelAfter >= 0 && std::chrono::duration_cast<std::chrono::milliseconds>(。
                (cancelAfter >= 0 && std::chrono::duration_cast<std::chrono::milliseconds>(
                    // 【逐行批注】 调用 now 完成本行所需操作：std::chrono::steady_clock::now() - started).count() >= cancelAfter);。
                    std::chrono::steady_clock::now() - started).count() >= cancelAfter);
            // 【逐行批注】 判断条件 (cancelled && !cancelObserved.empty()) { std::ofstream output(st…，成立时执行对应分支。
            if (cancelled && !cancelObserved.empty()) { std::ofstream output(std::filesystem::u8path(cancelObserved)); }
            // 【逐行批注】 结束当前函数并返回结果：return cancelled;。
            return cancelled;
        // 【逐行批注】 执行当前声明或表达式：};。
        };
        // 【逐行批注】 执行当前声明或表达式：gangyi::ResourceQueryResult result;。
        gangyi::ResourceQueryResult result;
        // 【逐行批注】 判断条件 (input.value("legacy", false)) { result.resources = client.searc…，成立时执行对应分支。
        if (input.value("legacy", false)) { result.resources = client.search(request.query); result.status = "legacy"; }
        // 【逐行批注】 执行前一条件不成立时的分支。
        else result = client.query(request);
        // 【逐行批注】 计算右侧结果并写入 Json resources，更新当前运行状态。
        Json resources = Json::array();
        // 【逐行批注】 按循环规则重复处理：for (const auto& resource : result.resources)。
        for (const auto& resource : result.resources)
            // 【逐行批注】 执行当前声明或表达式：resources.push_back({{"url", resource.url}, {"title", resource.title}, {"description…。
            resources.push_back({{"url", resource.url}, {"title", resource.title}, {"description", resource.description},
                // 【逐行批注】 执行当前声明或表达式：{"source", resource.source}, {"publishedAt", resource.publishedAt}, {"providers", re…。
                {"source", resource.source}, {"publishedAt", resource.publishedAt}, {"providers", resource.searchProviders}});
        // 【逐行批注】 结束当前函数并返回结果：return {{"status", result.status}, {"attempts", result.attempts}, {"c…。
        return {{"status", result.status}, {"attempts", result.attempts}, {"cached", result.cached}, {"shared", result.shared},
            // 【逐行批注】 执行当前声明或表达式：{"providers", result.providers}, {"resources", resources}, {"elapsedMs", std::chrono…。
            {"providers", result.providers}, {"resources", resources}, {"elapsedMs", std::chrono::duration_cast<std::chrono::milliseconds>(
                // 【逐行批注】 调用 now 完成本行所需操作：std::chrono::steady_clock::now() - started).count()}};。
                std::chrono::steady_clock::now() - started).count()}};
    // 【逐行批注】 执行当前声明或表达式：} catch (const std::exception& error) { return {{"error", error.what()}}; }。
    } catch (const std::exception& error) { return {{"error", error.what()}}; }
// 【逐行批注】 结束当前代码块。
}

// 【逐行批注】 定义函数 main，执行该函数声明的职责。
int main() {
    // 【逐行批注】 执行当前声明或表达式：std::string line;。
    std::string line;
    // 【逐行批注】 在条件成立期间循环处理：while (std::getline(std::cin, line)) {。
    while (std::getline(std::cin, line)) {
        // 【逐行批注】 开始可能抛出异常的保护代码块。
        try {
            // 【逐行批注】 调用 runQuery、parse 完成本行所需操作：std::cout << runQuery(Json::parse(line)).dump() << std::endl;。
            std::cout << runQuery(Json::parse(line)).dump() << std::endl;
        // 【逐行批注】 执行当前声明或表达式：} catch (const std::exception& error) { std::cout << Json{{"error", error.what()}}.d…。
        } catch (const std::exception& error) { std::cout << Json{{"error", error.what()}}.dump() << std::endl; }
    // 【逐行批注】 结束当前代码块。
    }
// 【逐行批注】 结束当前代码块。
}
