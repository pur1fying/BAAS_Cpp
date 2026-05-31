include_guard(GLOBAL)

function(baas_try_enable_nlohmann_json out_var)
    if(TARGET BAAS::nlohmann_json)
        set(${out_var} TRUE PARENT_SCOPE)
        return()
    endif()

    get_property(_ready GLOBAL PROPERTY BAAS_DEPENDENCY_nlohmann_json_READY)
    if(NOT _ready)
        set(${out_var} FALSE PARENT_SCOPE)
        return()
    endif()

    get_property(_package_dir GLOBAL PROPERTY BAAS_DEPENDENCY_nlohmann_json_PACKAGE_DIR)
    if(NOT EXISTS "${_package_dir}/include/nlohmann/json.hpp")
        set(${out_var} FALSE PARENT_SCOPE)
        return()
    endif()

    add_library(BAAS::nlohmann_json INTERFACE IMPORTED GLOBAL)
    set_target_properties(
            BAAS::nlohmann_json
            PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${_package_dir}/include"
    )

    set(${out_var} TRUE PARENT_SCOPE)
endfunction()

function(baas_require_nlohmann_json_target)
    get_property(_ready GLOBAL PROPERTY BAAS_DEPENDENCY_nlohmann_json_READY)
    if(NOT _ready)
        message(FATAL_ERROR "Dependency 'nlohmann_json' is not ready in BAAS dependency manifest: ${BAAS_DEPENDENCY_INDEX}")
    endif()
    baas_try_enable_nlohmann_json(_nlohmann_json_available)
    if(NOT _nlohmann_json_available)
        get_property(_package_dir GLOBAL PROPERTY BAAS_DEPENDENCY_nlohmann_json_PACKAGE_DIR)
        message(FATAL_ERROR "BAAS::nlohmann_json package is ready but required files are missing under ${_package_dir}. Run python -m deploy.bootstrap_dependency --dependency nlohmann_json --build-type ${BAAS_DEPENDENCY_BUILD_TYPE}")
    endif()
endfunction()
