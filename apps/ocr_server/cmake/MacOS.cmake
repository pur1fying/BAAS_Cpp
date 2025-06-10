BAAS_sub_title_LOG("BAAS_ocr_server MacOS Configure")

target_link_directories(
        BAAS_ocr_server
        PRIVATE
        ${BAAS_PROJECT_PATH}/dll/${CURRENT_OS_NAME}
)

SET(
        DLL_COMMON
        libonnxruntime.dylib
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(FATAL_ERROR "Please use Release in MacOS")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    SET(
            DLL_RELEASE
            libopencv_world.409.dylib
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

set(
    DLL_MOVE
    ${DLL_RAW}
    libonnxruntime.1.22.0.dylib
    libopencv_world.4.9.0.dylib
)

foreach (dll ${DLL_MOVE})
    set(FULL_PATH ${BAAS_PROJECT_PATH}/dll/${CURRENT_OS_NAME}/${dll})
    file(COPY ${FULL_PATH} DESTINATION ${CMAKE_BINARY_DIR}/bin)
endforeach ()

add_custom_command(TARGET BAAS_ocr_server POST_BUILD
    COMMAND install_name_tool -delete_rpath ${BAAS_PROJECT_PATH}/dll/MacOS $<TARGET_FILE:BAAS_ocr_server>
    COMMAND install_name_tool -add_rpath @executable_path $<TARGET_FILE:BAAS_ocr_server>
    COMMENT "Updating rpath for BAAS_ocr_server"
)

target_link_libraries(
        BAAS_ocr_server
        ${DLL_RAW}
)
