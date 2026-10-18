# Portable parser and native file adapter; deferred authoring/schema are not linked.
find_package(Python3 COMPONENTS Interpreter REQUIRED)

if(BUILD_APP_BAAS OR NOT BAAS_WORKFLOW_MODEL_DIR)
    baas_fetch_resources(yolo_models)
    set(BAAS_WORKFLOW_MODEL_DIR "${BAAS_FETCHED_RESOURCE_ROOT}/yolo_models")
endif()
set(BAAS_WORKFLOW_IMAGE_DIR "${BAAS_PROJECT_PATH}/apps/BAAS/resource/image")
set(BAAS_WORKFLOW_GENERATED_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/generated/WorkflowResources.cpp")
file(GLOB_RECURSE BAAS_WORKFLOW_RESOURCE_INPUTS CONFIGURE_DEPENDS
        "${BAAS_WORKFLOW_IMAGE_DIR}/*.json" "${BAAS_WORKFLOW_IMAGE_DIR}/*.png"
        "${BAAS_WORKFLOW_MODEL_DIR}/*.yaml" "${BAAS_WORKFLOW_MODEL_DIR}/*.onnx")
add_custom_command(
        OUTPUT "${BAAS_WORKFLOW_GENERATED_SOURCE}"
        COMMAND "${Python3_EXECUTABLE}" "${BAAS_PROJECT_PATH}/tools/auto_fight_workflow/generate_resources.py"
                --static "${BAAS_PROJECT_PATH}/resource/static.json"
                --images "${BAAS_WORKFLOW_IMAGE_DIR}"
                --models "${BAAS_WORKFLOW_MODEL_DIR}"
                --output "${BAAS_WORKFLOW_GENERATED_SOURCE}"
        DEPENDS "${BAAS_PROJECT_PATH}/tools/auto_fight_workflow/generate_resources.py"
                "${BAAS_PROJECT_PATH}/resource/static.json"
                "${BAAS_PROJECT_PATH}/resources.lock.json" ${BAAS_WORKFLOW_RESOURCE_INPUTS}
        VERBATIM
)
set(BAAS_WORKFLOW_CORE_SOURCES
        "${BAAS_PROJECT_PATH}/apps/BAAS/src/module/auto_fight/workflow/WorkflowModel.cpp"
        "${BAAS_PROJECT_PATH}/apps/BAAS/src/module/auto_fight/workflow/ParseFields.cpp"
        "${BAAS_PROJECT_PATH}/apps/BAAS/src/module/auto_fight/workflow/ActionParser.cpp"
        "${BAAS_PROJECT_PATH}/apps/BAAS/src/module/auto_fight/workflow/ConditionParser.cpp"
        "${BAAS_PROJECT_PATH}/apps/BAAS/src/module/auto_fight/workflow/StateParser.cpp"
        "${BAAS_PROJECT_PATH}/apps/BAAS/src/module/auto_fight/workflow/WorkflowBasicInfoParser.cpp"
        "${BAAS_PROJECT_PATH}/apps/BAAS/src/module/auto_fight/workflow/WorkflowGraph.cpp"
        "${BAAS_PROJECT_PATH}/apps/BAAS/src/module/auto_fight/workflow/WorkflowParser.cpp"
        "${BAAS_WORKFLOW_GENERATED_SOURCE}"
)
add_library(BAAS_auto_fight_workflow_core STATIC ${BAAS_WORKFLOW_CORE_SOURCES})
add_library(BAAS::AutoFightWorkflowCore ALIAS BAAS_auto_fight_workflow_core)
target_compile_features(BAAS_auto_fight_workflow_core PUBLIC cxx_std_20)
target_include_directories(BAAS_auto_fight_workflow_core PUBLIC
        "${BAAS_PROJECT_PATH}/apps/BAAS/include"
        "${BAAS_PROJECT_PATH}/include")
target_link_libraries(BAAS_auto_fight_workflow_core PUBLIC BAAS::nlohmann_json)
# Support the existing Debug preset that uses Release-only header-library metadata.
get_target_property(_json_includes BAAS::nlohmann_json INTERFACE_INCLUDE_DIRECTORIES)
if(_json_includes)
    string(REPLACE "$<CONFIG:Release>" "$<CONFIG:Debug>" _json_debug_includes "${_json_includes}")
    target_include_directories(BAAS_auto_fight_workflow_core SYSTEM PUBLIC ${_json_includes} ${_json_debug_includes})
endif()
if(MSVC)
    target_compile_options(BAAS_auto_fight_workflow_core PUBLIC /utf-8)
endif()
if(BAAS_AUTO_FIGHT_ENABLE_SANITIZERS)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        message(FATAL_ERROR "Sanitizers require Clang or GCC")
    endif()
    target_compile_options(BAAS_auto_fight_workflow_core PUBLIC -fno-omit-frame-pointer -fsanitize=address,undefined)
    target_link_options(BAAS_auto_fight_workflow_core PUBLIC -fsanitize=address,undefined)
endif()

add_library(BAAS_auto_fight_workflow_native STATIC
        "${BAAS_PROJECT_PATH}/apps/BAAS/src/module/auto_fight/workflow/WorkflowFileParser.cpp")
add_library(BAAS::AutoFightWorkflowNative ALIAS BAAS_auto_fight_workflow_native)
target_link_libraries(BAAS_auto_fight_workflow_native PUBLIC BAAS::AutoFightWorkflowCore)
