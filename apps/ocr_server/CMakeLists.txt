BAAS_hr_LOG("APP BAAS_OCR_SERVER")
project(BAAS_OCR_SERVER VERSION 1.0.0)

add_executable(
        BAAS_ocr_server
        baas_ocr_main.cpp
)

target_include_directories(
        BAAS_ocr_server PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_BINARY_DIR}
)

configure_file(
        ${CMAKE_CURRENT_LIST_DIR}/version.h.in
        ${CMAKE_BINARY_DIR}/BAAS_OCR_Version.h
)