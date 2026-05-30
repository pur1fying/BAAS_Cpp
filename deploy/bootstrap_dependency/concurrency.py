from __future__ import annotations

import argparse
import math
import os
import threading
from concurrent.futures import Future, ThreadPoolExecutor
from dataclasses import dataclass
from pathlib import Path
from typing import Any

from .planner import PlanEntry
from .repository import provider_payload
from .source_package import SourcePackageResolver


def clamp(value: int, min_value: int, max_value: int) -> int:
    return max(min_value, min(value, max_value))


def positive_int(value: Any) -> int | None:
    if value is None:
        return None
    text = str(value).strip()
    if not text or text.lower() == "auto":
        return None
    try:
        parsed = int(text)
    except ValueError as exc:
        raise ValueError(f"expected positive integer or auto, got: {value}") from exc
    if parsed < 1:
        raise ValueError(f"expected positive integer or auto, got: {value}")
    return parsed


def read_int(path: Path) -> int | None:
    try:
        text = path.read_text(encoding="utf-8").strip()
    except OSError:
        return None
    try:
        return int(text)
    except ValueError:
        return None


def cgroup_v2_cpu_count(root: Path = Path("/sys/fs/cgroup")) -> int | None:
    try:
        parts = (root / "cpu.max").read_text(encoding="utf-8").strip().split()
    except OSError:
        return None
    if len(parts) < 2 or parts[0] == "max":
        return None
    try:
        quota = int(parts[0])
        period = int(parts[1])
    except ValueError:
        return None
    if quota <= 0 or period <= 0:
        return None
    return max(1, math.ceil(quota / period))


def cgroup_v1_cpu_count(root: Path = Path("/sys/fs/cgroup")) -> int | None:
    quota = read_int(root / "cpu" / "cpu.cfs_quota_us")
    period = read_int(root / "cpu" / "cpu.cfs_period_us")
    if quota is None or period is None or quota <= 0 or period <= 0:
        return None
    return max(1, math.ceil(quota / period))


def detect_effective_cpu_count(cgroup_root: Path = Path("/sys/fs/cgroup")) -> int:
    candidates: list[int] = []
    if hasattr(os, "sched_getaffinity"):
        try:
            candidates.append(len(os.sched_getaffinity(0)))  # type: ignore[attr-defined]
        except OSError:
            pass
    for value in (cgroup_v2_cpu_count(cgroup_root), cgroup_v1_cpu_count(cgroup_root), os.cpu_count()):
        if value and value > 0:
            candidates.append(int(value))
    return max(1, min(candidates) if candidates else 1)


@dataclass(frozen=True)
class ResourcePlan:
    effective_cpu_count: int
    cpu_budget: int
    download_jobs: int
    stage_jobs: int
    cmake_jobs_per_dependency: int


class ResourcePlanner:
    def __init__(self, cgroup_root: Path = Path("/sys/fs/cgroup")) -> None:
        self.cgroup_root = cgroup_root

    def compute(self, args: argparse.Namespace) -> ResourcePlan:
        explicit_cpu_budget = positive_int(getattr(args, "cpu_budget", None))
        env_cpu_budget = positive_int(os.environ.get("BAAS_CPU_BUDGET"))
        effective_cpus = explicit_cpu_budget or env_cpu_budget or detect_effective_cpu_count(self.cgroup_root)
        cpu_budget = explicit_cpu_budget or env_cpu_budget or effective_cpus

        auto_download_jobs = clamp(math.ceil(effective_cpus / 4), 2, 8)
        auto_stage_jobs = clamp(math.ceil(effective_cpus / 8), 1, 4)
        download_jobs = positive_int(getattr(args, "download_jobs", None)) or auto_download_jobs
        stage_jobs = positive_int(getattr(args, "stage_jobs", None)) or auto_stage_jobs

        reserved_for_stage = min(stage_jobs, max(1, effective_cpus // 8))
        auto_cmake_jobs = max(1, cpu_budget - reserved_for_stage)
        cmake_jobs = positive_int(getattr(args, "jobs", None)) or auto_cmake_jobs
        return ResourcePlan(
            effective_cpu_count=effective_cpus,
            cpu_budget=cpu_budget,
            download_jobs=download_jobs,
            stage_jobs=stage_jobs,
            cmake_jobs_per_dependency=cmake_jobs,
        )


class WeightedSemaphore:
    def __init__(self, capacity: int) -> None:
        self.capacity = max(1, capacity)
        self.available = self.capacity
        self.condition = threading.Condition()

    def acquire(self, amount: int) -> None:
        amount = max(1, min(amount, self.capacity))
        with self.condition:
            while self.available < amount:
                self.condition.wait()
            self.available -= amount

    def release(self, amount: int) -> None:
        amount = max(1, min(amount, self.capacity))
        with self.condition:
            self.available = min(self.capacity, self.available + amount)
            self.condition.notify_all()


@dataclass(frozen=True)
class DownloadedArtifact:
    entry_name: str
    path: Path


class DownloadScheduler:
    def __init__(self, source_resolver: SourcePackageResolver, max_workers: int) -> None:
        self.source_resolver = source_resolver
        self.executor = ThreadPoolExecutor(max_workers=max(1, max_workers), thread_name_prefix="baas-download")
        self.futures_by_key: dict[tuple[str, str, str], Future[DownloadedArtifact]] = {}
        self.futures_by_entry: dict[str, Future[DownloadedArtifact]] = {}
        self.lock = threading.Lock()

    def requires_artifact(self, entry: PlanEntry) -> bool:
        return entry.provider_type in {
            "archive",
            "prebuilt_archive",
            "header_archive",
            "file",
            "single_header",
            "local_archive",
        }

    def artifact_key(self, entry: PlanEntry, provider_data: dict[str, Any]) -> tuple[str, str, str]:
        urls = self.source_resolver.archive_urls(provider_data)
        source = "|".join(urls) if urls else str(provider_data.get("path_env") or provider_data.get("path_var") or "")
        return (entry.provider_type, source, str(provider_data.get("sha256", "")))

    def submit(self, entry: PlanEntry, dep: dict[str, Any]) -> Future[DownloadedArtifact] | None:
        if not self.requires_artifact(entry):
            return None
        provider_data = provider_payload(dep, entry.provider)
        key = self.artifact_key(entry, provider_data)
        with self.lock:
            future = self.futures_by_key.get(key)
            if future is None:
                future = self.executor.submit(self.download_entry, entry, provider_data)
                self.futures_by_key[key] = future
            self.futures_by_entry[entry.name] = future
            return future

    def download_entry(self, entry: PlanEntry, provider_data: dict[str, Any]) -> DownloadedArtifact:
        path = self.source_resolver.prepare_download(entry.name, entry, provider_data)
        return DownloadedArtifact(entry.name, path)

    def artifact_for(self, entry: PlanEntry) -> DownloadedArtifact | None:
        future = self.futures_by_entry.get(entry.name)
        return future.result() if future is not None else None

    def shutdown(self, cancel: bool = False) -> None:
        self.executor.shutdown(wait=not cancel, cancel_futures=cancel)
