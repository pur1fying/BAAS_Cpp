BAAS_hr_LOG("APP BAAS_OCR_SERVER")

set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/Version.cmake)

project(BAAS_OCR_SERVER VERSION ${BAAS_OCR_SERVER_VERSION})

configure_file(
        ${CMAKE_CURRENT_LIST_DIR}/version.h.in
        ${CMAKE_BINARY_DIR}/BAAS_OCR_version.h
)

configure_file(
        ${CMAKE_CURRENT_LIST_DIR}/version.txt.in
        ${CMAKE_BINARY_DIR}/BAAS_OCR_version.txt
)

# cuda is only available in Windows and Linux
option(BAAS_OCR_SERVER_USE_CUDA      "Ocr Server Use CUDA"       OFF)
option(BAAS_OCR_SERVER_ANDROID_BUILD "Build Android Application" OFF)

set(OCR_MODEL_RESOURCE_REQUIRED     ON PARENT_SCOPE)
set(STATIC_CONFIG_RESOURCE_REQUIRED ON PARENT_SCOPE)
set(GLOBAL_CONFIG_RESOURCE_REQUIRED ON PARENT_SCOPE)

if (MSVC)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/MP>")
    add_compile_options(/W0)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2 /DNDEBUG")
endif()

file(
        GLOB_RECURSE
        BAAS_OCR_SERVER_CORE_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
        ${BAAS_PROJECT_PATH}/src/config/*.cpp
        ${BAAS_PROJECT_PATH}/src/ocr/*.cpp
        ${BAAS_PROJECT_PATH}/src/BAASGlobals.cpp
        ${BAAS_PROJECT_PATH}/src/BAASLogger.cpp
        ${BAAS_PROJECT_PATH}/src/utils/*.cpp
        ${BAAS_PROJECT_PATH}/src/simdutf.cpp
)

# Generate jni lib in Android, build executable in [Windows, Linux, MacOS]

if(NOT ${TARGET_OS_NAME} STREQUAL "Android")
    list(
        APPEND
        BAAS_OCR_SERVER_CORE_SOURCES
        ${BAAS_PROJECT_PATH}/src/BAASExternalIPC.cpp
    )

    add_executable(
            BAAS_ocr_server
            baas_ocr_main.cpp
            ${BAAS_OCR_SERVER_CORE_SOURCES}
    )
else()
    add_library(
            BAAS_ocr_server
            SHARED
            native-lib.cpp
            ${BAAS_OCR_SERVER_CORE_SOURCES}
    )
endif()

BAAS_sub_title_LOG("BAAS_ocr_server ${TARGET_OS_NAME} Configure")
include(${CMAKE_CURRENT_LIST_DIR}/cmake/${TARGET_OS_NAME}.cmake)

target_include_directories(
        BAAS_ocr_server
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_BINARY_DIR}
)

# Generate icon in Windows
if(TARGET_OS_NAME STREQUAL "Windows")
    set(ICON_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/src/rc/logo.ico")
    set(RES_FILE "${CMAKE_CURRENT_LIST_DIR}/src/rc/app.rc")
    target_sources(BAAS_ocr_server PRIVATE ${RES_FILE})
endif()