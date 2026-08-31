cmake_minimum_required(VERSION 3.20)

project(gangyiAI VERSION 0.1.0 LANGUAGES CXX)

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

find_package(SQLite3 REQUIRED)
find_package(CURL REQUIRED)

add_executable(gangyiAI src/main.cpp src/db.cpp src/ai_client.cpp src/json_fix.cpp)

target_include_directories(gangyiAI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(gangyiAI PRIVATE
    Crow::Crow
    nlohmann_json::nlohmann_json
    SQLite::SQLite3
    CURL::libcurl
)

option(GANGYI_BUILD_DB_TEST "Build the standalone database smoke test" ON)
if(GANGYI_BUILD_DB_TEST)
    add_executable(db_test src/db_main_test.cpp src/db.cpp)
    target_include_directories(db_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    target_link_libraries(db_test PRIVATE SQLite::SQLite3)
endif()

option(GANGYI_BUILD_JSON_FIX_TEST "Build the AI JSON parser test" ON)
if(GANGYI_BUILD_JSON_FIX_TEST)
    add_executable(json_fix_test src/json_fix_test.cpp src/json_fix.cpp src/ai_client.cpp)
    target_include_directories(json_fix_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(json_fix_test PRIVATE nlohmann_json::nlohmann_json CURL::libcurl)
endif()

add_custom_command(TARGET gangyiAI POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/public
        $<TARGET_FILE_DIR:gangyiAI>/public
)
