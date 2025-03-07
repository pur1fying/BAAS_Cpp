BAAS_sub_title_LOG("MacOS Lib Configure")

target_link_directories(
        BAAS_ocr_server
        PRIVATE
        ${BAAS_PROJECT_PATH}/dll/${CUURENT_OS_NAME}
)