include_guard(GLOBAL)

function(baas_try_enable_opencv out_var)
    if(TARGET BAAS::OpenCV)
        set(${out_var} TRUE PARENT_SCOPE)
        return()
    endif()

    get_property(_ready GLOBAL PROPERTY BAAS_DEPENDENCY_opencv_READY)
    if(NOT _ready)
        set(${out_var} FALSE PARENT_SCOPE)
        return()
    endif()

    get_property(_package_dir GLOBAL PROPERTY BAAS_DEPENDENCY_opencv_PACKAGE_DIR)
    if(BAAS_DEPENDENCY_BUILD_TYPE STREQUAL "Debug")
        baas_find_first_glob(_opencv_lib "${_package_dir}/lib/opencv_world490d.lib" "${_package_dir}/**/opencv_world490d.lib")
        baas_find_first_glob(_opencv_runtime "${_package_dir}/bin/opencv_world490d.dll" "${_package_dir}/**/opencv_world490d.dll")
    else()
        baas_find_first_glob(_opencv_lib "${_package_dir}/lib/opencv_world490.lib" "${_package_dir}/**/opencv_world490.lib")
        baas_find_first_glob(_opencv_runtime "${_package_dir}/bin/opencv_world490.dll" "${_package_dir}/**/opencv_world490.dll")
    endif()

    if(NOT _opencv_lib OR NOT EXISTS "${_package_dir}/include/opencv2/core.hpp")
        set(${out_var} FALSE PARENT_SCOPE)
        return()
    endif()

    add_library(BAAS::OpenCV UNKNOWN IMPORTED GLOBAL)
    set_target_properties(
            BAAS::OpenCV
            PROPERTIES
            IMPORTED_LOCATION "${_opencv_lib}"
            INTERFACE_INCLUDE_DIRECTORIES "${_package_dir}/include"
    )

    if(_opencv_runtime)
        baas_register_runtime_file("opencv" "${_opencv_runtime}")
    endif()

    set(${out_var} TRUE PARENT_SCOPE)
endfunction()

function(baas_require_opencv_target)
    get_property(_ready GLOBAL PROPERTY BAAS_DEPENDENCY_opencv_READY)
    if(NOT _ready)
        message(FATAL_ERROR "Dependency 'opencv' is not ready in BAAS dependency manifest: ${BAAS_DEPENDENCY_INDEX}")
    endif()
    baas_try_enable_opencv(_opencv_available)
    if(NOT _opencv_available)
        get_property(_package_dir GLOBAL PROPERTY BAAS_DEPENDENCY_opencv_PACKAGE_DIR)
        message(FATAL_ERROR "BAAS::OpenCV package is ready but required files are missing under ${_package_dir}. Run python -m deploy.bootstrap_dependency --dependency opencv --build-type ${BAAS_DEPENDENCY_BUILD_TYPE}")
    endif()
endfunction()

function(baas_try_enable_opencv_dnn out_var)
    if(TARGET BAAS::OpenCVDnn)
        set(${out_var} TRUE PARENT_SCOPE)
        return()
    endif()

    get_property(_ready GLOBAL PROPERTY BAAS_DEPENDENCY_opencv_dnn_READY)
    if(NOT _ready)
        set(${out_var} FALSE PARENT_SCOPE)
        return()
    endif()

    get_property(_package_dir GLOBAL PROPERTY BAAS_DEPENDENCY_opencv_dnn_PACKAGE_DIR)
    if(TARGET_OS_NAME STREQUAL "Windows")
        baas_find_first_glob(_opencv_dnn_lib "${_package_dir}/lib/opencv_world490.lib" "${_package_dir}/**/opencv_world490.lib")
    elseif(TARGET_OS_NAME STREQUAL "MacOS")
        baas_find_first_glob(_opencv_dnn_lib "${_package_dir}/lib/libopencv_world.dylib" "${_package_dir}/**/libopencv_world.dylib")
    else()
        baas_find_first_glob(_opencv_dnn_lib "${_package_dir}/lib/libopencv_world.so" "${_package_dir}/**/libopencv_world.so")
    endif()

    if(NOT _opencv_dnn_lib
            OR NOT EXISTS "${_package_dir}/include/opencv2/core.hpp"
            OR NOT EXISTS "${_package_dir}/include/opencv2/dnn.hpp")
        set(${out_var} FALSE PARENT_SCOPE)
        return()
    endif()

    add_library(BAAS::OpenCVDnn UNKNOWN IMPORTED GLOBAL)
    set_target_properties(
            BAAS::OpenCVDnn
            PROPERTIES
            IMPORTED_LOCATION "${_opencv_dnn_lib}"
            INTERFACE_INCLUDE_DIRECTORIES "${_package_dir}/include"
    )

    set(${out_var} TRUE PARENT_SCOPE)
endfunction()

function(baas_require_opencv_dnn_target)
    get_property(_ready GLOBAL PROPERTY BAAS_DEPENDENCY_opencv_dnn_READY)
    if(NOT _ready)
        message(FATAL_ERROR "Dependency 'opencv_dnn' is not ready in BAAS dependency manifest: ${BAAS_DEPENDENCY_INDEX}")
    endif()
    baas_try_enable_opencv_dnn(_opencv_dnn_available)
    if(NOT _opencv_dnn_available)
        get_property(_package_dir GLOBAL PROPERTY BAAS_DEPENDENCY_opencv_dnn_PACKAGE_DIR)
        message(FATAL_ERROR "BAAS::OpenCVDnn package is ready but required files are missing under ${_package_dir}. Run python -m deploy.bootstrap_dependency --dependency opencv_dnn --build-type Release")
    endif()
endfunction()
