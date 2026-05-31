include_guard(GLOBAL)

function(baas_try_enable_simdutf out_var)
    if(TARGET BAAS::simdutf)
        set(${out_var} TRUE PARENT_SCOPE)
        return()
    endif()

    get_property(_ready GLOBAL PROPERTY BAAS_DEPENDENCY_simdutf_READY)
    if(NOT _ready)
        set(${out_var} FALSE PARENT_SCOPE)
        return()
    endif()

    get_property(_package_dir GLOBAL PROPERTY BAAS_DEPENDENCY_simdutf_PACKAGE_DIR)
    if(NOT EXISTS "${_package_dir}/include/simdutf.h")
        set(${out_var} FALSE PARENT_SCOPE)
        return()
    endif()

    file(GLOB _runtime_dlls CONFIGURE_DEPENDS "${_package_dir}/bin/*.dll")
    if(NOT _runtime_dlls)
        set(${out_var} FALSE PARENT_SCOPE)
        return()
    endif()

    find_package(simdutf CONFIG QUIET PATHS "${_package_dir}" NO_DEFAULT_PATH)
    if(NOT TARGET simdutf::simdutf)
        set(${out_var} FALSE PARENT_SCOPE)
        return()
    endif()

    add_library(BAAS::simdutf INTERFACE IMPORTED GLOBAL)
    set_target_properties(
            BAAS::simdutf
            PROPERTIES
            INTERFACE_LINK_LIBRARIES simdutf::simdutf
            INTERFACE_COMPILE_DEFINITIONS SIMDUTF_USING_LIBRARY=1
    )

    foreach(_dll IN LISTS _runtime_dlls)
        baas_register_runtime_file("simdutf" "${_dll}")
    endforeach()

    set(${out_var} TRUE PARENT_SCOPE)
endfunction()

function(baas_require_simdutf_target)
    get_property(_ready GLOBAL PROPERTY BAAS_DEPENDENCY_simdutf_READY)
    if(NOT _ready)
        message(FATAL_ERROR "Dependency 'simdutf' is not ready in BAAS dependency manifest: ${BAAS_DEPENDENCY_INDEX}")
    endif()
    baas_try_enable_simdutf(_simdutf_available)
    if(NOT _simdutf_available)
        get_property(_package_dir GLOBAL PROPERTY BAAS_DEPENDENCY_simdutf_PACKAGE_DIR)
        message(FATAL_ERROR "BAAS::simdutf package is ready but required files are missing under ${_package_dir}. Run python -m deploy.bootstrap_dependency --dependency simdutf --build-type ${BAAS_DEPENDENCY_BUILD_TYPE}")
    endif()
endfunction()
