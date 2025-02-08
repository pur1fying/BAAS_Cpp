message(STATUS "BUILD TYPE: ${CMAKE_BUILD_TYPE}")
cmake_minimum_required(VERSION 3.25)

project(BAAS VERSION 1.0.0)

#set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR} CACHE PATH)
#set(SRC_DIR ${PROJECT_ROOT_DIR}/src CACHE PATH)
#set(INCLUDE_DIR ${PROJECT_ROOT_DIR}/include CACHE PATH)
#set(LIB_DIR ${PROJECT_ROOT_DIR}/lib CACHE PATH)
#set(DLL_DIR ${PROJECT_ROOT_DIR}/dll CACHE PATH)
#set(RESOURCE_DIR ${PROJECT_ROOT_DIR}/resource CACHE PATH)

set(CMAKE_CXX_STANDARD 20)

configure_file(
        ${CMAKE_CURRENT_LIST_DIR}/version.h.in
        ${CMAKE_BINARY_DIR}/version.h
)

if (MSVC)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options(/W0)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2 /DNDEBUG")
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
include_directories(
        BAAS
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/external
        ${CMAKE_BINARY_DIR}
)

link_directories(
        ${CMAKE_CURRENT_LIST_DIR}/lib
)

file(
        GLOB_RECURSE
        HEADERS
        ${CMAKE_CURRENT_LIST_DIR}/include/*.h
)
file(
        GLOB_RECURSE
        SOURCES
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
        CONFIG_FILES
        ${CMAKE_CURRENT_LIST_DIR}/resource/config_name_change.json
        ${CMAKE_CURRENT_LIST_DIR}/resource/default_config.json
        ${CMAKE_CURRENT_LIST_DIR}/resource/static.json
        ${CMAKE_CURRENT_LIST_DIR}/resource/global_setting.json
)

file (
        GLOB
        OCR_MODELS
        ${CMAKE_CURRENT_LIST_DIR}/resource/ocr_models/*.onnx
        ${CMAKE_CURRENT_LIST_DIR}/resource/ocr_models/*.txt
)




foreach (source ${SOURCES})
    message(STATUS "Found source: ${source}")
endforeach ()

add_library(lib_BAAS SHARED src/BAASExternalIPC.cpp)

SET(
        LIB_COMMON
        avcodec
        avdevice
        avfilter
        avformat
        avutil
        liblz4
        onnxruntime
        onnxruntime_providers_cuda
        onnxruntime_providers_shared
        postproc
        swresample
        swscale
        ws2_32
        shlwapi
)

SET(
        DLL_COMMON
        avcodec-60
        avdevice-60
        avfilter-9
        avformat-60
        avutil-58
        liblz4
        onnxruntime
        onnxruntime_providers_cuda
        onnxruntime_providers_shared
        postproc-57
        swresample-4
        swscale-7
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(
            LIB_DEBUG
            opencv_world490d
            benchmark_d
            benchmark_main_d
    )
    SET(
            DLL_DEBUG
            opencv_world490d
    )
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    SET(
            LIB_RELEASE
            opencv_world490
            benchmark
            benchmark_main
    )
    SET(
            DLL_RELEASE
            opencv_world490
    )
endif ()

set(
        LIB_RAW
        ${LIB_COMMON}
        ${LIB_DEBUG}
        ${LIB_RELEASE}
)

set(
        DLL_RAW
        ${DLL_COMMON}
        ${DLL_DEBUG}
        ${DLL_RELEASE}
)

# log found dll name
message(STATUS "LIB RAW :")
foreach (LIB ${LIB_RAW})
    message(STATUS "${LIB}")
endforeach ()
message(STATUS "---------------------------------")
message(STATUS "DLL RAW :")
foreach (DLL ${DLL_RAW})
    message(STATUS "${DLL}")
endforeach ()

# get dll full path and copy to binary dir
foreach (dll ${DLL_RAW})
    set(FULL_PATH ${CMAKE_CURRENT_LIST_DIR}/dll/${dll}.dll)
    file(COPY ${FULL_PATH} DESTINATION ${CMAKE_BINARY_DIR})
endforeach ()

add_executable(
        BAAS
        main.cpp
        ${SOURCES}
)

target_compile_definitions(
        BAAS PRIVATE
        __CUDA__
        BAAS_BUILD_DLL
        BENCHMARK_STATIC_DEFINE
)

target_link_libraries(
        BAAS
        ${LIB_RAW}
)

file(
        COPY
        ${CONFIG_FILES}
        ${SCRCPY_SERVER}

        DESTINATION ${CMAKE_BINARY_DIR}/resource
)

file(
        COPY
        ${ADB_BINARY}
        DESTINATION ${CMAKE_BINARY_DIR}
)

file(
        COPY
        ${CMAKE_CURRENT_LIST_DIR}/resource/image
        DESTINATION ${CMAKE_BINARY_DIR}/resource
)

file(
        COPY
        ${CMAKE_CURRENT_LIST_DIR}/resource/nemu_dll
        DESTINATION ${CMAKE_BINARY_DIR}/resource
)

file(
        COPY
        ${CMAKE_CURRENT_LIST_DIR}/resource/bin
        DESTINATION ${CMAKE_BINARY_DIR}/resource
)

file(
        COPY
        ${CMAKE_CURRENT_LIST_DIR}/resource/features
        DESTINATION ${CMAKE_BINARY_DIR}/resource
)

file(
        COPY
        ${CMAKE_CURRENT_LIST_DIR}/resource/procedure
        DESTINATION ${CMAKE_BINARY_DIR}/resource
)

file(
        COPY
        ${CMAKE_CURRENT_LIST_DIR}/resource/ocr_models
        DESTINATION ${CMAKE_BINARY_DIR}/resource
)

