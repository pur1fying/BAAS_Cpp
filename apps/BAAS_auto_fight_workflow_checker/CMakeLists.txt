BAAS_hr_LOG("BAAS Auto Fight Workflow Checker")


file(
        COPY
        ${BAAS_APP_BAAS_CPP_DIR}/resource/image/CN/zh-cn/image_info/skill_active.json
        ${BAAS_APP_BAAS_CPP_DIR}/resource/yolo_models/data.yaml
        DESTINATION
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)


add_executable(
        BAAS_workflow_checker
        ${BAAS_PROJECT_PATH}/src/BAASLogger.cpp
        ${BAAS_PROJECT_PATH}/src/BAASGlobals.cpp
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
