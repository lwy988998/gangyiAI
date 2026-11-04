// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once
// 【逐行批注】 引入头文件 <nlohmann/json.hpp>，为后续实现提供类型或接口声明。
#include <nlohmann/json.hpp>
// 【逐行批注】 引入头文件 <functional>，为后续实现提供类型或接口声明。
#include <functional>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>
// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {
// 【逐行批注】 计算右侧结果并写入 using LibraryCancel，更新当前运行状态。
using LibraryCancel = std::function<bool()>;
// 【逐行批注】 声明数据结构或类型 LibraryDocument 及其成员接口。
struct LibraryDocument {
    // 【逐行批注】 执行当前声明或表达式：std::string url, body, mime, error;。
    std::string url, body, mime, error;
    // 【逐行批注】 计算右侧结果并写入 long status，更新当前运行状态。
    long status = 0;
    // 【逐行批注】 计算右侧结果并写入 int attempts，更新当前运行状态。
    int attempts = 0;
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 声明对象 libraryPublicURL，并通过构造函数初始化其状态。
bool libraryPublicURL(const std::string& url);
// 【逐行批注】 声明对象 libraryCanonicalURL，并通过构造函数初始化其状态。
std::string libraryCanonicalURL(const std::string& url);
// 【逐行批注】 声明对象 libraryDigest，并通过构造函数初始化其状态。
std::string libraryDigest(const std::string& text);
// 【逐行批注】 声明对象 libraryNow，并通过构造函数初始化其状态。
std::string libraryNow();
// 【逐行批注】 声明对象 libraryFetch，并通过构造函数初始化其状态。
LibraryDocument libraryFetch(const std::string& url, const LibraryCancel& cancelled = {});
// 【逐行批注】 声明对象 libraryParseCaptions，并通过构造函数初始化其状态。
nlohmann::json libraryParseCaptions(const std::string& text, const std::string& mime);
// 【逐行批注】 声明对象 libraryParseDocument，并通过构造函数初始化其状态。
nlohmann::json libraryParseDocument(const LibraryDocument& document, const LibraryCancel& cancelled = {});
// 【逐行批注】 结束当前代码块。
}
