#pragma once

namespace gangyi {

#ifdef GANGYI_OFFLINE_CANDIDATE
inline constexpr const char* kVersion = "v5.6.5 离线候选";
#elif defined(GANGYI_TEACHING_PREVIEW)
inline constexpr const char* kVersion = "v5.6.5 教学试用1";
#else
inline constexpr const char* kVersion = "v5.6.5";
#endif

}  // namespace gangyi
