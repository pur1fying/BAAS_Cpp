cmake_minimum_required(VERSION 3.25)
project(BAAS_CXX_Refactor)

set(CMAKE_CXX_STANDARD 20)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

include_directories(
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/external
)

link_directories(
        ${CMAKE_CURRENT_LIST_DIR}/lib
)
file(
        GLOB
        SOURCES "src/*.cpp"
)
file(
        GLOB DLL
        "dll/*.dll"
)
add_executable(
        BAAS_CXX_Refactor
        main.cpp
        ${SOURCES}
)

target_link_libraries(
        BAAS_CXX_Refactor
        opencv_world490.lib
        avcodec.lib
        avdevice.lib
        avfilter.lib
        avformat.lib
        avutil.lib
        postproc.lib
        swresample.lib
        swscale.lib
)
file(COPY ${CMAKE_SOURCE_DIR}/resource DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${DLL} DESTINATION ${CMAKE_BINARY_DIR})