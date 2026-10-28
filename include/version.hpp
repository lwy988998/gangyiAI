#pragma once

namespace gangyi {

#ifdef GANGYI_OFFLINE_CANDIDATE
inline constexpr const char* kVersion = "v5.7.0 离线候选";
#elif defined(GANGYI_TEACHING_PREVIEW)
inline constexpr const char* kVersion = "v5.7.0 教学试用1";
#else
inline constexpr const char* kVersion = "v5.7.0";
#endif

}  // namespace gangyi
