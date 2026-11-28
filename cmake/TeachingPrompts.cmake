# 提示词来自受版本管理的固定目录；构建时嵌入，不从学生资料或运行目录加载规则。
set(GANGYI_TEACHING_MODULE_IDS
    core/system core/behavior-policy core/safety-policy
    teaching/teaching-planner teaching/concept-explainer teaching/socratic-tutor
    teaching/answer-diagnosis teaching/feedback-generator teaching/mastery-evaluator
    subjects/mathematics subjects/physics subjects/chemistry subjects/biology
    subjects/history subjects/geography subjects/chinese subjects/english subjects/computer-science
    components/component-selector components/component-generator components/component-explainer
    evaluation/quality-evaluator evaluation/factuality-checker)
set(_teaching_header "#pragma once\nnamespace gangyi {\nstruct TeachingPromptModule { const char* id; const char* version; const char* digest; const char* body; };\ninline constexpr TeachingPromptModule kTeachingPromptModules[] = {\n")
foreach(_module IN LISTS GANGYI_TEACHING_MODULE_IDS)
    set(_path "${CMAKE_CURRENT_SOURCE_DIR}/prompts/${_module}.md")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_path}")
    file(READ "${_path}" _content)
    foreach(_field 版本 角色 输入 输出 教学目标 约束 错误处理 评估标准)
        if(NOT _content MATCHES "${_field}：[^\n]+")
            message(FATAL_ERROR "教学模块 ${_module} 缺少 ${_field}")
        endif()
    endforeach()
    string(FIND "${_content}" ")GY_PROMPT\"" _delimiter)
    if(NOT _delimiter EQUAL -1)
        message(FATAL_ERROR "教学模块 ${_module} 含不允许的嵌入分隔符")
    endif()
    # 原有主控规则逐字迁移；元数据留在源文件和摘要中，不反复扩充模型上下文。
    string(REGEX REPLACE ".*\n约束：" "" _body "${_content}")
    string(REGEX REPLACE "\n错误处理：.*" "" _body "${_body}")
    # file(READ)会规范化Windows换行；记录原始文件摘要才能与实际源码逐字节核对。
    file(SHA256 "${_path}" _digest)
    string(REGEX MATCH "版本：([A-Za-z0-9-]+)" _version_match "${_content}")
    set(_version "${CMAKE_MATCH_1}")
    if(_version STREQUAL "")
        message(FATAL_ERROR "教学模块 ${_module} 版本格式无效")
    endif()
    string(APPEND _teaching_header "{\"${_module}\", \"${_version}\", \"${_digest}\", u8R\"GY_PROMPT(${_body})GY_PROMPT\"},\n")
endforeach()
string(APPEND _teaching_header "};\n}\n")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/generated/teaching_prompt_modules.tmp" "${_teaching_header}")
configure_file("${CMAKE_CURRENT_BINARY_DIR}/generated/teaching_prompt_modules.tmp"
               "${CMAKE_CURRENT_BINARY_DIR}/generated/teaching_prompt_modules.hpp" COPYONLY)
add_library(gangyi-teaching-prompts INTERFACE)
target_include_directories(gangyi-teaching-prompts INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/generated")
