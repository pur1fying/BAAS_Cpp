include_guard(GLOBAL)

define_property(GLOBAL PROPERTY BAAS_RUNTIME_FILES BRIEF_DOCS "BAAS runtime files" FULL_DOCS "Registered runtime files copied after target build")
define_property(GLOBAL PROPERTY BAAS_RUNTIME_DIRS BRIEF_DOCS "BAAS runtime dirs" FULL_DOCS "Registered runtime directories copied after target build")

function(baas_register_runtime_file name path)
    get_property(_files GLOBAL PROPERTY BAAS_RUNTIME_FILES)
    list(APPEND _files "${name}|${path}")
    set_property(GLOBAL PROPERTY BAAS_RUNTIME_FILES "${_files}")
endfunction()

function(baas_register_runtime_dir name path)
    get_property(_dirs GLOBAL PROPERTY BAAS_RUNTIME_DIRS)
    list(APPEND _dirs "${name}|${path}")
    set_property(GLOBAL PROPERTY BAAS_RUNTIME_DIRS "${_dirs}")
endfunction()

function(baas_copy_runtime_dependencies target)
    get_property(_files GLOBAL PROPERTY BAAS_RUNTIME_FILES)
    foreach(_entry IN LISTS _files)
        string(REPLACE "|" ";" _parts "${_entry}")
        list(GET _parts 1 _path)
        if(EXISTS "${_path}")
            add_custom_command(
                    TARGET ${target}
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_path}" "$<TARGET_FILE_DIR:${target}>"
            )
        endif()
    endforeach()
endfunction()

function(baas_copy_runtime_resources target)
    get_property(_dirs GLOBAL PROPERTY BAAS_RUNTIME_DIRS)
    foreach(_entry IN LISTS _dirs)
        string(REPLACE "|" ";" _parts "${_entry}")
        list(GET _parts 1 _path)
        if(EXISTS "${_path}")
            add_custom_command(
                    TARGET ${target}
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory "${_path}" "$<TARGET_FILE_DIR:${target}>/resource"
            )
        endif()
    endforeach()
endfunction()

function(baas_resolve_adb out_var)
    if(BAAS_ADB_EXECUTABLE AND EXISTS "${BAAS_ADB_EXECUTABLE}")
        set(${out_var} "${BAAS_ADB_EXECUTABLE}" PARENT_SCOPE)
        return()
    endif()

    if(BAAS_USE_SYSTEM_ADB)
        find_program(_baas_adb adb)
        if(_baas_adb)
            set(${out_var} "${_baas_adb}" PARENT_SCOPE)
            return()
        endif()
    endif()

    foreach(_sdk_var ANDROID_HOME ANDROID_SDK_ROOT)
        if(DEFINED ENV{${_sdk_var}})
            if(WIN32)
                set(_candidate "$ENV{${_sdk_var}}/platform-tools/adb.exe")
            else()
                set(_candidate "$ENV{${_sdk_var}}/platform-tools/adb")
            endif()
            if(EXISTS "${_candidate}")
                set(${out_var} "${_candidate}" PARENT_SCOPE)
                return()
            endif()
        endif()
    endforeach()

    set(${out_var} "" PARENT_SCOPE)
endfunction()
