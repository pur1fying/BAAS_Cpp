cmake_minimum_required(VERSION 3.25)
project(BAAS)

set(CMAKE_CXX_STANDARD 20)

if (MSVC)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options(/W0)
endif()


include_directories(
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/external
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
        GLOB_RECURSE
        IMAGE_RESOURCE
        ${CMAKE_CURRENT_LIST_DIR}/resource/image/*.png
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


foreach (header ${HEADERS})
    message(STATUS "Found header: ${header}")
endforeach ()

foreach (source ${SOURCES})
    message(STATUS "Found source: ${source}")
endforeach ()

file(
        GLOB DLL
        "dll/*.dll"
)

add_executable(
        BAAS
        main.cpp
        ${SOURCES}
)

target_compile_definitions(BAAS PRIVATE __CUDA__)

target_link_libraries(
        BAAS
        opencv_world490d.lib
        avcodec.lib
        avdevice.lib
        avfilter.lib
        avformat.lib
        avutil.lib
        postproc.lib
        swresample.lib
        swscale.lib
        liblz4.lib
        onnxruntime.lib
        onnxruntime_providers_cuda.lib
        onnxruntime_providers_shared.lib
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

file(COPY ${DLL} DESTINATION ${CMAKE_BINARY_DIR})