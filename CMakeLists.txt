cmake_minimum_required(VERSION 3.20)

project(gangyiAI VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# 优先使用本地依赖；缺失时由 FetchContent 拉取。
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/Crow-1.2.0)
    set(FETCHCONTENT_SOURCE_DIR_CROW ${CMAKE_CURRENT_SOURCE_DIR}/third_party/Crow-1.2.0)
endif()
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/json-3.11.3)
    set(FETCHCONTENT_SOURCE_DIR_NLOHMANN_JSON ${CMAKE_CURRENT_SOURCE_DIR}/third_party/json-3.11.3)
endif()
# Crow 1.2.0 依赖旧版 Asio 接口；Windows 使用本地 1.28.2。
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

# SQLite：Windows 使用本地 amalgamation，Linux 使用系统库。
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
# CURL：Windows 使用本地 MinGW 开发包，Linux 使用系统库。
if(WIN32)
    set(CURL_DEV_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/curl)
    file(GLOB CURL_MINGW_DIR ${CURL_DEV_DIR}/curl-*-win64-mingw)
endif()
if(WIN32 AND CURL_MINGW_DIR AND EXISTS ${CURL_MINGW_DIR}/include/curl/curl.h)
    set(GANGYI_CURL_INCLUDE ${CURL_MINGW_DIR}/include)
    # 官方 Windows 包的 libcurl DLL 已静态包含传输层依赖，这里只链接导入库。
    set(GANGYI_CURL_LINK ${CURL_MINGW_DIR}/lib/libcurl.dll.a)
else()
    find_package(CURL REQUIRED)
    find_package(OpenSSL REQUIRED)
    set(GANGYI_CURL_INCLUDE "")
    set(GANGYI_CURL_LINK CURL::libcurl OpenSSL::Crypto)
endif()

add_executable(gangyiAI src/main.cpp src/db.cpp src/ai_client.cpp src/json_fix.cpp src/plan_generator.cpp src/page_renderer.cpp
    src/plan_cache.cpp src/plan_adapter.cpp src/search_client.cpp src/quality_gate.cpp src/course_service.cpp
    src/phase_generator.cpp src/ask_generator.cpp src/learning_generator.cpp src/image_goal_analyzer.cpp)

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

if(WIN32)
    add_executable(gangyiAI-launcher WIN32 src/windows_launcher.cpp)
    target_compile_definitions(gangyiAI-launcher PRIVATE
        UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX
    )
    target_link_libraries(gangyiAI-launcher PRIVATE advapi32 bcrypt gdi32 shell32 ole32 ws2_32)
endif()

if(MINGW)
    target_link_options(gangyiAI PRIVATE -static-libgcc -static-libstdc++)
    target_link_options(gangyiAI-launcher PRIVATE -municode -static-libgcc -static-libstdc++)
endif()

add_custom_command(TARGET gangyiAI POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/public
        $<TARGET_FILE_DIR:gangyiAI>/public
)

# Windows：复制 curl 运行时 DLL。
if(GANGYI_CURL_INCLUDE AND EXISTS ${CURL_MINGW_DIR}/bin/libcurl-x64.dll)
    add_custom_command(TARGET gangyiAI POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CURL_MINGW_DIR}/bin/libcurl-x64.dll
            $<TARGET_FILE_DIR:gangyiAI>/libcurl-x64.dll
    )
    if(EXISTS ${CURL_MINGW_DIR}/bin/curl-ca-bundle.crt)
        add_custom_command(TARGET gangyiAI POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CURL_MINGW_DIR}/bin/curl-ca-bundle.crt
                $<TARGET_FILE_DIR:gangyiAI>/curl-ca-bundle.crt
        )
    endif()
endif()
