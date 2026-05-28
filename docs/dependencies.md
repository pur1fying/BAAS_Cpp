# BAAS C++ dependency bootstrap

本文档描述依赖去仓库化第一阶段的本地目录和锁文件约定。本阶段只建立框架，不迁移任何依赖，不删除仓库内旧文件。

## 本地目录

默认本地工作目录为 `${CMAKE_SOURCE_DIR}/.baas`，可通过 CMake 或环境变量覆盖。

- `.baas/deps`: 已整理好的第三方依赖 package，供 BAAS 链接和运行时复制使用。
- `.baas/assets`: 运行资源和工具，例如 OCR 模型、YOLO 模型、platform-tools、scrcpy-server、ascreencap。
- `.baas/downloads`: 下载缓存。
- `.baas/build`: 第三方依赖源码解压和构建目录。
- `.baas/logs`: bootstrap 和第三方构建日志。

用户侧文档使用 `package_dir` 描述依赖产物目录，不使用 `install_dir`。如果第三方 CMake 项目内部需要 `cmake --install` 来整理产物，只作为实现细节处理。

## CMake 变量

- `BAAS_LOCAL_ROOT`: 默认 `${CMAKE_SOURCE_DIR}/.baas`
- `BAAS_DEPS_ROOT`: 默认 `${BAAS_LOCAL_ROOT}/deps`
- `BAAS_ASSETS_ROOT`: 默认 `${BAAS_LOCAL_ROOT}/assets`
- `BAAS_DOWNLOADS_ROOT`: 默认 `${BAAS_LOCAL_ROOT}/downloads`
- `BAAS_DEPS_BUILD_ROOT`: 默认 `${BAAS_LOCAL_ROOT}/build`
- `BAAS_DEPS_MODE`: `auto`, `download`, `system`, `source`
- `BAAS_ALLOW_DOWNLOADS`: 默认 `ON`
- `BAAS_USE_SYSTEM_ADB`: 默认 `ON`
- `BAAS_OPENCV_PROVIDER`: 默认 `source`
- `BAAS_FFMPEG_PROVIDER`: 默认 `prebuilt`
- `BAAS_ONNXRUNTIME_PROVIDER`: 默认 `cpu`
- `BAAS_RESOURCE_PROVIDER`: 默认 `download`

## Lock 文件

`deps.lock.json` 记录编译和链接依赖。`resources.lock.json` 记录运行时资源。正式启用下载或源码构建前，所有 release archive 和二进制资产必须填入真实 SHA256，不能使用 `TODO_LOCK_SHA256`。

每个依赖后续迁移时必须写入 marker 文件：

```json
{
  "schema": 1,
  "name": "opencv",
  "version": "4.9.0",
  "variant": "Windows-x64-MSVC-Release",
  "provider": "source",
  "fingerprint": "sha256:...",
  "source_sha256": "...",
  "cmake_options_hash": "...",
  "compiler_id": "MSVC",
  "compiler_version": "...",
  "config": "Release",
  "created_at": "...",
  "package_dir": "...",
  "outputs": {
    "include": [],
    "link": [],
    "runtime": []
  }
}
```

bootstrap 逻辑必须先检查 marker、fingerprint 和 required outputs。全部命中时输出 cache hit，并跳过下载和构建。

## 当前阶段边界

当前框架不会接管现有 `dll/`、`lib/Windows/`、`external/`、`resource/bin/`。后续每个依赖都必须单独提出待审核修改，验证通过后再逐步移除旧路径 fallback。
