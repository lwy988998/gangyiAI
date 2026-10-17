#pragma once
#include <nlohmann/json.hpp>
#include <functional>
#include <string>
namespace gangyi {
using LibraryCancel = std::function<bool()>;
struct LibraryDocument {
    std::string url, body, mime, error;
    long status = 0;
    int attempts = 0;
};
bool libraryPublicURL(const std::string& url);
std::string libraryCanonicalURL(const std::string& url);
std::string libraryDigest(const std::string& text);
std::string libraryNow();
LibraryDocument libraryFetch(const std::string& url, const LibraryCancel& cancelled = {});
nlohmann::json libraryParseCaptions(const std::string& text, const std::string& mime);
nlohmann::json libraryParseDocument(const LibraryDocument& document, const LibraryCancel& cancelled = {});
}
