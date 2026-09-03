#include "auth_service.hpp"

#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <iomanip>
#include <random>
#include <sstream>

namespace gangyi {
namespace {

std::string timestamp(std::chrono::system_clock::time_point point = std::chrono::system_clock::now()) {
    const auto value = std::chrono::system_clock::to_time_t(point);
    std::tm utc{};
#ifdef _WIN32
    gmtime_s(&utc, &value);
#else
    gmtime_r(&value, &utc);
#endif
    std::ostringstream out;
    out << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    return out.str();
}

std::string hex(const unsigned char* bytes, size_t size) {
    std::ostringstream out;
    out << std::hex << std::setfill('0');
    for (size_t index = 0; index < size; ++index) out << std::setw(2) << static_cast<unsigned>(bytes[index]);
    return out.str();
}

std::optional<std::vector<unsigned char>> unhex(const std::string& value) {
    if (value.size() % 2 != 0) return std::nullopt;
    std::vector<unsigned char> bytes(value.size() / 2);
    for (size_t index = 0; index < bytes.size(); ++index) {
        const auto pair = value.substr(index * 2, 2);
        try { bytes[index] = static_cast<unsigned char>(std::stoul(pair, nullptr, 16)); }
        catch (...) { return std::nullopt; }
    }
    return bytes;
}

bool randomBytes(unsigned char* output, size_t size) {
    return RAND_bytes(output, static_cast<int>(size)) == 1;
}

std::string randomToken(size_t bytes = 32) {
    std::vector<unsigned char> data(bytes);
    if (!randomBytes(data.data(), data.size())) return {};
    return hex(data.data(), data.size());
}

std::string sha256(const std::string& value) {
    std::array<unsigned char, EVP_MAX_MD_SIZE> digest{};
    unsigned int size = 0;
    if (EVP_Digest(value.data(), value.size(), digest.data(), &size, EVP_sha256(), nullptr) != 1) return {};
    return hex(digest.data(), size);
}

std::string hashPassword(const std::string& password) {
    std::array<unsigned char, 16> salt{};
    std::array<unsigned char, 64> key{};
    if (!randomBytes(salt.data(), salt.size()) ||
        PKCS5_PBKDF2_HMAC(password.data(), static_cast<int>(password.size()), salt.data(), static_cast<int>(salt.size()),
                          210000, EVP_sha256(), static_cast<int>(key.size()), key.data()) != 1) return {};
    return "pbkdf2$" + hex(salt.data(), salt.size()) + "$" + hex(key.data(), key.size());
}

bool verifyPassword(const std::string& password, const std::string& stored) {
    const auto first = stored.find('$');
    const auto second = stored.find('$', first == std::string::npos ? 0 : first + 1);
    if (first == std::string::npos || second == std::string::npos || stored.substr(0, first) != "pbkdf2") return false;
    const auto salt = unhex(stored.substr(first + 1, second - first - 1));
    const auto expected = unhex(stored.substr(second + 1));
    if (!salt || !expected || expected->size() != 64) return false;
    std::array<unsigned char, 64> key{};
    if (PKCS5_PBKDF2_HMAC(password.data(), static_cast<int>(password.size()), salt->data(), static_cast<int>(salt->size()),
                          210000, EVP_sha256(), static_cast<int>(key.size()), key.data()) != 1) return false;
    return CRYPTO_memcmp(key.data(), expected->data(), key.size()) == 0;
}

std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

bool validEmail(const std::string& email) {
    const auto at = email.find('@');
    return email.size() <= 120 && at > 0 && at + 3 < email.size() && email.find(' ', 0) == std::string::npos;
}

std::string cookieValue(const crow::request& request, const std::string& name) {
    const std::string cookie = request.get_header_value("Cookie");
    const std::string key = name + "=";
    size_t start = cookie.find(key);
    while (start != std::string::npos && start > 0 && cookie[start - 1] != ' ' && cookie[start - 1] != ';') start = cookie.find(key, start + key.size());
    if (start == std::string::npos) return {};
    start += key.size();
    const auto end = cookie.find(';', start);
    return cookie.substr(start, end == std::string::npos ? std::string::npos : end - start);
}

void attachAnonymousData(Database& db, const std::string& userId, const std::string& anonymousId) {
    if (anonymousId.empty()) return;
    for (auto course : db.listCourses()) {
        if (course.anonymousId.value_or("") != anonymousId || course.userId) continue;
        course.userId = userId;
        course.anonymousId.reset();
        db.update(course);
    }
    for (auto progress : db.listCourseProgress()) {
        if (progress.anonymousId.value_or("") != anonymousId || progress.userId) continue;
        progress.userId = userId;
        progress.anonymousId.reset();
        db.update(progress);
    }
}

AuthResult createSession(Database& db, const User& user, const std::string& anonymousId) {
    AuthResult result;
    const std::string token = randomToken();
    const std::string tokenHash = sha256(token);
    if (token.empty() || tokenHash.empty()) { result.error = "会话创建失败"; return result; }
    const std::string now = timestamp();
    UserSession session{randomToken(16), user.id, tokenHash, now, timestamp(std::chrono::system_clock::now() + std::chrono::hours(24 * 30))};
    if (session.id.empty() || !db.insert(session)) { result.error = "会话创建失败"; return result; }
    attachAnonymousData(db, user.id, anonymousId);
    result.ok = true;
    result.user = user;
    result.token = token;
    return result;
}

}  // namespace

AuthResult registerUser(Database& db, const std::string& rawEmail, const std::string& name,
                        const std::string& password, const std::string& anonymousId) {
    const std::string email = lower(rawEmail);
    if (!validEmail(email)) return {false, "请输入有效邮箱", {}, {}};
    if (password.size() < 6 || password.size() > 128) return {false, "密码长度应为 6 至 128 位", {}, {}};
    if (db.findByEmail(email)) return {false, "该邮箱已注册，请直接登录", {}, {}};
    const std::string now = timestamp();
    User user{randomToken(16), email, name.empty() ? std::nullopt : std::optional<std::string>(name.substr(0, 60)), hashPassword(password), "free", "active", {}, {}, now, now};
    if (user.id.empty() || user.passwordHash.empty() || !db.insert(user)) return {false, "注册失败，请稍后重试", {}, {}};
    return createSession(db, user, anonymousId);
}

AuthResult loginUser(Database& db, const std::string& rawEmail, const std::string& password,
                     const std::string& anonymousId) {
    const auto user = db.findByEmail(lower(rawEmail));
    if (!user || !verifyPassword(password, user->passwordHash)) return {false, "邮箱或密码不正确", {}, {}};
    return createSession(db, *user, anonymousId);
}

std::optional<User> currentUser(Database& db, const crow::request& request) {
    const std::string token = cookieValue(request, "ailines_session");
    const auto session = token.empty() ? std::nullopt : db.findSessionByTokenHash(sha256(token));
    if (!session || session->expiresAt <= timestamp()) return std::nullopt;
    return db.getUser(session->userId);
}

bool logoutUser(Database& db, const crow::request& request) {
    const std::string token = cookieValue(request, "ailines_session");
    const auto session = token.empty() ? std::nullopt : db.findSessionByTokenHash(sha256(token));
    return session && db.deleteUserSession(session->id);
}

nlohmann::json publicUser(const User& user) {
    return {{"id", user.id}, {"email", user.email}, {"name", user.name.value_or("")},
            {"membershipTier", user.membershipTier}, {"membershipStatus", user.membershipStatus}};
}

}  // namespace gangyi
