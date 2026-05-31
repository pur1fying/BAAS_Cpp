include_guard(GLOBAL)

function(baas_try_enable_httplib out_var)
    if(TARGET BAAS::httplib)
        set(${out_var} TRUE PARENT_SCOPE)
        return()
    endif()

    get_property(_ready GLOBAL PROPERTY BAAS_DEPENDENCY_cpp_httplib_READY)
    if(NOT _ready)
        set(${out_var} FALSE PARENT_SCOPE)
        return()
    endif()

    get_property(_package_dir GLOBAL PROPERTY BAAS_DEPENDENCY_cpp_httplib_PACKAGE_DIR)
    if(NOT EXISTS "${_package_dir}/include/httplib.h")
        set(${out_var} FALSE PARENT_SCOPE)
        return()
    endif()

    add_library(BAAS::httplib INTERFACE IMPORTED GLOBAL)
    set_target_properties(
            BAAS::httplib
            PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${_package_dir}/include"
    )

    set(${out_var} TRUE PARENT_SCOPE)
endfunction()

function(baas_require_httplib_target)
    get_property(_ready GLOBAL PROPERTY BAAS_DEPENDENCY_cpp_httplib_READY)
    if(NOT _ready)
        message(FATAL_ERROR "Dependency 'cpp_httplib' is not ready in BAAS dependency manifest: ${BAAS_DEPENDENCY_INDEX}")
    endif()
    baas_try_enable_httplib(_httplib_available)
    if(NOT _httplib_available)
        get_property(_package_dir GLOBAL PROPERTY BAAS_DEPENDENCY_cpp_httplib_PACKAGE_DIR)
        message(FATAL_ERROR "BAAS::httplib package is ready but required files are missing under ${_package_dir}. Run python -m deploy.bootstrap_dependency --dependency cpp_httplib --build-type ${BAAS_DEPENDENCY_BUILD_TYPE}")
    endif()
endfunction()
