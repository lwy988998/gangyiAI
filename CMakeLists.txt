cmake_minimum_required(VERSION 3.20)

project(gangyiAI VERSION 5.4.0 LANGUAGES C CXX)

if(WIN32)
    enable_language(RC)
    set(GANGYI_FILE_DESCRIPTION "钢一定制AI 服务")
    set(GANGYI_ORIGINAL_FILENAME "gangyiAI.exe")
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/windows/gangyiAI.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/gangyiAI-service.rc
        @ONLY
    )
    set(GANGYI_SERVICE_RESOURCE ${CMAKE_CURRENT_BINARY_DIR}/gangyiAI-service.rc)
    set(GANGYI_FILE_DESCRIPTION "钢一定制AI 启动器")
    set(GANGYI_ORIGINAL_FILENAME "gangyiAI-launcher.exe")
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/windows/gangyiAI.rc.in
        ${CMAKE_CURRENT_BINARY_DIR}/gangyiAI-launcher.rc
        @ONLY
    )
    set(GANGYI_LAUNCHER_RESOURCE ${CMAKE_CURRENT_BINARY_DIR}/gangyiAI-launcher.rc)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(MSVC)
    add_compile_options(/utf-8)
endif()

include(FetchContent)

# 优先使用本地依赖；缺失时由 FetchContent 拉取。
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/Crow-1.3.0)
    set(FETCHCONTENT_SOURCE_DIR_CROW ${CMAKE_CURRENT_SOURCE_DIR}/third_party/Crow-1.3.0)
endif()
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/json-3.11.3)
    set(FETCHCONTENT_SOURCE_DIR_NLOHMANN_JSON ${CMAKE_CURRENT_SOURCE_DIR}/third_party/json-3.11.3)
endif()
# Windows 使用固定版本的 Asio，Crow 使用正式版本修复连接释放问题。
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
    GIT_TAG 537b44d3d3faa88095885c876aad2059d3315e93
    GIT_SHALLOW FALSE
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
# Crow 1.2.0 的空请求构造函数未初始化协议标志，复用连接时会触发未定义行为。
set(crowRequestHeader "${crow_SOURCE_DIR}/include/crow/http_request.h")
file(READ "${crowRequestHeader}" crowRequestSource)
set(crowOriginalConstructor "method(HTTPMethod::Get)")
set(crowFixedConstructor "method(HTTPMethod::Get), http_ver_major(0), http_ver_minor(0), keep_alive(false), close_connection(false), upgrade(false)")
if(NOT crowRequestSource MATCHES "keep_alive\\(false\\), close_connection\\(false\\), upgrade\\(false\\)")
    string(REPLACE "${crowOriginalConstructor}" "${crowFixedConstructor}" crowPatchedSource "${crowRequestSource}")
    if(crowPatchedSource STREQUAL crowRequestSource)
        message(FATAL_ERROR "无法修复 Crow 1.2.0 的请求初始化")
    endif()
    file(WRITE "${crowRequestHeader}" "${crowPatchedSource}")
endif()
# Crow 1.2.0 在客户端提前断开后仍会读取对端地址，remote_endpoint() 抛异常会让工作线程直接退出。
set(crowSocketHeader "${crow_SOURCE_DIR}/include/crow/socket_adaptors.h")
file(READ "${crowSocketHeader}" crowSocketSource)
if(NOT crowSocketSource MATCHES "remote_endpoint\\(ec\\)")
    set(crowSocketPatched "${crowSocketSource}")
    string(REPLACE
        "            return socket_.remote_endpoint();"
        "            error_code ec;\n            const auto endpoint = socket_.remote_endpoint(ec);\n            return ec ? tcp::endpoint() : endpoint;"
        crowSocketPatched "${crowSocketPatched}")
    string(REPLACE
        "            return raw_socket().remote_endpoint();"
        "            error_code ec;\n            const auto endpoint = raw_socket().remote_endpoint(ec);\n            return ec ? tcp::endpoint() : endpoint;"
        crowSocketPatched "${crowSocketPatched}")
    if(crowSocketPatched STREQUAL crowSocketSource)
        message(FATAL_ERROR "无法修复 Crow 1.2.0 的对端地址读取")
    endif()
    file(WRITE "${crowSocketHeader}" "${crowSocketPatched}")
endif()
# 请求处理还会调用 address()；提前断开的连接不能让服务线程异常退出。
file(READ "${crowSocketHeader}" crowSocketSource)
set(crowAddressPatched "${crowSocketSource}")
string(REPLACE
    "            return socket_.remote_endpoint().address().to_string();"
    "            error_code ec;\n            const auto endpoint = socket_.remote_endpoint(ec);\n            return ec ? std::string() : endpoint.address().to_string();"
    crowAddressPatched "${crowAddressPatched}")
string(REPLACE
    "            return ssl_socket_->lowest_layer().remote_endpoint().address().to_string();"
    "            error_code ec;\n            const auto endpoint = ssl_socket_->lowest_layer().remote_endpoint(ec);\n            return ec ? std::string() : endpoint.address().to_string();"
    crowAddressPatched "${crowAddressPatched}")
if(crowAddressPatched MATCHES "remote_endpoint\\(\\)\\.address\\(\\)")
    message(FATAL_ERROR "Crow 仍存在提前断开时会抛异常的地址读取")
endif()
if(NOT crowAddressPatched STREQUAL crowSocketSource)
    file(WRITE "${crowSocketHeader}" "${crowAddressPatched}")
endif()
if(WIN32)
    FetchContent_Declare(
        webview
        GIT_REPOSITORY https://github.com/webview/webview.git
        GIT_TAG 0.12.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(webview)
endif()

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
    src/progress_service.cpp src/profile_service.cpp src/phase_generator.cpp src/ask_generator.cpp src/learning_generator.cpp src/classroom_service.cpp src/image_goal_analyzer.cpp src/home_recommendations.cpp src/learning_flow.cpp
    ${GANGYI_SERVICE_RESOURCE})

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
    add_executable(gangyiAI-launcher WIN32 src/windows_launcher.cpp src/launcher_support.cpp src/ai_client.cpp src/search_client.cpp ${GANGYI_LAUNCHER_RESOURCE})
    target_include_directories(gangyiAI-launcher PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${GANGYI_CURL_INCLUDE}
    )
    target_compile_definitions(gangyiAI-launcher PRIVATE
        UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX
    )
    target_link_libraries(gangyiAI-launcher PRIVATE
        webview::core
        nlohmann_json::nlohmann_json
        ${GANGYI_SQLITE_LIB}
        ${GANGYI_CURL_LINK}
        advapi32 bcrypt comdlg32 gdi32 shell32 ole32 ws2_32
    )
endif()

if(MINGW)
    target_link_options(gangyiAI PRIVATE -static-libgcc -static-libstdc++)
    target_link_options(gangyiAI-launcher PRIVATE -municode -static-libgcc -static-libstdc++)
endif()

include(CTest)
if(BUILD_TESTING)
    find_package(Threads REQUIRED)
    add_executable(socket-adaptor-tests tests/socket_adaptor_tests.cpp)
    target_link_libraries(socket-adaptor-tests PRIVATE Crow::Crow Threads::Threads)
    if(WIN32)
        target_link_libraries(socket-adaptor-tests PRIVATE ws2_32)
    endif()
    add_test(NAME socket-adaptor COMMAND socket-adaptor-tests)
    add_executable(ai-client-tests tests/ai_client_tests.cpp src/ai_client.cpp src/search_client.cpp src/ask_generator.cpp
        src/json_fix.cpp src/plan_generator.cpp)
    target_include_directories(ai-client-tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${GANGYI_CURL_INCLUDE}
    )
    target_link_libraries(ai-client-tests PRIVATE nlohmann_json::nlohmann_json ${GANGYI_CURL_LINK} Threads::Threads)
    if(WIN32)
        target_compile_definitions(ai-client-tests PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
        target_link_libraries(ai-client-tests PRIVATE ws2_32)
    endif()
    add_test(NAME ai-client COMMAND ai-client-tests)

    add_executable(ai-content-generation-tests tests/ai_content_generation_tests.cpp
        src/learning_generator.cpp src/phase_generator.cpp src/ai_client.cpp src/search_client.cpp src/json_fix.cpp)
    target_include_directories(ai-content-generation-tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${GANGYI_CURL_INCLUDE}
    )
    target_link_libraries(ai-content-generation-tests PRIVATE
        nlohmann_json::nlohmann_json ${GANGYI_CURL_LINK} Threads::Threads)
    if(WIN32)
        target_compile_definitions(ai-content-generation-tests PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
        target_link_libraries(ai-content-generation-tests PRIVATE ws2_32)
    endif()
    add_test(NAME ai-content-generation COMMAND ai-content-generation-tests)

    add_executable(progress-service-tests tests/progress_service_tests.cpp src/db.cpp src/progress_service.cpp)
    target_include_directories(progress-service-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(progress-service-tests PRIVATE nlohmann_json::nlohmann_json ${GANGYI_SQLITE_LIB})
    add_test(NAME progress-service COMMAND progress-service-tests)

    add_executable(profile-service-tests tests/profile_service_tests.cpp src/db.cpp src/profile_service.cpp src/ai_client.cpp src/search_client.cpp src/json_fix.cpp)
    target_include_directories(profile-service-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${GANGYI_CURL_INCLUDE})
    target_link_libraries(profile-service-tests PRIVATE nlohmann_json::nlohmann_json ${GANGYI_SQLITE_LIB} ${GANGYI_CURL_LINK} Threads::Threads)
    if(WIN32)
        target_link_libraries(profile-service-tests PRIVATE ws2_32)
    endif()
    add_test(NAME profile-service COMMAND profile-service-tests)

    add_executable(page-renderer-tests tests/page_renderer_tests.cpp src/page_renderer.cpp)
    target_include_directories(page-renderer-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(page-renderer-tests PRIVATE nlohmann_json::nlohmann_json)
    add_test(NAME page-renderer COMMAND page-renderer-tests)

    add_executable(classroom-service-tests tests/classroom_service_tests.cpp src/classroom_service.cpp
        src/db.cpp src/ai_client.cpp src/search_client.cpp src/json_fix.cpp src/course_service.cpp)
    target_include_directories(classroom-service-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${GANGYI_CURL_INCLUDE})
    target_link_libraries(classroom-service-tests PRIVATE nlohmann_json::nlohmann_json
        ${GANGYI_SQLITE_LIB} ${GANGYI_CURL_LINK} Threads::Threads)
    if(WIN32)
        target_link_libraries(classroom-service-tests PRIVATE ws2_32)
    endif()
    add_test(NAME classroom-service COMMAND classroom-service-tests)
endif()
if(BUILD_TESTING AND WIN32)
    add_executable(launcher-support-tests tests/launcher_support_tests.cpp src/launcher_support.cpp)
    target_include_directories(launcher-support-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_compile_definitions(launcher-support-tests PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
    target_link_libraries(launcher-support-tests PRIVATE ${GANGYI_SQLITE_LIB})
    add_test(NAME launcher-support COMMAND launcher-support-tests)
endif()
if(BUILD_TESTING)
    find_package(Python3 COMPONENTS Interpreter QUIET)
    if(Python3_Interpreter_FOUND)
        add_test(NAME personalization-smoke COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/personalization_smoke.py $<TARGET_FILE:gangyiAI>)
        add_test(NAME radar-smoke COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/radar_smoke.py $<TARGET_FILE:gangyiAI>)
        add_test(NAME home-recommendations-smoke COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/home_recommendations_smoke.py $<TARGET_FILE:gangyiAI>)
        add_test(NAME api-smoke COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/api_smoke.py $<TARGET_FILE:gangyiAI>)
        add_test(NAME learning-flow-smoke COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/learning_flow_smoke.py $<TARGET_FILE:gangyiAI>)
    endif()
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
