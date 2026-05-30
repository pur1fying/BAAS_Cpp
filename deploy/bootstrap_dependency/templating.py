from __future__ import annotations

from typing import Any


class TemplateExpander:
    def cmake_config_name(self, ctx: Any) -> str:
        return "Debug" if ctx.config == "debug" else "Release"

    def template_vars(self, ctx: Any, name: str, version: str, provider: str) -> dict[str, str]:
        return {
            "BAAS_LOCAL_ROOT": str(ctx.local_root),
            "BAAS_DEPENDENCY_ROOT": str(ctx.deps_root),
            "BAAS_ASSETS_ROOT": str(ctx.assets_root),
            "BAAS_DOWNLOADS_ROOT": str(ctx.downloads_root),
            "BAAS_DEPS_BUILD_ROOT": str(ctx.build_root),
            "name": name,
            "variant": ctx.variant,
            "config": ctx.config,
            "cmake_config": self.cmake_config_name(ctx),
            "platform": ctx.platform_key,
            "arch": ctx.arch_key,
            "android_abi": ctx.android_abi or "${android_abi}",
            "version": version,
            "provider": provider,
        }

    def expand(self, value: str, ctx: Any, name: str, version: str, provider: str) -> str:
        expanded = value or ""
        for key, replacement in self.template_vars(ctx, name, version, provider).items():
            expanded = expanded.replace("${" + key + "}", replacement)
        return expanded
