include_guard(GLOBAL)

function(baas_dependency_index_key dependency_name out_var)
    string(REGEX REPLACE "[^A-Za-z0-9]" "_" _dependency_key "${dependency_name}")
    string(TOUPPER "${_dependency_key}" _dependency_key)
    set(${out_var} "${_dependency_key}" PARENT_SCOPE)
endfunction()

function(baas_include_dependency_index_if_exists)
    if(NOT EXISTS "${BAAS_DEPENDENCY_INDEX}")
        set(BAAS_DEPENDENCY_INDEX_LOADED FALSE CACHE INTERNAL "Whether the BAAS dependency index has been loaded" FORCE)
        return()
    endif()

    include("${BAAS_DEPENDENCY_INDEX}")
    get_cmake_property(_baas_index_variables VARIABLES)
    foreach(_variable IN LISTS _baas_index_variables)
        if(_variable MATCHES "^BAAS_(DEPENDENCY_ROOT|ASSETS_ROOT|PLATFORM_KEY|ARCH_KEY|COMPILER_KEY|BUILD_TYPE_KEY|VARIANT)$"
                OR _variable MATCHES "^BAAS_DEP_[A-Z0-9_]+_(VERSION|PROVIDER|READY|PACKAGE_DIR)$")
            set(${_variable} "${${_variable}}" CACHE INTERNAL "Loaded from BAAS dependency index" FORCE)
        endif()
    endforeach()
    set(BAAS_DEPENDENCY_INDEX_LOADED TRUE CACHE INTERNAL "Whether the BAAS dependency index has been loaded" FORCE)
endfunction()

function(baas_bootstrap_dependency dependency_name)
    set(_options "")
    set(_one_value_args PROVIDER)
    set(_multi_value_args "")
    cmake_parse_arguments(_baas_bootstrap "${_options}" "${_one_value_args}" "${_multi_value_args}" ${ARGN})

    if(DEFINED BAAS_PYTHON_EXECUTABLE AND NOT BAAS_PYTHON_EXECUTABLE STREQUAL "")
        set(_python_executable "${BAAS_PYTHON_EXECUTABLE}")
    else()
        find_package(Python3 COMPONENTS Interpreter REQUIRED)
        set(_python_executable "${Python3_EXECUTABLE}")
    endif()

    set(_bootstrap_args -m deploy.bootstrap_dependency --dependency "${dependency_name}")
    if(DEFINED CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE STREQUAL "")
        list(APPEND _bootstrap_args --config "${CMAKE_BUILD_TYPE}")
    else()
        list(APPEND _bootstrap_args --config Release)
    endif()

    if(DEFINED BAAS_BOOTSTRAP_PLATFORM AND NOT BAAS_BOOTSTRAP_PLATFORM STREQUAL "")
        list(APPEND _bootstrap_args --platform "${BAAS_BOOTSTRAP_PLATFORM}")
    elseif(DEFINED TARGET_OS_NAME AND NOT TARGET_OS_NAME STREQUAL "")
        list(APPEND _bootstrap_args --platform "${TARGET_OS_NAME}")
    elseif(DEFINED CMAKE_SYSTEM_NAME AND NOT CMAKE_SYSTEM_NAME STREQUAL "")
        list(APPEND _bootstrap_args --platform "${CMAKE_SYSTEM_NAME}")
    endif()

    if(DEFINED BAAS_BOOTSTRAP_ARCH AND NOT BAAS_BOOTSTRAP_ARCH STREQUAL "")
        list(APPEND _bootstrap_args --arch "${BAAS_BOOTSTRAP_ARCH}")
    elseif(DEFINED ANDROID_ABI AND NOT ANDROID_ABI STREQUAL "")
        list(APPEND _bootstrap_args --android-abi "${ANDROID_ABI}")
    elseif(DEFINED CMAKE_SYSTEM_PROCESSOR AND NOT CMAKE_SYSTEM_PROCESSOR STREQUAL "")
        list(APPEND _bootstrap_args --arch "${CMAKE_SYSTEM_PROCESSOR}")
    endif()

    if(DEFINED CMAKE_CXX_COMPILER_ID AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "")
        list(APPEND _bootstrap_args --compiler-id "${CMAKE_CXX_COMPILER_ID}")
    endif()
    if(DEFINED CMAKE_CXX_COMPILER_VERSION AND NOT CMAKE_CXX_COMPILER_VERSION STREQUAL "")
        list(APPEND _bootstrap_args --compiler-version "${CMAKE_CXX_COMPILER_VERSION}")
    endif()
    if(DEFINED CMAKE_TOOLCHAIN_FILE AND NOT CMAKE_TOOLCHAIN_FILE STREQUAL "")
        list(APPEND _bootstrap_args --toolchain-file "${CMAKE_TOOLCHAIN_FILE}")
    endif()
    if(DEFINED _baas_bootstrap_PROVIDER AND NOT _baas_bootstrap_PROVIDER STREQUAL "")
        list(APPEND _bootstrap_args --provider "${_baas_bootstrap_PROVIDER}")
    endif()

    message(STATUS "Bootstrapping BAAS dependency '${dependency_name}'")
    execute_process(
            COMMAND "${_python_executable}" ${_bootstrap_args}
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            RESULT_VARIABLE _bootstrap_result
            OUTPUT_VARIABLE _bootstrap_stdout
            ERROR_VARIABLE _bootstrap_stderr
    )
    if(NOT _bootstrap_result EQUAL 0)
        string(REPLACE ";" " " _bootstrap_command "${_python_executable};${_bootstrap_args}")
        message(FATAL_ERROR
                "BAAS dependency bootstrap failed for '${dependency_name}'.\n"
                "Command: ${_bootstrap_command}\n"
                "Logs: ${CMAKE_SOURCE_DIR}/output/log/dependency\n"
                "stdout:\n${_bootstrap_stdout}\n"
                "stderr:\n${_bootstrap_stderr}")
    endif()

    baas_include_dependency_index_if_exists()
endfunction()

function(baas_get_dependency_ready dependency_name out_var)
    baas_include_dependency_index_if_exists()
    if(NOT EXISTS "${BAAS_DEPENDENCY_INDEX}")
        set(${out_var} FALSE PARENT_SCOPE)
        return()
    endif()

    baas_dependency_index_key("${dependency_name}" _dependency_key)
    set(_ready_var "BAAS_DEP_${_dependency_key}_READY")
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

function(baas_ensure_dependency_ready dependency_name)
    set(_options "")
    set(_one_value_args PROVIDER)
    set(_multi_value_args "")
    cmake_parse_arguments(_baas_ensure "${_options}" "${_one_value_args}" "${_multi_value_args}" ${ARGN})

    baas_get_dependency_ready("${dependency_name}" _dependency_ready)
    if(NOT _dependency_ready)
        if(DEFINED _baas_ensure_PROVIDER AND NOT _baas_ensure_PROVIDER STREQUAL "")
            baas_bootstrap_dependency("${dependency_name}" PROVIDER "${_baas_ensure_PROVIDER}")
        else()
            baas_bootstrap_dependency("${dependency_name}")
        endif()
        baas_get_dependency_ready("${dependency_name}" _dependency_ready)
    endif()

    if(NOT _dependency_ready)
        message(FATAL_ERROR
                "Dependency '${dependency_name}' is not ready after BAAS bootstrap. "
                "Index: ${BAAS_DEPENDENCY_INDEX}")
    endif()
endfunction()

function(baas_require_dependency_ready dependency_name)
    baas_ensure_dependency_ready("${dependency_name}" ${ARGN})
endfunction()

function(baas_get_dependency_package_dir dependency_name out_var)
    baas_ensure_dependency_ready("${dependency_name}" ${ARGN})
    baas_dependency_index_key("${dependency_name}" _dependency_key)
    set(_package_var "BAAS_DEP_${_dependency_key}_PACKAGE_DIR")
    if(NOT DEFINED ${_package_var})
        message(FATAL_ERROR
                "Dependency '${dependency_name}' does not have PACKAGE_DIR in BAAS dependency index: ${BAAS_DEPENDENCY_INDEX}.")
    endif()
    if("${${_package_var}}" STREQUAL "")
        message(FATAL_ERROR
                "Dependency '${dependency_name}' has an empty PACKAGE_DIR in BAAS dependency index: ${BAAS_DEPENDENCY_INDEX}.")
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

baas_include_dependency_index_if_exists()
