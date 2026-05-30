from __future__ import annotations

import argparse
import hashlib
import http.client
import io
import json
import os
import shutil
import subprocess
import sys
import tempfile
import threading
import time
import unittest
from contextlib import redirect_stdout
from dataclasses import replace
from pathlib import Path
from unittest.mock import patch

from deploy.bootstrap_dependency import main, parse_args
from deploy.bootstrap_dependency.cmake import CMakeBuilder
from deploy.bootstrap_dependency.cmake_index import CMakeDependencyIndex
from deploy.bootstrap_dependency.concurrency import (
    DownloadScheduler,
    ResourcePlanner as BootstrapResourcePlanner,
    WeightedSemaphore,
    cgroup_v2_cpu_count,
)
from deploy.bootstrap_dependency.context import BootstrapContext, normalized_arch, normalized_platform
from deploy.bootstrap_dependency.json_store import JsonStore
from deploy.bootstrap_dependency.logger import DependencyLogger
from deploy.bootstrap_dependency.locks import LockRepository
from deploy.bootstrap_dependency.outputs import OutputResolver, OutputValidator
from deploy.bootstrap_dependency.planner import DependencyPlanner, PlanBuilder, PlanEntry, ResourcePlanner
from deploy.bootstrap_dependency.repository import (
    PathResolver,
    RepoPaths,
    StateStore,
    provider_for_dependency,
    provider_payload,
)
from deploy.bootstrap_dependency.reporter import Reporter
from deploy.bootstrap_dependency.source_package import SourcePackageResolver
from deploy.bootstrap_dependency.templating import TemplateExpander
from deploy.bootstrap_dependency.utils import CommandRunner, SafeFilesystem


def args_for(*items: str) -> argparse.Namespace:
    return parse_args(list(items))


def write_minimal_locks(root: Path) -> RepoPaths:
    deps_lock = {
        "schema": 1,
        "lock_type": "baas-cpp-dependencies",
        "paths": {
            "downloads": "${BAAS_DOWNLOADS_ROOT}",
            "source": "${BAAS_DEPS_BUILD_ROOT}/${name}/${version}/${variant}/${provider}/src",
            "build": "${BAAS_DEPS_BUILD_ROOT}/${name}/${version}/${variant}/${provider}/build",
            "package": "${BAAS_DEPENDENCY_ROOT}/${name}/${version}/${variant}/${provider}",
        },
        "dependencies": {
            "demo": {
                "version": "1.0.0",
                "providers": {
                    "source_archive": {
                        "type": "archive",
                        "url": "https://example.invalid/demo-1.0.0.tar.gz",
                        "sha256": "0" * 64,
                        "archive_type": "tar.gz",
                        "strip_prefix": "demo-1.0.0",
                    },
                    "todo": {
                        "type": "archive",
                        "url": "https://example.invalid/demo-1.0.0.tar.gz",
                        "sha256": "TODO_LOCK_SHA256",
                    },
                    "local_source": {
                        "type": "local_source",
                        "path_env": "BAAS_DEMO_SOURCE_DIR",
                    },
                },
                "provider_by_platform": {
                    "default": "source_archive",
                },
                "build": {
                    "system": "cmake",
                    "cmake_options": {
                        "CMAKE_BUILD_TYPE": "${cmake_config}",
                        "CMAKE_INSTALL_PREFIX": "${package}",
                    },
                },
                "outputs": {
                    "windows": {
                        "release": {
                            "include": ["include/demo.h"],
                            "runtime": [],
                            "link": ["lib/demo.lib"],
                            "cmake": ["lib/cmake/demo/demoConfig.cmake"],
                        }
                    },
                    "linux": {
                        "include": ["include/demo.h"],
                        "runtime": [],
                        "link": ["lib/libdemo.a"],
                    },
                },
                "validation": {
                    "required_outputs": ["include", "link", "cmake"],
                },
            }
        },
    }
    resources_lock = {
        "schema": 1,
        "lock_type": "baas-cpp-resources",
        "paths": {
            "downloads": "${BAAS_DOWNLOADS_ROOT}",
            "package": "${BAAS_ASSETS_ROOT}/${name}/${version}/${provider}",
        },
        "resources": {
            "asset": {
                "version": "TODO_CONFIRM",
                "provider": "download",
                "url": "TODO_LOCK_URL",
                "sha256": "TODO_LOCK_SHA256",
                "outputs": {"runtime": ["asset.dat"]},
            }
        },
    }
    (root / "deps.lock.json").write_text(json.dumps(deps_lock), encoding="utf-8")
    (root / "resources.lock.json").write_text(json.dumps(resources_lock), encoding="utf-8")
    return RepoPaths(root, root / "deps.lock.json", root / "resources.lock.json")


def make_services(repo_paths: RepoPaths):
    expander = TemplateExpander()
    output_resolver = OutputResolver(expander)
    output_validator = OutputValidator()
    path_resolver = PathResolver(expander)
    state_store = StateStore(JsonStore())
    lock_repo = LockRepository(repo_paths, JsonStore())
    dep_planner = DependencyPlanner(output_resolver, output_validator, path_resolver, state_store)
    res_planner = ResourcePlanner(output_resolver, output_validator, path_resolver, state_store)
    return expander, None, output_resolver, output_validator, path_resolver, state_store, lock_repo, PlanBuilder(lock_repo, state_store, dep_planner, res_planner)


class BootstrapDepsTests(unittest.TestCase):
    def test_platform_arch_config_normalization(self) -> None:
        self.assertEqual(normalized_platform("Windows"), "windows")
        self.assertEqual(normalized_platform("Darwin"), "macos")
        self.assertEqual(normalized_arch("AMD64"), "x64")
        self.assertEqual(normalized_arch("aarch64"), "arm64")
        self.assertEqual(args_for("--archive-download-retry-cnt", "5").archive_download_retry_cnt, 5)
        self.assertEqual(args_for("--retry-cnt", "6").archive_download_retry_cnt, 6)
        self.assertEqual(args_for("--jobs", "auto").jobs, "auto")
        self.assertEqual(args_for("--jobs", "4").jobs, "4")

    def test_context_env_var_construction(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            repo_paths = write_minimal_locks(root)
            local = root / "custom-local"
            with patch.dict(os.environ, {"BAAS_LOCAL_ROOT": str(local), "BAAS_COMPILER_VERSION": "19.43"}, clear=False):
                ctx = BootstrapContext.from_args(args_for("--platform", "Windows", "--arch", "AMD64", "--config", "Debug"), repo_paths)
            self.assertEqual(ctx.variant, "windows-x64-msvc-debug")
            self.assertEqual(ctx.local_root, local.resolve())
            self.assertEqual(ctx.deps_root, (local / "dependency").resolve())
            self.assertEqual(ctx.compiler_version, "19.43")

    def test_template_expansion(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            repo_paths = write_minimal_locks(root)
            ctx = BootstrapContext.from_args(args_for("--platform", "windows", "--arch", "x64"), repo_paths)
            expanded = TemplateExpander().expand("${BAAS_DEPENDENCY_ROOT}/${name}/${version}/${variant}/${provider}/${cmake_config}", ctx, "demo", "1.0.0", "source_archive")
            self.assertIn("demo/1.0.0/windows-x64-msvc-release/source_archive/Release", expanded.replace("\\", "/"))

    def test_provider_selection(self) -> None:
        dep = {"provider_by_platform": {"windows": "win", "default": "source"}, "provider": "fallback", "providers": {"win": {"type": "archive"}}}
        ctx = argparse.Namespace(platform_key="windows")
        self.assertEqual(provider_for_dependency("demo", dep, ctx, None), "win")
        self.assertEqual(provider_for_dependency("demo", dep, ctx, "local_source"), "local_source")
        self.assertEqual(provider_payload(dep, "win"), {"type": "archive"})

    def test_outputs_and_glob_validation(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            repo_paths = write_minimal_locks(root)
            ctx = BootstrapContext.from_args(args_for("--platform", "windows", "--arch", "x64"), repo_paths)
            expander, _, output_resolver, validator, *_ = make_services(repo_paths)
            dep = LockRepository(repo_paths).deps_lock["dependencies"]["demo"]
            outputs = output_resolver.expand(output_resolver.for_dependency(dep, ctx), ctx, "demo", "1.0.0", "source_archive")
            required = output_resolver.required_kinds(dep, outputs)
            package = root / "pkg"
            (package / "include").mkdir(parents=True)
            (package / "lib" / "cmake" / "demo").mkdir(parents=True)
            (package / "include" / "demo.h").write_text("", encoding="utf-8")
            (package / "lib" / "demo.lib").write_text("", encoding="utf-8")
            (package / "lib" / "cmake" / "demo" / "demoConfig.cmake").write_text("", encoding="utf-8")
            ok, missing = validator.check(package, outputs, required)
            self.assertTrue(ok)
            self.assertEqual(missing, [])
            self.assertTrue(validator.path_exists_with_glob(package, "lib/*.lib"))

    def test_path_resolver_dependency_and_resource(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            repo_paths = write_minimal_locks(root)
            ctx = BootstrapContext.from_args(args_for("--platform", "windows", "--arch", "x64"), repo_paths)
            path_resolver = PathResolver(TemplateExpander())
            locks = LockRepository(repo_paths)
            dep_paths = path_resolver.resolve(locks.deps_lock, locks.deps_lock["dependencies"]["demo"], ctx, "demo", "1.0.0", "source_archive", False)
            res_paths = path_resolver.resolve(locks.resources_lock, locks.resources_lock["resources"]["asset"], ctx, "asset", "TODO_CONFIRM", "download", True)
            self.assertEqual(dep_paths["package"], (ctx.deps_root / "demo" / "1.0.0" / ctx.variant / "source_archive").resolve())
            self.assertEqual(res_paths["package"], (ctx.assets_root / "asset" / "TODO_CONFIRM" / "download").resolve())

    def test_cmake_dependency_index_marks_ready_cache_hits(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            repo_paths = write_minimal_locks(root)
            with patch.dict(os.environ, {"BAAS_LOCAL_ROOT": str(root / ".baas")}, clear=False):
                ctx = BootstrapContext.from_args(args_for("--platform", "windows", "--arch", "AMD64"), repo_paths)
                *_, state_store, _, builder = make_services(repo_paths)
                entry = builder.build(args_for("--dependency", "demo"), ctx)[0]
                package = Path(entry.package)
                (package / "include").mkdir(parents=True)
                (package / "lib" / "cmake" / "demo").mkdir(parents=True)
                (package / "include" / "demo.h").write_text("", encoding="utf-8")
                (package / "lib" / "demo.lib").write_text("", encoding="utf-8")
                (package / "lib" / "cmake" / "demo" / "demoConfig.cmake").write_text("", encoding="utf-8")
                state = state_store.empty()
                state.setdefault("dependencies", {}).setdefault("demo", {}).setdefault("1.0.0", {}).setdefault("source_archive", {})[ctx.variant] = {
                    "fingerprint": entry.fingerprint
                }
                JsonStore().write_atomic(ctx.state_file, state)

                cache_hit = builder.build(args_for("--dependency", "demo"), ctx)[0]
                path = CMakeDependencyIndex().write(ctx, [cache_hit])
                text = path.read_text(encoding="utf-8")

            self.assertIn('set(BAAS_DEPENDENCY_ROOT "', text)
            self.assertIn('set(BAAS_PLATFORM_KEY "windows")', text)
            self.assertIn('set(BAAS_ARCH_KEY "x64")', text)
            self.assertIn('set(BAAS_VARIANT "windows-x64-msvc-release")', text)
            self.assertIn('set(BAAS_DEP_DEMO_VERSION "1.0.0")', text)
            self.assertIn('set(BAAS_DEP_DEMO_PROVIDER "source_archive")', text)
            self.assertIn("set(BAAS_DEP_DEMO_READY TRUE)", text)
            self.assertIn("BAAS_DEP_DEMO_PACKAGE_DIR", text)
            self.assertIn("/", text)
            self.assertNotIn("\\", text)
            self.assertFalse(path.with_suffix(path.suffix + ".tmp").exists())

    def test_cmake_dependency_index_does_not_expose_unready_package_dirs(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            repo_paths = write_minimal_locks(root)
            ctx = BootstrapContext.from_args(args_for("--platform", "windows", "--arch", "x64"), repo_paths)
            *_, builder = make_services(repo_paths)
            entry = builder.build(args_for("--dependency", "demo"), ctx)[0]
            text = CMakeDependencyIndex().render(ctx, [entry])

            self.assertIn("set(BAAS_DEP_DEMO_READY FALSE)", text)
            self.assertNotIn("BAAS_DEP_DEMO_PACKAGE_DIR", text)

    def test_state_store_read_write_and_schema(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            repo_paths = write_minimal_locks(root)
            ctx = BootstrapContext.from_args(args_for("--platform", "windows", "--arch", "x64"), repo_paths)
            store = StateStore(JsonStore())
            state = store.empty()
            store.save(ctx, state)
            self.assertEqual(store.load(ctx)["schema"], 1)
            ctx.state_file.write_text('{"schema": 99}', encoding="utf-8")
            with self.assertRaises(ValueError):
                store.load(ctx)

    def test_plan_statuses(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            repo_paths = write_minimal_locks(root)
            with patch.dict(os.environ, {"BAAS_LOCAL_ROOT": str(root / ".baas")}, clear=False):
                ctx = BootstrapContext.from_args(args_for("--platform", "windows", "--arch", "x64"), repo_paths)
                *_, state_store, lock_repo, builder = make_services(repo_paths)
                entries = builder.build(args_for("--dependency", "demo"), ctx)
                self.assertEqual(entries[0].status, "cache_miss_state_missing")

                todo = builder.build(args_for("--dependency", "demo", "--provider", "todo"), ctx)[0]
                self.assertEqual(todo.status, "blocked_until_sha256_locked")

                package = Path(entries[0].package)
                (package / "include").mkdir(parents=True)
                (package / "lib" / "cmake" / "demo").mkdir(parents=True)
                (package / "include" / "demo.h").write_text("", encoding="utf-8")
                (package / "lib" / "demo.lib").write_text("", encoding="utf-8")
                (package / "lib" / "cmake" / "demo" / "demoConfig.cmake").write_text("", encoding="utf-8")
                state = state_store.empty()
                state.setdefault("dependencies", {}).setdefault("demo", {}).setdefault("1.0.0", {}).setdefault("source_archive", {})[ctx.variant] = {
                    "fingerprint": entries[0].fingerprint
                }
                JsonStore().write_atomic(ctx.state_file, state)
                self.assertEqual(builder.build(args_for("--dependency", "demo"), ctx)[0].status, "cache_hit")

                state["dependencies"]["demo"]["1.0.0"]["source_archive"][ctx.variant]["fingerprint"] = "sha256:changed"
                JsonStore().write_atomic(ctx.state_file, state)
                self.assertEqual(builder.build(args_for("--dependency", "demo"), ctx)[0].status, "cache_miss_fingerprint_changed")

                state["dependencies"]["demo"]["1.0.0"]["source_archive"][ctx.variant]["fingerprint"] = entries[0].fingerprint
                JsonStore().write_atomic(ctx.state_file, state)
                (package / "lib" / "demo.lib").unlink()
                self.assertEqual(builder.build(args_for("--dependency", "demo"), ctx)[0].status, "cache_miss_required_outputs_missing")

    def test_safe_filesystem_rejects_outside_and_root_delete(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            fs = SafeFilesystem()
            with self.assertRaises(ValueError):
                fs.ensure_inside(root.parent, root)
            with self.assertRaises(ValueError):
                fs.safe_rmtree(root, root)

    def test_command_runner_writes_process_output_to_log_only(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            out = io.StringIO()
            logger = DependencyLogger(root / "logs", out=out)
            runner = CommandRunner(logger)
            log_file = root / "command.log"
            runner.run([sys.executable, "-c", "print('cmake progress line')"], log_file, 30)
            self.assertNotIn("\ncmake progress line\n", out.getvalue())
            self.assertIn("cmake progress line", log_file.read_text(encoding="utf-8"))

    def test_command_runner_terminates_process_tree_on_keyboard_interrupt(self) -> None:
        class FakeProcess:
            pid = 12345

            def __init__(self) -> None:
                self.stdout = io.StringIO("")
                self.wait_calls = 0

            def wait(self, timeout: int) -> int:
                self.wait_calls += 1
                if self.wait_calls == 1:
                    raise subprocess.TimeoutExpired(["cmake"], timeout)
                raise KeyboardInterrupt

        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            out = io.StringIO()
            logger = DependencyLogger(root / "logs", out=out)
            runner = CommandRunner(logger)
            terminated: list[int] = []

            def fake_popen(*args, **kwargs):  # noqa: ANN001, ANN202
                return FakeProcess()

            def fake_terminate(process):  # noqa: ANN001, ANN202
                terminated.append(process.pid)

            with patch("deploy.bootstrap_dependency.utils.subprocess.Popen", side_effect=fake_popen):
                with patch.object(runner, "terminate_process_tree", side_effect=fake_terminate):
                    with self.assertRaises(KeyboardInterrupt):
                        runner.run(["cmake", "--build", "demo"], root / "command.log", 30)

            self.assertEqual(terminated, [12345])
            self.assertIn("terminating process tree", out.getvalue())

    def test_resource_plan_auto_values(self) -> None:
        with patch("deploy.bootstrap_dependency.concurrency.detect_effective_cpu_count", return_value=32):
            plan = BootstrapResourcePlanner().compute(args_for("--jobs", "auto"))
        self.assertEqual(plan.effective_cpu_count, 32)
        self.assertEqual(plan.cpu_budget, 32)
        self.assertGreaterEqual(plan.download_jobs, 4)
        self.assertLessEqual(plan.download_jobs, 8)
        self.assertEqual(plan.stage_jobs, 4)
        self.assertEqual(plan.cmake_jobs_per_dependency, 28)

    def test_resource_plan_manual_jobs_only_affects_cmake_build_jobs(self) -> None:
        with patch("deploy.bootstrap_dependency.concurrency.detect_effective_cpu_count", return_value=32):
            plan = BootstrapResourcePlanner().compute(args_for("--jobs", "4", "--download-jobs", "auto", "--stage-jobs", "auto"))
        self.assertEqual(plan.cmake_jobs_per_dependency, 4)
        self.assertEqual(plan.download_jobs, 8)
        self.assertEqual(plan.stage_jobs, 4)

    def test_resource_plan_auto_cmake_jobs_for_common_cpu_counts(self) -> None:
        expected = {
            2: (2, 1, 1),
            4: (2, 1, 3),
            8: (2, 1, 7),
            16: (4, 2, 14),
            32: (8, 4, 28),
        }
        for cpus, values in expected.items():
            with self.subTest(cpus=cpus):
                with patch("deploy.bootstrap_dependency.concurrency.detect_effective_cpu_count", return_value=cpus):
                    plan = BootstrapResourcePlanner().compute(args_for("--jobs", "auto"))
                self.assertEqual((plan.download_jobs, plan.stage_jobs, plan.cmake_jobs_per_dependency), values)

    def test_cgroup_v2_cpu_count(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "cpu.max").write_text("800000 100000", encoding="utf-8")
            self.assertEqual(cgroup_v2_cpu_count(root), 8)

    def test_weighted_semaphore_limits_parallel_cmake_tokens(self) -> None:
        semaphore = WeightedSemaphore(8)
        active = 0
        max_active = 0
        lock = threading.Lock()

        def worker() -> None:
            nonlocal active, max_active
            semaphore.acquire(4)
            try:
                with lock:
                    active += 1
                    max_active = max(max_active, active)
                time.sleep(0.02)
            finally:
                with lock:
                    active -= 1
                semaphore.release(4)

        threads = [threading.Thread(target=worker) for _ in range(4)]
        for thread in threads:
            thread.start()
        for thread in threads:
            thread.join()
        self.assertLessEqual(max_active, 2)

    def test_download_scheduler_deduplicates_same_artifact(self) -> None:
        class FakeResolver:
            def __init__(self) -> None:
                self.calls = 0

            def archive_urls(self, provider_data):  # noqa: ANN001, ANN202
                return [provider_data["url"]]

            def prepare_download(self, name, entry, provider_data):  # noqa: ANN001, ANN202
                self.calls += 1
                return Path(provider_data["path"])

        def entry(name: str) -> PlanEntry:
            return PlanEntry(
                kind="dependency",
                name=name,
                version="1.0.0",
                provider="source",
                provider_type="archive",
                provider_sha256="0" * 64,
                provider_path="",
                sha_locked=True,
                variant="windows-x64-msvc-release",
                state_file="state.json",
                downloads="downloads",
                source="src",
                build="build",
                package="package",
                fingerprint="sha256:test",
                fingerprint_payload={},
                required_outputs=[],
                outputs={},
                missing_outputs=[],
                status="cache_miss_state_missing",
            )

        resolver = FakeResolver()
        scheduler = DownloadScheduler(resolver, max_workers=2)  # type: ignore[arg-type]
        dep = {
            "providers": {
                "source": {
                    "type": "archive",
                    "url": "https://example.invalid/a.tar.gz",
                    "sha256": "0" * 64,
                    "path": "D:/tmp/a.tar.gz",
                }
            }
        }
        try:
            first = scheduler.submit(entry("dep_a"), dep)
            second = scheduler.submit(entry("dep_b"), dep)
            self.assertIs(first, second)
            self.assertEqual(first.result().path, Path("D:/tmp/a.tar.gz"))
            self.assertEqual(resolver.calls, 1)
        finally:
            scheduler.shutdown()

    def test_archive_helpers_and_local_archive(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            archive = root / "demo.tar.gz"
            archive.write_bytes(b"demo")
            sha = hashlib.sha256(b"demo").hexdigest()
            resolver = SourcePackageResolver(SafeFilesystem(), DependencyLogger.stderr_only())
            provider = {"url": "https://example.invalid/path/demo.tar.gz", "sha256": sha}
            self.assertEqual(resolver.archive_file_name("demo", "1.0.0", provider), "demo.tar.gz")
            self.assertEqual(resolver.archive_urls(provider), ["https://example.invalid/path/demo.tar.gz"])
            self.assertTrue(resolver.verify_sha256(archive, sha))
            with patch.dict(os.environ, {"BAAS_DEMO_ARCHIVE": str(archive)}, clear=False):
                self.assertEqual(resolver.local_archive("demo", {"path_env": "BAAS_DEMO_ARCHIVE", "sha256": sha}), archive.resolve())

    def test_download_retries_incomplete_read(self) -> None:
        class FakeResponse:
            def __init__(self, payload: bytes, include_length: bool = True) -> None:
                self.payload = io.BytesIO(payload)
                self.headers = {"Content-Length": str(len(payload))} if include_length else {}

            def __enter__(self):  # noqa: ANN204
                return self

            def __exit__(self, exc_type, exc, tb):  # noqa: ANN001, ANN204
                return False

            def read(self, size: int = -1) -> bytes:
                return self.payload.read(size)

        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            payload = b"downloaded archive"
            sha = hashlib.sha256(payload).hexdigest()
            output = io.StringIO()
            resolver = SourcePackageResolver(SafeFilesystem(), DependencyLogger(None, out=output), archive_download_retry_cnt=2)
            provider = {
                "url": "https://example.invalid/demo.tar.gz",
                "sha256": sha,
            }
            calls = {"count": 0}

            def fake_urlopen(request, timeout=120):  # noqa: ANN001, ANN202
                calls["count"] += 1
                if calls["count"] == 1:
                    raise http.client.IncompleteRead(b"partial")
                return FakeResponse(payload)

            with patch("deploy.bootstrap_dependency.source_package.urllib.request.urlopen", side_effect=fake_urlopen):
                result = resolver.downloaded_file("demo", "1.0.0", provider, root / "downloads")

            self.assertEqual(calls["count"], 2)
            self.assertEqual(result.read_bytes(), payload)
            self.assertFalse(result.with_suffix(result.suffix + ".tmp").exists())
            self.assertIn("archive", output.getvalue())
            self.assertIn("download complete:", output.getvalue())
            self.assertIn("Retry: 1/1", output.getvalue())
            self.assertNotIn("(attempt 1/2)", output.getvalue())

    def test_download_spinner_when_content_length_is_unknown(self) -> None:
        class FakeResponse:
            def __init__(self, payload: bytes) -> None:
                self.payload = io.BytesIO(payload)
                self.headers = {}

            def __enter__(self):  # noqa: ANN204
                return self

            def __exit__(self, exc_type, exc, tb):  # noqa: ANN001, ANN204
                return False

            def read(self, size: int = -1) -> bytes:
                return self.payload.read(size)

        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            payload = b"x" * (2 * 1024 * 1024)
            sha = hashlib.sha256(payload).hexdigest()
            output = io.StringIO()
            resolver = SourcePackageResolver(SafeFilesystem(), DependencyLogger(None, out=output), archive_download_retry_cnt=1)
            provider = {
                "url": "https://example.invalid/demo.tar.gz",
                "sha256": sha,
            }

            with patch("deploy.bootstrap_dependency.source_package.urllib.request.urlopen", return_value=FakeResponse(payload)):
                resolver.downloaded_file("demo", "1.0.0", provider, root / "downloads")

            text = output.getvalue()
            self.assertIn("archive", text)
            self.assertIn("downloaded", text)
            self.assertNotIn("unknown total", text)

    def test_download_retry_count_is_global_only(self) -> None:
        with patch.dict(
            os.environ,
            {"BAAS_DEPENDENCY_ARCHIVE_DOWNLOAD_RETRY_CNT": "4"},
            clear=False,
        ):
            resolver = SourcePackageResolver(SafeFilesystem(), DependencyLogger.stderr_only())
            self.assertEqual(resolver.download_attempts(), 4)

        resolver = SourcePackageResolver(SafeFilesystem(), DependencyLogger.stderr_only(), archive_download_retry_cnt=6)
        self.assertEqual(resolver.download_attempts(), 6)

    def test_cmake_command_construction(self) -> None:
        class FakeRunner:
            def __init__(self, log_dir: Path) -> None:
                self.commands: list[list[str]] = []
                self.logger = DependencyLogger(log_dir=log_dir)

            def run(self, command, log_file, timeout_seconds):  # noqa: ANN001
                self.commands.append(command)

        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            repo_paths = write_minimal_locks(root)
            toolchain = root / "toolchain.cmake"
            toolchain.write_text("", encoding="utf-8")
            ctx = BootstrapContext.from_args(args_for("--platform", "windows", "--arch", "x64", "--toolchain-file", str(toolchain)), repo_paths)
            runner = FakeRunner(root / "logs")
            builder = CMakeBuilder(runner, TemplateExpander())
            builder.build_source_package(
                "demo",
                "1.0.0",
                "source_archive",
                ctx,
                root / "src",
                root / "build",
                root / "package",
                {"CMAKE_BUILD_TYPE": "${cmake_config}", "CMAKE_INSTALL_PREFIX": "${package}"},
                jobs=7,
                timeout_seconds=123,
            )
            self.assertEqual(runner.commands[0][:4], ["cmake", "-S", str(root / "src"), "-B"])
            self.assertIn(f"-DCMAKE_TOOLCHAIN_FILE={toolchain.resolve()}", runner.commands[0])
            self.assertEqual(runner.commands[1][-2:], ["-j", "7"])
            self.assertEqual(runner.commands[2][:2], ["cmake", "--install"])

    def test_cli_default_no_args_prints_plan_with_temp_repo(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            write_minimal_locks(root)
            old_cwd = Path.cwd()
            os.chdir(root)
            try:
                with patch.dict(os.environ, {"BAAS_LOCAL_ROOT": str(root / ".baas")}, clear=False):
                    output = io.StringIO()
                    with redirect_stdout(output):
                        code = main([])
            finally:
                os.chdir(old_cwd)
            self.assertEqual(code, 0)
            self.assertIn("BAAS bootstrap plan", output.getvalue())
            self.assertIn("dependency demo", output.getvalue())

    def test_cli_verify_only_writes_cmake_dependency_index(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            repo_paths = write_minimal_locks(root)
            with patch.dict(os.environ, {"BAAS_LOCAL_ROOT": str(root / ".baas")}, clear=False):
                ctx = BootstrapContext.from_args(args_for("--platform", "windows", "--arch", "x64"), repo_paths)
                *_, state_store, _, builder = make_services(repo_paths)
                entry = builder.build(args_for("--dependency", "demo"), ctx)[0]
                package = Path(entry.package)
                (package / "include").mkdir(parents=True)
                (package / "lib" / "cmake" / "demo").mkdir(parents=True)
                (package / "include" / "demo.h").write_text("", encoding="utf-8")
                (package / "lib" / "demo.lib").write_text("", encoding="utf-8")
                (package / "lib" / "cmake" / "demo" / "demoConfig.cmake").write_text("", encoding="utf-8")
                state = state_store.empty()
                state.setdefault("dependencies", {}).setdefault("demo", {}).setdefault("1.0.0", {}).setdefault("source_archive", {})[ctx.variant] = {
                    "fingerprint": entry.fingerprint
                }
                JsonStore().write_atomic(ctx.state_file, state)

                old_cwd = Path.cwd()
                os.chdir(root)
                try:
                    output = io.StringIO()
                    with redirect_stdout(output):
                        code = main(["--verify-only", "--dependency", "demo", "--platform", "windows", "--arch", "x64"])
                finally:
                    os.chdir(old_cwd)

            index_path = root / ".baas" / "cmake" / "BAASDependencyIndex.cmake"
            self.assertEqual(code, 0)
            self.assertTrue(index_path.exists())
            self.assertIn("BAAS_DEP_DEMO_READY TRUE", index_path.read_text(encoding="utf-8"))

    def test_cmake_dependency_helper_bootstraps_missing_index(self) -> None:
        cmake = shutil.which("cmake")
        if not cmake:
            self.skipTest("cmake executable not found")

        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            fake_package = root / "package"
            fake_package.mkdir()
            package_value = fake_package.as_posix()
            deploy_dir = root / "deploy" / "bootstrap_dependency"
            deploy_dir.mkdir(parents=True)
            (root / "deploy" / "__init__.py").write_text("", encoding="utf-8")
            (deploy_dir / "__init__.py").write_text("", encoding="utf-8")
            (deploy_dir / "__main__.py").write_text(
                "\n".join(
                    [
                        "from __future__ import annotations",
                        "import sys",
                        "from pathlib import Path",
                        "root = Path.cwd()",
                        "(root / 'bootstrap_args.txt').write_text(' '.join(sys.argv[1:]), encoding='utf-8')",
                        "index = root / '.baas' / 'cmake' / 'BAASDependencyIndex.cmake'",
                        "index.parent.mkdir(parents=True, exist_ok=True)",
                        "index.write_text('\\n'.join([",
                        "    'set(BAAS_DEP_DEMO_READY TRUE)',",
                        f"    'set(BAAS_DEP_DEMO_PACKAGE_DIR \"{package_value}\")',",
                        "    ''",
                        "]), encoding='utf-8')",
                    ]
                ),
                encoding="utf-8",
            )

            helper = (Path(__file__).resolve().parents[1] / "cmake" / "deps" / "DependencyIndex.cmake").as_posix()
            script = root / "check.cmake"
            script.write_text(
                "\n".join(
                    [
                        f'set(BAAS_DEPENDENCY_INDEX "{(root / ".baas" / "cmake" / "BAASDependencyIndex.cmake").as_posix()}" CACHE FILEPATH "")',
                        f'set(BAAS_PYTHON_EXECUTABLE "{Path(sys.executable).as_posix()}" CACHE FILEPATH "")',
                        'set(CMAKE_BUILD_TYPE "Release")',
                        f'include("{helper}")',
                        "baas_get_dependency_package_dir(demo package_dir)",
                        f'if(NOT package_dir STREQUAL "{package_value}")',
                        '    message(FATAL_ERROR "unexpected package dir: ${package_dir}")',
                        "endif()",
                    ]
                ),
                encoding="utf-8",
            )

            result = subprocess.run([cmake, "-P", str(script)], cwd=root, text=True, capture_output=True, check=False)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            args_text = (root / "bootstrap_args.txt").read_text(encoding="utf-8")
            self.assertIn("--dependency demo", args_text)

    def test_cmake_dependency_helper_reuses_ready_index(self) -> None:
        cmake = shutil.which("cmake")
        if not cmake:
            self.skipTest("cmake executable not found")

        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            fake_package = root / "package"
            fake_package.mkdir()
            index = root / ".baas" / "cmake" / "BAASDependencyIndex.cmake"
            index.parent.mkdir(parents=True)
            package_value = fake_package.as_posix()
            index.write_text(
                "\n".join(
                    [
                        "set(BAAS_DEP_DEMO_READY TRUE)",
                        f'set(BAAS_DEP_DEMO_PACKAGE_DIR "{package_value}")',
                        "",
                    ]
                ),
                encoding="utf-8",
            )
            fake_python = root / "fake_python.cmd"
            fake_python.write_text(
                "@echo off\r\necho should-not-run > \"%~dp0bootstrap_ran.txt\"\r\nexit /b 7\r\n",
                encoding="utf-8",
            )

            helper = (Path(__file__).resolve().parents[1] / "cmake" / "deps" / "DependencyIndex.cmake").as_posix()
            script = root / "check.cmake"
            script.write_text(
                "\n".join(
                    [
                        f'set(BAAS_DEPENDENCY_INDEX "{index.as_posix()}" CACHE FILEPATH "")',
                        f'set(BAAS_PYTHON_EXECUTABLE "{fake_python.as_posix()}" CACHE FILEPATH "")',
                        f'include("{helper}")',
                        "baas_get_dependency_package_dir(demo package_dir)",
                        f'if(NOT package_dir STREQUAL "{package_value}")',
                        '    message(FATAL_ERROR "unexpected package dir: ${package_dir}")',
                        "endif()",
                    ]
                ),
                encoding="utf-8",
            )

            result = subprocess.run([cmake, "-P", str(script)], cwd=root, text=True, capture_output=True, check=False)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertFalse((root / "bootstrap_ran.txt").exists())

    def test_reporter_prints_bootstrap_summary_table(self) -> None:
        entry = PlanEntry(
            kind="dependency",
            name="demo",
            version="1.0.0",
            provider="source",
            provider_type="archive",
            provider_sha256="0" * 64,
            provider_path="",
            sha_locked=True,
            variant="windows-x64-msvc-release",
            state_file="state.json",
            downloads="downloads",
            source="src",
            build="build",
            package="package",
            fingerprint="sha256:test",
            fingerprint_payload={},
            required_outputs=[],
            outputs={},
            missing_outputs=[],
            status="cache_miss_state_missing",
        )
        out = io.StringIO()
        reporter = Reporter(DependencyLogger(None, out=out))
        cached_entry = replace(entry, name="cached_demo", status="cache_hit")
        staged_entry = replace(entry, name="staged_demo", provider_type="prebuilt_archive")
        reporter.begin_summary([entry, cached_entry, staged_entry])
        reporter.phase(entry, "Download", "success")
        reporter.phase(entry, "Configure", "success")
        reporter.phase(entry, "Build", "success")
        reporter.phase(entry, "Install", "success")
        reporter.finish_entry(entry, "build")
        reporter.phase(staged_entry, "Download", "success")
        reporter.phase(staged_entry, "Install", "success")
        reporter.finish_entry(staged_entry, "stage")
        reporter.print_summary()
        text = out.getvalue()
        self.assertIn("Dependency bootstrap summary", text)
        self.assertRegex(text, r"\| Dependency\s+\| Version \| Action \| Download \| Build")
        self.assertNotIn("| Cache |", text)
        self.assertNotIn("| Configure | Build", text)
        self.assertIn("build", text)
        self.assertRegex(text, r"build\s+\| success\s+\| success")
        self.assertIn("reuse", text)
        self.assertNotIn("reused-cache", text)
        self.assertRegex(text, r"reuse\s+\| skip\s+\| skip")
        self.assertRegex(text, r"stage\s+\| success\s+\| -")
        self.assertNotIn("skipped", text)
        lines = text.splitlines()
        title_index = lines.index("Dependency bootstrap summary")
        self.assertRegex(lines[title_index + 1], r"^-+$")
        self.assertRegex(lines[-1], r"^-+$")
        self.assertEqual(len(lines[title_index + 1]), len(lines[-1]))


if __name__ == "__main__":
    unittest.main()
