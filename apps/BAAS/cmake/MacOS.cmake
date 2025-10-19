BAAS_sub_title_LOG("BAAS_APP MacOS Configure")

target_link_directories(
        BAAS_APP
        PRIVATE
        ${BAAS_DEFAULT_SEARCH_DLL_PATH}
)

SET(
        DLL_COMMON
        libonnxruntime.dylib
        libavcodec.60.dylib
        libavutil.58.dylib
        libswresample.4.dylib
        libswscale.7.dylib
        liblz4.1.dylib
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
    libavcodec.60.31.102.dylib
    libavutil.58.29.100.dylib
    libswresample.4.12.100.dylib
    libswscale.7.5.100.dylib
    liblz4.1.10.0.dylib
)

foreach (dll ${DLL_MOVE})
    set(FULL_PATH ${BAAS_PROJECT_PATH}/dll/${TARGET_OS_NAME}/${dll})
    file(COPY ${FULL_PATH} DESTINATION ${CMAKE_BINARY_DIR}/bin)
endforeach ()

add_custom_command(
    TARGET BAAS_APP 
    POST_BUILD
    COMMAND install_name_tool -delete_rpath ${BAAS_PROJECT_PATH}/dll/MacOS $<TARGET_FILE:BAAS_APP>
    COMMAND install_name_tool -add_rpath @executable_path $<TARGET_FILE:BAAS_APP>
    COMMENT "Updating rpath for BAAS_APP"
)

target_link_libraries(
        BAAS_APP
        ${DLL_RAW}
)