include_guard(GLOBAL)

if(NOT BUILD_BAAS_NMS_BENCHMARK)
    return()
endif()

baas_try_enable_benchmark(BAAS_NMS_BENCHMARK_AVAILABLE)
if(NOT BAAS_NMS_BENCHMARK_AVAILABLE)
    message(FATAL_ERROR "BAAS::benchmark package was not found. Run: python -m deploy.bootstrap_dependency --dependency benchmark --build-type ${BAAS_DEPENDENCY_BUILD_TYPE}")
endif()

baas_require_opencv_dnn_target()

add_executable(
        baas_nms_benchmark
        ${CMAKE_CURRENT_LIST_DIR}/../apps/benchmarks/nms/baas_nms_benchmark.cpp
        ${CMAKE_CURRENT_LIST_DIR}/../src/utils/BAASImageNms.cpp
)

target_include_directories(
        baas_nms_benchmark
        PRIVATE
        ${BAAS_PROJECT_PATH}/include
)

target_link_libraries(
        baas_nms_benchmark
        PRIVATE
        BAAS::OpenCVDnn
        BAAS::benchmark
)
