#pragma once

#include "db.hpp"
#include "library_io.hpp"

#include <functional>
#include <string>

namespace gangyi {

using ExamSourceJson = nlohmann::json;
using ExamSourceFetcher = std::function<LibraryDocument(const std::string&, const LibraryCancel&)>;

// 只能读取会话中已有的搜索候选；返回可供人工核对的不可变原文快照。
ExamSourceJson examSourceRead(Database& db, const std::string& examId, const std::string& url,
                              const LibraryCancel& cancelled = {}, const ExamSourceFetcher& fetcher = {});
ExamSourceJson examSourceSnapshot(Database& db, const std::string& examId, const std::string& snapshotId);

// 用户逐题确认，所有摘录必须来自服务器保存的当前快照。
ExamSourceJson examSourceConfirm(Database& db, const std::string& examId, const ExamSourceJson& request);
ExamSourceJson examSourceRevoke(Database& db, const std::string& examId, const ExamSourceJson& request);
ExamSourceJson examSourceConfirmations(Database& db, const std::string& examId);

} // namespace gangyi
