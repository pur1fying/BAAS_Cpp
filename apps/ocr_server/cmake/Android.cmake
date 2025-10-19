target_link_directories(
        BAAS_ocr_server
        PRIVATE
        ${BAAS_DEFAULT_SEARCH_DLL_PATH}
)

target_link_libraries(
        BAAS_ocr_server
        PRIVATE
        log
        android
                
        ${BAAS_DEFAULT_SEARCH_DLL_PATH}/${ANDROID_ABI}/libopencv_java4.so
        ${BAAS_DEFAULT_SEARCH_DLL_PATH}/${ANDROID_ABI}/libonnxruntime.so
)

set_target_properties(
        BAAS_ocr_server
        PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${ANDROID_ABI}
)