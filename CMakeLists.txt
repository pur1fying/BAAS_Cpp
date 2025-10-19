cmake_minimum_required(VERSION 3.22)

get_filename_component(BAAS_PROJECT_PATH "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE)

include("${CMAKE_CURRENT_LIST_DIR}/cmake/utils.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/platform.cmake")

BAAS_hr_LOG("PROJECT BAAS")

# Build type
if (NOT CMAKE_BUILD_TYPE OR NOT (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "Debug"))
    message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}. Allowed : Debug, Release")
endif()

BAAS_sub_title_LOG("BUILD TYPE: ${CMAKE_BUILD_TYPE}")

# Target Platform
detect_platform(TARGET_OS_NAME)

if(NOT DEFINED TARGET_OS_NAME)
    message(FATAL_ERROR "Unknown Target OS: ${CMAKE_SYSTEM_NAME}")
endif()

BAAS_sub_title_LOG("Target OS : ${TARGET_OS_NAME}")
set(BAAS_DEFAULT_SEARCH_LIB_PATH "${CMAKE_CURRENT_LIST_DIR}/lib/${TARGET_OS_NAME}")
set(BAAS_DEFAULT_SEARCH_DLL_PATH "${CMAKE_CURRENT_LIST_DIR}/dll/${TARGET_OS_NAME}")

if(TARGET_OS_NAME STREQUAL "Android")
    Android_NDK_check()
endif()


include("${CMAKE_CURRENT_LIST_DIR}/cmake/Version.cmake")
project(BAAS VERSION ${BAAS_VERSION})

# git configuration
get_git_info(GIT_BRANCH git rev-parse --abbrev-ref HEAD)
get_git_info(GIT_COMMIT_HASH  git rev-parse HEAD)
get_git_info(GIT_COMMIT_AUTHOR git log -1 --format=%an)
get_git_info(GIT_COMMIT_EMAIL  git log -1 --format=%ae)
get_git_info(GIT_COMMIT_TITLE  git log -1 --format=%s)
get_git_info(GIT_COMMIT_BODY   git log -1 --format=%b)
get_git_info(GIT_COMMIT_DATE   git log -1 --format=%cd --date=iso)

# build time
string(TIMESTAMP BUILD_TIME "%Y-%m-%d %H:%M:%S (UTC)" UTC)

configure_file(
        version.h.in
        BAAS_version.h
)

configure_file(
        version.txt.in
        BAAS_version.txt
)

BAAS_sub_title_LOG("GIT INFO")
message(STATUS "Hash        : ${GIT_COMMIT_HASH}")
message(STATUS "Branch      : ${GIT_BRANCH}")
message(STATUS "Author      : ${GIT_COMMIT_AUTHOR} <${GIT_COMMIT_EMAIL}>")
message(STATUS "Commit      : ${GIT_COMMIT_TITLE}")
message(STATUS "Date        : ${GIT_COMMIT_DATE}")
message(STATUS "Build time  : ${BUILD_TIME}")


set(CMAKE_CXX_STANDARD 20)

# app options
option(BUILD_APP_BAAS               "Build BAAS app          "  OFF)
option(BUILD_APP_ISA                "Build ISA  app          "  OFF)

# other
option(BUILD_BAAS_OCR           "build the ocr server app"               OFF)
option(BUILD_BAAS_AW_CHECKER    "build BAAS Auto Fight Workflow Checker" OFF)

check_app_options(
        BUILD_APP_BAAS
        BUILD_APP_ISA
        BUILD_BAAS_OCR
        BUILD_BAAS_AW_CHECKER
)

set(BIN_RESOURCE_REQUIRED OFF)
set(ADB_BINARY_RESOURCE_REQUIRED OFF)
set(OCR_MODEL_RESOURCE_REQUIRED OFF)
set(USER_CONFIG_RESOURCE_REQUIRED OFF)
set(STATIC_CONFIG_RESOURCE_REQUIRED OFF)
set(GLOBAL_CONFIG_RESOURCE_REQUIRED OFF)
set(BAAS_FEATURE_RESOURCE_REQUIRED OFF)
set(BAAS_PROCEDURE_RESOURCE_REQUIRED OFF)
set(BAAS_IMAGE_RESOURCE_REQUIRED OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(
        ${BAAS_PROJECT_PATH}/external
        ${BAAS_PROJECT_PATH}/include
        ${BAAS_PROJECT_PATH}/external/cuda
)

file(
        GLOB_RECURSE
        BAAS_CORE_HEADERS
        ${CMAKE_CURRENT_LIST_DIR}/include/*.h
)

file(
        GLOB_RECURSE
        BAAS_CORE_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
)


file(
        GLOB
        GLOBAL_CONFIG_FILE
        ${CMAKE_CURRENT_LIST_DIR}/resource/global_setting.json
)

file(
        GLOB
        STATIC_CONFIG_FILE
        ${CMAKE_CURRENT_LIST_DIR}/resource/static.json
)

file(
        COPY
        ${CONFIG_FILES}
        ${SCRCPY_SERVER}
        DESTINATION ${CMAKE_BINARY_DIR}/resource
)

set(BAAS_APP_BAAS_CPP_DIR  "${CMAKE_CURRENT_LIST_DIR}/apps/BAAS")
set(BAAS_APP_ISA_DIR       "${CMAKE_CURRENT_LIST_DIR}/apps/ISA")
set(BAAS_OCR_SERVER_DIR    "${CMAKE_CURRENT_LIST_DIR}/apps/ocr_server")
set(BAAS_AW_CHECKER_DIR    "${CMAKE_CURRENT_LIST_DIR}/apps/BAAS_auto_fight_workflow_checker")

# --------------  BAAS APP  ------------------
if (BUILD_APP_BAAS)
    add_subdirectory(apps/BAAS)
endif()

# --------------  ISA APP  -------------------
if (BUILD_APP_ISA)
    add_subdirectory(apps/ISA)
endif()

# --------------  OCR SERVER  ----------------
if (BUILD_BAAS_OCR)
    add_subdirectory(apps/ocr_server)
endif()

# --------------  BAAS AUTO FIGHT WORKFLOW CHECKER  ----------------
if (BUILD_BAAS_AW_CHECKER)
    add_subdirectory(apps/BAAS_auto_fight_workflow_checker)
endif()

add_library(
        BAAS_ipc
        SHARED
        ${BAAS_PROJECT_PATH}/src/BAASExternalIPC.cpp
)

if (ADB_BINARY_RESOURCE_REQUIRED)
    set_ADB_BINARY()
    file(
            COPY
            ${ADB_BINARY}
            DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
endif ()

if (GLOBAL_CONFIG_RESOURCE_REQUIRED)
    file(
            COPY
            ${GLOBAL_CONFIG_FILE}
            DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resource
    )
endif ()

if (USER_CONFIG_RESOURCE_REQUIRED)
    file(
            COPY
            ${USER_CONFIG_FILE}
            DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resource
    )
endif ()

if (STATIC_CONFIG_RESOURCE_REQUIRED)
    file(
            COPY
            ${STATIC_CONFIG_FILE}
            DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resource
    )
endif ()

if (BIN_RESOURCE_REQUIRED)
    file(
            COPY
            ${CMAKE_CURRENT_LIST_DIR}/resource/bin
            DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resource
    )
endif ()

if (OCR_MODEL_RESOURCE_REQUIRED)
    file(
            COPY
            ${CMAKE_CURRENT_LIST_DIR}/resource/ocr_models
            DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resource
    )
endif ()
