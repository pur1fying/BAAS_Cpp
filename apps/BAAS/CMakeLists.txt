if (NOT ${CURRENT_OS_NAME} STREQUAL "Windows")
    message(STATUS "BAAS APP only support Windows")
    return()
endif ()

BAAS_hr_LOG("APP BAAS")

option(WITH_BAAS "Build Script BAAS  "    OFF)
option(WITH_ISA  "Build Script ISA   "    OFF)

project(BAAS_APP VERSION 1.0.0)

configure_file(
        ${CMAKE_CURRENT_LIST_DIR}/version.h.in
        ${CMAKE_BINARY_DIR}/BAAS_Version.h
)

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

if (MSVC)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/MP>")
    add_compile_options(/W0)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Ob2 /DNDEBUG")
endif()

option(BAAS_APP_USE_CUDA "BAAS_APP Use CUDA" OFF)
message(STATUS "BAAS_APP Use CUDA: ${USE_CUDA}")


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

target_link_directories(
        BAAS_APP
        PRIVATE
        ${BAAS_PROJECT_PATH}/lib/${CURRENT_OS_NAME}
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
        BAAS_APP PRIVATE
        BAAS_BUILD_DLL
        BENCHMARK_STATIC_DEFINE
)


if (BAAS_APP_USE_CUDA)
    target_compile_definitions(
            BAAS_APP
            PRIVATE
            __CUDA__
    )
    set(
            LIB_ONNXRUNTIME_CUDA
            onnxruntime_providers_cuda
            onnxruntime_providers_shared
    )
    set(
            DLL_ONNXRUNTIME_CUDA
            onnxruntime_providers_cuda
            onnxruntime_providers_shared
    )
endif ()

SET(
        LIB_COMMON
        avcodec
#        avdevice
#        avfilter
#        avformat
        avutil
        liblz4
        onnxruntime
#        postproc
#        swresample
        swscale
        ws2_32
        shlwapi
)

SET(
        DLL_COMMON
        avcodec-60
#        avdevice-60
#        avfilter-9
#        avformat-60
        avutil-58
        liblz4
        onnxruntime
#        postproc-57
#        swresample-4
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
        ${LIB_ONNXRUNTIME_CUDA}
)

set(
        DLL_RAW
        ${DLL_COMMON}
        ${DLL_DEBUG}
        ${DLL_RELEASE}
        ${DLL_ONNXRUNTIME_CUDA}
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
    set(FULL_PATH ${BAAS_PROJECT_PATH}/dll/${CURRENT_OS_NAME}/${dll}.dll)
    file(COPY ${FULL_PATH} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endforeach ()

target_link_libraries(
        BAAS_APP
        BAAS_ipc
        ${LIB_RAW}
)

