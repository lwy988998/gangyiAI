#pragma once

#include "db.hpp"
#include <nlohmann/json.hpp>
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace gangyi {

// 只供本机服务内部读取；content 内含私有评分依据，公开响应必须另过白名单。
nlohmann::json preparedLesson(Database& db, const std::string& lessonTaskId);
nlohmann::json preparedLessonState(Database& db, const nlohmann::json& body);
nlohmann::json finishPreparedLesson(Database& db, const nlohmann::json& body);

class NextLessonService {
public:
    explicit NextLessonService(std::string databasePath);
    ~NextLessonService();
    nlohmann::json create(const nlohmann::json& body);
    nlohmann::json view(const std::string& id) const;
    nlohmann::json events(const std::string& id, int afterSeq = 0) const;
    nlohmann::json cancel(const std::string& id);
    nlohmann::json retry(const std::string& id);
    void stop();
private:
    void launch(const std::string& id);
    void run(const std::string& id, const std::shared_ptr<std::atomic_bool>& cancelled);
    std::string databasePath_;
    std::atomic_bool stopped_{false};
    mutable std::mutex workerMutex_;
    std::unordered_map<std::string, std::shared_ptr<std::atomic_bool>> cancellations_;
    std::vector<std::thread> workers_;
};
}
