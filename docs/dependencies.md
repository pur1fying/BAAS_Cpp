# BAAS C++ dependency bootstrap

本文档记录 BAAS_Cpp 依赖去仓库化的本地目录、lock 文件、bootstrap 脚本和当前 Windows x64 迁移状态。

## 本地目录

默认本地工作目录为 `${CMAKE_SOURCE_DIR}/.baas`，可以通过 CMake 变量或环境变量覆盖。

- `.baas/dependency`: 已整理好的第三方 dependency package，供 BAAS 编译、链接、运行时复制和最终打包使用。
- `.baas/assets`: 运行资源和工具，例如 OCR 模型、YOLO 模型、platform-tools、scrcpy-server、ascreencap。当前阶段尚未迁移资源。
- `.baas/downloads`: 下载缓存。删除 build 目录后仍可复用下载包。
- `.baas/build`: 第三方依赖源码解压和构建目录。
- `.baas/state.json`: 集中依赖状态文件，记录 fingerprint、package、outputs 等信息。
- `output/log/dependency/<yyyy-mm-dd_hh.mm.ss>`: bootstrap 和第三方构建日志。

用户侧目录字段统一使用 `package` 或 package directory，不使用 `install` / `install_dir`。第三方 CMake 项目内部仍可能执行 `cmake --install`，这只是整理第三方产物的实现细节。

## 固定版本策略

当前阶段只验证固定版本链路，不引入 `range/recommended/resolved` 版本范围结构。每个依赖的 `version` 必须是非空字符串。

多版本兼容会在固定版本的下载、构建、state、fingerprint 和 CMake target 全部稳定后，再按依赖单独验证。

## 已启用的 Windows x64 编译依赖

以下依赖已经可以通过 `python -m deploy.bootstrap_dependency` 生成到 `.baas/dependency`：

- OpenCV `4.9.0`: source build，生成 `BAAS::OpenCV`。
- ONNX Runtime `1.22.0`: Microsoft release package，CPU provider 生成 `BAAS::ONNXRuntime`，CUDA provider 额外生成 `BAAS::ONNXRuntimeCUDAProvider`。
- FFmpeg `6.1-abi60`: BtbN n6.1 LGPL shared package，生成 `BAAS::FFmpeg`。
- LZ4 `1.10.0`: source build，生成 `BAAS::LZ4`。
- nlohmann/json `3.11.3`: header archive，生成 `BAAS::nlohmann_json`。
- spdlog `1.15.3`: source build，生成 `BAAS::spdlog`。
- cpp-httplib `0.18.0`: single header，生成 `BAAS::httplib`。
- simdutf `6.2.0`: source build，生成 `BAAS::simdutf`。
- Google Benchmark `1.9.1`: source build，生成 `BAAS::benchmark` 和 `BAAS::benchmark_main`。
- CUDA Toolkit `12.2`: system provider，通过 `find_package(CUDAToolkit 12.2)` 使用本机环境，不下载到仓库。

资源类条目仍在 `resources.lock.json` 中保留 TODO，占位不会触发下载或构建。

## Bootstrap 命令

打印计划：

```powershell
python -m deploy.bootstrap_dependency --print-plan --build-type Release
```

生成所有 Windows x64 编译依赖：

```powershell
python -m deploy.bootstrap_dependency --dependency all --build-type Release --platform windows --arch x64
```

生成 CUDA provider 的 ONNX Runtime：

```powershell
python -m deploy.bootstrap_dependency --dependency onnxruntime --provider cuda --build-type Release --platform windows --arch x64
```

只验证 lock 文件：

```powershell
python scripts/verify_dependency.py --locks-only
```

清理单个依赖 package：

```powershell
python -m deploy.bootstrap_dependency --clean cpp_httplib
```

如果 state 中 fingerprint 命中且 package 内 required outputs 存在，二次运行会显示 `cache_hit` 并跳过下载/构建。缺少 required output 时会显示 `cache_miss_required_outputs_missing` 并重新生成。

## CMake 使用方式

Windows BAAS app、OCR server 和 ISA 的 CMake 入口通过 `cmake/BAASDependency.cmake` 加载 dependency helpers。依赖模块会先检查 `.baas/state.json` 和 package outputs，再创建 `BAAS::*` target。

示例：

```cmake
baas_require_opencv_target()
baas_require_onnxruntime_target()
baas_require_ffmpeg_target()
baas_require_lz4_target()
target_link_libraries(BAAS_APP BAAS::OpenCV BAAS::ONNXRuntime BAAS::FFmpeg BAAS::LZ4)
baas_copy_runtime_dependencies(BAAS_APP)
```

`baas_copy_runtime_dependencies(target)` 会复制已注册的 DLL 到目标输出目录。ADB、scrcpy-server、ascreencap、OCR/YOLO 模型仍按旧资源路径处理，等待后续资源迁移。

## 当前边界

本阶段不删除仓库内 `dll/`、`lib/Windows/`、`external/`、`resource/bin/` 或模型目录，也不清理 Git 历史。

Windows BAAS app 和 OCR server 已经可以通过 `.baas/dependency` 生成的 dependency 完成编译。ISA 当前可以完成依赖解析和大部分编译，但仍有 ISA 业务代码层面的 `BAAS::solve_procedure` 调用签名不匹配，未在本次依赖迁移中修改。
