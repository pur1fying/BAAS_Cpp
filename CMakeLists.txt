cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)

get_filename_component(BAAS_PROJECT_PATH "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE)

include("${CMAKE_CURRENT_LIST_DIR}/cmake/utils.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/platform.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/BAASResources.cmake")

BAAS_hr_LOG("PROJECT BAAS")

# Build type
if (CMAKE_BUILD_TYPE AND NOT (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "Debug"))
    message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}. Allowed : Debug, Release")
endif()

if(CMAKE_BUILD_TYPE)
    BAAS_sub_title_LOG("BUILD TYPE: ${CMAKE_BUILD_TYPE}")
else()
    BAAS_sub_title_LOG("BUILD TYPE: not set; dependency config defaults to Release")
endif()

# Target Platform
detect_target_platform(TARGET_OS_NAME)

if(NOT DEFINED TARGET_OS_NAME)
    message(FATAL_ERROR "Unknown Target OS: ${CMAKE_SYSTEM_NAME}")
endif()

BAAS_sub_title_LOG("Target OS : ${TARGET_OS_NAME}")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/BAASDependency.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/BAASTarget.cmake")

if(TARGET_OS_NAME STREQUAL "Android")
    baas_setup_android_ndk()
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
        ${CMAKE_CURRENT_LIST_DIR}/cmake/version.h.in
        BAAS_version.h
)

configure_file(
        ${CMAKE_CURRENT_LIST_DIR}/cmake/version.txt.in
        BAAS_version.txt
)

BAAS_sub_title_LOG("GIT INFO")
message(STATUS "Hash        : ${GIT_COMMIT_HASH}")
message(STATUS "Branch      : ${GIT_BRANCH}")
message(STATUS "Author      : ${GIT_COMMIT_AUTHOR} <${GIT_COMMIT_EMAIL}>")
message(STATUS "Commit      : ${GIT_COMMIT_TITLE}")
message(STATUS "Date        : ${GIT_COMMIT_DATE}")
message(STATUS "Build time  : ${BUILD_TIME}")


# app options
option(BUILD_APP_BAAS               "Build BAAS app          "  OFF)
option(BUILD_APP_ISA                "Build ISA  app          "  OFF)

# other
option(BUILD_BAAS_OCR           "build the ocr server app"               OFF)
option(BUILD_BAAS_NMS_BENCHMARK "Build BAAS vs OpenCV dnn NMS Google Benchmark executable" OFF)
option(BAAS_AUTO_FIGHT_ENABLE_SANITIZERS "Instrument Auto Fight Workflow Core with ASan and UBSan" OFF)

check_app_options(
        BUILD_APP_BAAS
        BUILD_APP_ISA
        BUILD_BAAS_OCR
)

if(BUILD_BAAS_NMS_BENCHMARK)
    baas_request_dependencies(benchmark opencv_dnn)
endif()

if(BUILD_APP_BAAS)
    option(BAAS_APP_USE_CUDA "BAAS_APP Use CUDA" OFF)
    baas_request_dependencies(opencv ffmpeg lz4 nlohmann_json cpp_httplib spdlog simdutf benchmark)
    if(BAAS_APP_USE_CUDA)
        baas_request_dependencies(
                onnxruntime PROVIDER cuda
        )
    else()
        baas_request_dependencies(onnxruntime)
    endif()
endif()

if(BUILD_APP_ISA)
    baas_request_dependencies(opencv ffmpeg lz4 nlohmann_json cpp_httplib spdlog simdutf benchmark)
    baas_request_dependencies(
            onnxruntime PROVIDER cuda
    )
endif()

if(BUILD_BAAS_OCR)
    option(BAAS_OCR_SERVER_USE_CUDA "Ocr Server Use CUDA" OFF)
    baas_request_dependencies(opencv nlohmann_json cpp_httplib spdlog simdutf)
    if(BAAS_OCR_SERVER_USE_CUDA)
        baas_request_dependencies(
                onnxruntime PROVIDER cuda
        )
    else()
        baas_request_dependencies(onnxruntime)
    endif()
endif()

baas_request_dependencies(nlohmann_json)

baas_load_requested_dependencies()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(
        ${BAAS_PROJECT_PATH}/include
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

list(FILTER BAAS_CORE_SOURCES EXCLUDE REGEX "/src/auto_fight/authoring/.*\\.cpp$")

if(TARGET_OS_NAME STREQUAL "Windows")
    list(REMOVE_ITEM BAAS_CORE_SOURCES "${CMAKE_CURRENT_LIST_DIR}/src/simdutf.cpp")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/cmake/NmsBenchmark.cmake")

if(BUILD_APP_BAAS)
    include("${CMAKE_CURRENT_LIST_DIR}/apps/BAAS/cmake/AutoFightWorkflow.cmake")
endif()
# Authoring/schema remain source-only until migrated to the new parser API.

set(BAAS_APP_BAAS_CPP_DIR  "${CMAKE_CURRENT_LIST_DIR}/apps/BAAS")
set(BAAS_APP_ISA_DIR       "${CMAKE_CURRENT_LIST_DIR}/apps/ISA")
set(BAAS_OCR_SERVER_DIR    "${CMAKE_CURRENT_LIST_DIR}/apps/ocr_server")

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

add_library(
        BAAS_ipc
        SHARED
        ${BAAS_PROJECT_PATH}/src/BAASExternalIPC.cpp
)

baas_install_required_runtime_resources()
