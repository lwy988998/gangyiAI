#include "search_client.hpp"
#include <chrono>
#include <iostream>
#include <string>
#include <future>
#include <thread>
#include <filesystem>
#include <fstream>

using Json = nlohmann::json;
Json runQuery(const Json& input) {
    try {
        if (input.contains("parallel")) {
            std::vector<std::future<Json>> pending;
            for (const auto& item : input.at("parallel"))
                pending.push_back(std::async(std::launch::async, [item] { return runQuery(item); }));
            Json rows = Json::array();
            for (auto& item : pending) rows.push_back(item.get());
            return {{"parallel", rows}};
        }
        if (input.contains("startAfterFile")) {
            const auto signal = std::filesystem::u8path(input.at("startAfterFile").get<std::string>());
            const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
            while (!std::filesystem::exists(signal)) {
                if (std::chrono::steady_clock::now() >= deadline) throw std::runtime_error("本机检索夹具等待信号超时");
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(input.value("startDelayMs", 0)));
        gangyi::SearchClient client;
        gangyi::ResourceQuery request;
        request.query = input.at("query"); request.freshness = input.value("freshness", "noLimit");
        request.count = input.value("count", 10); request.refresh = input.value("refresh", false);
        request.scopeId = input.value("scopeId", "");
        const auto started = std::chrono::steady_clock::now();
        const auto cancelAfter = input.value("cancelAfterMs", -1);
        const auto cancelSignal = input.value("cancelOnFile", "");
        const auto cancelObserved = input.value("cancelObservedFile", "");
        const auto checksAfter = input.value("cancelCheckAfterFile", "");
        const auto checksFile = input.value("cancelChecksFile", "");
        request.cancelled = [started, cancelAfter, cancelSignal, cancelObserved, checksAfter, checksFile] {
            if (!checksAfter.empty() && !checksFile.empty() && std::filesystem::exists(std::filesystem::u8path(checksAfter))) {
                std::ofstream output(std::filesystem::u8path(checksFile), std::ios::app); output << '\n';
            }
            const bool cancelled = (!cancelSignal.empty() && std::filesystem::exists(std::filesystem::u8path(cancelSignal))) ||
                (cancelAfter >= 0 && std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - started).count() >= cancelAfter);
            if (cancelled && !cancelObserved.empty()) { std::ofstream output(std::filesystem::u8path(cancelObserved)); }
            return cancelled;
        };
        gangyi::ResourceQueryResult result;
        if (input.value("legacy", false)) { result.resources = client.search(request.query); result.status = "legacy"; }
        else result = client.query(request);
        Json resources = Json::array();
        for (const auto& resource : result.resources)
            resources.push_back({{"url", resource.url}, {"title", resource.title}, {"description", resource.description},
                {"source", resource.source}, {"publishedAt", resource.publishedAt}, {"providers", resource.searchProviders}});
        return {{"status", result.status}, {"attempts", result.attempts}, {"cached", result.cached}, {"shared", result.shared},
            {"providers", result.providers}, {"resources", resources}, {"elapsedMs", std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - started).count()}};
    } catch (const std::exception& error) { return {{"error", error.what()}}; }
}

int main() {
    std::string line;
    while (std::getline(std::cin, line)) {
        try {
            std::cout << runQuery(Json::parse(line)).dump() << std::endl;
        } catch (const std::exception& error) { std::cout << Json{{"error", error.what()}}.dump() << std::endl; }
    }
}
