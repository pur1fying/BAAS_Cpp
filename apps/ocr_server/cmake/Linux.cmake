BAAS_sub_title_LOG("Linux Lib Configure")

target_link_directories(
        BAAS_ocr_server
        PRIVATE
        ${BAAS_PROJECT_PATH}/dll/${CURRENT_OS_NAME}
)

SET(
        DLL_COMMON
        libonnxruntime.so.1.17.1
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    SET(
            DLL_DEBUG
            opencv_worldd
    )
elseif(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    SET(
            DLL_RELEASE
            libopencv_world.so.409
    )
endif ()

if (BAAS_OCR_SERVER_USE_CUDA)
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
        DLL_RAW
        ${DLL_COMMON}
        ${DLL_RELEASE}
        ${DLL_DEBUG}
)
LOG_LINE()
message(STATUS "DLL RAW :")
foreach (DLL ${DLL_RAW})
    message(STATUS "${DLL}")
endforeach ()

foreach (dll ${DLL_RAW})
    set(FULL_PATH ${BAAS_PROJECT_PATH}/dll/${CURRENT_OS_NAME}/${dll})
    file(COPY ${FULL_PATH} DESTINATION ${CMAKE_BINARY_DIR})
endforeach ()

target_link_libraries(
        BAAS_ocr_server
        ${DLL_RAW}
)