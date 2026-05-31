include_guard(GLOBAL)

function(baas_try_enable_lz4 out_var)
    if(TARGET BAAS::LZ4)
        set(${out_var} TRUE PARENT_SCOPE)
        return()
    endif()

    get_property(_ready GLOBAL PROPERTY BAAS_DEPENDENCY_lz4_READY)
    if(NOT _ready)
        set(${out_var} FALSE PARENT_SCOPE)
        return()
    endif()

    get_property(_package_dir GLOBAL PROPERTY BAAS_DEPENDENCY_lz4_PACKAGE_DIR)
    baas_find_first_glob(_lz4_lib "${_package_dir}/lib/*.lib" "${_package_dir}/**/*.lib")
    if(NOT _lz4_lib OR NOT EXISTS "${_package_dir}/include/lz4.h")
        set(${out_var} FALSE PARENT_SCOPE)
        return()
    endif()

    add_library(BAAS::LZ4 UNKNOWN IMPORTED GLOBAL)
    set_target_properties(
            BAAS::LZ4
            PROPERTIES
            IMPORTED_LOCATION "${_lz4_lib}"
            INTERFACE_INCLUDE_DIRECTORIES "${_package_dir}/include"
    )

    file(GLOB _lz4_dlls CONFIGURE_DEPENDS "${_package_dir}/bin/*.dll")
    foreach(_dll IN LISTS _lz4_dlls)
        baas_register_runtime_file("lz4" "${_dll}")
    endforeach()

    set(${out_var} TRUE PARENT_SCOPE)
endfunction()

function(baas_require_lz4_target)
    get_property(_ready GLOBAL PROPERTY BAAS_DEPENDENCY_lz4_READY)
    if(NOT _ready)
        message(FATAL_ERROR "Dependency 'lz4' is not ready in BAAS dependency manifest: ${BAAS_DEPENDENCY_INDEX}")
    endif()
    baas_try_enable_lz4(_lz4_available)
    if(NOT _lz4_available)
        get_property(_package_dir GLOBAL PROPERTY BAAS_DEPENDENCY_lz4_PACKAGE_DIR)
        message(FATAL_ERROR "BAAS::LZ4 package is ready but required files are missing under ${_package_dir}. Run python -m deploy.bootstrap_dependency --dependency lz4 --build-type ${BAAS_DEPENDENCY_BUILD_TYPE}")
    endif()
endfunction()
