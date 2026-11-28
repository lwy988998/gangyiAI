// 【逐行批注】 要求预处理器保证当前头文件在一次编译中只被包含一次。
#pragma once

// 【逐行批注】 引入头文件 "db.hpp"，为后续实现提供类型或接口声明。
#include "db.hpp"
// 【逐行批注】 引入头文件 <nlohmann/json.hpp>，为后续实现提供类型或接口声明。
#include <nlohmann/json.hpp>
// 【逐行批注】 引入头文件 <atomic>，为后续实现提供类型或接口声明。
#include <atomic>
// 【逐行批注】 引入头文件 <memory>，为后续实现提供类型或接口声明。
#include <memory>
// 【逐行批注】 引入头文件 <mutex>，为后续实现提供类型或接口声明。
#include <mutex>
// 【逐行批注】 引入头文件 <string>，为后续实现提供类型或接口声明。
#include <string>
// 【逐行批注】 引入头文件 <thread>，为后续实现提供类型或接口声明。
#include <thread>
// 【逐行批注】 引入头文件 <unordered_map>，为后续实现提供类型或接口声明。
#include <unordered_map>
// 【逐行批注】 引入头文件 <vector>，为后续实现提供类型或接口声明。
#include <vector>

// 【逐行批注】 声明或进入命名空间：namespace gangyi {。
namespace gangyi {

// 只供本机服务内部读取；content 内含私有评分依据，公开响应必须另过白名单。
// 【逐行批注】 声明对象 preparedLesson，并通过构造函数初始化其状态。
nlohmann::json preparedLesson(Database& db, const std::string& lessonTaskId);
// 【逐行批注】 声明对象 preparedLessonState，并通过构造函数初始化其状态。
nlohmann::json preparedLessonState(Database& db, const nlohmann::json& body);
// 【逐行批注】 声明对象 finishPreparedLesson，并通过构造函数初始化其状态。
nlohmann::json finishPreparedLesson(Database& db, const nlohmann::json& body);

// 【逐行批注】 声明数据结构或类型 NextLessonService 及其成员接口。
class NextLessonService {
// 【逐行批注】 执行当前声明或表达式：public:。
public:
    // 【逐行批注】 声明对象 NextLessonService，并通过构造函数初始化其状态。
    explicit NextLessonService(std::string databasePath);
    // 【逐行批注】 调用 NextLessonService 完成本行所需操作：~NextLessonService();。
    ~NextLessonService();
    // 【逐行批注】 声明对象 create，并通过构造函数初始化其状态。
    nlohmann::json create(const nlohmann::json& body);
    // 【逐行批注】 声明对象 view，并通过构造函数初始化其状态。
    nlohmann::json view(const std::string& id) const;
    // 【逐行批注】 声明对象 events，并通过构造函数初始化其状态。
    nlohmann::json events(const std::string& id, int afterSeq = 0) const;
    // 【逐行批注】 声明对象 cancel，并通过构造函数初始化其状态。
    nlohmann::json cancel(const std::string& id);
    // 【逐行批注】 声明对象 retry，并通过构造函数初始化其状态。
    nlohmann::json retry(const std::string& id);
    // 【逐行批注】 声明对象 stop，并通过构造函数初始化其状态。
    void stop();
// 【逐行批注】 执行当前声明或表达式：private:。
private:
    // 【逐行批注】 声明对象 launch，并通过构造函数初始化其状态。
    void launch(const std::string& id);
    // 【逐行批注】 声明对象 run，并通过构造函数初始化其状态。
    void run(const std::string& id, const std::shared_ptr<std::atomic_bool>& cancelled);
    // 【逐行批注】 执行当前声明或表达式：std::string databasePath_;。
    std::string databasePath_;
    // 【逐行批注】 执行当前声明或表达式：std::atomic_bool stopped_{false};。
    std::atomic_bool stopped_{false};
    // 【逐行批注】 执行当前声明或表达式：mutable std::mutex workerMutex_;。
    mutable std::mutex workerMutex_;
    // 【逐行批注】 执行当前声明或表达式：std::unordered_map<std::string, std::shared_ptr<std::atomic_bool>> cancellations_;。
    std::unordered_map<std::string, std::shared_ptr<std::atomic_bool>> cancellations_;
    // 【逐行批注】 执行当前声明或表达式：std::vector<std::thread> workers_;。
    std::vector<std::thread> workers_;
// 【逐行批注】 执行当前声明或表达式：};。
};
// 【逐行批注】 结束当前代码块。
}
