include_guard(GLOBAL)

function(baas_try_enable_ffmpeg out_var)
    if(TARGET BAAS::FFmpeg)
        set(${out_var} TRUE PARENT_SCOPE)
        return()
    endif()

    get_property(_ready GLOBAL PROPERTY BAAS_DEPENDENCY_ffmpeg_READY)
    if(NOT _ready)
        set(${out_var} FALSE PARENT_SCOPE)
        return()
    endif()

    get_property(_package_dir GLOBAL PROPERTY BAAS_DEPENDENCY_ffmpeg_PACKAGE_DIR)
    set(_include_dir "${_package_dir}/include")
    set(_ffmpeg_libs
            "${_package_dir}/lib/avcodec.lib"
            "${_package_dir}/lib/avformat.lib"
            "${_package_dir}/lib/avutil.lib"
            "${_package_dir}/lib/swresample.lib"
            "${_package_dir}/lib/swscale.lib"
    )
    foreach(_lib IN LISTS _ffmpeg_libs)
        if(NOT EXISTS "${_lib}")
            set(${out_var} FALSE PARENT_SCOPE)
            return()
        endif()
    endforeach()
    if(NOT EXISTS "${_include_dir}/libavcodec/avcodec.h")
        set(${out_var} FALSE PARENT_SCOPE)
        return()
    endif()

    add_library(BAAS::FFmpeg INTERFACE IMPORTED GLOBAL)
    set_target_properties(
            BAAS::FFmpeg
            PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${_include_dir}"
            INTERFACE_LINK_LIBRARIES "${_ffmpeg_libs}"
    )

    file(GLOB _ffmpeg_dlls CONFIGURE_DEPENDS "${_package_dir}/bin/*.dll")
    foreach(_dll IN LISTS _ffmpeg_dlls)
        baas_register_runtime_file("ffmpeg" "${_dll}")
    endforeach()

    set(${out_var} TRUE PARENT_SCOPE)
endfunction()

function(baas_require_ffmpeg_target)
    get_property(_ready GLOBAL PROPERTY BAAS_DEPENDENCY_ffmpeg_READY)
    if(NOT _ready)
        message(FATAL_ERROR "Dependency 'ffmpeg' is not ready in BAAS dependency manifest: ${BAAS_DEPENDENCY_INDEX}")
    endif()
    baas_try_enable_ffmpeg(_ffmpeg_available)
    if(NOT _ffmpeg_available)
        get_property(_package_dir GLOBAL PROPERTY BAAS_DEPENDENCY_ffmpeg_PACKAGE_DIR)
        message(FATAL_ERROR "BAAS::FFmpeg package is ready but required files are missing under ${_package_dir}. Run python -m deploy.bootstrap_dependency --dependency ffmpeg --build-type ${BAAS_DEPENDENCY_BUILD_TYPE}")
    endif()
endfunction()
