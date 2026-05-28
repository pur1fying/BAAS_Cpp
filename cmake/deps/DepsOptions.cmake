include_guard(GLOBAL)

set(BAAS_LOCAL_ROOT "${CMAKE_SOURCE_DIR}/.baas" CACHE PATH "BAAS local dependency workspace")
set(BAAS_DEPS_ROOT "${BAAS_LOCAL_ROOT}/deps" CACHE PATH "BAAS staged dependency package root")
set(BAAS_ASSETS_ROOT "${BAAS_LOCAL_ROOT}/assets" CACHE PATH "BAAS runtime asset root")
set(BAAS_DOWNLOADS_ROOT "${BAAS_LOCAL_ROOT}/downloads" CACHE PATH "BAAS dependency download cache root")
set(BAAS_DEPS_BUILD_ROOT "${BAAS_LOCAL_ROOT}/build" CACHE PATH "BAAS third-party dependency build root")

set(BAAS_DEPS_MODE "auto" CACHE STRING "Dependency resolution mode: auto, download, system, source")
set_property(CACHE BAAS_DEPS_MODE PROPERTY STRINGS auto download system source)

option(BAAS_ALLOW_DOWNLOADS "Allow dependency bootstrap scripts to download archives" ON)
option(BAAS_USE_SYSTEM_ADB "Prefer a system Android Debug Bridge executable before downloaded assets" ON)
option(BAAS_USE_CUDA "Enable CUDA dependency discovery for shared code paths" OFF)

set(BAAS_FFMPEG_PROVIDER "prebuilt" CACHE STRING "FFmpeg provider: prebuilt, source, system")
set_property(CACHE BAAS_FFMPEG_PROVIDER PROPERTY STRINGS prebuilt source system)

set(BAAS_OPENCV_PROVIDER "source" CACHE STRING "OpenCV provider: prebuilt, source, system")
set_property(CACHE BAAS_OPENCV_PROVIDER PROPERTY STRINGS prebuilt source system)

set(BAAS_ONNXRUNTIME_PROVIDER "cpu" CACHE STRING "ONNX Runtime provider: cpu, cuda")
set_property(CACHE BAAS_ONNXRUNTIME_PROVIDER PROPERTY STRINGS cpu cuda)

set(BAAS_RESOURCE_PROVIDER "download" CACHE STRING "Runtime resource provider: download, local")
set_property(CACHE BAAS_RESOURCE_PROVIDER PROPERTY STRINGS download local)

set(BAAS_ADB_EXECUTABLE "" CACHE FILEPATH "Explicit adb executable path")
