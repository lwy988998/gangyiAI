#include "search_client.hpp"
#include <chrono>
#include <iostream>
#include <string>
#include <future>
#include <thread>

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
        std::this_thread::sleep_for(std::chrono::milliseconds(input.value("startDelayMs", 0)));
        gangyi::SearchClient client;
        gangyi::ResourceQuery request;
        request.query = input.at("query"); request.freshness = input.value("freshness", "noLimit");
        request.count = input.value("count", 10); request.refresh = input.value("refresh", false);
        request.scopeId = input.value("scopeId", "");
        const auto started = std::chrono::steady_clock::now();
        const auto cancelAfter = input.value("cancelAfterMs", -1);
        request.cancelled = [started, cancelAfter] {
            return cancelAfter >= 0 && std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - started).count() >= cancelAfter;
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
