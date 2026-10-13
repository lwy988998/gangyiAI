#include "library_io.hpp"
#include "search_client.hpp"
#include <curl/curl.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <memory>
#include <random>
#include <regex>
#include <sstream>
#include <thread>
#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
namespace gangyi {
namespace {
using Json = nlohmann::json;
using CurlURL = std::unique_ptr<CURLU, decltype(&curl_url_cleanup)>;
std::string part(CURLU* url, CURLUPart name) {
    char* raw = nullptr;
    if (curl_url_get(url, name, &raw, 0) != CURLUE_OK) return {};
    std::string result(raw); curl_free(raw); return result;
}
bool publicAddress(const sockaddr* address) {
    if (address->sa_family == AF_INET) {
        const auto value = ntohl(reinterpret_cast<const sockaddr_in*>(address)->sin_addr.s_addr);
        const auto first = value >> 24, second = (value >> 16) & 255;
        return first != 0 && first != 10 && first != 127 && first < 224 &&
            !(first == 169 && second == 254) && !(first == 172 && second >= 16 && second <= 31) &&
            !(first == 192 && second == 168) && !(first == 100 && second >= 64 && second <= 127);
    }
    if (address->sa_family == AF_INET6) {
        const auto* bytes = reinterpret_cast<const unsigned char*>(&reinterpret_cast<const sockaddr_in6*>(address)->sin6_addr);
        if ((bytes[0] & 0xfe) == 0xfc || bytes[0] == 0xff || (bytes[0] == 0xfe && (bytes[1] & 0xc0) == 0x80)) return false;
        bool zero = true; for (int i = 0; i < 15; ++i) zero = zero && bytes[i] == 0;
        if (zero) return false;
        bool mapped = bytes[10] == 255 && bytes[11] == 255;
        for (int i = 0; i < 10; ++i) mapped = mapped && bytes[i] == 0;
        if (mapped) {
            sockaddr_in ipv4{}; ipv4.sin_family = AF_INET;
            std::copy(bytes + 12, bytes + 16, reinterpret_cast<unsigned char*>(&ipv4.sin_addr));
            return publicAddress(reinterpret_cast<const sockaddr*>(&ipv4));
        }
        return true;
    }
    return false;
}
curl_socket_t openPublicSocket(void*, curlsocktype purpose, curl_sockaddr* address) {
    if (purpose != CURLSOCKTYPE_IPCXN || !publicAddress(&address->addr)) return CURL_SOCKET_BAD;
    return socket(address->family, address->socktype, address->protocol);
}
struct Transfer {
    std::string body;
    const LibraryCancel* cancelled = nullptr;
    bool tooLarge = false;
};
size_t receive(char* data, size_t size, size_t count, void* raw) {
    auto& transfer = *static_cast<Transfer*>(raw);
    const auto length = size * count;
    if (length > 24 * 1024 * 1024 || transfer.body.size() + length > 24 * 1024 * 1024) {
        transfer.tooLarge = true; return 0;
    }
    transfer.body.append(data, length); return length;
}
int progress(void* raw, curl_off_t, curl_off_t, curl_off_t, curl_off_t) {
    const auto& state = *static_cast<Transfer*>(raw);
    try { return state.cancelled && *state.cancelled && (*state.cancelled)() ? 1 : 0; } catch (...) { return 1; }
}
double captionTime(const std::string& value) {
    if (!std::regex_match(value, std::regex(R"((\d{2,}:)?[0-5]\d:[0-5]\d[.,]\d{3})"))) return -1;
    auto raw = value; std::replace(raw.begin(), raw.end(), ',', '.');
    std::vector<double> parts; std::istringstream input(raw); std::string part;
    while (std::getline(input, part, ':')) parts.push_back(std::stod(part));
    double result = 0; for (const auto number : parts) result = result * 60 + number; return result;
}
}
std::string libraryNow() {
    const auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()); std::tm utc{};
#ifdef _WIN32
    gmtime_s(&utc, &time);
#else
    gmtime_r(&time, &utc);
#endif
    std::ostringstream result; result << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ"); return result.str();
}
std::string libraryDigest(const std::string& text) {
    return resourceDigest(text);
}
bool libraryPublicURL(const std::string& value) {
    if (value.size() > 8000 || value.find_first_of("\r\n") != std::string::npos) return false;
    CurlURL url(curl_url(), curl_url_cleanup);
    if (!url || curl_url_set(url.get(), CURLUPART_URL, value.c_str(), 0) != CURLUE_OK) return false;
    auto host = part(url.get(), CURLUPART_HOST);
    std::transform(host.begin(), host.end(), host.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    const auto scheme = part(url.get(), CURLUPART_SCHEME);
    if ((scheme != "http" && scheme != "https") || !part(url.get(), CURLUPART_USER).empty() ||
        !part(url.get(), CURLUPART_PASSWORD).empty() || host.empty()) return false;
    if (host.back() == '.') host.pop_back();
    auto numeric = host;
    if (numeric.front() == '[' && numeric.back() == ']') numeric = numeric.substr(1, numeric.size() - 2);
    sockaddr_in ipv4{}; ipv4.sin_family = AF_INET;
    sockaddr_in6 ipv6{}; ipv6.sin6_family = AF_INET6;
    if (inet_pton(AF_INET, numeric.c_str(), &ipv4.sin_addr) == 1) return publicAddress(reinterpret_cast<const sockaddr*>(&ipv4));
    if (inet_pton(AF_INET6, numeric.c_str(), &ipv6.sin6_addr) == 1) return publicAddress(reinterpret_cast<const sockaddr*>(&ipv6));
    if (host == "localhost" || host.find(".localhost") != std::string::npos || host.find(".local") != std::string::npos ||
        host == "[::1]" || host == "0.0.0.0" || host.rfind("127.", 0) == 0 || host.rfind("10.", 0) == 0 ||
        host.rfind("192.168.", 0) == 0 || host.rfind("169.254.", 0) == 0) return false;
    // 最终连接的实际IP仍由套接字回调检查，阻止重定向及DNS解析进入本机网络。
    return true;
}
std::string libraryCanonicalURL(const std::string& value) {
    CurlURL url(curl_url(), curl_url_cleanup);
    if (!url || curl_url_set(url.get(), CURLUPART_URL, value.c_str(), 0) != CURLUE_OK) return {};
    curl_url_set(url.get(), CURLUPART_FRAGMENT, nullptr, 0);
    auto host = part(url.get(), CURLUPART_HOST);
    std::transform(host.begin(), host.end(), host.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    curl_url_set(url.get(), CURLUPART_HOST, host.c_str(), 0);
    return part(url.get(), CURLUPART_URL);
}
LibraryDocument libraryFetch(const std::string& input, const LibraryCancel& cancelled) {
    LibraryDocument document; document.url = input;
    for (int redirect = 0; redirect <= 5; ++redirect) {
        if (cancelled && cancelled()) { document.error = "cancelled"; return document; }
        if (!libraryPublicURL(document.url)) { document.error = "restricted_url"; return document; }
        std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(curl_easy_init(), curl_easy_cleanup);
        if (!curl) { document.error = "network_unavailable"; return document; }
        Transfer transfer; transfer.cancelled = &cancelled;
        curl_easy_setopt(curl.get(), CURLOPT_URL, document.url.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_PROTOCOLS_STR, "http,https");
        curl_easy_setopt(curl.get(), CURLOPT_PROXY, "");
        curl_easy_setopt(curl.get(), CURLOPT_OPENSOCKETFUNCTION, openPublicSocket);
        curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, receive);
        curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &transfer);
        curl_easy_setopt(curl.get(), CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl.get(), CURLOPT_XFERINFOFUNCTION, progress);
        curl_easy_setopt(curl.get(), CURLOPT_XFERINFODATA, &transfer);
        curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, "GangyiAI/5.6.1 learning-source-reader");
        curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT_MS, 8000L);
        curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT_MS, 25000L);
        curl_easy_setopt(curl.get(), CURLOPT_ACCEPT_ENCODING, "");
        curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(curl.get(), CURLOPT_NOSIGNAL, 1L);
        ++document.attempts;
        const auto code = curl_easy_perform(curl.get());
        curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &document.status);
        char* location = nullptr; curl_easy_getinfo(curl.get(), CURLINFO_REDIRECT_URL, &location);
        if (document.status >= 300 && document.status < 400 && location && code == CURLE_OK) { document.url = location; continue; }
        char* mime = nullptr; curl_easy_getinfo(curl.get(), CURLINFO_CONTENT_TYPE, &mime);
        document.mime = mime ? mime : "";
        document.body = std::move(transfer.body);
        if (transfer.tooLarge) document.error = "document_too_large";
        else if (code == CURLE_ABORTED_BY_CALLBACK) document.error = "cancelled";
        else if (code != CURLE_OK) document.error = "network_error";
        else if (document.status == 401 || document.status == 403) document.error = "login_or_access_required";
        else if (document.status < 200 || document.status >= 300) document.error = "http_" + std::to_string(document.status);
        return document;
    }
    document.error = "too_many_redirects"; return document;
}
Json libraryParseCaptions(const std::string& text, const std::string& mime) {
    Json result = {{"segments", Json::array()}, {"kind", "captions"}, {"status", "unreadable"}, {"automatic", nullptr}};
    const auto parsed = Json::parse(text, nullptr, false);
    if (parsed.is_object() && parsed.value("body", Json()).is_array()) {
        if(parsed.value("automatic",Json()).is_boolean())result["automatic"]=parsed["automatic"];
        for (const auto& item : parsed["body"]) if (item.is_object() && item.value("content", Json()).is_string() && item.value("from", Json()).is_number()) {
            const auto end = item.value("to", item["from"]);
            const double start = item["from"].get<double>();
            if (end.is_number() && std::isfinite(start) && std::isfinite(end.get<double>()) && start >= 0 && end.get<double>() >= start)
                result["segments"].push_back({{"text", item["content"]}, {"start", start}, {"end", end}});
        }
    } else if (text.rfind("WEBVTT", 0) == 0 || mime.find("vtt") != std::string::npos || mime.find("subrip") != std::string::npos ||
        std::regex_search(text.substr(0, 1000), std::regex(R"(\d\d:\d\d[.,]\d\d\d\s+-->)"))) {
        std::istringstream lines(text); std::string line, content; double start = -1, end = -1;
        const std::regex timing(R"(([\d:.]+[.,]\d{3})\s+-->\s+([\d:.]+[.,]\d{3}))");
        const auto append = [&] { if (start >= 0 && end >= start && !content.empty()) result["segments"].push_back({{"text", std::regex_replace(content, std::regex("<[^>]+>"), "")}, {"start", start}, {"end", end}}); content.clear(); };
        while (std::getline(lines, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            std::smatch match;
            if (std::regex_search(line, match, timing)) { append(); start = captionTime(match[1]); end = captionTime(match[2]); }
            else if (line.empty()) { append(); start = -1; }
            else if (start >= 0) content += (content.empty() ? "" : "\n") + line;
        }
        append();
    }
    int index = 0; for (auto& segment : result["segments"]) segment["id"] = "t" + std::to_string(++index);
    if (index) result["status"] = "read";
    return result;
}
Json libraryParseDocument(const LibraryDocument& document, const LibraryCancel& cancelled) {
    if (!document.error.empty()) return {{"status", "unreadable"}, {"reason", document.error}, {"segments", Json::array()}};
    auto captions = libraryParseCaptions(document.body, document.mime);
    if (captions["status"] == "read") return captions;
#ifdef _WIN32
    wchar_t executable[MAX_PATH]{}; GetModuleFileNameW(nullptr, executable, MAX_PATH);
    auto base = std::filesystem::path(executable).parent_path();
    auto helper = base / L"gangyiAI-source-reader.exe";
    if (const auto* configured = std::getenv("LIBRARY_READER_PATH")) helper = std::filesystem::u8path(configured);
    if (!std::filesystem::exists(helper)) return {{"status", "unreadable"}, {"reason", "reader_unavailable"}, {"segments", Json::array()}};
    static thread_local std::mt19937_64 random(std::random_device{}());
    const auto scratch = std::filesystem::temp_directory_path() / ("gangyi-source-" + std::to_string(random()));
    std::filesystem::create_directories(scratch);
    struct Cleanup { std::filesystem::path path; ~Cleanup() { std::error_code error; std::filesystem::remove_all(path, error); } } cleanup{scratch};
    const auto input = scratch / "input.json", output = scratch / "output.json";
    { std::ofstream file(scratch / "document.bin", std::ios::binary); file.write(document.body.data(), static_cast<std::streamsize>(document.body.size())); }
    { std::ofstream file(input, std::ios::binary); file << Json{{"url", document.url}, {"mime", document.mime}}.dump(); }
    auto command = L"\"" + helper.wstring() + L"\" \"" + input.wstring() + L"\" \"" + output.wstring() + L"\"";
    STARTUPINFOW startup{}; startup.cb = sizeof(startup); startup.dwFlags = STARTF_USESHOWWINDOW; startup.wShowWindow = SW_HIDE;
    PROCESS_INFORMATION process{};
    if (!CreateProcessW(helper.c_str(), command.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, base.c_str(), &startup, &process))
        return {{"status", "unreadable"}, {"reason", "reader_start_failed"}, {"segments", Json::array()}};
    CloseHandle(process.hThread);
    const auto job = CreateJobObjectW(nullptr, nullptr);
    if (job) {
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
        limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE | JOB_OBJECT_LIMIT_PROCESS_MEMORY;
        limits.ProcessMemoryLimit = 768ULL * 1024 * 1024;
        SetInformationJobObject(job, JobObjectExtendedLimitInformation, &limits, sizeof(limits)); AssignProcessToJobObject(job, process.hProcess);
    }
    const auto began = std::chrono::steady_clock::now(); bool interrupted = false;
    while (WaitForSingleObject(process.hProcess, 100) == WAIT_TIMEOUT) {
        if ((cancelled && cancelled()) || std::chrono::steady_clock::now() - began > std::chrono::seconds(60)) {
            interrupted = true; TerminateProcess(process.hProcess, 1); WaitForSingleObject(process.hProcess, 3000); break;
        }
    }
    CloseHandle(process.hProcess); if (job) CloseHandle(job);
    if (interrupted) return {{"status", "unreadable"}, {"reason", "reader_interrupted"}, {"segments", Json::array()}};
    std::ifstream file(output, std::ios::binary); Json result;
    try { file >> result; } catch (...) { return {{"status", "unreadable"}, {"reason", "reader_failed"}, {"segments", Json::array()}}; }
    return result;
#else
    (void)cancelled;
    return {{"status", "unreadable"}, {"reason", "windows_reader_required"}, {"segments", Json::array()}};
#endif
}
}
