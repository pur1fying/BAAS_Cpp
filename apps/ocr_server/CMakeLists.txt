BAAS_hr_LOG("APP BAAS_OCR_SERVER")

set(CMAKE_CXX_STANDARD 20)
project(BAAS_OCR_SERVER VERSION 1.2.0)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

option(BAAS_OCR_SERVER_USE_CUDA "Ocr Server Use CUDA" OFF)

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

configure_file(
        ${CMAKE_CURRENT_LIST_DIR}/version.h.in
        ${CMAKE_BINARY_DIR}/BAAS_OCR_Version.h
)

file(
        GLOB_RECURSE
        BAAS_OCR_SERVER_CORE_SOURCES
        ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
        ${BAAS_PROJECT_PATH}/src/config/*.cpp
        ${BAAS_PROJECT_PATH}/src/ocr/*.cpp
        ${BAAS_PROJECT_PATH}/src/BAASGlobals.cpp
        ${BAAS_PROJECT_PATH}/src/BAASLogger.cpp
        ${BAAS_PROJECT_PATH}/src/utils/*cpp
        ${BAAS_PROJECT_PATH}/src/simdutf.cpp
        ${BAAS_PROJECT_PATH}/src/BAASExternalIPC.cpp
)

add_executable(
        BAAS_ocr_server
        baas_ocr_main.cpp
        ${BAAS_OCR_SERVER_CORE_SOURCES}
)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/${CURRENT_OS_NAME}.cmake)

target_include_directories(
        BAAS_ocr_server PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_BINARY_DIR}
)


if(CURRENT_OS_NAME STREQUAL "Windows")
    set(ICON_RESOURCE "${CMAKE_CURRENT_LIST_DIR}/src/rc/logo.ico")
    set(RES_FILE "${CMAKE_CURRENT_LIST_DIR}/src/rc/app.rc")
    target_sources(BAAS_ocr_server PRIVATE ${RES_FILE})
endif()