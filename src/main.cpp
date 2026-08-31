#include "config.hpp"
#include "version.hpp"

#include <crow.h>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {

std::string content_type_for(const std::filesystem::path& path) {
    const auto extension = path.extension().string();
    if (extension == ".html") return "text/html; charset=utf-8";
    if (extension == ".css") return "text/css; charset=utf-8";
    if (extension == ".js") return "application/javascript; charset=utf-8";
    if (extension == ".json") return "application/json; charset=utf-8";
    return "application/octet-stream";
}

}  // namespace

int main() {
    const auto config = gangyi::Config::from_environment();
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([] {
        crow::response response(nlohmann::json{{"app", "gangyiAI"}, {"status", "ok"}}.dump());
        response.set_header("Content-Type", "application/json; charset=utf-8");
        return response;
    });

    CROW_ROUTE(app, "/health")([] {
        crow::response response(nlohmann::json{{"status", "healthy"}}.dump());
        response.set_header("Content-Type", "application/json; charset=utf-8");
        return response;
    });

    CROW_ROUTE(app, "/<path>")([](const crow::request&, crow::response& response, std::string path) {
        const auto public_root = std::filesystem::weakly_canonical("public");
        const auto requested = std::filesystem::weakly_canonical(public_root / path);
        const auto root_text = public_root.generic_string();
        const auto requested_text = requested.generic_string();
        const bool is_under_public = requested_text == root_text ||
            (requested_text.rfind(root_text, 0) == 0 &&
             requested_text[root_text.size()] == '/');
        if (!is_under_public || !std::filesystem::is_regular_file(requested)) {
            response.code = 404;
            response.end("Not found");
            return;
        }

        std::ifstream file(requested, std::ios::binary);
        std::ostringstream body;
        body << file.rdbuf();
        response.set_header("Content-Type", content_type_for(requested));
        response.code = 200;
        response.end(body.str());
    });

    std::cout << "gangyiAI " << gangyi::kVersion << " listening on 0.0.0.0:" << config.port << '\n';
    app.port(config.port).multithreaded().run();
}
