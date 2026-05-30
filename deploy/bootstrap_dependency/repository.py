from __future__ import annotations

import os
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

from .constants import PLACEHOLDER_PREFIXES, STATE_SCHEMA
from .json_store import JsonStore, sha256_json


def is_placeholder(value: Any) -> bool:
    if value is None:
        return True
    if not isinstance(value, str):
        return False
    stripped = value.strip()
    if stripped == "":
        return True
    return stripped.startswith(PLACEHOLDER_PREFIXES) or stripped.endswith(">")


def provider_for_dependency(name: str, dep: dict[str, Any], ctx: Any, override: str | None) -> str:
    if override:
        return override
    providers = dep.get("provider_by_platform", {})
    return providers.get(ctx.platform_key) or providers.get("default") or dep.get("provider") or "unspecified"


def provider_payload(entry: dict[str, Any], provider: str) -> dict[str, Any]:
    providers = entry.get("providers", {})
    payload = providers.get(provider, {})
    return payload if isinstance(payload, dict) else {}


def provider_path(provider_data: dict[str, Any]) -> str:
    env_name = provider_data.get("path_env") or provider_data.get("path_var")
    if not env_name:
        return ""
    return os.environ.get(str(env_name), "")


@dataclass(frozen=True)
class RepoPaths:
    root: Path
    dependency_lock: Path
    resources_lock: Path

    @property
    def deps_lock(self) -> Path:
        return self.dependency_lock

    @classmethod
    def discover(cls, start: Path | None = None) -> "RepoPaths":
        starts: list[Path] = []
        if start is not None:
            starts.append(Path(start).resolve())
        starts.append(Path.cwd().resolve())
        starts.append(Path(__file__).resolve())

        seen: set[Path] = set()
        for item in starts:
            for candidate in cls._walk_up(item):
                if candidate in seen:
                    continue
                seen.add(candidate)
                dependency_lock = candidate / "deps.lock.json"
                resources_lock = candidate / "resources.lock.json"
                if dependency_lock.exists() and resources_lock.exists():
                    return cls(candidate, dependency_lock, resources_lock)

        raise FileNotFoundError("cannot discover BAAS_Cpp repository root; deps.lock.json and resources.lock.json were not found")

    @staticmethod
    def _walk_up(path: Path) -> list[Path]:
        start = path.parent if path.is_file() else path
        return [start, *start.parents]


class PathResolver:
    def __init__(self, expander: Any) -> None:
        self.expander = expander

    def resolve(
        self,
        lock: dict[str, Any],
        entry: dict[str, Any],
        ctx: Any,
        name: str,
        version: str,
        provider: str,
        resource: bool,
    ) -> dict[str, Path]:
        defaults = lock.get("paths", {})
        overrides = entry.get("paths", {})
        if not isinstance(defaults, dict):
            defaults = {}
        if not isinstance(overrides, dict):
            overrides = {}

        if resource:
            fallback = {
                "downloads": str(ctx.downloads_root),
                "package": str(ctx.assets_root / name / version / provider),
            }
        else:
            base = ctx.deps_root / name / version / ctx.variant / provider
            fallback = {
                "downloads": str(ctx.downloads_root),
                "source": str(ctx.build_root / name / version / ctx.variant / provider / "src"),
                "build": str(ctx.build_root / name / version / ctx.variant / provider / "build"),
                "package": str(base),
            }

        merged = {**fallback, **defaults, **overrides}
        return {
            key: Path(self.expander.expand(str(value), ctx, name, version, provider)).resolve()
            for key, value in merged.items()
        }


class StateStore:
    def __init__(self, json_store: JsonStore | None = None) -> None:
        self.json_store = json_store or JsonStore()

    def empty(self) -> dict[str, Any]:
        return {"schema": STATE_SCHEMA, "dependencies": {}, "resources": {}}

    def load(self, ctx: Any) -> dict[str, Any]:
        if not ctx.state_file.exists():
            return self.empty()
        state = self.json_store.load(ctx.state_file)
        if state.get("schema") != STATE_SCHEMA:
            raise ValueError(f"unsupported BAAS state schema in {ctx.state_file}")
        state.setdefault("dependencies", {})
        state.setdefault("resources", {})
        return state

    def record(self, state: dict[str, Any], kind: str, name: str, version: str, provider: str, variant: str) -> dict[str, Any] | None:
        root = "dependencies" if kind == "dependency" else "resources"
        record = state.get(root, {}).get(name, {}).get(version, {}).get(provider, {}).get(variant)
        return record if isinstance(record, dict) else None

    def set_record(self, state: dict[str, Any], entry: Any, ctx: Any, provider_data: dict[str, Any], cmake_options: dict[str, Any], cmake_config: str) -> None:
        data = entry.to_dict() if hasattr(entry, "to_dict") else dict(entry)
        root = "dependencies" if data["kind"] == "dependency" else "resources"
        name = data["name"]
        version = data["version"]
        provider = data["provider"]
        variant = data["variant"]
        state.setdefault(root, {}).setdefault(name, {}).setdefault(version, {}).setdefault(provider, {})[variant] = {
            "schema": STATE_SCHEMA,
            "name": name,
            "version": version,
            "variant": variant,
            "provider": provider,
            "fingerprint": data["fingerprint"],
            "source_sha256": provider_data.get("sha256", ""),
            "cmake_options_hash": sha256_json(cmake_options),
            "compiler_id": ctx.compiler_id,
            "compiler_version": ctx.compiler_version,
            "config": cmake_config,
            "created_at": datetime.now(timezone.utc).isoformat(),
            "package": data["package"],
            "outputs": data["outputs"],
        }

    def save(self, ctx: Any, state: dict[str, Any]) -> None:
        self.json_store.write_atomic(ctx.state_file, state)
