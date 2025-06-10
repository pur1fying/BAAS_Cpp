BAAS_sub_title_LOG("BAAS_APP Windows Configure")

target_link_directories(
        BAAS_APP
        PRIVATE
        ${BAAS_PROJECT_PATH}/dll/${CURRENT_OS_NAME}
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
            zlibwapi
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
        avcodec-62
        #        avdevice-62
        #        avfilter-11
        #        avformat-62
        avutil-60
        liblz4
        onnxruntime
        #        postproc-59
        swresample-6
        swscale-9
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
    file(COPY ${FULL_PATH} DESTINATION ${CMAKE_BINARY_DIR}/bin)
endforeach ()

target_link_libraries(
        BAAS_APP
        BAAS_ipc
        ${LIB_RAW}
)


set(ICON_RESOURCE "${BAAS_APP_BAAS_CPP_DIR}/src/rc/logo.ico")
set(RES_FILE "${BAAS_APP_BAAS_CPP_DIR}/src/rc/app.rc")
target_sources(BAAS_APP PRIVATE ${RES_FILE})