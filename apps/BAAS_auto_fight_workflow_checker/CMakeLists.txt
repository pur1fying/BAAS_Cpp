BAAS_hr_LOG("BAAS Auto Fight Workflow Checker")

# data.yaml for valid yolo obj names
# skill_active.json for all skill names

if (MSVC)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/MP>")
    add_compile_options(/W0)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2 /DNDEBUG")
endif()

file(
        COPY
        ${BAAS_APP_BAAS_CPP_DIR}/resource/image/CN/zh-cn/image_info/skill_active.json
        ${BAAS_APP_BAAS_CPP_DIR}/resource/yolo_models/data.yaml
        DESTINATION
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)


add_executable(
        BAAS_workflow_checker
        # ${BAAS_CORE_SOURCES}
        ${BAAS_APP_BAAS_CPP_DIR}/src/module/auto_fight/constants.cpp
        ${BAAS_PROJECT_PATH}/src/BAASLogger.cpp
        ${BAAS_PROJECT_PATH}/src/BAASGlobals.cpp
        ${BAAS_PROJECT_PATH}/src/simdutf.cpp
        ${BAAS_PROJECT_PATH}/src/config/BAASConfig.cpp
        ${BAAS_PROJECT_PATH}/src/utils/BAASStringUtil.cpp
        main.cpp
)

target_include_directories (
        BAAS_workflow_checker
        PRIVATE
        ${BAAS_APP_BAAS_CPP_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${BAAS_PROJECT_PATH}/external
        ${BAAS_PROJECT_PATH}/include
        ${CMAKE_BINARY_DIR}
)
