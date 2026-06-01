include_guard(GLOBAL)

include("${CMAKE_CURRENT_LIST_DIR}/dependency/DependencyOptions.cmake")
if(EXISTS "${BAAS_DEPENDENCY_INDEX}")
    include("${BAAS_DEPENDENCY_INDEX}")
endif()
include("${CMAKE_CURRENT_LIST_DIR}/dependency/RuntimeCopy.cmake")

function(baas_request_dependencies)
    get_property(_requested GLOBAL PROPERTY BAAS_REQUESTED_DEPENDENCIES)
    set(_last_dependency "")
    set(_provider_dependency "")
    foreach(_arg IN LISTS ARGN)
        if(_arg STREQUAL "")
            continue()
        endif()

        if(_provider_dependency)
            set_property(GLOBAL PROPERTY "BAAS_REQUESTED_DEPENDENCY_PROVIDER_${_provider_dependency}" "${_arg}")
            set(_provider_dependency "")
            continue()
        endif()

        if(_arg STREQUAL "PROVIDER")
            if(_last_dependency STREQUAL "")
                message(FATAL_ERROR "baas_request_dependencies PROVIDER requires a dependency name before it")
            endif()
            set(_provider_dependency "${_last_dependency}")
            continue()
        endif()

        list(APPEND _requested "${_arg}")
        set(_last_dependency "${_arg}")
    endforeach()
    if(_provider_dependency)
        message(FATAL_ERROR "baas_request_dependencies missing provider value for dependency '${_provider_dependency}'")
    endif()
    if(_requested)
        list(REMOVE_DUPLICATES _requested)
    endif()
    set_property(GLOBAL PROPERTY BAAS_REQUESTED_DEPENDENCIES "${_requested}")
endfunction()

function(baas_request_dependency dependency_name)
    baas_request_dependencies("${dependency_name}" ${ARGN})
endfunction()

function(baas_bootstrap_requested_dependencies)
    get_property(_requested GLOBAL PROPERTY BAAS_REQUESTED_DEPENDENCIES)
    if(NOT _requested)
        return()
    endif()
    list(REMOVE_DUPLICATES _requested)

    if(DEFINED BAAS_PYTHON_EXECUTABLE AND NOT BAAS_PYTHON_EXECUTABLE STREQUAL "")
        set(_python_executable "${BAAS_PYTHON_EXECUTABLE}")
    else()
        find_package(Python3 COMPONENTS Interpreter REQUIRED)
        set(_python_executable "${Python3_EXECUTABLE}")
    endif()

    list(JOIN _requested "," _dependencies_arg)
    set(_bootstrap_args
            -m deploy.bootstrap_dependency
            --dependencies "${_dependencies_arg}"
            --build-type "${BAAS_DEPENDENCY_BUILD_TYPE}"
            --cmake-manifest "${BAAS_DEPENDENCY_INDEX}"
    )

    if(DEFINED TARGET_OS_NAME AND NOT TARGET_OS_NAME STREQUAL "")
        list(APPEND _bootstrap_args --platform "${TARGET_OS_NAME}")
    endif()

    if(DEFINED ANDROID_ABI AND NOT ANDROID_ABI STREQUAL "")
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

    set(_provider_overrides "")
    foreach(_dependency IN LISTS _requested)
        get_property(_provider GLOBAL PROPERTY "BAAS_REQUESTED_DEPENDENCY_PROVIDER_${_dependency}")
        if(_provider)
            list(APPEND _provider_overrides "${_dependency}=${_provider}")
        endif()
    endforeach()
    if(_provider_overrides)
        list(JOIN _provider_overrides "," _provider_overrides_arg)
        list(APPEND _bootstrap_args --provider-overrides "${_provider_overrides_arg}")
    endif()

    message(STATUS "Bootstrapping BAAS dependencies: ${_dependencies_arg}")
    execute_process(
            COMMAND "${_python_executable}" ${_bootstrap_args}
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            RESULT_VARIABLE _bootstrap_result
            OUTPUT_VARIABLE _bootstrap_stdout
            ERROR_VARIABLE _bootstrap_stderr
            ECHO_OUTPUT_VARIABLE
            ECHO_ERROR_VARIABLE
    )
    if(NOT _bootstrap_result EQUAL 0)
        string(REPLACE ";" " " _bootstrap_command "${_python_executable};${_bootstrap_args}")
        message(FATAL_ERROR
                "BAAS dependency bootstrap failed.\n"
                "Command: ${_bootstrap_command}\n"
                "Logs: ${CMAKE_SOURCE_DIR}/output/log/dependency\n"
                "stdout:\n${_bootstrap_stdout}\n"
                "stderr:\n${_bootstrap_stderr}")
    endif()

    get_property(_previous_bootstrapped_dependencies GLOBAL PROPERTY BAAS_BOOTSTRAPPED_DEPENDENCIES)
    foreach(_dependency IN LISTS _previous_bootstrapped_dependencies)
        set_property(GLOBAL PROPERTY "BAAS_DEPENDENCY_${_dependency}_VERSION" "")
        set_property(GLOBAL PROPERTY "BAAS_DEPENDENCY_${_dependency}_PROVIDER" "")
        set_property(GLOBAL PROPERTY "BAAS_DEPENDENCY_${_dependency}_READY" "")
        set_property(GLOBAL PROPERTY "BAAS_DEPENDENCY_${_dependency}_CONFIG" "")
        set_property(GLOBAL PROPERTY "BAAS_DEPENDENCY_${_dependency}_PACKAGE_DIR" "")
    endforeach()
    if(EXISTS "${BAAS_DEPENDENCY_INDEX}")
        include("${BAAS_DEPENDENCY_INDEX}")
    endif()
    foreach(_dependency IN LISTS _requested)
        get_property(_ready GLOBAL PROPERTY "BAAS_DEPENDENCY_${_dependency}_READY")
        if(NOT _ready)
            message(FATAL_ERROR
                    "Dependency '${_dependency}' is not ready after BAAS dependency bootstrap.\n"
                    "Manifest: ${BAAS_DEPENDENCY_INDEX}\n"
                    "Run:\n"
                    "  python -m deploy.bootstrap_dependency --dependency ${_dependency} --build-type ${BAAS_DEPENDENCY_BUILD_TYPE}")
        endif()
    endforeach()
endfunction()

include("${CMAKE_CURRENT_LIST_DIR}/dependency/OpenCV.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/dependency/ONNXRuntime.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/dependency/FFmpeg.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/dependency/LZ4.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/dependency/Benchmark.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/dependency/NlohmannJson.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/dependency/CppHttplib.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/dependency/Spdlog.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/dependency/Simdutf.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/dependency/CUDAToolkit.cmake")
