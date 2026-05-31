from __future__ import annotations

import argparse
import os
import platform
from dataclasses import dataclass
from pathlib import Path

from .repository import RepoPaths


def normalized_platform(value: str | None = None) -> str:
    raw = (value or platform.system() or "linux").lower()
    if raw in {"windows", "win32", "cygwin"}:
        return "windows"
    if raw in {"darwin", "mac", "macos"}:
        return "macos"
    if raw == "android":
        return "android"
    return "linux"


def normalized_arch(value: str | None = None) -> str:
    raw = (value or platform.machine() or "unknown").lower()
    if raw in {"amd64", "x86_64"}:
        return "x64"
    if raw in {"aarch64", "arm64"}:
        return "arm64"
    return raw


def normalize_token(value: str | None, default: str) -> str:
    token = (value or default).strip()
    return token.lower() if token else default.lower()


def default_compiler_id(platform_key: str) -> str:
    if platform_key == "windows":
        return "msvc"
    return "unknown"


@dataclass(frozen=True)
class BootstrapContext:
    platform_key: str
    arch_key: str
    compiler_id: str
    compiler_version: str
    config: str
    toolchain_file: str
    android_abi: str
    variant: str
    workspace_root: Path
    deps_root: Path
    assets_root: Path
    downloads_root: Path
    build_root: Path
    state_file: Path
    cmake_manifest: str

    @classmethod
    def from_args(cls, args: argparse.Namespace, repo_paths: RepoPaths) -> "BootstrapContext":
        platform_key = normalized_platform(getattr(args, "platform", None))
        android_abi = getattr(args, "android_abi", None) or ""
        arch_key = android_abi or normalized_arch(getattr(args, "arch", None))
        if android_abi:
            platform_key = "android"

        compiler_id = normalize_token(
            getattr(args, "compiler_id", None) or os.environ.get("BAAS_COMPILER_ID"),
            default_compiler_id(platform_key),
        )
        compiler_version = getattr(args, "compiler_version", None) or os.environ.get("BAAS_COMPILER_VERSION", "unknown")
        config = normalize_token(getattr(args, "build_type", "Release"), "release")
        toolchain_value = getattr(args, "toolchain_file", None)
        toolchain_file = str(Path(toolchain_value).resolve()) if toolchain_value else ""
        variant = f"{platform_key}-{arch_key}-{compiler_id}-{config}"

        workspace_root = Path(os.environ.get("BAAS_WORKSPACE_ROOT", repo_paths.root / ".baas")).resolve()
        dependency_root = os.environ.get("BAAS_DEPENDENCY_ROOT")
        deps_root = Path(dependency_root or (workspace_root / "dependency")).resolve()
        assets_root = Path(os.environ.get("BAAS_ASSETS_ROOT", workspace_root / "assets")).resolve()
        downloads_root = Path(os.environ.get("BAAS_DOWNLOADS_ROOT", workspace_root / "downloads")).resolve()
        build_root = Path(os.environ.get("BAAS_DEPS_BUILD_ROOT", workspace_root / "build")).resolve()
        state_file = Path(os.environ.get("BAAS_STATE_FILE", workspace_root / "state.json")).resolve()
        cmake_manifest_value = getattr(args, "cmake_manifest", None) or ""
        cmake_manifest = str(Path(cmake_manifest_value).resolve()) if cmake_manifest_value else ""

        return cls(
            platform_key=platform_key,
            arch_key=arch_key,
            compiler_id=compiler_id,
            compiler_version=compiler_version,
            config=config,
            toolchain_file=toolchain_file,
            android_abi=android_abi,
            variant=variant,
            workspace_root=workspace_root,
            deps_root=deps_root,
            assets_root=assets_root,
            downloads_root=downloads_root,
            build_root=build_root,
            state_file=state_file,
            cmake_manifest=cmake_manifest,
        )
