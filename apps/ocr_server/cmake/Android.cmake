set(
        DLL_RAW
        libopencv_java4.so
        libonnxruntime.so
)


target_link_directories(
        BAAS_ocr_server
        PRIVATE
        ${BAAS_DEFAULT_SEARCH_DLL_PATH}/${ANDROID_ABI}
)

LOG_LINE()
message(STATUS "DLL RAW :")
foreach (DLL ${DLL_RAW})
    message(STATUS "${DLL}")
endforeach ()

set(
        BAAS_OCR_LIBRARY_OUTPUT_DIRECTORY
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lib/${ANDROID_ABI}
)

set_target_properties(
        BAAS_ocr_server
        PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY
        ${BAAS_OCR_LIBRARY_OUTPUT_DIRECTORY}
)

copy_android_libcxx_shared(${BAAS_OCR_LIBRARY_OUTPUT_DIRECTORY})

foreach (dll ${DLL_RAW})
    set(FULL_PATH ${BAAS_DEFAULT_SEARCH_DLL_PATH}/${ANDROID_ABI}/${dll})
    file(COPY ${FULL_PATH} DESTINATION ${BAAS_OCR_LIBRARY_OUTPUT_DIRECTORY})
endforeach ()

target_link_libraries(
        BAAS_ocr_server
        PRIVATE
        log
        android
                
        ${BAAS_DEFAULT_SEARCH_DLL_PATH}/${ANDROID_ABI}/libopencv_java4.so
        ${BAAS_DEFAULT_SEARCH_DLL_PATH}/${ANDROID_ABI}/libonnxruntime.so
)

