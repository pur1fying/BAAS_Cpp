include_guard(GLOBAL)

function(baas_dependency_index_key dep_name out_var)
    string(REGEX REPLACE "[^A-Za-z0-9]" "_" _dep_key "${dep_name}")
    string(TOUPPER "${_dep_key}" _dep_key)
    set(${out_var} "${_dep_key}" PARENT_SCOPE)
endfunction()

function(baas_require_dependency_index)
    if(NOT EXISTS "${BAAS_DEPENDENCY_INDEX}")
        message(FATAL_ERROR
                "BAAS dependency index not found: ${BAAS_DEPENDENCY_INDEX}. "
                "Run python -m deploy.bootstrap_dependency first.")
    endif()
endfunction()

baas_require_dependency_index()
include("${BAAS_DEPENDENCY_INDEX}")

function(baas_get_dependency_ready dep_name out_var)
    baas_dependency_index_key("${dep_name}" _dep_key)
    set(_ready_var "BAAS_DEP_${_dep_key}_READY")
    set(_ready_value FALSE)
    if(DEFINED ${_ready_var})
        set(_ready_value "${${_ready_var}}")
    endif()
    if(_ready_value)
        set(${out_var} TRUE PARENT_SCOPE)
    else()
        set(${out_var} FALSE PARENT_SCOPE)
    endif()
endfunction()

function(baas_require_dependency_ready dep_name)
    baas_get_dependency_ready("${dep_name}" _ready)
    if(NOT _ready)
        message(FATAL_ERROR
                "Dependency '${dep_name}' is not ready in BAAS dependency index: ${BAAS_DEPENDENCY_INDEX}. "
                "Run python -m deploy.bootstrap_dependency first.")
    endif()
endfunction()

function(baas_get_dependency_package_dir dep_name out_var)
    baas_require_dependency_ready("${dep_name}")
    baas_dependency_index_key("${dep_name}" _dep_key)
    set(_package_var "BAAS_DEP_${_dep_key}_PACKAGE_DIR")
    if(NOT DEFINED ${_package_var})
        message(FATAL_ERROR
                "Dependency '${dep_name}' does not have PACKAGE_DIR in BAAS dependency index: ${BAAS_DEPENDENCY_INDEX}. "
                "Run python -m deploy.bootstrap_dependency first.")
    endif()
    if("${${_package_var}}" STREQUAL "")
        message(FATAL_ERROR
                "Dependency '${dep_name}' has an empty PACKAGE_DIR in BAAS dependency index: ${BAAS_DEPENDENCY_INDEX}. "
                "Run python -m deploy.bootstrap_dependency first.")
    endif()
    set(${out_var} "${${_package_var}}" PARENT_SCOPE)
endfunction()

function(baas_find_first_glob out_var)
    foreach(_pattern IN LISTS ARGN)
        file(GLOB_RECURSE _matches CONFIGURE_DEPENDS "${_pattern}")
        if(_matches)
            list(SORT _matches)
            list(GET _matches 0 _first)
            set(${out_var} "${_first}" PARENT_SCOPE)
            return()
        endif()
    endforeach()
    set(${out_var} "" PARENT_SCOPE)
endfunction()
