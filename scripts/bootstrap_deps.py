#!/usr/bin/env python3
"""BAAS dependency bootstrap skeleton.

This first version plans and verifies local dependency packages. It does not
perform downloads or builds until individual dependencies are migrated and their
SHA256 values are locked.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import platform
import shutil
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
DEPS_LOCK = ROOT / "deps.lock.json"
RESOURCES_LOCK = ROOT / "resources.lock.json"

def load_json(path: Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as fh:
        return json.load(fh)

def host_platform() -> str:
    system = platform.system().lower()
    if system == "darwin":
        return "MacOS"
    if system.startswith("windows"):
        return "Windows"
    return "Linux"

def host_arch() -> str:
    machine = platform.machine().lower()
    if machine in {"amd64", "x86_64"}:
        return "x64"
    if machine in {"aarch64", "arm64"}:
        return "arm64"
    return machine

def variant(config: str, android_abi: str | None = None) -> str:
    compiler = os.environ.get("BAAS_COMPILER_ID", "Unknown")
    arch = android_abi or host_arch()
    platform_key = "Android" if android_abi else host_platform()
    return f"{platform_key}-{arch}-{compiler}-{config}"

def fingerprint(payload: dict[str, Any]) -> str:
    data = json.dumps(payload, sort_keys=True, separators=(",", ":")).encode("utf-8")
    return "sha256:" + hashlib.sha256(data).hexdigest()

def marker_valid(package_dir: Path, expected_fingerprint: str, outputs: list[str]) -> bool:
    marker = package_dir / ".baas-complete.json"
    if not marker.exists():
        return False
    try:
        data = load_json(marker)
    except (OSError, json.JSONDecodeError):
        return False
    if data.get("fingerprint") != expected_fingerprint:
        return False
    return all((package_dir / output).exists() for output in outputs)

def selected_dependencies(lock: dict[str, Any], names: str | None) -> dict[str, Any]:
    deps = lock.get("dependencies", {})
    if not names or names == "all":
        return deps
    requested = {name.strip() for name in names.split(",") if name.strip()}
    return {name: deps[name] for name in requested if name in deps}

def print_plan(args: argparse.Namespace) -> int:
    deps_lock = load_json(DEPS_LOCK)
    resources_lock = load_json(RESOURCES_LOCK)
    current_variant = variant(args.config, args.android_abi)
    print("BAAS bootstrap plan")
    print(f"  variant: {current_variant}")
    print(f"  deps lock: {DEPS_LOCK}")
    print(f"  resources lock: {RESOURCES_LOCK}")
    for name, dep in selected_dependencies(deps_lock, args.deps).items():
        providers = dep.get("provider_by_platform", {})
        provider = args.provider or providers.get(host_platform()) or providers.get("default") or dep.get("provider") or "unspecified"
        print(f"  dependency {name}: version={dep.get('version', dep.get('abi', 'unspecified'))} provider={provider}")
    resource_entries = resources_lock.get("resources", {})
    if args.resources and args.resources != "all":
        requested = {name.strip() for name in args.resources.split(",") if name.strip()}
        resource_entries = {name: resource_entries[name] for name in requested if name in resource_entries}
    for name, resource in resource_entries.items():
        print(f"  resource {name}: version={resource.get('version')} provider={resource.get('provider')}")
    return 0

def verify_only(args: argparse.Namespace) -> int:
    return print_plan(args)

def clean(args: argparse.Namespace) -> int:
    if not args.clean:
        return 0
    local_root = Path(os.environ.get("BAAS_LOCAL_ROOT", ROOT / ".baas"))
    targets = [item.strip() for item in args.clean.split(",") if item.strip()]
    for target in targets:
        for base in (local_root / "deps", local_root / "assets", local_root / "build"):
            path = base / target
            if path.exists():
                print(f"remove {path}")
                shutil.rmtree(path)
    return 0

def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Bootstrap BAAS dependencies")
    parser.add_argument("--all", action="store_true", help="Plan all dependencies and resources")
    parser.add_argument("--deps", help="Comma-separated dependency names, or all")
    parser.add_argument("--resources", help="Comma-separated resource names, or all")
    parser.add_argument("--provider", help="Override provider for the selected dependency")
    parser.add_argument("--config", default="Release", choices=("Debug", "Release"))
    parser.add_argument("--android-abi")
    parser.add_argument("--print-plan", action="store_true")
    parser.add_argument("--verify-only", action="store_true")
    parser.add_argument("--clean", help="Clean one or more dependency/resource package roots")
    return parser.parse_args()

def main() -> int:
    args = parse_args()
    if args.clean:
        return clean(args)
    if args.verify_only:
        return verify_only(args)
    if args.print_plan or args.all or args.deps or args.resources:
        return print_plan(args)
    return print_plan(args)

if __name__ == "__main__":
    sys.exit(main())
