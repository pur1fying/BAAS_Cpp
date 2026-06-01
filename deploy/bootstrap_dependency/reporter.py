from __future__ import annotations

import argparse
import threading
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

from .logger import DependencyLogger
from .planner import PlanBuilder, PlanEntry
from .repository import RepoPaths


PHASES = ("Download", "Configure", "Build", "Install")


@dataclass
class ReportRow:
    kind: str
    name: str
    version: str
    package: str
    action: str = "pending"
    download: str = "not-run"
    configure: str = "not-run"
    build: str = "not-run"
    install: str = "not-run"
    log: str = ""
    error: str = ""
    started_at: float = field(default_factory=time.monotonic)
    ended_at: float | None = None

    def set_phase(self, phase: str, status: str) -> None:
        attr = phase.lower()
        if attr == "download":
            self.download = status
        elif attr == "configure":
            self.configure = status
        elif attr == "build":
            self.build = status
        elif attr == "install":
            self.install = status

    def finish(self, action: str | None = None, error: str = "") -> None:
        if action:
            self.action = action
        if error:
            self.error = error
        self.ended_at = time.monotonic()

    def duration_text(self) -> str:
        end = self.ended_at if self.ended_at is not None else time.monotonic()
        return f"{max(0.0, end - self.started_at):.1f}s"

    def cmake_text(self) -> str:
        statuses = [self.configure, self.build, self.install]
        if self.configure == "not-run" and self.build == "not-run" and self.install != "skip":
            return "-"
        if len(set(statuses)) == 1:
            return "-" if statuses[0] == "not-run" else statuses[0]
        return "/".join("-" if status == "not-run" else status for status in statuses)


class Reporter:
    def __init__(self, logger: DependencyLogger) -> None:
        self.logger = logger
        self._rows: dict[str, ReportRow] = {}
        self._lock = threading.Lock()

    def begin_summary(self, entries: list[PlanEntry]) -> None:
        with self._lock:
            self._rows = {}
            for entry in entries:
                row = ReportRow(
                    kind=entry.kind,
                    name=entry.name,
                    version=entry.version,
                    package=entry.package,
                )
                if entry.status == "cache_hit":
                    row.action = "reuse"
                    for phase in PHASES:
                        row.set_phase(phase, "skip")
                    row.finish()
                elif entry.status == "blocked_until_sha256_locked":
                    row.action = "failed"
                    row.error = "locked SHA256 is required before bootstrapping"
                self._rows[entry.name] = row

    def set_action(self, entry: PlanEntry, action: str) -> None:
        with self._lock:
            row = self._rows.get(entry.name)
            if row:
                row.action = action

    def set_log(self, entry: PlanEntry, log_file: Path | None) -> None:
        if log_file is None:
            return
        with self._lock:
            row = self._rows.get(entry.name)
            if row:
                row.log = str(log_file)

    def phase(self, entry: PlanEntry, phase: str, status: str) -> None:
        with self._lock:
            row = self._rows.get(entry.name)
            if row:
                row.set_phase(phase, status)

    def finish_entry(self, entry: PlanEntry, action: str, error: str = "") -> None:
        with self._lock:
            row = self._rows.get(entry.name)
            if row:
                row.finish(action, error)

    def mark_failed(self, entry: PlanEntry, error: str) -> None:
        with self._lock:
            row = self._rows.get(entry.name)
            if row:
                row.action = "failed"
                if row.download == "running":
                    row.download = "failed"
                if row.configure == "running":
                    row.configure = "failed"
                if row.build == "running":
                    row.build = "failed"
                if row.install == "running":
                    row.install = "failed"
                row.finish("failed", error)

    def mark_reused(self, entry: PlanEntry) -> None:
        with self._lock:
            row = self._rows.get(entry.name)
            if row:
                row.action = "reuse"
                row.error = ""
                for phase in PHASES:
                    row.set_phase(phase, "skip")
                row.finish("reuse")

    def mark_skipped(self, entry: PlanEntry, reason: str) -> None:
        with self._lock:
            row = self._rows.get(entry.name)
            if row:
                row.action = "skip"
                row.error = reason
                for phase in PHASES:
                    if getattr(row, phase.lower()) == "not-run":
                        row.set_phase(phase, "skip")
                row.finish()

    def print_summary(self) -> None:
        with self._lock:
            rows = list(self._rows.values())
        headers = ["Dependency", "Version", "Action", "Download", "Build", "Duration", "Install Path", "Log"]
        table_rows = [
            [
                row.name,
                row.version,
                row.action,
                row.download,
                row.cmake_text(),
                row.duration_text(),
                row.package,
                row.log,
            ]
            for row in rows
        ]
        summary_lines = self.format_table(headers, table_rows)
        failures = [row for row in rows if row.action == "failed"]
        if failures:
            summary_lines.append("")
            summary_lines.append("Failed dependencies:")
            for row in failures:
                suffix = f"; log: {row.log}" if row.log else ""
                summary_lines.append(f"  {row.name}: {row.error or 'failed'}{suffix}")

        border = "-" * max(len(line) for line in summary_lines)
        self.logger.info("")
        self.logger.info("Dependency bootstrap summary")
        self.logger.info(border)
        for line in summary_lines:
            self.logger.info(line)
        self.logger.info(border)

    def format_table(self, headers: list[str], rows: list[list[str]]) -> list[str]:
        widths = [len(header) for header in headers]
        for row in rows:
            for index, value in enumerate(row):
                widths[index] = max(widths[index], len(value))
        lines = [
            "| " + " | ".join(header.ljust(widths[index]) for index, header in enumerate(headers)) + " |",
            "|-" + "-|-".join("-" * width for width in widths) + "-|",
        ]
        for row in rows:
            lines.append("| " + " | ".join(value.ljust(widths[index]) for index, value in enumerate(row)) + " |")
        return lines

    def print_table(self, headers: list[str], rows: list[list[str]]) -> None:
        for line in self.format_table(headers, rows):
            self.logger.info(line)

    def print_entry(self, entry: PlanEntry, verbose: bool) -> None:
        data = entry.to_dict()
        label = f"{data['kind']} {data['name']}"
        self.logger.info(f"  {label}: version={data.get('version')} provider={data.get('provider')} status={data['status']}")
        self.logger.info(f"    package: {data['package']}")
        self.logger.info(f"    state_file: {data['state_file']}")
        self.logger.info(f"    fingerprint: {data['fingerprint']}")
        if data.get("source"):
            self.logger.info(f"    source: {data['source']}")
        if data.get("build"):
            self.logger.info(f"    build: {data['build']}")
        if not data.get("sha_locked", True):
            self.logger.info("    note: SHA256 is not locked; download/build is blocked for this entry.")
        if data.get("missing_outputs"):
            self.logger.info("    missing_outputs: " + ", ".join(data["missing_outputs"]))
        if verbose:
            self.logger.info("    required_outputs: " + ", ".join(data.get("required_outputs", [])))

    def print_plan_entries(self, args: argparse.Namespace, ctx: Any, repo_paths: RepoPaths, entries: list[PlanEntry]) -> int:
        self.logger.info("BAAS bootstrap plan")
        self.logger.info(f"  variant: {ctx.variant}")
        self.logger.info(f"  platform: {ctx.platform_key}")
        self.logger.info(f"  arch: {ctx.arch_key}")
        self.logger.info(f"  compiler: {ctx.compiler_id} {ctx.compiler_version}")
        self.logger.info(f"  build_type: {ctx.config}")
        self.logger.info(f"  workspace_root: {ctx.workspace_root}")
        self.logger.info(f"  state_file: {ctx.state_file}")
        self.logger.info(f"  dependency lock: {repo_paths.dependency_lock}")
        self.logger.info(f"  resources lock: {repo_paths.resources_lock}")
        if self.logger.log_dir is not None:
            self.logger.info(f"  log_dir: {self.logger.log_dir}")
        for entry in entries:
            self.print_entry(entry, args.verbose)
        return 0

    def print_plan(self, args: argparse.Namespace, ctx: Any, repo_paths: RepoPaths, plan_builder: PlanBuilder) -> int:
        return self.print_plan_entries(args, ctx, repo_paths, plan_builder.build(args, ctx))

    def verify_only(self, args: argparse.Namespace, ctx: Any, repo_paths: RepoPaths, plan_builder: PlanBuilder) -> int:
        entries = plan_builder.build(args, ctx)
        failures = [entry for entry in entries if entry.status.startswith("cache_miss")]
        self.print_plan_entries(args, ctx, repo_paths, entries)
        if failures:
            self.logger.info(f"verify summary: {len(failures)} cache miss entry/entries; no download or build was attempted.")
        else:
            self.logger.info("verify summary: no cache miss entries.")
        return 0

    def print_bootstrap_header(self, ctx: Any) -> None:
        self.logger.info("BAAS dependency bootstrap")
        self.logger.info(f"  variant: {ctx.variant}")
        if self.logger.log_dir is not None:
            self.logger.info(f"  log_dir: {self.logger.log_dir}")
