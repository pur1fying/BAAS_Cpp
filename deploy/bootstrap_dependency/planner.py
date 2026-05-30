from __future__ import annotations

import argparse
from dataclasses import dataclass, field
from typing import Any

from .json_store import sha256_json
from .locks import LockRepository, locked_version
from .outputs import OutputResolver, OutputValidator
from .repository import PathResolver, StateStore, is_placeholder, provider_for_dependency, provider_path, provider_payload


@dataclass(frozen=True)
class PlanEntry:
    kind: str
    name: str
    version: str
    provider: str
    sha_locked: bool
    variant: str
    state_file: str
    downloads: str
    package: str
    fingerprint: str
    fingerprint_payload: dict[str, Any]
    required_outputs: list[str]
    outputs: dict[str, list[str]]
    missing_outputs: list[str]
    status: str
    provider_type: str = ""
    provider_sha256: str = ""
    provider_path: str = ""
    sha256: str = ""
    source: str = ""
    build: str = ""

    def to_dict(self) -> dict[str, Any]:
        result = {
            "kind": self.kind,
            "name": self.name,
            "version": self.version,
            "provider": self.provider,
            "sha_locked": self.sha_locked,
            "variant": self.variant,
            "state_file": self.state_file,
            "downloads": self.downloads,
            "package": self.package,
            "fingerprint": self.fingerprint,
            "fingerprint_payload": self.fingerprint_payload,
            "required_outputs": self.required_outputs,
            "outputs": self.outputs,
            "missing_outputs": self.missing_outputs,
            "status": self.status,
        }
        if self.kind == "dependency":
            result.update(
                {
                    "provider_type": self.provider_type,
                    "provider_sha256": self.provider_sha256,
                    "provider_path": self.provider_path,
                    "source": self.source,
                    "build": self.build,
                }
            )
        else:
            result["sha256"] = self.sha256
        return result

    def __getitem__(self, key: str) -> Any:
        return self.to_dict()[key]

    def get(self, key: str, default: Any = None) -> Any:
        return self.to_dict().get(key, default)


class DependencyPlanner:
    def __init__(
        self,
        output_resolver: OutputResolver,
        output_validator: OutputValidator,
        path_resolver: PathResolver,
        state_store: StateStore,
    ) -> None:
        self.output_resolver = output_resolver
        self.output_validator = output_validator
        self.path_resolver = path_resolver
        self.state_store = state_store

    def plan(self, lock: dict[str, Any], state: dict[str, Any], name: str, dep: dict[str, Any], ctx: Any, provider_override: str | None) -> PlanEntry:
        version = locked_version("dependency", name, dep)
        provider = provider_for_dependency(name, dep, ctx, provider_override)
        provider_data = provider_payload(dep, provider)
        provider_sha = str(provider_data.get("sha256", ""))
        provider_type = str(provider_data.get("type", ""))
        paths = self.path_resolver.resolve(lock, dep, ctx, name, version, provider, resource=False)
        outputs = self.output_resolver.expand(self.output_resolver.for_dependency(dep, ctx), ctx, name, version, provider)
        required = self.output_resolver.required_kinds(dep, outputs)
        fingerprint_payload = {
            "name": name,
            "version": version,
            "provider": provider,
            "provider_type": provider_type,
            "provider_sha256": provider_sha,
            "provider_path": provider_path(provider_data),
            "platform": ctx.platform_key,
            "arch": ctx.arch_key,
            "compiler": {
                "id": ctx.compiler_id,
                "version": ctx.compiler_version,
            },
            "config": ctx.config,
            "toolchain_file": ctx.toolchain_file,
            "build": dep.get("build", {}),
            "status": dep.get("status", ""),
        }
        if "staging" in dep:
            fingerprint_payload["staging"] = dep.get("staging", {})
        expected_fingerprint = sha256_json(fingerprint_payload)
        record = self.state_store.record(state, "dependency", name, version, provider, ctx.variant)
        outputs_ok, missing_outputs = self.output_validator.check(paths["package"], outputs, required)
        sha_locked = provider_type == "local_source" or provider_type == "system" or provider == "none" or not is_placeholder(provider_sha)
        status = plan_status(record, expected_fingerprint, outputs_ok, sha_locked)
        if not outputs_ok and status == "cache_miss":
            status = "cache_miss_required_outputs_missing"
        return PlanEntry(
            kind="dependency",
            name=name,
            version=version,
            provider=provider,
            provider_type=provider_type,
            provider_sha256=provider_sha,
            provider_path=provider_path(provider_data),
            sha_locked=sha_locked,
            variant=ctx.variant,
            state_file=str(ctx.state_file),
            downloads=str(paths.get("downloads", ctx.downloads_root)),
            source=str(paths.get("source", "")),
            build=str(paths.get("build", "")),
            package=str(paths["package"]),
            fingerprint=expected_fingerprint,
            fingerprint_payload=fingerprint_payload,
            required_outputs=required,
            outputs=outputs,
            missing_outputs=missing_outputs,
            status=status,
        )


class ResourcePlanner:
    def __init__(
        self,
        output_resolver: OutputResolver,
        output_validator: OutputValidator,
        path_resolver: PathResolver,
        state_store: StateStore,
    ) -> None:
        self.output_resolver = output_resolver
        self.output_validator = output_validator
        self.path_resolver = path_resolver
        self.state_store = state_store

    def plan(self, lock: dict[str, Any], state: dict[str, Any], name: str, resource: dict[str, Any], ctx: Any) -> PlanEntry:
        version = locked_version("resource", name, resource)
        provider = str(resource.get("provider", "unspecified"))
        sha = str(resource.get("sha256", ""))
        paths = self.path_resolver.resolve(lock, resource, ctx, name, version, provider, resource=True)
        outputs = self.output_resolver.expand(self.output_resolver.for_resource(resource), ctx, name, version, provider)
        required = self.output_resolver.required_kinds(resource, outputs)
        fingerprint_payload = {
            "name": name,
            "version": version,
            "provider": provider,
            "sha256": sha,
            "platform": ctx.platform_key,
            "arch": ctx.arch_key,
            "android_abi": ctx.android_abi,
        }
        expected_fingerprint = sha256_json(fingerprint_payload)
        record = self.state_store.record(state, "resource", name, version, provider, ctx.variant)
        outputs_ok, missing_outputs = self.output_validator.check(paths["package"], outputs, required)
        sha_locked = not is_placeholder(sha)
        status = plan_status(record, expected_fingerprint, outputs_ok, sha_locked)
        if not outputs_ok and status == "cache_miss":
            status = "cache_miss_required_outputs_missing"
        return PlanEntry(
            kind="resource",
            name=name,
            version=version,
            provider=provider,
            sha256=sha,
            sha_locked=sha_locked,
            variant=ctx.variant,
            state_file=str(ctx.state_file),
            downloads=str(paths.get("downloads", ctx.downloads_root)),
            package=str(paths["package"]),
            fingerprint=expected_fingerprint,
            fingerprint_payload=fingerprint_payload,
            required_outputs=required,
            outputs=outputs,
            missing_outputs=missing_outputs,
            status=status,
        )


def plan_status(record: dict[str, Any] | None, expected_fingerprint: str, outputs_ok: bool, sha_locked: bool) -> str:
    cache_hit = bool(record and record.get("fingerprint") == expected_fingerprint and outputs_ok)
    if cache_hit:
        return "cache_hit"
    if not sha_locked:
        return "blocked_until_sha256_locked"
    if not record:
        return "cache_miss_state_missing"
    if record.get("fingerprint") != expected_fingerprint:
        return "cache_miss_fingerprint_changed"
    if not outputs_ok:
        return "cache_miss_required_outputs_missing"
    return "cache_miss"


class PlanBuilder:
    def __init__(
        self,
        lock_repo: LockRepository,
        state_store: StateStore,
        dependency_planner: DependencyPlanner,
        resource_planner: ResourcePlanner,
    ) -> None:
        self.lock_repo = lock_repo
        self.state_store = state_store
        self.dependency_planner = dependency_planner
        self.resource_planner = resource_planner

    def build(self, args: argparse.Namespace, ctx: Any) -> list[PlanEntry]:
        deps_lock = self.lock_repo.deps_lock
        resources_lock = self.lock_repo.resources_lock
        state = self.state_store.load(ctx)
        entries: list[PlanEntry] = []

        dependency_selector = getattr(args, "dependency", None)
        include_all_dependencies = args.all or bool(dependency_selector)
        include_all_resources = args.all or bool(args.resources)
        if not include_all_dependencies and not include_all_resources:
            include_all_dependencies = True
            include_all_resources = True

        if include_all_dependencies:
            dependencies = self.lock_repo.select_dependencies(dependency_selector, args.all or not dependency_selector)
            for name, dep in dependencies.items():
                entries.append(self.dependency_planner.plan(deps_lock, state, name, dep, ctx, args.provider))

        if include_all_resources:
            resources = self.lock_repo.select_resources(args.resources, args.all or not args.resources)
            for name, resource in resources.items():
                entries.append(self.resource_planner.plan(resources_lock, state, name, resource, ctx))

        return entries
