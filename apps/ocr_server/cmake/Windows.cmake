if(BAAS_OCR_SERVER_USE_CUDA)
    target_compile_definitions(
            BAAS_ocr_server
            PRIVATE
            __CUDA__
    )
endif()

baas_require_opencv_target()
baas_require_onnxruntime_target()
baas_require_nlohmann_json_target()
baas_require_httplib_target()
baas_require_spdlog_target()
baas_require_simdutf_target()

set(
        LIB_RAW
        BAAS_ipc
        BAAS::OpenCV
        BAAS::ONNXRuntime
        BAAS::nlohmann_json
        BAAS::httplib
        BAAS::spdlog
        BAAS::simdutf
        ws2_32
        shlwapi
)

if(BAAS_OCR_SERVER_USE_CUDA)
    get_property(_onnxruntime_provider GLOBAL PROPERTY BAAS_DEPENDENCY_onnxruntime_PROVIDER)
    if(NOT _onnxruntime_provider STREQUAL "cuda")
        message(FATAL_ERROR "BAAS_OCR_SERVER_USE_CUDA requires ONNXRuntime provider 'cuda' in BAAS dependency index. Run: python -m deploy.bootstrap_dependency --dependency onnxruntime --provider cuda --build-type ${BAAS_DEPENDENCY_BUILD_TYPE}")
    endif()
    baas_require_cuda_target()
    list(APPEND LIB_RAW BAAS::CUDA)
    if(TARGET BAAS::ONNXRuntimeCUDAProvider)
        list(APPEND LIB_RAW BAAS::ONNXRuntimeCUDAProvider)
    endif()
endif()

message(STATUS "LIB RAW :")
foreach (LIB ${LIB_RAW})
    message(STATUS "${LIB}")
endforeach ()
LOG_LINE()

target_link_libraries(
        BAAS_ocr_server
        ${LIB_RAW}
)

baas_copy_runtime_dependencies(BAAS_ocr_server)
