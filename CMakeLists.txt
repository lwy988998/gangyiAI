# 【逐行批注】 声明最低 CMake 版本要求：cmake_minimum_required(VERSION 3.20)。
cmake_minimum_required(VERSION 3.20)

# 【逐行批注】 声明项目名称、版本与编译语言：project(gangyiAI VERSION 5.7.2 LANGUAGES C CXX)。
project(gangyiAI VERSION 5.7.2 LANGUAGES C CXX)

# 提示词编译进程序，并以源码摘要追踪每次请求实际使用的模块。
include(cmake/TeachingPrompts.cmake)

# 【逐行批注】 根据构建条件选择配置：if(WIN32)。
if(WIN32)
    # 【逐行批注】 执行 CMake 构建配置：enable_language(RC)。
    enable_language(RC)
    # 【逐行批注】 设置构建变量或源码列表：set(GANGYI_FILE_DESCRIPTION "钢一定制AI 服务")。
    set(GANGYI_FILE_DESCRIPTION "钢一定制AI 服务")
    # 【逐行批注】 设置构建变量或源码列表：set(GANGYI_ORIGINAL_FILENAME "gangyiAI.exe")。
    set(GANGYI_ORIGINAL_FILENAME "gangyiAI.exe")
    # 【逐行批注】 根据模板生成平台资源文件：configure_file(。
    configure_file(
        # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/resources/windows/gangyiAI.rc.in。
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/windows/gangyiAI.rc.in
        # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_BINARY_DIR}/gangyiAI-service.rc。
        ${CMAKE_CURRENT_BINARY_DIR}/gangyiAI-service.rc
        # 【逐行批注】 执行 CMake 构建配置：@ONLY。
        @ONLY
    # 【逐行批注】 执行 CMake 构建配置：)。
    )
    # 【逐行批注】 设置构建变量或源码列表：set(GANGYI_SERVICE_RESOURCE ${CMAKE_CURRENT_BINARY_DIR}/gangyiAI-service.rc)。
    set(GANGYI_SERVICE_RESOURCE ${CMAKE_CURRENT_BINARY_DIR}/gangyiAI-service.rc)
    # 【逐行批注】 设置构建变量或源码列表：set(GANGYI_FILE_DESCRIPTION "钢一定制AI 启动器")。
    set(GANGYI_FILE_DESCRIPTION "钢一定制AI 启动器")
    # 【逐行批注】 设置构建变量或源码列表：set(GANGYI_ORIGINAL_FILENAME "gangyiAI-launcher.exe")。
    set(GANGYI_ORIGINAL_FILENAME "gangyiAI-launcher.exe")
    # 【逐行批注】 根据模板生成平台资源文件：configure_file(。
    configure_file(
        # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/resources/windows/gangyiAI.rc.in。
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/windows/gangyiAI.rc.in
        # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_BINARY_DIR}/gangyiAI-launcher.rc。
        ${CMAKE_CURRENT_BINARY_DIR}/gangyiAI-launcher.rc
        # 【逐行批注】 执行 CMake 构建配置：@ONLY。
        @ONLY
    # 【逐行批注】 执行 CMake 构建配置：)。
    )
    # 【逐行批注】 设置构建变量或源码列表：set(GANGYI_LAUNCHER_RESOURCE ${CMAKE_CURRENT_BINARY_DIR}/gangyiAI-launcher.rc)。
    set(GANGYI_LAUNCHER_RESOURCE ${CMAKE_CURRENT_BINARY_DIR}/gangyiAI-launcher.rc)
# 【逐行批注】 结束构建条件块：endif()。
endif()

# 【逐行批注】 设置构建变量或源码列表：set(CMAKE_CXX_STANDARD 17)。
set(CMAKE_CXX_STANDARD 17)
# 【逐行批注】 设置构建变量或源码列表：set(CMAKE_CXX_STANDARD_REQUIRED ON)。
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 【逐行批注】 设置构建变量或源码列表：set(CMAKE_CXX_EXTENSIONS OFF)。
set(CMAKE_CXX_EXTENSIONS OFF)
# 【逐行批注】 根据构建条件选择配置：if(MSVC)。
if(MSVC)
    # 【逐行批注】 执行 CMake 构建配置：add_compile_options(/utf-8)。
    add_compile_options(/utf-8)
# 【逐行批注】 结束构建条件块：endif()。
endif()
# 部分 MinGW GCC 16.1 包遗漏原子计数配置，混用互斥锁与原子操作会损坏共享指针。
# 对应上游修复：https://github.com/msys2/MINGW-packages/pull/29502
# 【逐行批注】 根据构建条件选择配置：if(MINGW AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 16.1。
if(MINGW AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 16.1
    # 【逐行批注】 执行 CMake 构建配置：AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 16.2 AND CMAKE_SIZEOF_VOID_P EQUAL 8)。
    AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 16.2 AND CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 【逐行批注】 加载额外 CMake 配置：include(CheckCXXSourceCompiles)。
    include(CheckCXXSourceCompiles)
    # 【逐行批注】 执行 CMake 构建配置：check_cxx_source_compiles("#include <bits/c++config.h>\n#ifndef _GLIBCXX_ATOMIC_WORD_BUIL…。
    check_cxx_source_compiles("#include <bits/c++config.h>\n#ifndef _GLIBCXX_ATOMIC_WORD_BUILTINS\n#error missing_atomic_refcount_configuration\n#endif\nint main() { return 0; }"
        # 【逐行批注】 执行 CMake 构建配置：GANGYI_TOOLCHAIN_ATOMIC_REFCOUNT)。
        GANGYI_TOOLCHAIN_ATOMIC_REFCOUNT)
    # 【逐行批注】 根据构建条件选择配置：if(NOT GANGYI_TOOLCHAIN_ATOMIC_REFCOUNT)。
    if(NOT GANGYI_TOOLCHAIN_ATOMIC_REFCOUNT)
        # 【逐行批注】 执行 CMake 构建配置：add_compile_definitions(_GLIBCXX_ATOMIC_WORD_BUILTINS=1)。
        add_compile_definitions(_GLIBCXX_ATOMIC_WORD_BUILTINS=1)
        # 【逐行批注】 执行 CMake 构建配置：message(STATUS "已补齐 MinGW GCC 16.1 的共享指针原子计数配置")。
        message(STATUS "已补齐 MinGW GCC 16.1 的共享指针原子计数配置")
    # 【逐行批注】 结束构建条件块：endif()。
    endif()
# 【逐行批注】 结束构建条件块：endif()。
endif()

# 【逐行批注】 加载额外 CMake 配置：include(FetchContent)。
include(FetchContent)

# 优先使用本地依赖；缺失时由 FetchContent 拉取。
# 【逐行批注】 根据构建条件选择配置：if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/Crow-1.3.0)。
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/Crow-1.3.0)
    # 【逐行批注】 设置构建变量或源码列表：set(FETCHCONTENT_SOURCE_DIR_CROW ${CMAKE_CURRENT_SOURCE_DIR}/third_party/Crow-1.3.0)。
    set(FETCHCONTENT_SOURCE_DIR_CROW ${CMAKE_CURRENT_SOURCE_DIR}/third_party/Crow-1.3.0)
# 【逐行批注】 结束构建条件块：endif()。
endif()
# 【逐行批注】 根据构建条件选择配置：if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/json-3.11.3)。
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/json-3.11.3)
    # 【逐行批注】 设置构建变量或源码列表：set(FETCHCONTENT_SOURCE_DIR_NLOHMANN_JSON ${CMAKE_CURRENT_SOURCE_DIR}/third_party/json-3.…。
    set(FETCHCONTENT_SOURCE_DIR_NLOHMANN_JSON ${CMAKE_CURRENT_SOURCE_DIR}/third_party/json-3.11.3)
# 【逐行批注】 结束构建条件块：endif()。
endif()
# Windows 使用固定版本的 Asio，Crow 使用正式版本修复连接释放问题。
# 【逐行批注】 根据构建条件选择配置：if(WIN32)。
if(WIN32)
    # 【逐行批注】 设置构建变量或源码列表：set(ASIO_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/asio-asio-1-28-2/asio/includ…。
    set(ASIO_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/asio-asio-1-28-2/asio/include)
    # 【逐行批注】 根据构建条件选择配置：if(EXISTS ${ASIO_INCLUDE_DIR}/asio.hpp)。
    if(EXISTS ${ASIO_INCLUDE_DIR}/asio.hpp)
        # 【逐行批注】 设置构建变量或源码列表：set(FETCHCONTENT_SOURCE_DIR_ASIO ${CMAKE_CURRENT_SOURCE_DIR}/third_party/asio-asio-1-28-2)。
        set(FETCHCONTENT_SOURCE_DIR_ASIO ${CMAKE_CURRENT_SOURCE_DIR}/third_party/asio-asio-1-28-2)
        # 【逐行批注】 设置构建变量或源码列表：set(ASIO_FOUND TRUE CACHE INTERNAL "")。
        set(ASIO_FOUND TRUE CACHE INTERNAL "")
    # 【逐行批注】 结束构建条件块：endif()。
    endif()
# 【逐行批注】 结束构建条件块：endif()。
endif()

# 【逐行批注】 执行 CMake 构建配置：FetchContent_Declare(。
FetchContent_Declare(
    # 【逐行批注】 执行 CMake 构建配置：Crow。
    Crow
    # 【逐行批注】 执行 CMake 构建配置：GIT_REPOSITORY https://github.com/CrowCpp/Crow.git。
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    # 【逐行批注】 执行 CMake 构建配置：GIT_TAG 537b44d3d3faa88095885c876aad2059d3315e93。
    GIT_TAG 537b44d3d3faa88095885c876aad2059d3315e93
    # 【逐行批注】 执行 CMake 构建配置：GIT_SHALLOW FALSE。
    GIT_SHALLOW FALSE
# 【逐行批注】 执行 CMake 构建配置：)。
)

# 【逐行批注】 执行 CMake 构建配置：FetchContent_Declare(。
FetchContent_Declare(
    # 【逐行批注】 执行 CMake 构建配置：nlohmann_json。
    nlohmann_json
    # 【逐行批注】 执行 CMake 构建配置：GIT_REPOSITORY https://github.com/nlohmann/json.git。
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    # 【逐行批注】 执行 CMake 构建配置：GIT_TAG v3.11.3。
    GIT_TAG v3.11.3
    # 【逐行批注】 执行 CMake 构建配置：GIT_SHALLOW TRUE。
    GIT_SHALLOW TRUE
# 【逐行批注】 执行 CMake 构建配置：)。
)

# 【逐行批注】 设置构建变量或源码列表：set(CROW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)。
set(CROW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
# 【逐行批注】 设置构建变量或源码列表：set(CROW_BUILD_TESTS OFF CACHE BOOL "" FORCE)。
set(CROW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
# 【逐行批注】 执行 CMake 构建配置：FetchContent_MakeAvailable(Crow nlohmann_json)。
FetchContent_MakeAvailable(Crow nlohmann_json)
# 延迟响应会清空持有连接的完成回调；发送期间必须另持有连接，避免提前析构。
# 【逐行批注】 设置构建变量或源码列表：set(crowHttpHeader "${crow_SOURCE_DIR}/include/crow/http_connection.h")。
set(crowHttpHeader "${crow_SOURCE_DIR}/include/crow/http_connection.h")
# 【逐行批注】 执行文件或路径相关配置：file(READ "${crowHttpHeader}" crowHttpSource)。
file(READ "${crowHttpHeader}" crowHttpSource)
# 【逐行批注】 根据构建条件选择配置：if(NOT crowHttpSource MATCHES "response_lifetime = this->shared_from_this")。
if(NOT crowHttpSource MATCHES "response_lifetime = this->shared_from_this")
    # 【逐行批注】 执行 CMake 构建配置：string(REPLACE "void complete_request()\n {"。
    string(REPLACE "void complete_request()\n        {"
        # 【逐行批注】 执行 CMake 构建配置："void complete_request()\n {\n const auto response_lifetime = this->shared_from_this();"。
        "void complete_request()\n        {\n            const auto response_lifetime = this->shared_from_this();"
        # 【逐行批注】 执行 CMake 构建配置：crowHttpPatched "${crowHttpSource}")。
        crowHttpPatched "${crowHttpSource}")
    # 【逐行批注】 根据构建条件选择配置：if(crowHttpPatched STREQUAL crowHttpSource)。
    if(crowHttpPatched STREQUAL crowHttpSource)
        # 【逐行批注】 执行 CMake 构建配置：message(FATAL_ERROR "无法保护 Crow 延迟响应的连接生命周期")。
        message(FATAL_ERROR "无法保护 Crow 延迟响应的连接生命周期")
    # 【逐行批注】 结束构建条件块：endif()。
    endif()
    # 【逐行批注】 执行文件或路径相关配置：file(WRITE "${crowHttpHeader}" "${crowHttpPatched}")。
    file(WRITE "${crowHttpHeader}" "${crowHttpPatched}")
# 【逐行批注】 结束构建条件块：endif()。
endif()
# 完成回调会清空响应自身保存的回调；局部副本必须覆盖 end() 后续的成员写入。
# 【逐行批注】 设置构建变量或源码列表：set(crowResponseHeader "${crow_SOURCE_DIR}/include/crow/http_response.h")。
set(crowResponseHeader "${crow_SOURCE_DIR}/include/crow/http_response.h")
# 【逐行批注】 执行文件或路径相关配置：file(READ "${crowResponseHeader}" crowResponseSource)。
file(READ "${crowResponseHeader}" crowResponseSource)
# 【逐行批注】 根据构建条件选择配置：if(NOT crowResponseSource MATCHES "const auto completion_handler = complete_request_handl…。
if(NOT crowResponseSource MATCHES "const auto completion_handler = complete_request_handler_")
    # 【逐行批注】 执行 CMake 构建配置：string(REPLACE " if (complete_request_handler_)\n {\n complete_request_handler_();"。
    string(REPLACE "                if (complete_request_handler_)\n                {\n                    complete_request_handler_();"
        # 【逐行批注】 执行 CMake 构建配置：" const auto completion_handler = complete_request_handler_;\n if (completion_handler)\n …。
        "                const auto completion_handler = complete_request_handler_;\n                if (completion_handler)\n                {\n                    completion_handler();"
        # 【逐行批注】 执行 CMake 构建配置：crowResponsePatched "${crowResponseSource}")。
        crowResponsePatched "${crowResponseSource}")
    # 【逐行批注】 根据构建条件选择配置：if(crowResponsePatched STREQUAL crowResponseSource)。
    if(crowResponsePatched STREQUAL crowResponseSource)
        # 【逐行批注】 执行 CMake 构建配置：message(FATAL_ERROR "无法保护 Crow 响应结束时的完成回调")。
        message(FATAL_ERROR "无法保护 Crow 响应结束时的完成回调")
    # 【逐行批注】 结束构建条件块：endif()。
    endif()
    # 【逐行批注】 执行文件或路径相关配置：file(WRITE "${crowResponseHeader}" "${crowResponsePatched}")。
    file(WRITE "${crowResponseHeader}" "${crowResponsePatched}")
# 【逐行批注】 结束构建条件块：endif()。
endif()
# 流式备课重放会连续发送多帧；上一轮写入完成前不能交换仍被异步使用的缓冲区。
# 【逐行批注】 设置构建变量或源码列表：set(crowWebsocketHeader "${crow_SOURCE_DIR}/include/crow/websocket.h")。
set(crowWebsocketHeader "${crow_SOURCE_DIR}/include/crow/websocket.h")
# 【逐行批注】 执行文件或路径相关配置：file(READ "${crowWebsocketHeader}" crowWebsocketSource)。
file(READ "${crowWebsocketHeader}" crowWebsocketSource)
# 【逐行批注】 根据构建条件选择配置：if(NOT crowWebsocketSource MATCHES "write_buffers_\\.empty\\(\\) \\|\\| !sending_buffers_…。
if(NOT crowWebsocketSource MATCHES "write_buffers_\\.empty\\(\\) \\|\\| !sending_buffers_\\.empty\\(\\)")
    # 【逐行批注】 执行 CMake 构建配置：string(REPLACE "if (write_buffers_.empty()) return;"。
    string(REPLACE "if (write_buffers_.empty()) return;"
        # 【逐行批注】 执行 CMake 构建配置："if (write_buffers_.empty() || !sending_buffers_.empty()) return;"。
        "if (write_buffers_.empty() || !sending_buffers_.empty()) return;"
        # 【逐行批注】 执行 CMake 构建配置：crowWebsocketPatched "${crowWebsocketSource}")。
        crowWebsocketPatched "${crowWebsocketSource}")
    # 【逐行批注】 根据构建条件选择配置：if(crowWebsocketPatched STREQUAL crowWebsocketSource)。
    if(crowWebsocketPatched STREQUAL crowWebsocketSource)
        # 【逐行批注】 执行 CMake 构建配置：message(FATAL_ERROR "无法保护 WebSocket 在途写入缓冲区")。
        message(FATAL_ERROR "无法保护 WebSocket 在途写入缓冲区")
    # 【逐行批注】 结束构建条件块：endif()。
    endif()
    # 【逐行批注】 执行文件或路径相关配置：file(WRITE "${crowWebsocketHeader}" "${crowWebsocketPatched}")。
    file(WRITE "${crowWebsocketHeader}" "${crowWebsocketPatched}")
# 【逐行批注】 结束构建条件块：endif()。
endif()
# Crow 1.2.0 的空请求构造函数未初始化协议标志，复用连接时会触发未定义行为。
# 【逐行批注】 设置构建变量或源码列表：set(crowRequestHeader "${crow_SOURCE_DIR}/include/crow/http_request.h")。
set(crowRequestHeader "${crow_SOURCE_DIR}/include/crow/http_request.h")
# 【逐行批注】 执行文件或路径相关配置：file(READ "${crowRequestHeader}" crowRequestSource)。
file(READ "${crowRequestHeader}" crowRequestSource)
# 【逐行批注】 设置构建变量或源码列表：set(crowOriginalConstructor "method(HTTPMethod::Get)")。
set(crowOriginalConstructor "method(HTTPMethod::Get)")
# 【逐行批注】 设置构建变量或源码列表：set(crowFixedConstructor "method(HTTPMethod::Get), http_ver_major(0), http_ver_minor(0), …。
set(crowFixedConstructor "method(HTTPMethod::Get), http_ver_major(0), http_ver_minor(0), keep_alive(false), close_connection(false), upgrade(false)")
# 【逐行批注】 根据构建条件选择配置：if(NOT crowRequestSource MATCHES "keep_alive\\(false\\), close_connection\\(false\\), upg…。
if(NOT crowRequestSource MATCHES "keep_alive\\(false\\), close_connection\\(false\\), upgrade\\(false\\)")
    # 【逐行批注】 执行 CMake 构建配置：string(REPLACE "${crowOriginalConstructor}" "${crowFixedConstructor}" crowPatchedSource "…。
    string(REPLACE "${crowOriginalConstructor}" "${crowFixedConstructor}" crowPatchedSource "${crowRequestSource}")
    # 【逐行批注】 根据构建条件选择配置：if(crowPatchedSource STREQUAL crowRequestSource)。
    if(crowPatchedSource STREQUAL crowRequestSource)
        # 【逐行批注】 执行 CMake 构建配置：message(FATAL_ERROR "无法修复 Crow 1.2.0 的请求初始化")。
        message(FATAL_ERROR "无法修复 Crow 1.2.0 的请求初始化")
    # 【逐行批注】 结束构建条件块：endif()。
    endif()
    # 【逐行批注】 执行文件或路径相关配置：file(WRITE "${crowRequestHeader}" "${crowPatchedSource}")。
    file(WRITE "${crowRequestHeader}" "${crowPatchedSource}")
# 【逐行批注】 结束构建条件块：endif()。
endif()
# Crow 1.2.0 在客户端提前断开后仍会读取对端地址，remote_endpoint() 抛异常会让工作线程直接退出。
# 【逐行批注】 设置构建变量或源码列表：set(crowSocketHeader "${crow_SOURCE_DIR}/include/crow/socket_adaptors.h")。
set(crowSocketHeader "${crow_SOURCE_DIR}/include/crow/socket_adaptors.h")
# 【逐行批注】 执行文件或路径相关配置：file(READ "${crowSocketHeader}" crowSocketSource)。
file(READ "${crowSocketHeader}" crowSocketSource)
# 【逐行批注】 根据构建条件选择配置：if(NOT crowSocketSource MATCHES "remote_endpoint\\(ec\\)")。
if(NOT crowSocketSource MATCHES "remote_endpoint\\(ec\\)")
    # 【逐行批注】 设置构建变量或源码列表：set(crowSocketPatched "${crowSocketSource}")。
    set(crowSocketPatched "${crowSocketSource}")
    # 【逐行批注】 执行 CMake 构建配置：string(REPLACE。
    string(REPLACE
        # 【逐行批注】 执行 CMake 构建配置：" return socket_.remote_endpoint();"。
        "            return socket_.remote_endpoint();"
        # 【逐行批注】 执行 CMake 构建配置：" error_code ec;\n const auto endpoint = socket_.remote_endpoint(ec);\n return ec ? tcp::…。
        "            error_code ec;\n            const auto endpoint = socket_.remote_endpoint(ec);\n            return ec ? tcp::endpoint() : endpoint;"
        # 【逐行批注】 执行 CMake 构建配置：crowSocketPatched "${crowSocketPatched}")。
        crowSocketPatched "${crowSocketPatched}")
    # 【逐行批注】 执行 CMake 构建配置：string(REPLACE。
    string(REPLACE
        # 【逐行批注】 执行 CMake 构建配置：" return raw_socket().remote_endpoint();"。
        "            return raw_socket().remote_endpoint();"
        # 【逐行批注】 执行 CMake 构建配置：" error_code ec;\n const auto endpoint = raw_socket().remote_endpoint(ec);\n return ec ? …。
        "            error_code ec;\n            const auto endpoint = raw_socket().remote_endpoint(ec);\n            return ec ? tcp::endpoint() : endpoint;"
        # 【逐行批注】 执行 CMake 构建配置：crowSocketPatched "${crowSocketPatched}")。
        crowSocketPatched "${crowSocketPatched}")
    # 【逐行批注】 根据构建条件选择配置：if(crowSocketPatched STREQUAL crowSocketSource)。
    if(crowSocketPatched STREQUAL crowSocketSource)
        # 【逐行批注】 执行 CMake 构建配置：message(FATAL_ERROR "无法修复 Crow 1.2.0 的对端地址读取")。
        message(FATAL_ERROR "无法修复 Crow 1.2.0 的对端地址读取")
    # 【逐行批注】 结束构建条件块：endif()。
    endif()
    # 【逐行批注】 执行文件或路径相关配置：file(WRITE "${crowSocketHeader}" "${crowSocketPatched}")。
    file(WRITE "${crowSocketHeader}" "${crowSocketPatched}")
# 【逐行批注】 结束构建条件块：endif()。
endif()
# 请求处理还会调用 address()；提前断开的连接不能让服务线程异常退出。
# 【逐行批注】 执行文件或路径相关配置：file(READ "${crowSocketHeader}" crowSocketSource)。
file(READ "${crowSocketHeader}" crowSocketSource)
# 【逐行批注】 设置构建变量或源码列表：set(crowAddressPatched "${crowSocketSource}")。
set(crowAddressPatched "${crowSocketSource}")
# 【逐行批注】 执行 CMake 构建配置：string(REPLACE。
string(REPLACE
    # 【逐行批注】 执行 CMake 构建配置：" return socket_.remote_endpoint().address().to_string();"。
    "            return socket_.remote_endpoint().address().to_string();"
    # 【逐行批注】 执行 CMake 构建配置：" error_code ec;\n const auto endpoint = socket_.remote_endpoint(ec);\n return ec ? std::…。
    "            error_code ec;\n            const auto endpoint = socket_.remote_endpoint(ec);\n            return ec ? std::string() : endpoint.address().to_string();"
    # 【逐行批注】 执行 CMake 构建配置：crowAddressPatched "${crowAddressPatched}")。
    crowAddressPatched "${crowAddressPatched}")
# 【逐行批注】 执行 CMake 构建配置：string(REPLACE。
string(REPLACE
    # 【逐行批注】 执行 CMake 构建配置：" return ssl_socket_->lowest_layer().remote_endpoint().address().to_string();"。
    "            return ssl_socket_->lowest_layer().remote_endpoint().address().to_string();"
    # 【逐行批注】 执行 CMake 构建配置：" error_code ec;\n const auto endpoint = ssl_socket_->lowest_layer().remote_endpoint(ec);…。
    "            error_code ec;\n            const auto endpoint = ssl_socket_->lowest_layer().remote_endpoint(ec);\n            return ec ? std::string() : endpoint.address().to_string();"
    # 【逐行批注】 执行 CMake 构建配置：crowAddressPatched "${crowAddressPatched}")。
    crowAddressPatched "${crowAddressPatched}")
# 【逐行批注】 根据构建条件选择配置：if(crowAddressPatched MATCHES "remote_endpoint\\(\\)\\.address\\(\\)")。
if(crowAddressPatched MATCHES "remote_endpoint\\(\\)\\.address\\(\\)")
    # 【逐行批注】 执行 CMake 构建配置：message(FATAL_ERROR "Crow 仍存在提前断开时会抛异常的地址读取")。
    message(FATAL_ERROR "Crow 仍存在提前断开时会抛异常的地址读取")
# 【逐行批注】 结束构建条件块：endif()。
endif()
# 【逐行批注】 根据构建条件选择配置：if(NOT crowAddressPatched STREQUAL crowSocketSource)。
if(NOT crowAddressPatched STREQUAL crowSocketSource)
    # 【逐行批注】 执行文件或路径相关配置：file(WRITE "${crowSocketHeader}" "${crowAddressPatched}")。
    file(WRITE "${crowSocketHeader}" "${crowAddressPatched}")
# 【逐行批注】 结束构建条件块：endif()。
endif()
# 【逐行批注】 根据构建条件选择配置：if(WIN32)。
if(WIN32)
    # 【逐行批注】 执行 CMake 构建配置：FetchContent_Declare(。
    FetchContent_Declare(
        # 【逐行批注】 执行 CMake 构建配置：webview。
        webview
        # 【逐行批注】 执行 CMake 构建配置：GIT_REPOSITORY https://github.com/webview/webview.git。
        GIT_REPOSITORY https://github.com/webview/webview.git
        # 【逐行批注】 执行 CMake 构建配置：GIT_TAG 0.12.0。
        GIT_TAG 0.12.0
        # 【逐行批注】 执行 CMake 构建配置：GIT_SHALLOW TRUE。
        GIT_SHALLOW TRUE
    # 【逐行批注】 执行 CMake 构建配置：)。
    )
    # 【逐行批注】 执行 CMake 构建配置：FetchContent_MakeAvailable(webview)。
    FetchContent_MakeAvailable(webview)
# 【逐行批注】 结束构建条件块：endif()。
endif()

# SQLite：Windows 使用本地 amalgamation，Linux 使用系统库。
# 【逐行批注】 根据构建条件选择配置：if(WIN32)。
if(WIN32)
    # 【逐行批注】 设置构建变量或源码列表：set(SQLITE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sqlite)。
    set(SQLITE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sqlite)
    # 【逐行批注】 执行文件或路径相关配置：file(GLOB SQLITE_AMALG_DIR ${SQLITE_SRC}/sqlite-amalgamation-*)。
    file(GLOB SQLITE_AMALG_DIR ${SQLITE_SRC}/sqlite-amalgamation-*)
# 【逐行批注】 结束构建条件块：endif()。
endif()
# 【逐行批注】 根据构建条件选择配置：if(WIN32 AND SQLITE_AMALG_DIR AND EXISTS ${SQLITE_AMALG_DIR}/sqlite3.c)。
if(WIN32 AND SQLITE_AMALG_DIR AND EXISTS ${SQLITE_AMALG_DIR}/sqlite3.c)
    # 【逐行批注】 声明库目标：add_library(sqlite3_amalgamation STATIC ${SQLITE_AMALG_DIR}/sqlite3.c)。
    add_library(sqlite3_amalgamation STATIC ${SQLITE_AMALG_DIR}/sqlite3.c)
    # 【逐行批注】 为构建目标设置头文件路径：target_include_directories(sqlite3_amalgamation PUBLIC ${SQLITE_AMALG_DIR})。
    target_include_directories(sqlite3_amalgamation PUBLIC ${SQLITE_AMALG_DIR})
    # 【逐行批注】 设置构建变量或源码列表：set(GANGYI_SQLITE_LIB sqlite3_amalgamation)。
    set(GANGYI_SQLITE_LIB sqlite3_amalgamation)
# 【逐行批注】 执行条件不成立时的构建配置：else()。
else()
    # 【逐行批注】 查找并加载外部依赖包：find_package(SQLite3 REQUIRED)。
    find_package(SQLite3 REQUIRED)
    # 【逐行批注】 设置构建变量或源码列表：set(GANGYI_SQLITE_LIB SQLite::SQLite3)。
    set(GANGYI_SQLITE_LIB SQLite::SQLite3)
# 【逐行批注】 结束构建条件块：endif()。
endif()
# CURL：Windows 使用本地 MinGW 开发包，Linux 使用系统库。
# 【逐行批注】 根据构建条件选择配置：if(WIN32)。
if(WIN32)
    # 【逐行批注】 设置构建变量或源码列表：set(CURL_DEV_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/curl)。
    set(CURL_DEV_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/curl)
    # 【逐行批注】 执行文件或路径相关配置：file(GLOB CURL_MINGW_DIR ${CURL_DEV_DIR}/curl-*-win64-mingw)。
    file(GLOB CURL_MINGW_DIR ${CURL_DEV_DIR}/curl-*-win64-mingw)
# 【逐行批注】 结束构建条件块：endif()。
endif()
# 【逐行批注】 根据构建条件选择配置：if(WIN32 AND CURL_MINGW_DIR AND EXISTS ${CURL_MINGW_DIR}/include/curl/curl.h)。
if(WIN32 AND CURL_MINGW_DIR AND EXISTS ${CURL_MINGW_DIR}/include/curl/curl.h)
    # 【逐行批注】 设置构建变量或源码列表：set(GANGYI_CURL_INCLUDE ${CURL_MINGW_DIR}/include)。
    set(GANGYI_CURL_INCLUDE ${CURL_MINGW_DIR}/include)
    # 官方 Windows 包的 libcurl DLL 已静态包含传输层依赖，这里只链接导入库。
    # 【逐行批注】 设置构建变量或源码列表：set(GANGYI_CURL_LINK ${CURL_MINGW_DIR}/lib/libcurl.dll.a)。
    set(GANGYI_CURL_LINK ${CURL_MINGW_DIR}/lib/libcurl.dll.a)
# 【逐行批注】 执行条件不成立时的构建配置：else()。
else()
    # 【逐行批注】 查找并加载外部依赖包：find_package(CURL REQUIRED)。
    find_package(CURL REQUIRED)
    # 【逐行批注】 查找并加载外部依赖包：find_package(OpenSSL REQUIRED)。
    find_package(OpenSSL REQUIRED)
    # 【逐行批注】 设置构建变量或源码列表：set(GANGYI_CURL_INCLUDE "")。
    set(GANGYI_CURL_INCLUDE "")
    # 【逐行批注】 设置构建变量或源码列表：set(GANGYI_CURL_LINK CURL::libcurl OpenSSL::Crypto)。
    set(GANGYI_CURL_LINK CURL::libcurl OpenSSL::Crypto)
# 【逐行批注】 结束构建条件块：endif()。
endif()

# 【逐行批注】 声明可执行程序目标：add_executable(gangyiAI src/my_ai_pages.cpp src/main.cpp src/db.cpp src/ai_client.cpp src…。
add_executable(gangyiAI src/my_ai_pages.cpp src/main.cpp src/db.cpp src/ai_client.cpp src/ai_activity.cpp src/json_fix.cpp src/plan_generator.cpp src/page_renderer.cpp
    # 【逐行批注】 执行 CMake 构建配置：src/plan_cache.cpp src/plan_adapter.cpp src/search_client.cpp src/quality_gate.cpp src/co…。
    src/plan_cache.cpp src/plan_adapter.cpp src/search_client.cpp src/quality_gate.cpp src/course_service.cpp
    # 【逐行批注】 执行 CMake 构建配置：src/progress_service.cpp src/profile_service.cpp src/phase_generator.cpp src/ask_generato…。
    src/progress_service.cpp src/profile_service.cpp src/phase_generator.cpp src/ask_generator.cpp src/learning_generator.cpp src/classroom_service.cpp src/image_goal_analyzer.cpp src/home_recommendations.cpp src/learning_flow.cpp src/next_lesson.cpp
    # 【逐行批注】 执行 CMake 构建配置：src/deep_course.cpp src/learning_agent.cpp src/learning_library.cpp src/library_io.cpp sr…。
    src/deep_course.cpp src/learning_agent.cpp src/learning_library.cpp src/library_io.cpp src/agent_stream.cpp src/agent_preferences.cpp src/agent_lessons.cpp src/agent_profiles.cpp src/agent_curriculum.cpp src/my_ai.cpp src/my_ai_tools.cpp src/independent_chat.cpp src/exam_explanation.cpp src/exam_sources.cpp src/exam_page.cpp
    # 【逐行批注】 执行 CMake 构建配置：${GANGYI_SERVICE_RESOURCE})。
    ${GANGYI_SERVICE_RESOURCE})

# 【逐行批注】 为构建目标设置头文件路径：target_include_directories(gangyiAI PRIVATE。
target_include_directories(gangyiAI PRIVATE
    # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/include。
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/src。
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    # 【逐行批注】 执行 CMake 构建配置：${ASIO_INCLUDE_DIR}。
    ${ASIO_INCLUDE_DIR}
    # 【逐行批注】 执行 CMake 构建配置：${GANGYI_CURL_INCLUDE}。
    ${GANGYI_CURL_INCLUDE}
# 【逐行批注】 执行 CMake 构建配置：)。
)
target_link_libraries(gangyiAI PRIVATE gangyi-teaching-prompts)

# 【逐行批注】 为构建目标链接依赖库：target_link_libraries(gangyiAI PRIVATE。
target_link_libraries(gangyiAI PRIVATE
    # 【逐行批注】 执行 CMake 构建配置：Crow::Crow。
    Crow::Crow
    # 【逐行批注】 执行 CMake 构建配置：nlohmann_json::nlohmann_json。
    nlohmann_json::nlohmann_json
    # 【逐行批注】 执行 CMake 构建配置：${GANGYI_SQLITE_LIB}。
    ${GANGYI_SQLITE_LIB}
    # 【逐行批注】 执行 CMake 构建配置：${GANGYI_CURL_LINK}。
    ${GANGYI_CURL_LINK}
# 【逐行批注】 执行 CMake 构建配置：)。
)

# 【逐行批注】 根据构建条件选择配置：if(WIN32)。
if(WIN32)
    # 【逐行批注】 为构建目标链接依赖库：target_link_libraries(gangyiAI PRIVATE ws2_32 mswsock bcrypt crypt32 user32)。
    target_link_libraries(gangyiAI PRIVATE ws2_32 mswsock bcrypt crypt32 user32)
# 【逐行批注】 结束构建条件块：endif()。
endif()

# 【逐行批注】 根据构建条件选择配置：if(WIN32)。
if(WIN32)
    # 【逐行批注】 声明可执行程序目标：add_executable(gangyiAI-launcher WIN32 src/windows_launcher.cpp src/launcher_support.cpp …。
    add_executable(gangyiAI-launcher WIN32 src/windows_launcher.cpp src/launcher_support.cpp src/ai_client.cpp src/ai_activity.cpp src/search_client.cpp ${GANGYI_LAUNCHER_RESOURCE})
    # 【逐行批注】 为构建目标设置头文件路径：target_include_directories(gangyiAI-launcher PRIVATE。
    target_include_directories(gangyiAI-launcher PRIVATE
        # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/include。
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        # 【逐行批注】 执行 CMake 构建配置：${GANGYI_CURL_INCLUDE}。
        ${GANGYI_CURL_INCLUDE}
    # 【逐行批注】 执行 CMake 构建配置：)。
    )
    # 【逐行批注】 为目标设置编译宏：target_compile_definitions(gangyiAI-launcher PRIVATE。
    target_compile_definitions(gangyiAI-launcher PRIVATE
        # 【逐行批注】 执行 CMake 构建配置：UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX。
        UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX
    # 【逐行批注】 执行 CMake 构建配置：)。
    )
    # 【逐行批注】 为构建目标链接依赖库：target_link_libraries(gangyiAI-launcher PRIVATE。
    target_link_libraries(gangyiAI-launcher PRIVATE
        # 【逐行批注】 执行 CMake 构建配置：webview::core。
        webview::core
        # 【逐行批注】 执行 CMake 构建配置：nlohmann_json::nlohmann_json。
        nlohmann_json::nlohmann_json
        # 【逐行批注】 执行 CMake 构建配置：${GANGYI_SQLITE_LIB}。
        ${GANGYI_SQLITE_LIB}
        # 【逐行批注】 执行 CMake 构建配置：${GANGYI_CURL_LINK}。
        ${GANGYI_CURL_LINK}
        # 【逐行批注】 执行 CMake 构建配置：advapi32 bcrypt comdlg32 gdi32 shell32 ole32 ws2_32。
        advapi32 bcrypt comdlg32 gdi32 shell32 ole32 ws2_32
    # 【逐行批注】 执行 CMake 构建配置：)。
    )
# 【逐行批注】 结束构建条件块：endif()。
endif()

# 【逐行批注】 根据构建条件选择配置：if(WIN32)。
if(WIN32)
    # 【逐行批注】 声明可执行程序目标：add_executable(gangyiAI-source-reader src/source_reader.cpp)。
    add_executable(gangyiAI-source-reader src/source_reader.cpp)
    # 【逐行批注】 为构建目标链接依赖库：target_link_libraries(gangyiAI-source-reader PRIVATE webview::core nlohmann_json::nlohman…。
    target_link_libraries(gangyiAI-source-reader PRIVATE webview::core nlohmann_json::nlohmann_json ole32 user32)
    # 【逐行批注】 为目标设置编译宏：target_compile_definitions(gangyiAI-source-reader PRIVATE UNICODE _UNICODE WIN32_LEAN_AND…。
    target_compile_definitions(gangyiAI-source-reader PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
    # 【逐行批注】 执行 CMake 构建配置：add_custom_command(TARGET gangyiAI-source-reader POST_BUILD。
    add_custom_command(TARGET gangyiAI-source-reader POST_BUILD
        # 【逐行批注】 执行 CMake 构建配置：COMMAND ${CMAKE_COMMAND} -E copy_if_different。
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/third_party/pdfium-8100/bin/pdfium.dll $<TARGET_FILE_DIR:gang…。
            ${CMAKE_CURRENT_SOURCE_DIR}/third_party/pdfium-8100/bin/pdfium.dll $<TARGET_FILE_DIR:gangyiAI-source-reader>
        # 【逐行批注】 执行 CMake 构建配置：COMMAND ${CMAKE_COMMAND} -E copy_directory。
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/public/vendor/readability $<TARGET_FILE_DIR:gangyiAI-source-r…。
            ${CMAKE_CURRENT_SOURCE_DIR}/public/vendor/readability $<TARGET_FILE_DIR:gangyiAI-source-reader>/public/vendor/readability)
    # 【逐行批注】 根据构建条件选择配置：if(MINGW)。
    if(MINGW)
        # 【逐行批注】 执行 CMake 构建配置：target_link_options(gangyiAI-source-reader PRIVATE -municode -static-libgcc -static-libst…。
        target_link_options(gangyiAI-source-reader PRIVATE -municode -static-libgcc -static-libstdc++)
    # 【逐行批注】 结束构建条件块：endif()。
    endif()
# 【逐行批注】 结束构建条件块：endif()。
endif()
# 【逐行批注】 根据构建条件选择配置：if(MINGW)。
if(MINGW)
    # 【逐行批注】 执行 CMake 构建配置：target_link_options(gangyiAI PRIVATE -static-libgcc -static-libstdc++)。
    target_link_options(gangyiAI PRIVATE -static-libgcc -static-libstdc++)
    # 【逐行批注】 执行 CMake 构建配置：target_link_options(gangyiAI-launcher PRIVATE -municode -static-libgcc -static-libstdc++)。
    target_link_options(gangyiAI-launcher PRIVATE -municode -static-libgcc -static-libstdc++)
# 【逐行批注】 结束构建条件块：endif()。
endif()

# 【逐行批注】 加载额外 CMake 配置：include(CTest)。
# 交付源码不携带开发测试；缺少测试目录时仍可正常配置产品构建。
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/learning_agent_tests.cpp")
    include(CTest)
else()
    set(BUILD_TESTING OFF)
endif()
# 【逐行批注】 根据构建条件选择配置：if(BUILD_TESTING)。
if(BUILD_TESTING)
    add_executable(model-evidence-tests tests/model_evidence_log_tests.cpp)
    target_include_directories(model-evidence-tests PRIVATE include)
    target_link_libraries(model-evidence-tests PRIVATE nlohmann_json::nlohmann_json)
    add_test(NAME model-evidence COMMAND model-evidence-tests)
    add_executable(review-interval-tests tests/review_interval_tests.cpp)
    target_include_directories(review-interval-tests PRIVATE include)
    target_link_libraries(review-interval-tests PRIVATE nlohmann_json::nlohmann_json)
    add_test(NAME review-interval COMMAND review-interval-tests)
    add_executable(interactive-labs-tests tests/interactive_labs_tests.cpp)
    target_include_directories(interactive-labs-tests PRIVATE include)
    target_link_libraries(interactive-labs-tests PRIVATE nlohmann_json::nlohmann_json)
    add_test(NAME interactive-labs COMMAND interactive-labs-tests "${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/interactive-labs-v1.json")
    add_executable(teaching-policy-tests tests/teaching_policy_tests.cpp)
    target_include_directories(teaching-policy-tests PRIVATE include)
    target_link_libraries(teaching-policy-tests PRIVATE gangyi-teaching-prompts nlohmann_json::nlohmann_json)
    add_test(NAME teaching-policy COMMAND teaching-policy-tests)
    # vNext 的只读学习状态与离线评测不能依赖真实模型或个人档案。
    add_executable(learning-state-tests tests/learning_state_tests.cpp)
    target_include_directories(learning-state-tests PRIVATE include)
    target_link_libraries(learning-state-tests PRIVATE nlohmann_json::nlohmann_json)
    add_test(NAME learning-state COMMAND learning-state-tests)
    add_executable(interactive-sets-tests tests/interactive_sets_tests.cpp)
    target_include_directories(interactive-sets-tests PRIVATE include)
    target_link_libraries(interactive-sets-tests PRIVATE nlohmann_json::nlohmann_json)
    add_test(NAME interactive-sets COMMAND interactive-sets-tests)
    add_executable(exact-calculation-tests tests/exact_calculation_tests.cpp)
    target_include_directories(exact-calculation-tests PRIVATE include)
    target_link_libraries(exact-calculation-tests PRIVATE nlohmann_json::nlohmann_json)
    add_test(NAME exact-calculation COMMAND exact-calculation-tests)

    # 【逐行批注】 声明可执行程序目标：add_executable(next-lesson-tests tests/next_lesson_tests.cpp src/learning_generator.cpp。
    add_executable(next-lesson-tests tests/next_lesson_tests.cpp src/learning_generator.cpp
        # 【逐行批注】 执行 CMake 构建配置：src/ai_client.cpp src/ai_activity.cpp src/search_client.cpp src/json_fix.cpp)。
        src/ai_client.cpp src/ai_activity.cpp src/search_client.cpp src/json_fix.cpp)
    # 【逐行批注】 为构建目标设置头文件路径：target_include_directories(next-lesson-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include …。
    target_include_directories(next-lesson-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${GANGYI_CURL_INCLUDE})
    # 【逐行批注】 为构建目标链接依赖库：target_link_libraries(next-lesson-tests PRIVATE nlohmann_json::nlohmann_json ${GANGYI_CUR…。
    target_link_libraries(next-lesson-tests PRIVATE nlohmann_json::nlohmann_json ${GANGYI_CURL_LINK} Threads::Threads)
    # 【逐行批注】 根据构建条件选择配置：if(WIN32)。
    if(WIN32)
        # 【逐行批注】 为构建目标链接依赖库：target_link_libraries(next-lesson-tests PRIVATE ws2_32)。
        target_link_libraries(next-lesson-tests PRIVATE ws2_32)
    # 【逐行批注】 结束构建条件块：endif()。
    endif()
    # 【逐行批注】 注册自动化测试：add_test(NAME next-lesson COMMAND next-lesson-tests)。
    add_test(NAME next-lesson COMMAND next-lesson-tests)
    # 【逐行批注】 根据构建条件选择配置：if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/question_dialogue_tests.cpp")。
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/question_dialogue_tests.cpp")
        # 【逐行批注】 声明可执行程序目标：add_executable(question-dialogue-tests tests/question_dialogue_tests.cpp src/learning_flo…。
        add_executable(question-dialogue-tests tests/question_dialogue_tests.cpp src/learning_flow.cpp
            # 【逐行批注】 执行 CMake 构建配置：src/next_lesson.cpp src/classroom_service.cpp src/learning_generator.cpp src/profile_serv…。
            src/next_lesson.cpp src/classroom_service.cpp src/learning_generator.cpp src/profile_service.cpp
            # 【逐行批注】 执行 CMake 构建配置：src/db.cpp src/ai_client.cpp src/ai_activity.cpp src/search_client.cpp src/json_fix.cpp s…。
            src/db.cpp src/ai_client.cpp src/ai_activity.cpp src/search_client.cpp src/json_fix.cpp src/course_service.cpp src/progress_service.cpp)
        # 【逐行批注】 为构建目标设置头文件路径：target_include_directories(question-dialogue-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/in…。
        target_include_directories(question-dialogue-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${GANGYI_CURL_INCLUDE})
        # 【逐行批注】 为构建目标链接依赖库：target_link_libraries(question-dialogue-tests PRIVATE nlohmann_json::nlohmann_json ${GANG…。
        target_link_libraries(question-dialogue-tests PRIVATE nlohmann_json::nlohmann_json ${GANGYI_SQLITE_LIB} ${GANGYI_CURL_LINK} Threads::Threads)
        # 【逐行批注】 根据构建条件选择配置：if(WIN32)。
        if(WIN32)
            # 【逐行批注】 为构建目标链接依赖库：target_link_libraries(question-dialogue-tests PRIVATE ws2_32)。
            target_link_libraries(question-dialogue-tests PRIVATE ws2_32)
        # 【逐行批注】 结束构建条件块：endif()。
        endif()
        # 【逐行批注】 注册自动化测试：add_test(NAME question-dialogue COMMAND question-dialogue-tests)。
        add_test(NAME question-dialogue COMMAND question-dialogue-tests)
    # 【逐行批注】 结束构建条件块：endif()。
    endif()
    # 【逐行批注】 查找并加载外部依赖包：find_package(Threads REQUIRED)。
    find_package(Threads REQUIRED)
    # 【逐行批注】 声明可执行程序目标：add_executable(agent-stream-tests tests/agent_stream_tests.cpp src/agent_stream.cpp)。
    add_executable(agent-stream-tests tests/agent_stream_tests.cpp src/agent_stream.cpp)
    # 【逐行批注】 为构建目标设置头文件路径：target_include_directories(agent-stream-tests PRIVATE include)。
    target_include_directories(agent-stream-tests PRIVATE include)
    # 【逐行批注】 为构建目标链接依赖库：target_link_libraries(agent-stream-tests PRIVATE nlohmann_json::nlohmann_json)。
    target_link_libraries(agent-stream-tests PRIVATE nlohmann_json::nlohmann_json)
    # 【逐行批注】 注册自动化测试：add_test(NAME agent-stream COMMAND agent-stream-tests)。
    add_test(NAME agent-stream COMMAND agent-stream-tests)
    # 【逐行批注】 声明可执行程序目标：add_executable(learning-agent-tests tests/learning_agent_tests.cpp src/deep_course.cpp sr…。
    add_executable(learning-agent-tests tests/learning_agent_tests.cpp src/deep_course.cpp src/learning_agent.cpp src/learning_library.cpp src/library_io.cpp src/independent_chat.cpp src/exam_explanation.cpp src/exam_sources.cpp
        # 【逐行批注】 执行 CMake 构建配置：src/agent_stream.cpp src/agent_preferences.cpp src/agent_lessons.cpp src/agent_profiles.c…。
        src/agent_stream.cpp src/agent_preferences.cpp src/agent_lessons.cpp src/agent_profiles.cpp src/agent_curriculum.cpp src/my_ai.cpp src/my_ai_tools.cpp
        # 【逐行批注】 执行 CMake 构建配置：src/learning_flow.cpp src/next_lesson.cpp src/classroom_service.cpp src/learning_generato…。
        src/learning_flow.cpp src/next_lesson.cpp src/classroom_service.cpp src/learning_generator.cpp src/profile_service.cpp
        # 【逐行批注】 执行 CMake 构建配置：src/course_service.cpp src/progress_service.cpp src/db.cpp src/ai_client.cpp src/ai_activ…。
        src/course_service.cpp src/progress_service.cpp src/db.cpp src/ai_client.cpp src/ai_activity.cpp src/search_client.cpp src/json_fix.cpp)
    # 【逐行批注】 为构建目标设置头文件路径：target_include_directories(learning-agent-tests PRIVATE include ${GANGYI_CURL_INCLUDE})。
    target_include_directories(learning-agent-tests PRIVATE include ${GANGYI_CURL_INCLUDE})
    target_link_libraries(learning-agent-tests PRIVATE gangyi-teaching-prompts)
    # 【逐行批注】 为构建目标链接依赖库：target_link_libraries(learning-agent-tests PRIVATE nlohmann_json::nlohmann_json。
    target_link_libraries(learning-agent-tests PRIVATE nlohmann_json::nlohmann_json
        # 【逐行批注】 执行 CMake 构建配置：${GANGYI_SQLITE_LIB} ${GANGYI_CURL_LINK} Threads::Threads)。
        ${GANGYI_SQLITE_LIB} ${GANGYI_CURL_LINK} Threads::Threads)
    # 【逐行批注】 根据构建条件选择配置：if(WIN32)。
    if(WIN32)
        # 【逐行批注】 为构建目标链接依赖库：target_link_libraries(learning-agent-tests PRIVATE ws2_32)。
        target_link_libraries(learning-agent-tests PRIVATE ws2_32)
    # 【逐行批注】 结束构建条件块：endif()。
    endif()
    # 【逐行批注】 执行 CMake 构建配置：target_sources(learning-agent-tests PRIVATE tests/library_checks.cpp tests/deep_course_ch…。
    target_sources(learning-agent-tests PRIVATE tests/library_checks.cpp tests/deep_course_checks.cpp)
    # 【逐行批注】 注册自动化测试：add_test(NAME deep-course COMMAND learning-agent-tests --deep-course-only)。
    add_test(NAME deep-course COMMAND learning-agent-tests --deep-course-only)
    # 【逐行批注】 注册自动化测试：add_test(NAME learning-agent COMMAND learning-agent-tests)。
    add_test(NAME learning-agent COMMAND learning-agent-tests)
    add_test(NAME component-activity-agent COMMAND learning-agent-tests --component-activity-only)
    add_test(NAME exact-calculation-agent COMMAND learning-agent-tests --calculation-only)
    add_test(NAME learning-mastery-agent COMMAND learning-agent-tests --mastery-only)
    add_test(NAME teaching-preferences-agent COMMAND learning-agent-tests --teaching-preferences-only)
  # 【逐行批注】 注册自动化测试：add_test(NAME startup-reuse COMMAND learning-agent-tests --startup-reuse-only)。
  add_test(NAME startup-reuse COMMAND learning-agent-tests --startup-reuse-only)
    # 【逐行批注】 声明可执行程序目标：add_executable(socket-adaptor-tests tests/socket_adaptor_tests.cpp)。
    add_executable(socket-adaptor-tests tests/socket_adaptor_tests.cpp)
    # 【逐行批注】 为构建目标链接依赖库：target_link_libraries(socket-adaptor-tests PRIVATE Crow::Crow Threads::Threads)。
    target_link_libraries(socket-adaptor-tests PRIVATE Crow::Crow Threads::Threads)
    # 【逐行批注】 根据构建条件选择配置：if(WIN32)。
    if(WIN32)
        # 【逐行批注】 为构建目标链接依赖库：target_link_libraries(socket-adaptor-tests PRIVATE ws2_32)。
        target_link_libraries(socket-adaptor-tests PRIVATE ws2_32)
    # 【逐行批注】 结束构建条件块：endif()。
    endif()
    # 【逐行批注】 注册自动化测试：add_test(NAME socket-adaptor COMMAND socket-adaptor-tests)。
    add_test(NAME socket-adaptor COMMAND socket-adaptor-tests)
    # 【逐行批注】 声明可执行程序目标：add_executable(http-lifetime-tests tests/http_lifetime_tests.cpp)。
    add_executable(http-lifetime-tests tests/http_lifetime_tests.cpp)
    # 【逐行批注】 为构建目标链接依赖库：target_link_libraries(http-lifetime-tests PRIVATE Crow::Crow Threads::Threads)。
    target_link_libraries(http-lifetime-tests PRIVATE Crow::Crow Threads::Threads)
    # 【逐行批注】 根据构建条件选择配置：if(WIN32)。
    if(WIN32)
        # 【逐行批注】 为构建目标链接依赖库：target_link_libraries(http-lifetime-tests PRIVATE ws2_32 mswsock)。
        target_link_libraries(http-lifetime-tests PRIVATE ws2_32 mswsock)
    # 【逐行批注】 结束构建条件块：endif()。
    endif()
    # 【逐行批注】 注册自动化测试：add_test(NAME http-lifetime COMMAND http-lifetime-tests)。
    add_test(NAME http-lifetime COMMAND http-lifetime-tests)
    # 【逐行批注】 执行 CMake 构建配置：set_tests_properties(http-lifetime PROPERTIES TIMEOUT 20)。
    set_tests_properties(http-lifetime PROPERTIES TIMEOUT 20)
    # 【逐行批注】 声明可执行程序目标：add_executable(ai-client-tests tests/ai_client_tests.cpp src/ai_client.cpp src/ai_activit…。
    add_executable(ai-client-tests tests/ai_client_tests.cpp src/ai_client.cpp src/ai_activity.cpp src/search_client.cpp src/ask_generator.cpp
        # 【逐行批注】 执行 CMake 构建配置：src/json_fix.cpp src/plan_generator.cpp)。
        src/json_fix.cpp src/plan_generator.cpp)
    # 【逐行批注】 为构建目标设置头文件路径：target_include_directories(ai-client-tests PRIVATE。
    target_include_directories(ai-client-tests PRIVATE
        # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/include。
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        # 【逐行批注】 执行 CMake 构建配置：${GANGYI_CURL_INCLUDE}。
        ${GANGYI_CURL_INCLUDE}
    # 【逐行批注】 执行 CMake 构建配置：)。
    )
    # 【逐行批注】 为构建目标链接依赖库：target_link_libraries(ai-client-tests PRIVATE nlohmann_json::nlohmann_json ${GANGYI_CURL_…。
    target_link_libraries(ai-client-tests PRIVATE nlohmann_json::nlohmann_json ${GANGYI_CURL_LINK} Threads::Threads)
    # 【逐行批注】 根据构建条件选择配置：if(WIN32)。
    if(WIN32)
        # 【逐行批注】 为目标设置编译宏：target_compile_definitions(ai-client-tests PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)。
        target_compile_definitions(ai-client-tests PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
        # 【逐行批注】 为构建目标链接依赖库：target_link_libraries(ai-client-tests PRIVATE ws2_32)。
        target_link_libraries(ai-client-tests PRIVATE ws2_32)
    # 【逐行批注】 结束构建条件块：endif()。
    endif()
    # 【逐行批注】 注册自动化测试：add_test(NAME ai-client COMMAND ai-client-tests)。
    add_test(NAME ai-client COMMAND ai-client-tests)

    # 人民币预算专项调用实际AIClient；夹具端点严格限制为本机。
    add_executable(ai-money-budget-driver tests/ai_money_budget_driver.cpp
        src/ai_client.cpp src/ai_activity.cpp src/search_client.cpp)
    target_include_directories(ai-money-budget-driver PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${GANGYI_CURL_INCLUDE})
    target_link_libraries(ai-money-budget-driver PRIVATE nlohmann_json::nlohmann_json ${GANGYI_CURL_LINK} Threads::Threads)
    if(WIN32)
        target_compile_definitions(ai-money-budget-driver PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
        target_link_libraries(ai-money-budget-driver PRIVATE ws2_32)
        if(MINGW)
            target_link_options(ai-money-budget-driver PRIVATE -municode)
        endif()
    endif()

    # 【逐行批注】 声明可执行程序目标：add_executable(ai-content-generation-tests tests/ai_content_generation_tests.cpp。
    add_executable(ai-content-generation-tests tests/ai_content_generation_tests.cpp
        # 【逐行批注】 执行 CMake 构建配置：src/learning_generator.cpp src/phase_generator.cpp src/ai_client.cpp src/ai_activity.cpp …。
        src/learning_generator.cpp src/phase_generator.cpp src/ai_client.cpp src/ai_activity.cpp src/search_client.cpp src/json_fix.cpp)
    # 【逐行批注】 为构建目标设置头文件路径：target_include_directories(ai-content-generation-tests PRIVATE。
    target_include_directories(ai-content-generation-tests PRIVATE
        # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/include。
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        # 【逐行批注】 执行 CMake 构建配置：${GANGYI_CURL_INCLUDE}。
        ${GANGYI_CURL_INCLUDE}
    # 【逐行批注】 执行 CMake 构建配置：)。
    )
    # 【逐行批注】 为构建目标链接依赖库：target_link_libraries(ai-content-generation-tests PRIVATE。
    target_link_libraries(ai-content-generation-tests PRIVATE
        # 【逐行批注】 执行 CMake 构建配置：nlohmann_json::nlohmann_json ${GANGYI_CURL_LINK} Threads::Threads)。
        nlohmann_json::nlohmann_json ${GANGYI_CURL_LINK} Threads::Threads)
    # 【逐行批注】 根据构建条件选择配置：if(WIN32)。
    if(WIN32)
        # 【逐行批注】 为目标设置编译宏：target_compile_definitions(ai-content-generation-tests PRIVATE WIN32_LEAN_AND_MEAN NOMINM…。
        target_compile_definitions(ai-content-generation-tests PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
        # 【逐行批注】 为构建目标链接依赖库：target_link_libraries(ai-content-generation-tests PRIVATE ws2_32)。
        target_link_libraries(ai-content-generation-tests PRIVATE ws2_32)
    # 【逐行批注】 结束构建条件块：endif()。
    endif()
    # 【逐行批注】 注册自动化测试：add_test(NAME ai-content-generation COMMAND ai-content-generation-tests)。
    add_test(NAME ai-content-generation COMMAND ai-content-generation-tests)

    # 【逐行批注】 声明可执行程序目标：add_executable(progress-service-tests tests/progress_service_tests.cpp src/db.cpp src/pro…。
    add_executable(progress-service-tests tests/progress_service_tests.cpp src/db.cpp src/progress_service.cpp)
    # 【逐行批注】 为构建目标设置头文件路径：target_include_directories(progress-service-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inc…。
    target_include_directories(progress-service-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    # 【逐行批注】 为构建目标链接依赖库：target_link_libraries(progress-service-tests PRIVATE nlohmann_json::nlohmann_json ${GANGY…。
    target_link_libraries(progress-service-tests PRIVATE nlohmann_json::nlohmann_json ${GANGYI_SQLITE_LIB})
    # 【逐行批注】 注册自动化测试：add_test(NAME progress-service COMMAND progress-service-tests)。
    add_test(NAME progress-service COMMAND progress-service-tests)

    add_executable(course-deletion-tests tests/course_deletion_tests.cpp src/db.cpp src/course_service.cpp)
    target_include_directories(course-deletion-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(course-deletion-tests PRIVATE nlohmann_json::nlohmann_json ${GANGYI_SQLITE_LIB})
    add_test(NAME course-deletion COMMAND course-deletion-tests)

    # 【逐行批注】 声明可执行程序目标：add_executable(profile-service-tests tests/profile_service_tests.cpp src/db.cpp src/profi…。
    add_executable(profile-service-tests tests/profile_service_tests.cpp src/db.cpp src/profile_service.cpp src/ai_client.cpp src/ai_activity.cpp src/search_client.cpp src/json_fix.cpp)
    # 【逐行批注】 为构建目标设置头文件路径：target_include_directories(profile-service-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/incl…。
    target_include_directories(profile-service-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${GANGYI_CURL_INCLUDE})
    # 【逐行批注】 为构建目标链接依赖库：target_link_libraries(profile-service-tests PRIVATE nlohmann_json::nlohmann_json ${GANGYI…。
    target_link_libraries(profile-service-tests PRIVATE nlohmann_json::nlohmann_json ${GANGYI_SQLITE_LIB} ${GANGYI_CURL_LINK} Threads::Threads)
    # 【逐行批注】 根据构建条件选择配置：if(WIN32)。
    if(WIN32)
        # 【逐行批注】 为构建目标链接依赖库：target_link_libraries(profile-service-tests PRIVATE ws2_32)。
        target_link_libraries(profile-service-tests PRIVATE ws2_32)
    # 【逐行批注】 结束构建条件块：endif()。
    endif()
    # 【逐行批注】 注册自动化测试：add_test(NAME profile-service COMMAND profile-service-tests)。
    add_test(NAME profile-service COMMAND profile-service-tests)

    # 【逐行批注】 声明可执行程序目标：add_executable(page-renderer-tests tests/page_renderer_tests.cpp src/page_renderer.cpp)。
    add_executable(page-renderer-tests tests/page_renderer_tests.cpp src/page_renderer.cpp)
    # 【逐行批注】 为构建目标设置头文件路径：target_include_directories(page-renderer-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/includ…。
    target_include_directories(page-renderer-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    # 【逐行批注】 为构建目标链接依赖库：target_link_libraries(page-renderer-tests PRIVATE nlohmann_json::nlohmann_json)。
    target_link_libraries(page-renderer-tests PRIVATE nlohmann_json::nlohmann_json)
    # 【逐行批注】 注册自动化测试：add_test(NAME page-renderer COMMAND page-renderer-tests)。
    add_test(NAME page-renderer COMMAND page-renderer-tests)

    # 【逐行批注】 声明可执行程序目标：add_executable(classroom-service-tests tests/classroom_service_tests.cpp src/classroom_se…。
    add_executable(classroom-service-tests tests/classroom_service_tests.cpp src/classroom_service.cpp
        # 【逐行批注】 执行 CMake 构建配置：src/db.cpp src/ai_client.cpp src/ai_activity.cpp src/search_client.cpp src/json_fix.cpp s…。
        src/db.cpp src/ai_client.cpp src/ai_activity.cpp src/search_client.cpp src/json_fix.cpp src/course_service.cpp
        # 【逐行批注】 执行 CMake 构建配置：src/next_lesson.cpp src/learning_flow.cpp src/learning_generator.cpp src/profile_service.…。
        src/next_lesson.cpp src/learning_flow.cpp src/learning_generator.cpp src/profile_service.cpp src/progress_service.cpp)
    # 【逐行批注】 为构建目标设置头文件路径：target_include_directories(classroom-service-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/in…。
    target_include_directories(classroom-service-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${GANGYI_CURL_INCLUDE})
    # 【逐行批注】 为构建目标链接依赖库：target_link_libraries(classroom-service-tests PRIVATE nlohmann_json::nlohmann_json。
    target_link_libraries(classroom-service-tests PRIVATE nlohmann_json::nlohmann_json
        # 【逐行批注】 执行 CMake 构建配置：${GANGYI_SQLITE_LIB} ${GANGYI_CURL_LINK} Threads::Threads)。
        ${GANGYI_SQLITE_LIB} ${GANGYI_CURL_LINK} Threads::Threads)
    # 【逐行批注】 根据构建条件选择配置：if(WIN32)。
    if(WIN32)
        # 【逐行批注】 为构建目标链接依赖库：target_link_libraries(classroom-service-tests PRIVATE ws2_32)。
        target_link_libraries(classroom-service-tests PRIVATE ws2_32)
    # 【逐行批注】 结束构建条件块：endif()。
    endif()
    # 【逐行批注】 注册自动化测试：add_test(NAME classroom-service COMMAND classroom-service-tests)。
    add_test(NAME classroom-service COMMAND classroom-service-tests)
# 【逐行批注】 结束构建条件块：endif()。
endif()
# 【逐行批注】 根据构建条件选择配置：if(BUILD_TESTING AND WIN32)。
if(BUILD_TESTING AND WIN32)
    # 【逐行批注】 声明可执行程序目标：add_executable(launcher-support-tests tests/launcher_support_tests.cpp src/launcher_suppo…。
    add_executable(launcher-support-tests tests/launcher_support_tests.cpp src/launcher_support.cpp)
    # 【逐行批注】 为构建目标设置头文件路径：target_include_directories(launcher-support-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inc…。
    target_include_directories(launcher-support-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    # 【逐行批注】 为目标设置编译宏：target_compile_definitions(launcher-support-tests PRIVATE UNICODE _UNICODE WIN32_LEAN_AND…。
    target_compile_definitions(launcher-support-tests PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
    # 【逐行批注】 为构建目标链接依赖库：target_link_libraries(launcher-support-tests PRIVATE ${GANGYI_SQLITE_LIB})。
    target_link_libraries(launcher-support-tests PRIVATE ${GANGYI_SQLITE_LIB})
    # 【逐行批注】 注册自动化测试：add_test(NAME launcher-support COMMAND launcher-support-tests)。
    add_test(NAME launcher-support COMMAND launcher-support-tests)
    # 【逐行批注】 注册自动化测试：add_test(NAME launcher-native-http COMMAND gangyiAI-launcher --self-test)。
    add_test(NAME launcher-native-http COMMAND gangyiAI-launcher --self-test)
    # 【逐行批注】 执行 CMake 构建配置：set_tests_properties(launcher-native-http PROPERTIES TIMEOUT 30)。
    set_tests_properties(launcher-native-http PROPERTIES TIMEOUT 30)
# 【逐行批注】 结束构建条件块：endif()。
endif()
# 【逐行批注】 根据构建条件选择配置：if(BUILD_TESTING)。
if(BUILD_TESTING)
    # 【逐行批注】 声明可执行程序目标：add_executable(database-atomic-tests tests/database_atomic_tests.cpp src/db.cpp)。
    add_executable(database-atomic-tests tests/database_atomic_tests.cpp src/db.cpp)
    # 【逐行批注】 为构建目标设置头文件路径：target_include_directories(database-atomic-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/incl…。
    target_include_directories(database-atomic-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    # 【逐行批注】 为构建目标链接依赖库：target_link_libraries(database-atomic-tests PRIVATE nlohmann_json::nlohmann_json ${GANGYI…。
    target_link_libraries(database-atomic-tests PRIVATE nlohmann_json::nlohmann_json ${GANGYI_SQLITE_LIB})
    # 【逐行批注】 注册自动化测试：add_test(NAME database-atomic COMMAND database-atomic-tests)。
    add_test(NAME database-atomic COMMAND database-atomic-tests)
    add_executable(exam-explanation-service-tests tests/exam_explanation_service_tests.cpp src/exam_explanation.cpp src/exam_sources.cpp src/library_io.cpp src/search_client.cpp src/db.cpp)
    target_include_directories(exam-explanation-service-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${GANGYI_CURL_INCLUDE})
    target_link_libraries(exam-explanation-service-tests PRIVATE nlohmann_json::nlohmann_json ${GANGYI_SQLITE_LIB} ${GANGYI_CURL_LINK} Threads::Threads)
    if(WIN32)
        target_link_libraries(exam-explanation-service-tests PRIVATE ws2_32)
    endif()
    add_test(NAME exam-explanation-service COMMAND exam-explanation-service-tests)
    add_executable(exam-sources-tests tests/exam_sources_tests.cpp src/exam_sources.cpp src/exam_explanation.cpp src/db.cpp src/library_io.cpp src/search_client.cpp)
    target_include_directories(exam-sources-tests PRIVATE include ${GANGYI_CURL_INCLUDE})
    target_link_libraries(exam-sources-tests PRIVATE nlohmann_json::nlohmann_json ${GANGYI_SQLITE_LIB} ${GANGYI_CURL_LINK} Threads::Threads)
    if(WIN32)
        target_link_libraries(exam-sources-tests PRIVATE ws2_32)
    endif()
    add_test(NAME exam-sources COMMAND exam-sources-tests)
    add_executable(exam-message-stream-tests tests/exam_message_stream_tests.cpp)
    target_include_directories(exam-message-stream-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(exam-message-stream-tests PRIVATE nlohmann_json::nlohmann_json)
    add_test(NAME exam-message-stream COMMAND exam-message-stream-tests)
    # 【逐行批注】 声明可执行程序目标：add_executable(shared-pointer-tests tests/shared_pointer_tests.cpp)。
    add_executable(shared-pointer-tests tests/shared_pointer_tests.cpp)
    # 【逐行批注】 注册自动化测试：add_test(NAME shared-pointer COMMAND shared-pointer-tests)。
    add_test(NAME shared-pointer COMMAND shared-pointer-tests)
    # 【逐行批注】 执行 CMake 构建配置：set_tests_properties(shared-pointer PROPERTIES TIMEOUT 20)。
    set_tests_properties(shared-pointer PROPERTIES TIMEOUT 20)
    # 【逐行批注】 查找并加载外部依赖包：find_package(Python3 COMPONENTS Interpreter QUIET)。
    find_package(Python3 COMPONENTS Interpreter QUIET)
    # 【逐行批注】 执行 CMake 构建配置：find_program(GANGYI_NODE_EXECUTABLE NAMES node)。
    find_program(GANGYI_NODE_EXECUTABLE NAMES node)
    # 【逐行批注】 根据构建条件选择配置：if(GANGYI_NODE_EXECUTABLE)。
    if(GANGYI_NODE_EXECUTABLE)
        add_test(NAME interactive-labs-js COMMAND ${GANGYI_NODE_EXECUTABLE} --test ${CMAKE_CURRENT_SOURCE_DIR}/tests/interactive_labs_tests.cjs)
        # 【逐行批注】 注册自动化测试：add_test(NAME chat-links COMMAND ${GANGYI_NODE_EXECUTABLE}。
        add_test(NAME chat-links COMMAND ${GANGYI_NODE_EXECUTABLE}
            # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/tests/chat_link_tests.cjs)。
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/chat_link_tests.cjs)
        add_test(NAME chat-table COMMAND ${GANGYI_NODE_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/chat_table_tests.cjs)
        add_test(NAME exam-import COMMAND ${GANGYI_NODE_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/exam_import_module_tests.cjs)
        add_test(NAME exam-rich-output COMMAND ${GANGYI_NODE_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/exam_rich_output_tests.cjs)
        add_test(NAME interactive-learning COMMAND ${GANGYI_NODE_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/interactive_learning_tests.cjs)
        # 【逐行批注】 注册自动化测试：add_test(NAME help-display COMMAND ${GANGYI_NODE_EXECUTABLE}。
        add_test(NAME help-display COMMAND ${GANGYI_NODE_EXECUTABLE}
            # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/tests/help_display_smoke.js)。
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/help_display_smoke.js)
        # 【逐行批注】 注册自动化测试：add_test(NAME legacy-drafts COMMAND ${GANGYI_NODE_EXECUTABLE}。
        add_test(NAME legacy-drafts COMMAND ${GANGYI_NODE_EXECUTABLE}
            # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/tests/legacy_draft_tests.cjs)。
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/legacy_draft_tests.cjs)
        # 【逐行批注】 注册自动化测试：add_test(NAME agent-watch COMMAND ${GANGYI_NODE_EXECUTABLE}。
        add_test(NAME agent-watch COMMAND ${GANGYI_NODE_EXECUTABLE}
            # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/tests/agent_watch_tests.cjs)。
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/agent_watch_tests.cjs)
        # 【逐行批注】 注册自动化测试：add_test(NAME agent-status-panel COMMAND ${GANGYI_NODE_EXECUTABLE}。
        add_test(NAME agent-status-panel COMMAND ${GANGYI_NODE_EXECUTABLE}
            # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/tests/agent_status_panel_tests.cjs)。
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/agent_status_panel_tests.cjs)
        # 【逐行批注】 注册自动化测试：add_test(NAME agent-trace COMMAND ${GANGYI_NODE_EXECUTABLE}。
        add_test(NAME agent-trace COMMAND ${GANGYI_NODE_EXECUTABLE}
            # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/tests/agent_trace_tests.cjs)。
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/agent_trace_tests.cjs)
        # 【逐行批注】 注册自动化测试：add_test(NAME mentor-client COMMAND ${GANGYI_NODE_EXECUTABLE}。
        add_test(NAME mentor-client COMMAND ${GANGYI_NODE_EXECUTABLE}
            # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/tests/mentor_client_tests.cjs)。
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/mentor_client_tests.cjs)
        # 【逐行批注】 注册自动化测试：add_test(NAME library-course COMMAND ${GANGYI_NODE_EXECUTABLE}。
        add_test(NAME library-course COMMAND ${GANGYI_NODE_EXECUTABLE}
            # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/tests/library_course_tests.cjs)。
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/library_course_tests.cjs)
    # 【逐行批注】 结束构建条件块：endif()。
    endif()
    # 【逐行批注】 根据构建条件选择配置：if(Python3_Interpreter_FOUND)。
    if(Python3_Interpreter_FOUND)
        # 开发评测测试不进入源码交付包；原生入口由独立构建脚本刷新。
        add_test(NAME tutor-experiment COMMAND ${Python3_EXECUTABLE} -B -X utf8
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/tutor_experiment_tests.py -v)
        add_test(NAME ai-money-budget COMMAND ${Python3_EXECUTABLE} -B -X utf8
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/ai_money_budget_tests.py $<TARGET_FILE:ai-money-budget-driver>)
        set_tests_properties(ai-money-budget PROPERTIES TIMEOUT 90)
        if(WIN32 AND EXISTS "${CMAKE_BINARY_DIR}/tutor-eval-driver.exe")
            add_test(NAME tutor-experiment-native COMMAND ${Python3_EXECUTABLE} -B -X utf8
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/tutor_experiment_native_smoke.py
                ${CMAKE_BINARY_DIR}/tutor-eval-driver.exe)
            set_tests_properties(tutor-experiment-native PROPERTIES TIMEOUT 240)
        endif()
        add_test(NAME tutor-benchmark COMMAND ${Python3_EXECUTABLE} -B -X utf8
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/tutor_benchmark_tests.py -v)
        add_test(NAME tutor-component-scenes COMMAND ${Python3_EXECUTABLE} -B -X utf8
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/tutor_component_scenes_tests.py -v)
        # 【逐行批注】 注册自动化测试：add_test(NAME release-publish COMMAND ${Python3_EXECUTABLE} -B -X utf8。
        add_test(NAME release-publish COMMAND ${Python3_EXECUTABLE} -B -X utf8
            # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/tests/release_publish_tests.py -v)。
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/release_publish_tests.py -v)
        # 【逐行批注】 注册自动化测试：add_test(NAME local-request-guard COMMAND ${Python3_EXECUTABLE} -B -X utf8。
        add_test(NAME local-request-guard COMMAND ${Python3_EXECUTABLE} -B -X utf8
            # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/tests/local_request_guard_smoke.py $<TARGET_FILE:gangyiAI>)。
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/local_request_guard_smoke.py $<TARGET_FILE:gangyiAI>)
        # 【逐行批注】 执行 CMake 构建配置：set_tests_properties(local-request-guard PROPERTIES TIMEOUT 90)。
        set_tests_properties(local-request-guard PROPERTIES TIMEOUT 90)
        # 【逐行批注】 声明可执行程序目标：add_executable(dual-search-driver tests/dual_search_driver.cpp src/search_client.cpp)。
        add_executable(dual-search-driver tests/dual_search_driver.cpp src/search_client.cpp)
        # 【逐行批注】 为构建目标设置头文件路径：target_include_directories(dual-search-driver PRIVATE include ${GANGYI_CURL_INCLUDE})。
        target_include_directories(dual-search-driver PRIVATE include ${GANGYI_CURL_INCLUDE})
        # 【逐行批注】 为构建目标链接依赖库：target_link_libraries(dual-search-driver PRIVATE nlohmann_json::nlohmann_json ${GANGYI_CU…。
        target_link_libraries(dual-search-driver PRIVATE nlohmann_json::nlohmann_json ${GANGYI_CURL_LINK} Threads::Threads)
        # 【逐行批注】 注册自动化测试：add_test(NAME dual-search COMMAND ${Python3_EXECUTABLE} -X utf8。
        add_test(NAME dual-search COMMAND ${Python3_EXECUTABLE} -X utf8
            # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/tests/dual_search_smoke.py $<TARGET_FILE:dual-search-driver>)。
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/dual_search_smoke.py $<TARGET_FILE:dual-search-driver>)
        # 【逐行批注】 执行 CMake 构建配置：set_tests_properties(dual-search PROPERTIES TIMEOUT 50)。
        set_tests_properties(dual-search PROPERTIES TIMEOUT 50)
        # 【逐行批注】 注册自动化测试：add_test(NAME search-reuse COMMAND ${Python3_EXECUTABLE} -X utf8。
        add_test(NAME search-reuse COMMAND ${Python3_EXECUTABLE} -X utf8
            # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/tests/search_reuse_smoke.py $<TARGET_FILE:dual-search-driver>)。
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/search_reuse_smoke.py $<TARGET_FILE:dual-search-driver>)
        # 【逐行批注】 执行 CMake 构建配置：set_tests_properties(search-reuse PROPERTIES TIMEOUT 60)。
        set_tests_properties(search-reuse PROPERTIES TIMEOUT 60)
        # 【逐行批注】 注册自动化测试：add_test(NAME dual-search-library COMMAND ${Python3_EXECUTABLE} -X utf8。
        add_test(NAME dual-search-library COMMAND ${Python3_EXECUTABLE} -X utf8
            # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/tests/dual_search_library_smoke.py $<TARGET_FILE:gangyiAI>)。
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/dual_search_library_smoke.py $<TARGET_FILE:gangyiAI>)
        # 【逐行批注】 执行 CMake 构建配置：set_tests_properties(dual-search-library PROPERTIES TIMEOUT 60)。
        set_tests_properties(dual-search-library PROPERTIES TIMEOUT 60)
        # 【逐行批注】 注册自动化测试：add_test(NAME evaluation-entry COMMAND ${Python3_EXECUTABLE} -X utf8。
        add_test(NAME evaluation-entry COMMAND ${Python3_EXECUTABLE} -X utf8
            # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/tests/evaluation_entry_tests.py -v)。
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/evaluation_entry_tests.py -v)
        # 【逐行批注】 根据构建条件选择配置：if(WIN32)。
        if(WIN32)
          # 【逐行批注】 注册自动化测试：add_test(NAME library-reader COMMAND ${Python3_EXECUTABLE}。
          add_test(NAME library-reader COMMAND ${Python3_EXECUTABLE}
              # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/tests/library_reader_smoke.py $<TARGET_FILE:gangyiAI-source-r…。
              ${CMAKE_CURRENT_SOURCE_DIR}/tests/library_reader_smoke.py $<TARGET_FILE:gangyiAI-source-reader>)
          # 【逐行批注】 注册自动化测试：add_test(NAME library-api COMMAND ${Python3_EXECUTABLE}。
          add_test(NAME library-api COMMAND ${Python3_EXECUTABLE}
              # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/tests/library_api_smoke.py $<TARGET_FILE:gangyiAI>)。
              ${CMAKE_CURRENT_SOURCE_DIR}/tests/library_api_smoke.py $<TARGET_FILE:gangyiAI>)
        # 【逐行批注】 结束构建条件块：endif()。
        endif()
        # 【逐行批注】 注册自动化测试：add_test(NAME global-ai-activity COMMAND ${Python3_EXECUTABLE}。
        add_test(NAME global-ai-activity COMMAND ${Python3_EXECUTABLE}
            # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/tests/global_ai_activity_smoke.py $<TARGET_FILE:gangyiAI> $<T…。
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/global_ai_activity_smoke.py $<TARGET_FILE:gangyiAI> $<TARGET_FILE:ai-client-tests>)
        # 【逐行批注】 执行 CMake 构建配置：set_tests_properties(global-ai-activity PROPERTIES TIMEOUT 90)。
        set_tests_properties(global-ai-activity PROPERTIES TIMEOUT 90)
        # 旧独立后台与固定教学规则的运行时验收迁移为统一主控协议验收。
        # 【逐行批注】 注册自动化测试：add_test(NAME learning-agent-api COMMAND ${Python3_EXECUTABLE}。
        add_test(NAME learning-agent-api COMMAND ${Python3_EXECUTABLE}
            # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/tests/learning_agent_api_smoke.py $<TARGET_FILE:gangyiAI>)。
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/learning_agent_api_smoke.py $<TARGET_FILE:gangyiAI>)
        add_test(NAME learning-agent-recovery COMMAND ${Python3_EXECUTABLE} -X utf8
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/learning_agent_recovery_smoke.py $<TARGET_FILE:gangyiAI>)
        set_tests_properties(learning-agent-recovery PROPERTIES TIMEOUT 180)
        # 【逐行批注】 注册自动化测试：add_test(NAME v560-revision-api COMMAND ${Python3_EXECUTABLE}。
        add_test(NAME v560-revision-api COMMAND ${Python3_EXECUTABLE}
            # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/tests/v560_revision_api_smoke.py $<TARGET_FILE:gangyiAI>)。
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/v560_revision_api_smoke.py $<TARGET_FILE:gangyiAI>)
        # 【逐行批注】 注册自动化测试：add_test(NAME mentor-socket COMMAND ${Python3_EXECUTABLE}。
        add_test(NAME mentor-socket COMMAND ${Python3_EXECUTABLE}
            # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/tests/mentor_socket_smoke.py $<TARGET_FILE:gangyiAI>)。
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/mentor_socket_smoke.py $<TARGET_FILE:gangyiAI>)
    # 【逐行批注】 结束构建条件块：endif()。
    endif()
# 【逐行批注】 结束构建条件块：endif()。
endif()

# 【逐行批注】 执行 CMake 构建配置：add_custom_command(TARGET gangyiAI POST_BUILD。
add_custom_command(TARGET gangyiAI POST_BUILD
    # 【逐行批注】 执行 CMake 构建配置：COMMAND ${CMAKE_COMMAND} -E copy_directory。
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        # 【逐行批注】 执行 CMake 构建配置：${CMAKE_CURRENT_SOURCE_DIR}/public。
        ${CMAKE_CURRENT_SOURCE_DIR}/public
        # 【逐行批注】 执行 CMake 构建配置：$<TARGET_FILE_DIR:gangyiAI>/public。
        $<TARGET_FILE_DIR:gangyiAI>/public
# 【逐行批注】 执行 CMake 构建配置：)。
)

# Windows：复制 curl 运行时 DLL。
# 【逐行批注】 根据构建条件选择配置：if(GANGYI_CURL_INCLUDE AND EXISTS ${CURL_MINGW_DIR}/bin/libcurl-x64.dll)。
if(GANGYI_CURL_INCLUDE AND EXISTS ${CURL_MINGW_DIR}/bin/libcurl-x64.dll)
    # 【逐行批注】 执行 CMake 构建配置：add_custom_command(TARGET gangyiAI POST_BUILD。
    add_custom_command(TARGET gangyiAI POST_BUILD
        # 【逐行批注】 执行 CMake 构建配置：COMMAND ${CMAKE_COMMAND} -E copy_if_different。
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            # 【逐行批注】 执行 CMake 构建配置：${CURL_MINGW_DIR}/bin/libcurl-x64.dll。
            ${CURL_MINGW_DIR}/bin/libcurl-x64.dll
            # 【逐行批注】 执行 CMake 构建配置：$<TARGET_FILE_DIR:gangyiAI>/libcurl-x64.dll。
            $<TARGET_FILE_DIR:gangyiAI>/libcurl-x64.dll
    # 【逐行批注】 执行 CMake 构建配置：)。
    )
    # 【逐行批注】 根据构建条件选择配置：if(EXISTS ${CURL_MINGW_DIR}/bin/curl-ca-bundle.crt)。
    if(EXISTS ${CURL_MINGW_DIR}/bin/curl-ca-bundle.crt)
        # 【逐行批注】 执行 CMake 构建配置：add_custom_command(TARGET gangyiAI POST_BUILD。
        add_custom_command(TARGET gangyiAI POST_BUILD
            # 【逐行批注】 执行 CMake 构建配置：COMMAND ${CMAKE_COMMAND} -E copy_if_different。
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                # 【逐行批注】 执行 CMake 构建配置：${CURL_MINGW_DIR}/bin/curl-ca-bundle.crt。
                ${CURL_MINGW_DIR}/bin/curl-ca-bundle.crt
                # 【逐行批注】 执行 CMake 构建配置：$<TARGET_FILE_DIR:gangyiAI>/curl-ca-bundle.crt。
                $<TARGET_FILE_DIR:gangyiAI>/curl-ca-bundle.crt
        # 【逐行批注】 执行 CMake 构建配置：)。
        )
    # 【逐行批注】 结束构建条件块：endif()。
    endif()
# 【逐行批注】 结束构建条件块：endif()。
endif()

# 【逐行批注】 根据构建条件选择配置：if(BUILD_TESTING)。
if(BUILD_TESTING)
  # 【逐行批注】 声明可执行程序目标：add_executable(my-ai-tests tests/my_ai_tests.cpp src/my_ai.cpp src/db.cpp src/course_serv…。
  add_executable(my-ai-tests tests/my_ai_tests.cpp src/my_ai.cpp src/db.cpp src/course_service.cpp src/profile_service.cpp src/ai_client.cpp src/ai_activity.cpp src/search_client.cpp src/json_fix.cpp)
  # 【逐行批注】 为构建目标设置头文件路径：target_include_directories(my-ai-tests PRIVATE include ${GANGYI_CURL_INCLUDE})。
  target_include_directories(my-ai-tests PRIVATE include ${GANGYI_CURL_INCLUDE})
  # 【逐行批注】 为构建目标链接依赖库：target_link_libraries(my-ai-tests PRIVATE nlohmann_json::nlohmann_json ${GANGYI_SQLITE_LI…。
  target_link_libraries(my-ai-tests PRIVATE nlohmann_json::nlohmann_json ${GANGYI_SQLITE_LIB} ${GANGYI_CURL_LINK} Threads::Threads)
  # 【逐行批注】 注册自动化测试：add_test(NAME my-ai COMMAND my-ai-tests)。
  add_test(NAME my-ai COMMAND my-ai-tests)
# 【逐行批注】 结束构建条件块：endif()。
endif()

# 调用统计采用 SQLite，所有模型客户端目标都连接同一依赖。
# 【逐行批注】 执行 CMake 构建配置：get_property(gangyiTargets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)。
get_property(gangyiTargets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
# 【逐行批注】 遍历构建集合：foreach(gangyiTarget IN LISTS gangyiTargets)。
foreach(gangyiTarget IN LISTS gangyiTargets)
  # 【逐行批注】 执行 CMake 构建配置：get_target_property(gangyiTargetType ${gangyiTarget} TYPE)。
  get_target_property(gangyiTargetType ${gangyiTarget} TYPE)
  # 【逐行批注】 根据构建条件选择配置：if(gangyiTargetType STREQUAL "EXECUTABLE")。
  if(gangyiTargetType STREQUAL "EXECUTABLE")
    # 【逐行批注】 为构建目标链接依赖库：target_link_libraries(${gangyiTarget} PRIVATE ${GANGYI_SQLITE_LIB})。
    target_link_libraries(${gangyiTarget} PRIVATE ${GANGYI_SQLITE_LIB})
  # 【逐行批注】 结束构建条件块：endif()。
  endif()
# 【逐行批注】 结束构建循环：endforeach()。
endforeach()

# 【逐行批注】 声明可配置的构建选项：option(GANGYI_PREVIEW_BUILD "使用独立的我的AI试用档案" OFF)。
option(GANGYI_PREVIEW_BUILD "使用独立的我的AI试用档案" OFF)
# 【逐行批注】 声明可配置的构建选项：option(GANGYI_OFFLINE_CANDIDATE "显示离线候选版本说明" OFF)。
option(GANGYI_OFFLINE_CANDIDATE "显示离线候选版本说明" OFF)
# 【逐行批注】 根据构建条件选择配置：if(GANGYI_OFFLINE_CANDIDATE)。
if(GANGYI_OFFLINE_CANDIDATE)
  # 【逐行批注】 为目标设置编译宏：target_compile_definitions(gangyiAI PRIVATE GANGYI_OFFLINE_CANDIDATE=1)。
  target_compile_definitions(gangyiAI PRIVATE GANGYI_OFFLINE_CANDIDATE=1)
  # 【逐行批注】 根据构建条件选择配置：if(WIN32)。
  if(WIN32)
    # 【逐行批注】 为目标设置编译宏：target_compile_definitions(gangyiAI-launcher PRIVATE GANGYI_OFFLINE_CANDIDATE=1)。
    target_compile_definitions(gangyiAI-launcher PRIVATE GANGYI_OFFLINE_CANDIDATE=1)
  # 【逐行批注】 结束构建条件块：endif()。
  endif()
# 【逐行批注】 结束构建条件块：endif()。
endif()
# 【逐行批注】 声明可配置的构建选项：option(GANGYI_TEACHING_PREVIEW "使用独立的教学优化试用档案" OFF)。
option(GANGYI_TEACHING_PREVIEW "使用独立的教学优化试用档案" OFF)
# 【逐行批注】 根据构建条件选择配置：if(GANGYI_TEACHING_PREVIEW AND WIN32)。
if(GANGYI_TEACHING_PREVIEW AND WIN32)
  # 【逐行批注】 为目标设置编译宏：target_compile_definitions(gangyiAI-launcher PRIVATE GANGYI_TEACHING_PREVIEW=1)。
  target_compile_definitions(gangyiAI-launcher PRIVATE GANGYI_TEACHING_PREVIEW=1)
  # 【逐行批注】 为目标设置编译宏：target_compile_definitions(gangyiAI PRIVATE GANGYI_TEACHING_PREVIEW=1)。
  target_compile_definitions(gangyiAI PRIVATE GANGYI_TEACHING_PREVIEW=1)
# 【逐行批注】 结束构建条件块：endif()。
endif()
# 【逐行批注】 根据构建条件选择配置：if(GANGYI_PREVIEW_BUILD AND WIN32)。
if(GANGYI_PREVIEW_BUILD AND WIN32)
  # 【逐行批注】 为目标设置编译宏：target_compile_definitions(gangyiAI-launcher PRIVATE GANGYI_MY_AI_PREVIEW=1)。
  target_compile_definitions(gangyiAI-launcher PRIVATE GANGYI_MY_AI_PREVIEW=1)
# 【逐行批注】 结束构建条件块：endif()。
endif()
