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

#set(
#        EMBEDDED_CPP
#        ${CMAKE_OUTPUT}
#)
#
#add_custom_command(
#        OUTPUT
#        ${CMAKE_CURRENT_SOURCE_DIR}/src/generated/embedded_data.h
#        ${CMAKE_CURRENT_SOURCE_DIR}/src/generated/embedded_data.cpp
#        COMMAND python ${CMAKE_SOURCE_DIR}/embed_script.py
#        DEPENDS ${CMAKE_SOURCE_DIR}/resource/skill_active.json
#        ${CMAKE_SOURCE_DIR}/resource/data.yaml
#        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#        COMMENT "生成嵌入数据文件"
#)

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

#add_custom_target(
#        embed_data
#        DEPENDS
#        ${CMAKE_CURRENT_SOURCE_DIR}/src/generated/embedded_data.h
#        ${CMAKE_CURRENT_SOURCE_DIR}/src/generated/embedded_data.cpp
#)
#
#add_dependencies(BAAS_workflow_checker embed_data)

file(
        COPY
        ${BAAS_APP_BAAS_CPP_DIR}/resource/image/CN/zh-cn/image_info/skill_active.json
        ${BAAS_APP_BAAS_CPP_DIR}/resource/yolo_models/data.yaml
        DESTINATION
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
if(EMSCRIPTEN)
    BAAS_sub_title_LOG("Compiling for WebAssembly")
    set(CMAKE_EXECUTABLE_SUFFIX ".js")
    target_compile_options(
            BAAS_workflow_checker
            PRIVATE
            -Oz
            -flto
            -sWASM=1
            -sDISABLE_EXCEPTION_CATCHING=0
    )
    target_link_options(
            BAAS_workflow_checker
            PRIVATE
            -Oz
            -flto
            -sDISABLE_EXCEPTION_CATCHING=0
            -sEXPORTED_RUNTIME_METHODS=['cwrap','ccall']
            -sEXPORTED_FUNCTIONS=['_is_valid_workflow']
    )
endif()

target_include_directories (
        BAAS_workflow_checker
        PRIVATE
        ${BAAS_APP_BAAS_CPP_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${BAAS_PROJECT_PATH}/external
        ${BAAS_PROJECT_PATH}/include
        ${CMAKE_BINARY_DIR}
)
