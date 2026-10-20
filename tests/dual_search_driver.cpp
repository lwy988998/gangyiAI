#include "search_client.hpp"
#include <chrono>
#include <iostream>
#include <string>

int main() {
    using Json = nlohmann::json;
    gangyi::SearchClient client;
    std::string line;
    while (std::getline(std::cin, line)) {
        try {
            const auto input = Json::parse(line);
            gangyi::ResourceQuery request;
            request.query = input.at("query");
            request.freshness = input.value("freshness", "noLimit");
            request.count = input.value("count", 10);
            request.refresh = input.value("refresh", false);
            const auto started = std::chrono::steady_clock::now();
            const auto cancelAfter = input.value("cancelAfterMs", -1);
            request.cancelled = [started, cancelAfter] {
                return cancelAfter >= 0 && std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - started).count() >= cancelAfter;
            };
            gangyi::ResourceQueryResult result;
            if (input.value("legacy", false)) {
                result.resources = client.search(request.query);
                result.status = "legacy";
            } else result = client.query(request);
            Json resources = Json::array();
            for (const auto& resource : result.resources)
                resources.push_back({{"url", resource.url}, {"title", resource.title}, {"description", resource.description},
                    {"source", resource.source}, {"publishedAt", resource.publishedAt}, {"providers", resource.searchProviders}});
            std::cout << Json{{"status", result.status}, {"attempts", result.attempts}, {"cached", result.cached},
                {"providers", result.providers}, {"resources", resources}}.dump() << std::endl;
        } catch (const std::exception& error) { std::cout << Json{{"error", error.what()}}.dump() << std::endl; }
    }
}
