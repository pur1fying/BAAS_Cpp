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

BAAS_hr_LOG("PROJECT BAAS")
BAAS_sub_title_LOG("BUILD TYPE: ${CMAKE_BUILD_TYPE}")
cmake_minimum_required(VERSION 3.25)

project(BAAS VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 20)

get_filename_component(BAAS_PROJECT_PATH "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE)

option(BUILD_BAAS     "build BAAS app" ON)
option(BUILD_BAAS_OCR "build the ocr server app" ON)

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

file (
        GLOB
        OCR_MODELS
        ${CMAKE_CURRENT_LIST_DIR}/resource/ocr_models/*.onnx
        ${CMAKE_CURRENT_LIST_DIR}/resource/ocr_models/*.txt
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
add_library(lib_BAAS SHARED ${BAAS_PROJECT_PATH}/src/BAASExternalIPC.cpp)

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
