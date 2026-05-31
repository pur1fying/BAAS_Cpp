from __future__ import annotations

from typing import Any

from .json_store import JsonStore
from .repository import RepoPaths


def locked_version(entry_kind: str, name: str, entry: dict[str, Any]) -> str:
    value = entry.get("version")
    if not isinstance(value, str) or not value.strip():
        raise ValueError(
            f"{entry_kind} {name} version must be a non-empty string in this stage; "
            "version range/recommended/resolved objects will be added only after "
            "the fixed-version dependency flow is validated."
        )
    return value.strip()


def select_entries(all_entries: dict[str, Any], selector: str | None, include_all: bool) -> dict[str, Any]:
    if include_all or not selector or selector == "all":
        return dict(all_entries)
    requested = list(dict.fromkeys(item.strip() for item in selector.split(",") if item.strip()))
    missing = [name for name in requested if name not in all_entries]
    if missing:
        raise KeyError("unknown entries: " + ", ".join(missing))
    return {name: all_entries[name] for name in requested}


class LockRepository:
    def __init__(self, repo_paths: RepoPaths, json_store: JsonStore | None = None) -> None:
        self.repo_paths = repo_paths
        self.json_store = json_store or JsonStore()
        self._deps_lock: dict[str, Any] | None = None
        self._resources_lock: dict[str, Any] | None = None

    @property
    def deps_lock(self) -> dict[str, Any]:
        if self._deps_lock is None:
            self._deps_lock = self.json_store.load(self.repo_paths.deps_lock)
        return self._deps_lock

    @property
    def resources_lock(self) -> dict[str, Any]:
        if self._resources_lock is None:
            self._resources_lock = self.json_store.load(self.repo_paths.resources_lock)
        return self._resources_lock

    def select_dependencies(self, selector: str | None, include_all: bool) -> dict[str, Any]:
        return select_entries(self.deps_lock.get("dependencies", {}), selector, include_all)

    def select_resources(self, selector: str | None, include_all: bool) -> dict[str, Any]:
        return select_entries(self.resources_lock.get("resources", {}), selector, include_all)
