BAAS_hr_LOG("APP BAAS")

include(${CMAKE_CURRENT_LIST_DIR}/cmake/Version.cmake)

project(BAAS_APP VERSION ${BAAS_APP_VERSION})

configure_file(
        ${CMAKE_CURRENT_LIST_DIR}/version.h.in
        ${CMAKE_BINARY_DIR}/BAAS_APP_version.h
)

configure_file(
        ${CMAKE_CURRENT_LIST_DIR}/version.txt.in
        ${CMAKE_BINARY_DIR}/BAAS_APP_version.txt
)

if (MSVC)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/MP>")
    add_compile_options(/W0)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2 /DNDEBUG")
endif()

set(BIN_RESOURCE_REQUIRED               ON PARENT_SCOPE)
set(ADB_BINARY_RESOURCE_REQUIRED        ON PARENT_SCOPE)
set(OCR_MODEL_RESOURCE_REQUIRED         ON PARENT_SCOPE)
set(USER_CONFIG_RESOURCE_REQUIRED       ON PARENT_SCOPE)
set(STATIC_CONFIG_RESOURCE_REQUIRED     ON PARENT_SCOPE)
set(GLOBAL_CONFIG_RESOURCE_REQUIRED     ON PARENT_SCOPE)

file(
        GLOB
        USER_CONFIG_FILES
        ${CMAKE_CURRENT_LIST_DIR}/resource/config/*.json
)

file(
        COPY
        ${USER_CONFIG_FILES}
        DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resource
)
file(
        COPY
        ${CMAKE_CURRENT_LIST_DIR}/resource/image
        DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resource
)
file(
        COPY
        ${CMAKE_CURRENT_LIST_DIR}/resource/feature
        DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resource
)
file(
        COPY
        ${CMAKE_CURRENT_LIST_DIR}/resource/procedure
        DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resource
)
file(
        COPY
        ${CMAKE_CURRENT_LIST_DIR}/resource/auto_fight_workflow
        DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resource
)
file(
        COPY
        ${CMAKE_CURRENT_LIST_DIR}/resource/yolo_models
        DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resource
)

option(BAAS_APP_USE_CUDA "BAAS_APP Use CUDA" OFF)
message(STATUS "BAAS_APP Use CUDA : ${BAAS_APP_USE_CUDA}")

file(
        GLOB_RECURSE
        BAAS_MODULE_RESOURCE
        ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
)

add_executable(
        BAAS_APP
        main.cpp
        ${BAAS_CORE_SOURCES}
        ${BAAS_MODULE_RESOURCE}
)


target_include_directories(
        BAAS_APP
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${BAAS_PROJECT_PATH}/external
        ${BAAS_PROJECT_PATH}include
        ${CMAKE_BINARY_DIR}
)

target_compile_definitions(
        BAAS_APP
        PRIVATE
        BAAS_BUILD_DLL
        BENCHMARK_STATIC_DEFINE
)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/${TARGET_OS_NAME}.cmake)
