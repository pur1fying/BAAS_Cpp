BAAS_hr_LOG("APP BAAS")

include(${CMAKE_CURRENT_LIST_DIR}/cmake/Version.cmake)

project(BAAS_APP VERSION ${BAAS_APP_VERSION})

configure_file(
        ${CMAKE_CURRENT_LIST_DIR}/cmake/version.h.in
        ${CMAKE_BINARY_DIR}/BAAS_APP_version.h
)

configure_file(
        ${CMAKE_CURRENT_LIST_DIR}/cmake/version.txt.in
        ${CMAKE_BINARY_DIR}/BAAS_APP_version.txt
)

if (MSVC)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/MP>")
    add_compile_options(/W0)
endif()

baas_require_runtime_resources(
        global_config
        static_config
        adb
        scrcpy_server
        ocr_models
        yolo_models
)

baas_copy_local_runtime_resources(
        BASE_DIR "${CMAKE_CURRENT_LIST_DIR}/resource"
        ITEMS
        config
        image
        feature
        procedure
        auto_fight_workflow
)

option(BAAS_APP_USE_CUDA "BAAS_APP Use CUDA" OFF)
message(STATUS "BAAS_APP Use CUDA : ${BAAS_APP_USE_CUDA}")

file(
        GLOB_RECURSE
        BAAS_MODULE_RESOURCE
        ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
)

# Workflow sources belong to the separate Core/native targets; deferred schema
# sources in the same directory remain outside the application build.
list(FILTER BAAS_MODULE_RESOURCE EXCLUDE REGEX "/src/module/auto_fight/workflow/.*\\.cpp$")

add_executable(
        BAAS_APP
        main.cpp
        ${BAAS_CORE_SOURCES}
        ${BAAS_MODULE_RESOURCE}
)

target_link_libraries(
        BAAS_APP
        PRIVATE
        BAAS::AutoFightWorkflowNative
)


target_include_directories(
        BAAS_APP
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${BAAS_PROJECT_PATH}/include
        ${CMAKE_BINARY_DIR}
)

target_compile_definitions(
        BAAS_APP
        PRIVATE
        BAAS_BUILD_DLL
        BENCHMARK_STATIC_DEFINE
)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/${TARGET_OS_NAME}.cmake)
