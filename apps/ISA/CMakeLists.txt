if (NOT ${CURRENT_OS_NAME} STREQUAL "Windows")
    message(STATUS "ISA only support Windows")
    return()
endif ()

BAAS_hr_LOG("APP ISA")

project(ISA VERSION 1.0.0)

configure_file(
        ${CMAKE_CURRENT_LIST_DIR}/version.h.in
        ${CMAKE_BINARY_DIR}/BAAS_Version.h
)

set(BIN_RESOURCE_REQUIRED               ON PARENT_SCOPE)
set(ADB_BINARY_RESOURCE_REQUIRED        ON PARENT_SCOPE)
set(OCR_MODEL_RESOURCE_REQUIRED         ON PARENT_SCOPE)
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

option(USE_CUDA "Use CUDA" OFF)
message(STATUS "USE_CUDA: ${USE_CUDA}")

file(
        GLOB_RECURSE
        BAAS_MODULE_RESOURCE
        ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
)

add_executable(
        ISA
        main.cpp
        ${BAAS_CORE_SOURCES}
        ${BAAS_MODULE_RESOURCE}
)

target_link_directories(
        ISA
        PRIVATE
        ${BAAS_PROJECT_PATH}/lib/${CURRENT_OS_NAME}
)

target_include_directories(
        ISA
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${BAAS_PROJECT_PATH}/external
        ${BAAS_PROJECT_PATH}include
        ${CMAKE_BINARY_DIR}
)

target_compile_definitions(
        ISA PRIVATE
        __CUDA__
        BAAS_BUILD_DLL
        BENCHMARK_STATIC_DEFINE
)


SET(
        LIB_COMMON
        avcodec
#        avdevice
#        avfilter
#        avformat
        avutil
        liblz4
        onnxruntime
        onnxruntime_providers_cuda
        onnxruntime_providers_shared
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
        onnxruntime_providers_cuda
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
    set(FULL_PATH ${BAAS_PROJECT_PATH}/dll/${CURRENT_OS_NAME}/${dll}.dll)
    file(COPY ${FULL_PATH} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endforeach ()

target_link_libraries(
        ISA
        BAAS_ipc
        ${LIB_RAW}
)

