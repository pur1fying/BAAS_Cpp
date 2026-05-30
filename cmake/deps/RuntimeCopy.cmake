include_guard(GLOBAL)

define_property(GLOBAL PROPERTY BAAS_RUNTIME_FILES BRIEF_DOCS "BAAS runtime files" FULL_DOCS "Registered runtime files copied after target build")

function(baas_register_runtime_file name path)
    get_property(_files GLOBAL PROPERTY BAAS_RUNTIME_FILES)
    list(APPEND _files "${name}|${path}")
    set_property(GLOBAL PROPERTY BAAS_RUNTIME_FILES "${_files}")
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
