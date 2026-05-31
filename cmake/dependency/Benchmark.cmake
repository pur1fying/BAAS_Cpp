include_guard(GLOBAL)

function(baas_try_enable_benchmark out_var)
    if(TARGET BAAS::benchmark AND TARGET BAAS::benchmark_main)
        set(${out_var} TRUE PARENT_SCOPE)
        return()
    endif()

    get_property(_ready GLOBAL PROPERTY BAAS_DEPENDENCY_benchmark_READY)
    if(NOT _ready)
        set(${out_var} FALSE PARENT_SCOPE)
        return()
    endif()

    get_property(_benchmark_package_dir GLOBAL PROPERTY BAAS_DEPENDENCY_benchmark_PACKAGE_DIR)
    set(_benchmark_config "${_benchmark_package_dir}/lib/cmake/benchmark/benchmarkConfig.cmake")

    if(NOT EXISTS "${_benchmark_config}")
        set(${out_var} FALSE PARENT_SCOPE)
        return()
    endif()

    find_package(benchmark CONFIG QUIET PATHS "${_benchmark_package_dir}" NO_DEFAULT_PATH)
    if(NOT TARGET benchmark::benchmark)
        set(${out_var} FALSE PARENT_SCOPE)
        return()
    endif()

    add_library(BAAS::benchmark INTERFACE IMPORTED GLOBAL)
    target_link_libraries(BAAS::benchmark INTERFACE benchmark::benchmark)

    if(TARGET benchmark::benchmark_main)
        add_library(BAAS::benchmark_main INTERFACE IMPORTED GLOBAL)
        target_link_libraries(BAAS::benchmark_main INTERFACE benchmark::benchmark_main)
    else()
        add_library(BAAS::benchmark_main INTERFACE IMPORTED GLOBAL)
        target_link_libraries(BAAS::benchmark_main INTERFACE BAAS::benchmark)
    endif()

    set(${out_var} TRUE PARENT_SCOPE)
endfunction()
