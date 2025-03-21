BAAS_sub_title_LOG("MacOS Lib Configure")

target_link_directories(
        BAAS_ocr_server
        PRIVATE
        ${BAAS_PROJECT_PATH}/dll/${CURRENT_OS_NAME}
)

SET(
        DLL_COMMON
        libonnxruntime.1.17.1.dylib
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(FATAL_ERROR "Please use Release in MacOS")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    SET(
            DLL_RELEASE
            libopencv_world.4.9.0.dylib
    )
endif ()

if (BAAS_OCR_SERVER_USE_CUDA)
    message(STATUS "CUDA is not available in MacOS.")
endif ()

set(
        DLL_RAW
        ${DLL_COMMON}
        ${DLL_RELEASE}
)
LOG_LINE()
message(STATUS "DLL RAW :")
foreach (DLL ${DLL_RAW})
    message(STATUS "${DLL}")
endforeach ()

foreach (dll ${DLL_RAW})
    set(FULL_PATH ${BAAS_PROJECT_PATH}/dll/${CURRENT_OS_NAME}/${dll})
    file(COPY ${FULL_PATH} DESTINATION ${CMAKE_BINARY_DIR}/bin)
endforeach ()

target_link_libraries(
        BAAS_ocr_server
        ${DLL_RAW}
)