macro(BAAS_hr_LOG message)
    set(PAD_LENGTH 60)

    string(LENGTH "${message}" msg_length)

    math(EXPR LEFT_PAD "(${PAD_LENGTH} - ${msg_length}) / 2 - 1 " )
    math(EXPR RIGHT_PAD "(${PAD_LENGTH} - (${msg_length} + ${LEFT_PAD})) - 2")

    string(REPEAT "-" ${PAD_LENGTH} PADDING)
    string(REPEAT " " ${LEFT_PAD} LEFT_PADDING)
    string(REPEAT " " ${RIGHT_PAD} RIGHT_PADDING)

    message(STATUS  "${PADDING}")
    message(STATUS "|${LEFT_PADDING}${message}${RIGHT_PADDING}|")
    message(STATUS  "${PADDING}")
endmacro()

macro(BAAS_sub_title_LOG)
    string(REPEAT "<" 3 LEFT)
    string(REPEAT ">" 3 RIGHT)
    message(STATUS "${LEFT} ${ARGN} ${RIGHT}")
endmacro()

macro(LOG_LINE)
    message(STATUS "---------------------------------")
endmacro()

function(get_git_info OUTPUT_VAR)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs)
    cmake_parse_arguments(PARSE_ARGV 1 GET_GIT_INFO "${options}" "${oneValueArgs}" "${multiValueArgs}")

    execute_process(
            COMMAND ${GET_GIT_INFO_UNPARSED_ARGUMENTS}
            WORKING_DIRECTORY ${BAAS_PROJECT_PATH}
            OUTPUT_VARIABLE _git_output
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(${OUTPUT_VAR} "${_git_output}" PARENT_SCOPE)
endfunction()

function(check_app_options)
    set(true_count 0)

    foreach(option_name IN LISTS ARGN)
        if(${option_name})
            BAAS_sub_title_LOG("Option ${option_name} True")
            math(EXPR true_count "${true_count} + 1")
            if (${true_count} GREATER 1)
                message(FATAL_ERROR "You Can Build One App At a Time.")
            endif ()
        endif()
    endforeach()
endfunction()

function(set_ADB_BINARY)
    if (${TARGET_OS_NAME} STREQUAL "Windows")
        SET(
            ADB_FILES
            "adb.exe"
            "AdbWinApi.dll"
            "AdbWinUsbApi.dll"
        ) 
    elseif ((${TARGET_OS_NAME} STREQUAL "MacOS") OR (${TARGET_OS_NAME} STREQUAL "Linux"))
        SET(
            ADB_FILES
            "adb"
        )
    endif()

    set(ADB_BINARY "")

    foreach(adb_file IN LISTS ADB_FILES)
        set(FULL_PATH ${BAAS_PROJECT_PATH}/resource/bin/${TARGET_OS_NAME}/platform-tools/${adb_file})
        if(EXISTS ${FULL_PATH})
            message(STATUS "Found ADB binary: ${FULL_PATH}")
            list(APPEND ADB_BINARY ${FULL_PATH})
        else()
            message(FATAL_ERROR "ADB binary not found: ${FULL_PATH}")
        endif()
    endforeach()

    set(ADB_BINARY "${ADB_BINARY}" PARENT_SCOPE)
endfunction()

function(copy_android_libcxx_shared DEST_DIR)
    BAAS_sub_title_LOG("Copy libc++_shared.so from NDK")
    if (ANDROID_ABI STREQUAL "armeabi-v7a")
        set(ANDROID_TRIPLE arm-linux-androideabi)
    elseif (ANDROID_ABI STREQUAL "arm64-v8a")
        set(ANDROID_TRIPLE aarch64-linux-android)
    elseif (ANDROID_ABI STREQUAL "x86")
        set(ANDROID_TRIPLE i686-linux-android)
    elseif (ANDROID_ABI STREQUAL "x86_64")
        set(ANDROID_TRIPLE x86_64-linux-android)
    else()
        message(FATAL_ERROR "NDK Unsupported Android ABI: ${ABI}")
    endif()

    if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        set(NDK_HOST windows-x86_64)
    elseif (CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
        set(NDK_HOST darwin-x86_64)
    elseif (CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
        set(NDK_HOST linux-x86_64)
    else()
        message(FATAL_ERROR "NDK Unsupported host OS: ${CMAKE_HOST_SYSTEM_NAME}")
    endif()

    set(
            LIBCXX_PATH
            ${ANDROID_NDK_PATH}/toolchains/llvm/prebuilt/${NDK_HOST}/sysroot/usr/lib/${ANDROID_TRIPLE}/libc++_shared.so
    )
    if (NOT EXISTS ${LIBCXX_PATH})
        message(FATAL_ERROR "libc++_shared.so not found: ${LIBCXX_PATH}")
    endif()
    message(STATUS "Lib found at : ${LIBCXX_PATH}")
    file(COPY ${LIBCXX_PATH} DESTINATION ${DEST_DIR})
endfunction()