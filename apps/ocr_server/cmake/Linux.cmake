BAAS_sub_title_LOG("BAAS_ocr_server Linux Configure")

target_link_directories(
        BAAS_ocr_server
        PRIVATE
        ${BAAS_DEFAULT_SEARCH_DLL_PATH}
)

SET(
        DLL_COMMON
        libonnxruntime.so.1
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(FATAL_ERROR "Please use Release in Linux")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
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

set_target_properties(
    BAAS_ocr_server 
    PROPERTIES 
    INSTALL_RPATH "\$ORIGIN"
    BUILD_RPATH "\$ORIGIN"  
)

set(
    CMAKE_BUILD_RPATH_USE_ORIGIN 
    TRUE
)

target_link_libraries(
        BAAS_ocr_server
        PRIVATE
        ${DLL_RAW}
)