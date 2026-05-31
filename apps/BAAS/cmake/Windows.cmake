BAAS_sub_title_LOG("BAAS_APP Windows Configure")

if(BAAS_APP_USE_CUDA)
    target_compile_definitions(
            BAAS_APP
            PRIVATE
            __CUDA__
    )
endif()

baas_require_opencv_target()
baas_require_onnxruntime_target()
baas_require_ffmpeg_target()
baas_require_lz4_target()
baas_require_nlohmann_json_target()
baas_require_httplib_target()
baas_require_spdlog_target()
baas_require_simdutf_target()
baas_try_enable_benchmark(BAAS_BENCHMARK_AVAILABLE)

set(
        LIB_RAW
        BAAS_ipc
        BAAS::OpenCV
        BAAS::ONNXRuntime
        BAAS::FFmpeg
        BAAS::LZ4
        BAAS::nlohmann_json
        BAAS::httplib
        BAAS::spdlog
        BAAS::simdutf
        ws2_32
        shlwapi
)

if(BAAS_APP_USE_CUDA)
    get_property(_onnxruntime_provider GLOBAL PROPERTY BAAS_DEPENDENCY_onnxruntime_PROVIDER)
    if(NOT _onnxruntime_provider STREQUAL "cuda")
        message(FATAL_ERROR "BAAS_APP_USE_CUDA requires ONNXRuntime provider 'cuda' in BAAS dependency index. Run: python -m deploy.bootstrap_dependency --dependency onnxruntime --provider cuda --build-type ${BAAS_DEPENDENCY_BUILD_TYPE}")
    endif()
    baas_require_cuda_target()
    list(APPEND LIB_RAW BAAS::CUDA)
    if(TARGET BAAS::ONNXRuntimeCUDAProvider)
        list(APPEND LIB_RAW BAAS::ONNXRuntimeCUDAProvider)
    endif()
endif()

if(BAAS_BENCHMARK_AVAILABLE)
    list(APPEND LIB_RAW BAAS::benchmark BAAS::benchmark_main)
else()
    message(FATAL_ERROR "BAAS::benchmark package was not found. Run: python -m deploy.bootstrap_dependency --dependency benchmark --build-type ${BAAS_DEPENDENCY_BUILD_TYPE}")
endif()

message(STATUS "LIB RAW :")
foreach (LIB ${LIB_RAW})
    message(STATUS "${LIB}")
endforeach ()

target_link_libraries(
        BAAS_APP
        ${LIB_RAW}
)

baas_copy_runtime_dependencies(BAAS_APP)

set(ICON_RESOURCE "${BAAS_APP_BAAS_CPP_DIR}/src/rc/logo.ico")
set(RES_FILE "${BAAS_APP_BAAS_CPP_DIR}/src/rc/app.rc")
target_sources(BAAS_APP PRIVATE ${RES_FILE})
