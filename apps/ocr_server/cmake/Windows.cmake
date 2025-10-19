target_link_directories(
        BAAS_ocr_server
        PRIVATE
        ${BAAS_DEFAULT_SEARCH_LIB_PATH}
)

SET(
        LIB_COMMON
        onnxruntime
        ws2_32
        shlwapi
)

SET(
        DLL_COMMON
        onnxruntime
        concrt140
        msvcp140
        vcruntime140
        vcruntime140_1
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(
            LIB_DEBUG
            opencv_world490d
    )
    SET(
            DLL_DEBUG
            opencv_world490d
    )
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    SET(
            LIB_RELEASE
            opencv_world490
    )
    SET(
            DLL_RELEASE
            opencv_world490
    )
endif ()

if (BAAS_OCR_SERVER_USE_CUDA)
    SET(
            LIB_COMMON
            ${LIB_COMMON}
            onnxruntime_providers_cuda
            onnxruntime_providers_shared
    )
    SET(
            DLL_COMMON
            ${DLL_COMMON}
            onnxruntime_providers_cuda
    )
    target_compile_definitions(
            BAAS_ocr_server
            PRIVATE
            __CUDA__
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
LOG_LINE()
message(STATUS "DLL RAW :")
foreach (DLL ${DLL_RAW})
    message(STATUS "${DLL}")
endforeach ()

# get dll full path and copy to binary dir
foreach (dll ${DLL_RAW})
    set(FULL_PATH ${BAAS_PROJECT_PATH}/dll/${TARGET_OS_NAME}/${dll}.dll)
    file(COPY ${FULL_PATH} DESTINATION ${CMAKE_BINARY_DIR}/bin)
endforeach ()

target_link_libraries(
        BAAS_ocr_server
        BAAS_ipc
        ${LIB_RAW}
)
