#!/usr/bin/env python3
"""Verify BAAS dependency and resource lock files.

Stage 1 validates schema shape and reports placeholders. It does not download,
build, or mutate dependency packages.
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
DEPS_LOCK = ROOT / "deps.lock.json"
RESOURCES_LOCK = ROOT / "resources.lock.json"
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from deploy.bootstrap_dependency.logger import DependencyLogger


def load_json(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as fh:
        return json.load(fh)


def is_placeholder(value: Any) -> bool:
    if value is None:
        return True
    if not isinstance(value, str):
        return False
    stripped = value.strip()
    return stripped.startswith("TODO") or stripped.startswith("<") or stripped.endswith(">")


def walk_placeholders(value: Any, path: str, out: list[str]) -> None:
    if isinstance(value, dict):
        for key, child in value.items():
            walk_placeholders(child, f"{path}.{key}" if path else str(key), out)
    elif isinstance(value, list):
        for index, child in enumerate(value):
            walk_placeholders(child, f"{path}[{index}]", out)
    elif is_placeholder(value):
        out.append(path)


def require(condition: bool, message: str, errors: list[str]) -> None:
    if not condition:
        errors.append(message)


def validate_deps(lock: dict[str, Any]) -> list[str]:
    errors: list[str] = []
    require(lock.get("schema") == 1, "deps.lock.json schema must be 1", errors)
    require(lock.get("lock_type") == "baas-cpp-dependencies", "deps.lock.json lock_type is invalid", errors)
    paths = lock.get("paths")
    require(isinstance(paths, dict), "deps.lock.json paths must be an object", errors)
    if isinstance(paths, dict):
        for key in ("downloads", "source", "build", "package"):
            require(key in paths, f"deps.lock.json paths missing {key}", errors)
        require("install" not in paths and "install_dir" not in paths, "deps.lock.json paths must not use install/install_dir", errors)
    dependencies = lock.get("dependencies")
    require(isinstance(dependencies, dict) and bool(dependencies), "deps.lock.json dependencies must be a non-empty object", errors)
    if isinstance(dependencies, dict):
        for name, dep in dependencies.items():
            require(isinstance(dep, dict), f"dependency {name} must be an object", errors)
            if not isinstance(dep, dict):
                continue
            version = dep.get("version")
            require(
                isinstance(version, str) and bool(version.strip()),
                f"dependency {name} version must be a non-empty string in this fixed-version stage",
                errors,
            )
            require("provider_by_platform" in dep, f"dependency {name} missing provider_by_platform", errors)
            require("outputs" in dep, f"dependency {name} missing outputs", errors)
            require("validation" in dep, f"dependency {name} missing validation", errors)
            require("local_layout" not in dep, f"dependency {name} must not use legacy local_layout", errors)
            require("marker" not in dep, f"dependency {name} must not use per-package marker", errors)
            dep_paths = dep.get("paths", {})
            if isinstance(dep_paths, dict):
                require("install" not in dep_paths and "install_dir" not in dep_paths, f"dependency {name} paths must not use install/install_dir", errors)
    return errors


def validate_resources(lock: dict[str, Any]) -> list[str]:
    errors: list[str] = []
    require(lock.get("schema") == 1, "resources.lock.json schema must be 1", errors)
    require(lock.get("lock_type") == "baas-cpp-resources", "resources.lock.json lock_type is invalid", errors)
    paths = lock.get("paths")
    require(isinstance(paths, dict), "resources.lock.json paths must be an object", errors)
    if isinstance(paths, dict):
        for key in ("downloads", "package"):
            require(key in paths, f"resources.lock.json paths missing {key}", errors)
        require("install" not in paths and "install_dir" not in paths, "resources.lock.json paths must not use install/install_dir", errors)
    resources = lock.get("resources")
    require(isinstance(resources, dict) and bool(resources), "resources.lock.json resources must be a non-empty object", errors)
    if isinstance(resources, dict):
        for name, resource in resources.items():
            require(isinstance(resource, dict), f"resource {name} must be an object", errors)
            if not isinstance(resource, dict):
                continue
            version = resource.get("version")
            require(
                isinstance(version, str) and bool(version.strip()),
                f"resource {name} version must be a non-empty string in this fixed-version stage",
                errors,
            )
            require("provider" in resource, f"resource {name} missing provider", errors)
            require("outputs" in resource, f"resource {name} missing outputs", errors)
            require("validation" in resource, f"resource {name} missing validation", errors)
            require("package_dir" not in resource, f"resource {name} must not use legacy package_dir", errors)
            require("marker" not in resource, f"resource {name} must not use per-package marker", errors)
            require("install" not in resource and "install_dir" not in resource, f"resource {name} must not use install/install_dir", errors)
    return errors


def print_placeholders(title: str, placeholders: list[str], limit: int, logger: DependencyLogger) -> None:
    logger.info(f"{title}: {len(placeholders)} placeholder value(s)")
    for path in placeholders[:limit]:
        logger.info(f"  - {path}")
    if len(placeholders) > limit:
        logger.info(f"  ... {len(placeholders) - limit} more")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Verify BAAS dependency/resource lock files.")
    parser.add_argument("--locks-only", action="store_true", help="Validate lock files and report TODO placeholders.")
    parser.add_argument("--placeholder-limit", type=int, default=30, help="Maximum placeholder paths to print per lock file.")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    logger = DependencyLogger.create(ROOT)
    try:
        deps_lock = load_json(DEPS_LOCK)
        resources_lock = load_json(RESOURCES_LOCK)
    except (OSError, json.JSONDecodeError) as exc:
        logger.error(f"error: {exc}")
        return 2

    errors = validate_deps(deps_lock) + validate_resources(resources_lock)
    deps_placeholders: list[str] = []
    resources_placeholders: list[str] = []
    walk_placeholders(deps_lock, "deps", deps_placeholders)
    walk_placeholders(resources_lock, "resources", resources_placeholders)

    logger.info("BAAS dependency lock verification")
    logger.info(f"  dependency lock: {DEPS_LOCK}")
    logger.info(f"  resources lock: {RESOURCES_LOCK}")
    logger.info(f"  log_dir: {logger.log_dir}")
    print_placeholders("  deps.lock.json", deps_placeholders, args.placeholder_limit, logger)
    print_placeholders("  resources.lock.json", resources_placeholders, args.placeholder_limit, logger)

    if errors:
        logger.info("schema errors:")
        for error in errors:
            logger.info(f"  - {error}")
        return 1

    logger.info("schema status: ok")
    logger.info("note: placeholder SHA256/URL values are accepted in stage 1, but download/build is blocked until locked.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
