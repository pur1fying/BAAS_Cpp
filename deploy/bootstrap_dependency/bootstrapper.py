from __future__ import annotations

import argparse
import shutil
import subprocess
from concurrent.futures import Future, ThreadPoolExecutor
from pathlib import Path
from typing import Any

from .cmake import CMakeBuilder
from .concurrency import DownloadScheduler, DownloadedArtifact, ResourcePlan, WeightedSemaphore
from .constants import BOOTSTRAP_DEPENDENCIES
from .logger import DependencyLogger
from .locks import LockRepository
from .outputs import OutputValidator
from .planner import PlanBuilder, PlanEntry
from .process_lock import ProcessDirectoryLock
from .repository import StateStore, provider_payload
from .reporter import Reporter
from .source_package import SourcePackageResolver
from .templating import TemplateExpander
from .utils import SafeFilesystem


def cmake_options_for_config(build: dict[str, Any], config: str) -> dict[str, Any]:
    options = build.get("cmake_options", {})
    if not isinstance(options, dict):
        options = {}
    merged = dict(options)

    config_options = build.get("cmake_options_by_config", {})
    if isinstance(config_options, dict):
        selected = config_options.get(config.lower(), {})
        if isinstance(selected, dict):
            merged.update(selected)
    return merged


class BootstrapService:
    def __init__(
        self,
        plan_builder: PlanBuilder,
        lock_repo: LockRepository,
        source_resolver: SourcePackageResolver,
        cmake_builder: CMakeBuilder,
        state_store: StateStore,
        output_validator: OutputValidator,
        filesystem: SafeFilesystem,
        reporter: Reporter,
        expander: TemplateExpander,
        logger: DependencyLogger,
        resource_plan: ResourcePlan,
    ) -> None:
        self.plan_builder = plan_builder
        self.lock_repo = lock_repo
        self.source_resolver = source_resolver
        self.cmake_builder = cmake_builder
        self.state_store = state_store
        self.output_validator = output_validator
        self.filesystem = filesystem
        self.reporter = reporter
        self.expander = expander
        self.logger = logger
        self.resource_plan = resource_plan
        self.cpu_semaphore = WeightedSemaphore(resource_plan.cpu_budget)

    def commit_state(self, entry: PlanEntry, ctx: Any, provider_data: dict[str, Any], cmake_options: dict[str, Any]) -> None:
        self.state_store.update_record_locked(
            ctx,
            entry,
            provider_data,
            cmake_options,
            self.expander.cmake_config_name(ctx),
            self.logger,
        )
        self.logger.info(f"wrote state: {ctx.state_file}")

    def materialize_lock(self, ctx: Any) -> ProcessDirectoryLock:
        return ProcessDirectoryLock(
            ctx.workspace_root / "bootstrap-materialize.lock",
            owner={"path": str(ctx.workspace_root), "phase": "download/extract"},
            env_prefix="BAAS_DEPENDENCY_MATERIALIZE_LOCK",
            wait_message="bootstrap wait: dependency download/extract is running in another bootstrap process",
            stale_message="bootstrap stale materialize lock removed",
            timeout_message="timed out waiting for bootstrap materialize lock",
            logger=self.logger,
        )

    def validate_entries(self, entries: list[PlanEntry], args: argparse.Namespace) -> int:
        errors = 0
        for entry in entries:
            if entry.status == "cache_hit":
                continue
            if entry.kind != "dependency" or entry.name not in BOOTSTRAP_DEPENDENCIES:
                self.reporter.mark_skipped(entry, "bootstrap implementation is not enabled")
                self.logger.info(f"    skip: bootstrap implementation is not enabled for {entry.kind} {entry.name} yet.")
                continue
            if not entry.sha_locked:
                self.reporter.mark_failed(entry, "locked SHA256 is required before bootstrapping")
                self.logger.info(f"    error: locked SHA256 is required before bootstrapping {entry.name}.")
                errors += 1
                continue
        return errors

    def report_cmake_phase(self, entry: PlanEntry, phase: str, status: str, log_file: Path) -> None:
        self.reporter.set_log(entry, log_file)
        self.reporter.phase(entry, phase, status)
        if status == "running":
            self.logger.info(f"    {phase.lower()} start: {entry.name}; log: {log_file}")
        elif status == "success":
            self.logger.info(f"    {phase.lower()} complete: {entry.name}")

    def bootstrap_source_dependency(
        self,
        name: str,
        dep: dict[str, Any],
        entry: PlanEntry,
        ctx: Any,
        args: argparse.Namespace,
    ) -> None:
        provider = entry.provider
        provider_data = provider_payload(dep, provider)
        source_dir = Path(entry.source)
        build_dir = Path(entry.build)
        package_dir = Path(entry.package)

        build = dep.get("build", {})
        if not isinstance(build, dict):
            build = {}
        source_subdir = str(build.get("source_subdir", "")).strip()
        if entry.provider_type == "local_source":
            configure_source_dir = self.source_resolver.local_source_dir(name, provider_data)
        else:
            configure_source_dir = source_dir

        if source_subdir:
            configure_source_dir = configure_source_dir / source_subdir
        if not (configure_source_dir / "CMakeLists.txt").exists():
            raise RuntimeError(f"dependency {name} CMake source directory does not contain CMakeLists.txt: {configure_source_dir}")

        build_dir.mkdir(parents=True, exist_ok=True)
        package_dir.mkdir(parents=True, exist_ok=True)
        if build_dir.exists() and any(build_dir.iterdir()):
            self.logger.info(f"    reuse build: {build_dir}")

        cmake_options = cmake_options_for_config(build, ctx.config)

        self.cmake_builder.build_source_package(
            name=name,
            version=entry.version,
            provider=provider,
            ctx=ctx,
            configure_source_dir=configure_source_dir,
            build_dir=build_dir,
            package_dir=package_dir,
            cmake_options=cmake_options,
            jobs=self.resource_plan.cmake_jobs_per_dependency,
            timeout_seconds=args.command_timeout,
            cpu_semaphore=self.cpu_semaphore,
            cpu_tokens=self.resource_plan.cmake_jobs_per_dependency,
            phase_callback=lambda phase, status, log_file: self.report_cmake_phase(entry, phase, status, log_file),
        )

        outputs_ok, missing_outputs = self.output_validator.check(package_dir, entry.outputs, entry.required_outputs)
        if not outputs_ok:
            raise RuntimeError(f"dependency {name} built but required outputs are missing: {', '.join(missing_outputs)}")

        self.commit_state(entry, ctx, provider_data, cmake_options)
        self.reporter.finish_entry(entry, "build")

    def prepare_source_dependency(
        self,
        name: str,
        dep: dict[str, Any],
        entry: PlanEntry,
        ctx: Any,
        artifact: DownloadedArtifact | None = None,
    ) -> None:
        provider_data = provider_payload(dep, entry.provider)
        source_dir = Path(entry.source)
        build_dir = Path(entry.build)
        package_dir = Path(entry.package)

        build = dep.get("build", {})
        if not isinstance(build, dict):
            build = {}
        source_subdir = str(build.get("source_subdir", "")).strip()
        if entry.status == "cache_miss_fingerprint_changed":
            self.logger.info(f"    fingerprint changed; resetting source/build/package directories for {name}.")
            if entry.provider_type != "local_source":
                self.filesystem.safe_rmtree(source_dir, ctx.build_root)
            self.filesystem.safe_rmtree(build_dir, ctx.build_root)
            self.filesystem.safe_rmtree(package_dir, ctx.deps_root)

        if entry.provider_type == "local_source":
            self.source_resolver.local_source_dir(name, provider_data)
            return

        source_ready_dir = source_dir / source_subdir if source_subdir else source_dir
        if (source_ready_dir / "CMakeLists.txt").exists():
            self.logger.info(f"    reuse source: {source_dir}")
            return

        archive_path = artifact.path if artifact is not None else self.source_resolver.prepare_source(name, entry, provider_data)
        self.source_resolver.extract_archive(archive_path, provider_data, source_dir, ctx.build_root)

    def bootstrap_staged_dependency(
        self,
        name: str,
        dep: dict[str, Any],
        entry: PlanEntry,
        ctx: Any,
        artifact: DownloadedArtifact | None = None,
    ) -> None:
        provider = entry.provider
        provider_data = provider_payload(dep, provider)
        source_dir = Path(entry.source)
        package_dir = Path(entry.package)
        stage_log = self.logger.dependency_log_path(name, entry.version, "stage")
        self.reporter.set_log(entry, stage_log)
        self.reporter.phase(entry, "Install", "running")
        self.logger.detail(stage_log, f"$ stage {name} {entry.version} provider={provider}")

        self.filesystem.safe_rmtree(source_dir, ctx.build_root)
        self.filesystem.safe_rmtree(package_dir, ctx.deps_root)
        package_dir.mkdir(parents=True, exist_ok=True)

        if entry.provider_type in {"file", "single_header"}:
            file_path = artifact.path if artifact is not None else self.source_resolver.prepare_download(name, entry, provider_data)
            target_path = str(provider_data.get("target_path", "")).strip()
            if not target_path:
                raise RuntimeError(f"dependency {name} single file provider requires target_path")
            self.source_resolver.copy_single_file(file_path, package_dir, target_path)
        else:
            archive_path = artifact.path if artifact is not None else self.source_resolver.prepare_download(name, entry, provider_data)
            self.source_resolver.extract_archive(archive_path, provider_data, source_dir, ctx.build_root)
            staging = dep.get("staging", {})
            copy_items = staging.get("copy", []) if isinstance(staging, dict) else []
            if not isinstance(copy_items, list) or not copy_items:
                raise RuntimeError(f"dependency {name} staged provider requires staging.copy items")
            self.source_resolver.copy_stage_items(
                source_dir,
                package_dir,
                [item for item in copy_items if isinstance(item, dict)],
            )

        outputs_ok, missing_outputs = self.output_validator.check(package_dir, entry.outputs, entry.required_outputs)
        if not outputs_ok:
            raise RuntimeError(f"dependency {name} staged package is missing required outputs: {', '.join(missing_outputs)}")

        self.commit_state(entry, ctx, provider_data, {})
        self.reporter.phase(entry, "Install", "success")
        self.reporter.finish_entry(entry, "stage")

    def bootstrap_system_dependency(
        self,
        name: str,
        dep: dict[str, Any],
        entry: PlanEntry,
        ctx: Any,
    ) -> None:
        provider_data = provider_payload(dep, entry.provider)
        package_dir = Path(entry.package)
        package_dir.mkdir(parents=True, exist_ok=True)

        outputs_ok, missing_outputs = self.output_validator.check(package_dir, entry.outputs, entry.required_outputs)
        if not outputs_ok:
            raise RuntimeError(f"dependency {name} system provider is missing required outputs: {', '.join(missing_outputs)}")

        self.commit_state(entry, ctx, provider_data, {})
        for phase in ("Download", "Configure", "Build", "Install"):
            self.reporter.phase(entry, phase, "skip")
        self.reporter.finish_entry(entry, "system")
        self.logger.info(f"    system dependency {name} recorded; target resolution is delegated to CMake.")

    def is_actionable(self, entry: PlanEntry) -> bool:
        return entry.kind == "dependency" and entry.name in BOOTSTRAP_DEPENDENCIES and entry.status != "cache_hit" and entry.sha_locked

    def is_cmake_source_entry(self, dep: dict[str, Any]) -> bool:
        build = dep.get("build", {})
        return isinstance(build, dict) and build.get("system") == "cmake"

    def print_concurrency_plan(self) -> None:
        self.logger.info("Bootstrap concurrency plan:")
        self.logger.info(f"  effective CPUs: {self.resource_plan.effective_cpu_count}")
        self.logger.info(f"  cpu budget: {self.resource_plan.cpu_budget}")
        self.logger.info(f"  download jobs: {self.resource_plan.download_jobs}")
        self.logger.info(f"  stage jobs: {self.resource_plan.stage_jobs}")
        self.logger.info(f"  cmake jobs per dependency: {self.resource_plan.cmake_jobs_per_dependency}")
        self.logger.info("  cmake source builds: 1")

    def stage_task(self, entry: PlanEntry, dep: dict[str, Any], ctx: Any, download_scheduler: DownloadScheduler) -> None:
        try:
            self.reporter.phase(entry, "Download", "running")
            artifact = download_scheduler.artifact_for(entry)
            self.reporter.phase(entry, "Download", "success")
            self.bootstrap_staged_dependency(entry.name, dep, entry, ctx, artifact)
        except Exception as exc:
            self.reporter.mark_failed(entry, str(exc))
            raise

    def materialize_dependencies(self, args: argparse.Namespace, ctx: Any) -> tuple[list[PlanEntry], int]:
        errors = 0
        with self.materialize_lock(ctx):
            deps_lock = self.lock_repo.deps_lock
            entries = self.plan_builder.build(args, ctx)
            errors += self.validate_entries(entries, args)
            if errors:
                return entries, errors

            download_scheduler = DownloadScheduler(self.source_resolver, self.resource_plan.download_jobs)
            stage_workers = max(1, self.resource_plan.stage_jobs)
            stage_futures: dict[str, Future[None]] = {}
            try:
                for entry in entries:
                    if entry.status == "cache_hit":
                        self.reporter.mark_reused(entry)
                        continue
                    if not self.is_actionable(entry):
                        continue
                    dep = deps_lock.get("dependencies", {}).get(entry.name, {})
                    self.reporter.set_action(entry, "pending")
                    if download_scheduler.requires_artifact(entry):
                        self.reporter.set_log(entry, self.source_resolver.download_log_path(entry.name, entry.version))
                        download_scheduler.submit(entry, dep)

                with ThreadPoolExecutor(max_workers=stage_workers, thread_name_prefix="baas-stage") as stage_executor:
                    for entry in entries:
                        if not self.is_actionable(entry):
                            continue
                        dep = deps_lock.get("dependencies", {}).get(entry.name, {})
                        if self.is_cmake_source_entry(dep):
                            continue
                        if entry.provider_type in {"archive", "prebuilt_archive", "header_archive", "file", "single_header", "local_archive"}:
                            stage_futures[entry.name] = stage_executor.submit(self.stage_task, entry, dep, ctx, download_scheduler)
                        elif entry.provider_type == "system":
                            try:
                                self.bootstrap_system_dependency(entry.name, dep, entry, ctx)
                            except (OSError, ValueError, RuntimeError) as exc:
                                self.reporter.mark_failed(entry, str(exc))
                                self.logger.error(f"    error: {entry.name}: {exc}")
                                errors += 1
                        else:
                            self.reporter.mark_skipped(entry, f"unsupported provider type {entry.provider_type}")
                            self.logger.info(f"    skip: unsupported provider type {entry.provider_type} for {entry.name}.")

                    for entry in entries:
                        if not self.is_actionable(entry):
                            continue
                        dep = deps_lock.get("dependencies", {}).get(entry.name, {})
                        if not self.is_cmake_source_entry(dep):
                            continue
                        try:
                            if download_scheduler.requires_artifact(entry):
                                self.reporter.phase(entry, "Download", "running")
                                artifact = download_scheduler.artifact_for(entry)
                                self.reporter.phase(entry, "Download", "success")
                            else:
                                artifact = None
                                self.reporter.phase(entry, "Download", "skip")
                            self.prepare_source_dependency(entry.name, dep, entry, ctx, artifact)
                        except (OSError, ValueError, RuntimeError, subprocess.SubprocessError) as exc:
                            self.reporter.mark_failed(entry, str(exc))
                            self.logger.error(f"    error: {entry.name}: {exc}")
                            errors += 1

                    for entry in entries:
                        future = stage_futures.get(entry.name)
                        if future is None:
                            continue
                        try:
                            future.result()
                        except (OSError, ValueError, RuntimeError, subprocess.SubprocessError) as exc:
                            self.reporter.mark_failed(entry, str(exc))
                            self.logger.error(f"    error: {entry.name}: {exc}")
                            errors += 1
            finally:
                download_scheduler.shutdown(cancel=errors > 0)
        return entries, errors

    def run(self, args: argparse.Namespace, ctx: Any) -> int:
        deps_lock = self.lock_repo.deps_lock
        entries = self.plan_builder.build(args, ctx)
        errors = 0
        self.reporter.begin_summary(entries)
        self.reporter.print_bootstrap_header(ctx)
        self.print_concurrency_plan()
        for entry in entries:
            self.reporter.print_entry(entry, args.verbose)
        errors += self.validate_entries(entries, args)
        if errors:
            self.reporter.print_summary()
            return 1

        entries, materialize_errors = self.materialize_dependencies(args, ctx)
        errors += materialize_errors
        if errors:
            self.reporter.print_summary()
            return 1

        try:
            for entry in entries:
                if not self.is_actionable(entry):
                    continue
                dep = deps_lock.get("dependencies", {}).get(entry.name, {})
                if not self.is_cmake_source_entry(dep):
                    continue
                try:
                    self.bootstrap_source_dependency(entry.name, dep, entry, ctx, args)
                except (OSError, ValueError, RuntimeError, subprocess.SubprocessError) as exc:
                    self.reporter.mark_failed(entry, str(exc))
                    self.logger.error(f"    error: {entry.name}: {exc}")
                    errors += 1
        finally:
            self.reporter.print_summary()
        return 1 if errors else 0


def clean(args: argparse.Namespace, ctx: Any, filesystem: SafeFilesystem, logger: DependencyLogger) -> int:
    targets = [item.strip() for item in args.clean.split(",") if item.strip()]
    if not targets:
        return 0
    bases = (ctx.deps_root, ctx.assets_root, ctx.build_root)
    for target in targets:
        for base in bases:
            for path in base.glob(target):
                filesystem.ensure_inside(path, ctx.workspace_root)
                if path.exists():
                    logger.info(f"remove {path}")
                    shutil.rmtree(path)
            direct = base / target
            filesystem.ensure_inside(direct, ctx.workspace_root)
            if direct.exists():
                logger.info(f"remove {direct}")
                shutil.rmtree(direct)
    return 0
