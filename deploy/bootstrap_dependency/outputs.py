from __future__ import annotations

from pathlib import Path
from typing import Any


class OutputResolver:
    def __init__(self, expander: Any) -> None:
        self.expander = expander

    def for_dependency(self, entry: dict[str, Any], ctx: Any) -> dict[str, list[str]]:
        outputs = entry.get("outputs", {})
        if not isinstance(outputs, dict):
            return {"include": [], "runtime": [], "link": []}

        selected: Any = outputs.get(ctx.platform_key)
        if isinstance(selected, dict) and ctx.config in selected:
            selected = selected[ctx.config]
        if selected is None:
            selected = outputs.get("default", outputs)

        result: dict[str, list[str]] = {"include": [], "runtime": [], "link": []}
        if isinstance(selected, dict):
            for kind, values in selected.items():
                if isinstance(values, list):
                    result[str(kind)] = [str(item) for item in values]
        return result

    def for_resource(self, resource: dict[str, Any]) -> dict[str, list[str]]:
        outputs = resource.get("outputs", {})
        result: dict[str, list[str]] = {"include": [], "runtime": [], "link": []}
        if isinstance(outputs, dict):
            for kind, values in outputs.items():
                if isinstance(values, list):
                    result[str(kind)] = [str(item) for item in values]
        return result

    def required_kinds(self, entry: dict[str, Any], outputs: dict[str, list[str]]) -> list[str]:
        validation = entry.get("validation", {})
        required = validation.get("required_outputs", []) if isinstance(validation, dict) else []
        if not required:
            return [kind for kind, values in outputs.items() if values]
        return [str(kind) for kind in required]

    def expand(self, outputs: dict[str, list[str]], ctx: Any, name: str, version: str, provider: str) -> dict[str, list[str]]:
        return {
            kind: [self.expander.expand(value, ctx, name, version, provider) for value in values]
            for kind, values in outputs.items()
        }


class OutputValidator:
    def path_exists_with_glob(self, package_dir: Path, relative_pattern: str) -> bool:
        pattern = relative_pattern.replace("\\", "/")
        if any(char in pattern for char in "*?[]"):
            return any(package_dir.glob(pattern))
        return (package_dir / relative_pattern).exists()

    def check(self, package_dir: Path, outputs: dict[str, list[str]], required: list[str]) -> tuple[bool, list[str]]:
        missing: list[str] = []
        for kind in required:
            values = outputs.get(kind, [])
            if not values:
                continue
            for rel in values:
                if not self.path_exists_with_glob(package_dir, rel):
                    missing.append(f"{kind}:{rel}")
        return not missing, missing
