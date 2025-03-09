macro(BAAS_hr_LOG message)
    set(PAD_LENGTH 60)

    string(LENGTH "${message}" msg_length)

    math(EXPR LEFT_PAD "(${PAD_LENGTH} - ${msg_length}) / 2 - 1 " )
    math(EXPR RIGHT_PAD "(${PAD_LENGTH} - (${msg_length} + ${LEFT_PAD})) - 2")

    string(REPEAT "-" ${PAD_LENGTH} PADDING)
    string(REPEAT " " ${LEFT_PAD} LEFT_PADDING)
    string(REPEAT " " ${RIGHT_PAD} RIGHT_PADDING)

    message(STATUS  "${PADDING}")
    message(STATUS "|${LEFT_PADDING}${message}${RIGHT_PADDING}|")
    message(STATUS  "${PADDING}")
endmacro()

macro(BAAS_sub_title_LOG)
    string(REPEAT "<" 3 LEFT)
    string(REPEAT ">" 3 RIGHT)
    message(STATUS "${LEFT} ${ARGN} ${RIGHT}")
endmacro()

macro(LOG_LINE)
    message(STATUS "---------------------------------")
endmacro()

BAAS_hr_LOG("PROJECT BAAS")

# Build type
if (NOT CMAKE_BUILD_TYPE OR NOT (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "Debug"))
    message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}. Allowed : Debug, Release")
endif()

BAAS_sub_title_LOG("BUILD TYPE: ${CMAKE_BUILD_TYPE}")

# Platform
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(CURRENT_OS_NAME "Linux")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(CURRENT_OS_NAME "Windows")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    set(CURRENT_OS_NAME "MacOS")
else()
    message(STATUS "Unknown OS: ${CMAKE_SYSTEM_NAME}")
endif()
if ( NOT DEFINED CURRENT_OS_NAME)
    message(FATAL_ERROR "Unknown OS: ${CMAKE_SYSTEM_NAME}")
endif()

message(STATUS "OS : ${CURRENT_OS_NAME}")
set(BAAS_SEARCH_LIB_PATH "${CMAKE_CURRENT_LIST_DIR}/lib/${CURRENT_OS_NAME}")
set(BAAS_SEARCH_DLL_PATH "${CMAKE_CURRENT_LIST_DIR}/dll/${CURRENT_OS_NAME}")

cmake_minimum_required(VERSION 3.22)
project(BAAS VERSION 1.0.0)
configure_file(
        version.h.in
        version.h
)

set(CMAKE_CXX_STANDARD 20)

get_filename_component(BAAS_PROJECT_PATH "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE)

# app options
option(BUILD_BAAS               "build BAAS app"            OFF)
option(BUILD_BAAS_OCR           "build the ocr server app"  ON)
option(BUILD_TEST               "build the test"            OFF)


set(BIN_RESOURCE_REQUIRED OFF)
set(ADB_BINARY_RESOURCE_REQUIRED OFF)
set(OCR_MODEL_RESOURCE_REQUIRED OFF)
set(USER_CONFIG_RESOURCE_REQUIRED OFF)
set(STATIC_CONFIG_RESOURCE_REQUIRED OFF)
set(GLOBAL_CONFIG_RESOURCE_REQUIRED OFF)
set(BAAS_FEATURE_RESOURCE_REQUIRED OFF)
set(BAAS_PROCEDURE_RESOURCE_REQUIRED OFF)
set(BAAS_IMAGE_RESOURCE_REQUIRED OFF)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


include_directories(
        ${BAAS_PROJECT_PATH}/external
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


file(
        GLOB
        ADB_BINARY
        ${CMAKE_CURRENT_LIST_DIR}/resource/platform-tools/adb.exe
        ${CMAKE_CURRENT_LIST_DIR}/resource/platform-tools/AdbWinApi.dll
        ${CMAKE_CURRENT_LIST_DIR}/resource/platform-tools/AdbWinUsbApi.dll
)

file(
        GLOB
        GLOBAL_CONFIG_FILE
        ${CMAKE_CURRENT_LIST_DIR}/resource/global_setting.json
)
file(
        GLOB
        USER_CONFIG_FILE
        ${CMAKE_CURRENT_LIST_DIR}/resource/user_setting.json
        ${CMAKE_CURRENT_LIST_DIR}/resource/default_config.json
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

# --------------  BAAS APP  ------------------
if (BUILD_BAAS)
    add_subdirectory(apps/BAAS)
endif()
# --------------  OCR SERVER  ----------------
if (BUILD_BAAS_OCR)
    add_subdirectory(apps/ocr_server)
endif()
# --------------  TEST  ----------------------
if (BUILD_TEST)
    add_subdirectory(apps/test)
endif()

add_library(
        BAAS_ipc
        SHARED
        ${BAAS_PROJECT_PATH}/src/BAASExternalIPC.cpp
)

if (ADB_BINARY_RESOURCE_REQUIRED)
    file(
            COPY
            ${ADB_BINARY}
            DESTINATION ${CMAKE_BINARY_DIR}
    )
endif ()

if (GLOBAL_CONFIG_RESOURCE_REQUIRED)
    file(
            COPY
            ${GLOBAL_CONFIG_FILE}
            DESTINATION ${CMAKE_BINARY_DIR}/resource
    )
endif ()

if (USER_CONFIG_RESOURCE_REQUIRED)
    file(
            COPY
            ${USER_CONFIG_FILE}
            DESTINATION ${CMAKE_BINARY_DIR}/resource
    )
endif ()

if (STATIC_CONFIG_RESOURCE_REQUIRED)
    file(
            COPY
            ${STATIC_CONFIG_FILE}
            DESTINATION ${CMAKE_BINARY_DIR}/resource
    )
endif ()

if(BAAS_IMAGE_RESOURCE_REQUIRED)
    file(
            COPY
            ${CMAKE_CURRENT_LIST_DIR}/resource/image
            DESTINATION ${CMAKE_BINARY_DIR}/resource
    )
endif()

if (BIN_RESOURCE_REQUIRED)
    file(
            COPY
            ${CMAKE_CURRENT_LIST_DIR}/resource/bin
            DESTINATION ${CMAKE_BINARY_DIR}/resource
    )
endif ()

if (BAAS_FEATURE_RESOURCE_REQUIRED)
    file(
            COPY
            ${CMAKE_CURRENT_LIST_DIR}/resource/features
            DESTINATION ${CMAKE_BINARY_DIR}/resource
    )
endif ()

if (BAAS_PROCEDURE_RESOURCE_REQUIRED)
    file(
            COPY
            ${CMAKE_CURRENT_LIST_DIR}/resource/procedure
            DESTINATION ${CMAKE_BINARY_DIR}/resource
    )
endif ()

if (OCR_MODEL_RESOURCE_REQUIRED)
    file(
            COPY
            ${CMAKE_CURRENT_LIST_DIR}/resource/ocr_models
            DESTINATION ${CMAKE_BINARY_DIR}/resource
    )
endif ()
