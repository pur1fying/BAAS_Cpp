include_guard(GLOBAL)

function(baas_try_enable_spdlog out_var)
    if(TARGET BAAS::spdlog)
        set(${out_var} TRUE PARENT_SCOPE)
        return()
    endif()

    get_property(_ready GLOBAL PROPERTY BAAS_DEPENDENCY_spdlog_READY)
    if(NOT _ready)
        set(${out_var} FALSE PARENT_SCOPE)
        return()
    endif()

    get_property(_package_dir GLOBAL PROPERTY BAAS_DEPENDENCY_spdlog_PACKAGE_DIR)
    if(NOT EXISTS "${_package_dir}/include/spdlog/spdlog.h")
        set(${out_var} FALSE PARENT_SCOPE)
        return()
    endif()

    file(GLOB _runtime_dlls CONFIGURE_DEPENDS "${_package_dir}/bin/*.dll")
    if(NOT _runtime_dlls)
        set(${out_var} FALSE PARENT_SCOPE)
        return()
    endif()

    find_package(spdlog CONFIG QUIET PATHS "${_package_dir}" NO_DEFAULT_PATH)
    if(NOT TARGET spdlog::spdlog)
        set(${out_var} FALSE PARENT_SCOPE)
        return()
    endif()

    add_library(BAAS::spdlog INTERFACE IMPORTED GLOBAL)
    set_target_properties(
            BAAS::spdlog
            PROPERTIES
            INTERFACE_LINK_LIBRARIES spdlog::spdlog
    )

    foreach(_dll IN LISTS _runtime_dlls)
        baas_register_runtime_file("spdlog" "${_dll}")
    endforeach()

    set(${out_var} TRUE PARENT_SCOPE)
endfunction()

function(baas_require_spdlog_target)
    get_property(_ready GLOBAL PROPERTY BAAS_DEPENDENCY_spdlog_READY)
    if(NOT _ready)
        message(FATAL_ERROR "Dependency 'spdlog' is not ready in BAAS dependency manifest: ${BAAS_DEPENDENCY_INDEX}")
    endif()
    baas_try_enable_spdlog(_spdlog_available)
    if(NOT _spdlog_available)
        get_property(_package_dir GLOBAL PROPERTY BAAS_DEPENDENCY_spdlog_PACKAGE_DIR)
        message(FATAL_ERROR "BAAS::spdlog package is ready but required files are missing under ${_package_dir}. Run python -m deploy.bootstrap_dependency --dependency spdlog --build-type ${BAAS_DEPENDENCY_BUILD_TYPE}")
    endif()
endfunction()
