cmake_minimum_required(VERSION 3.20)

project(gangyiAI VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# 优先使用本地 third_party 依赖（服务器无法访问 github.com）
# 未提供本地目录时才回退到网络拉取
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/Crow-1.2.0)
    set(FETCHCONTENT_SOURCE_DIR_CROW ${CMAKE_CURRENT_SOURCE_DIR}/third_party/Crow-1.2.0)
endif()
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/json-3.11.3)
    set(FETCHCONTENT_SOURCE_DIR_NLOHMANN_JSON ${CMAKE_CURRENT_SOURCE_DIR}/third_party/json-3.11.3)
endif()
# Asio：服务器用系统 libasio-dev；Windows 本机用 third_party/asio（header-only）
# Crow 1.2.0 需要 asio::io_service（asio < 1.30），故用 1.28.2
if(WIN32)
    set(ASIO_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/asio-asio-1-28-2/asio/include)
    if(EXISTS ${ASIO_INCLUDE_DIR}/asio.hpp)
        set(FETCHCONTENT_SOURCE_DIR_ASIO ${CMAKE_CURRENT_SOURCE_DIR}/third_party/asio-asio-1-28-2)
        set(ASIO_FOUND TRUE CACHE INTERNAL "")
    endif()
endif()

FetchContent_Declare(
    Crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG v1.2.0
    GIT_SHALLOW TRUE
)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)

set(CROW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CROW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(Crow nlohmann_json)

# SQLite：本机 Windows 用 third_party amalgamation 源码；Linux 服务器用系统 libsqlite3-dev
if(WIN32)
    set(SQLITE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sqlite)
    file(GLOB SQLITE_AMALG_DIR ${SQLITE_SRC}/sqlite-amalgamation-*)
endif()
if(WIN32 AND SQLITE_AMALG_DIR AND EXISTS ${SQLITE_AMALG_DIR}/sqlite3.c)
    add_library(sqlite3_amalgamation STATIC ${SQLITE_AMALG_DIR}/sqlite3.c)
    target_include_directories(sqlite3_amalgamation PUBLIC ${SQLITE_AMALG_DIR})
    set(GANGYI_SQLITE_LIB sqlite3_amalgamation)
else()
    find_package(SQLite3 REQUIRED)
    set(GANGYI_SQLITE_LIB SQLite::SQLite3)
endif()
# CURL：本机 Windows 用 third_party curl 开发包（MinGW 缺 curl 头）；Linux 服务器用系统 libcurl
if(WIN32)
    set(CURL_DEV_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/curl)
    file(GLOB CURL_MINGW_DIR ${CURL_DEV_DIR}/curl-*-win64-mingw)
endif()
if(WIN32 AND CURL_MINGW_DIR AND EXISTS ${CURL_MINGW_DIR}/include/curl/curl.h)
    set(GANGYI_CURL_INCLUDE ${CURL_MINGW_DIR}/include)
    # MinGW 下 curl 头文件声明 __imp_ 导入符号，必须用动态导入库 libcurl.dll.a
    # （静态 libcurl.a 与 __imp_ 前缀不匹配）
    set(GANGYI_CURL_LIB
        ${CURL_MINGW_DIR}/lib/libcurl.dll.a
        ${CURL_MINGW_DIR}/lib/libssl.a
        ${CURL_MINGW_DIR}/lib/libcrypto.a
        ${CURL_MINGW_DIR}/lib/libz.a
        ${CURL_MINGW_DIR}/lib/libzstd.a
        ${CURL_MINGW_DIR}/lib/libssh2.a
        ${CURL_MINGW_DIR}/lib/libnghttp2.a
        ${CURL_MINGW_DIR}/lib/libpsl.a
        ${CURL_MINGW_DIR}/lib/libbrotlidec.a
        ${CURL_MINGW_DIR}/lib/libbrotlicommon.a
        ${CURL_MINGW_DIR}/lib/libnghttp3.a
        ${CURL_MINGW_DIR}/lib/libngtcp2.a
        ${CURL_MINGW_DIR}/lib/libngtcp2_crypto_libressl.a
    )
    set(GANGYI_CURL_LINK ${GANGYI_CURL_LIB})
else()
    find_package(CURL REQUIRED)
    set(GANGYI_CURL_INCLUDE "")
    set(GANGYI_CURL_LINK CURL::libcurl)
endif()

add_executable(gangyiAI src/main.cpp src/db.cpp src/ai_client.cpp src/json_fix.cpp src/plan_generator.cpp src/learning_generator.cpp src/page_renderer.cpp
    src/plan_cache.cpp src/plan_adapter.cpp src/search_client.cpp src/quality_gate.cpp src/course_service.cpp src/auth_service.cpp
    src/progress_service.cpp src/phase_generator.cpp src/ask_generator.cpp src/image_goal_analyzer.cpp)

target_include_directories(gangyiAI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${ASIO_INCLUDE_DIR}
    ${GANGYI_CURL_INCLUDE}
)

target_link_libraries(gangyiAI PRIVATE
    Crow::Crow
    nlohmann_json::nlohmann_json
    ${GANGYI_SQLITE_LIB}
    ${GANGYI_CURL_LINK}
)

if(WIN32)
    target_link_libraries(gangyiAI PRIVATE ws2_32 mswsock bcrypt crypt32 user32)
endif()


add_custom_command(TARGET gangyiAI POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/public
        $<TARGET_FILE_DIR:gangyiAI>/public
)

# 本机：复制 curl DLL 到可执行目录（运行时依赖）
if(GANGYI_CURL_INCLUDE AND EXISTS ${CURL_MINGW_DIR}/bin/libcurl-x64.dll)
    add_custom_command(TARGET gangyiAI POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CURL_MINGW_DIR}/bin/libcurl-x64.dll
            $<TARGET_FILE_DIR:gangyiAI>/libcurl-x64.dll
    )
endif()
