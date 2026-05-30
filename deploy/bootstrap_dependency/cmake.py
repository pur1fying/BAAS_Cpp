from __future__ import annotations

import os
import shutil
from pathlib import Path
from typing import Any, Callable

from .concurrency import WeightedSemaphore
from .utils import CommandRunner
from .templating import TemplateExpander


class CMakeBuilder:
    def __init__(self, command_runner: CommandRunner, expander: TemplateExpander) -> None:
        self.command_runner = command_runner
        self.expander = expander

    def cmake_generator_args(self) -> list[str]:
        generator = os.environ.get("BAAS_CMAKE_GENERATOR", "").strip()
        make_program = os.environ.get("BAAS_CMAKE_MAKE_PROGRAM", "").strip()
        if generator:
            args = ["-G", generator]
            if make_program:
                args.append(f"-DCMAKE_MAKE_PROGRAM={make_program}")
            return args

        ninja = shutil.which("ninja") or shutil.which("ninja.exe") or self.find_visual_studio_ninja()
        if ninja:
            return ["-G", "Ninja", f"-DCMAKE_MAKE_PROGRAM={ninja}"]
        return []

    def find_visual_studio_ninja(self) -> str:
        cl_path = shutil.which("cl") or shutil.which("cl.exe")
        if not cl_path:
            return ""
        for parent in Path(cl_path).resolve().parents:
            candidate = parent / "Common7" / "IDE" / "CommonExtensions" / "Microsoft" / "CMake" / "Ninja" / "ninja.exe"
            if candidate.exists():
                return str(candidate)
        return ""

    def option_args(
        self,
        options: dict[str, Any],
        ctx: Any,
        name: str,
        version: str,
        provider: str,
        package_dir: Path,
        source_dir: Path,
        build_dir: Path,
    ) -> list[str]:
        args: list[str] = []
        extra_vars = {
            "package": str(package_dir),
            "package_dir": str(package_dir),
            "source": str(source_dir),
            "source_dir": str(source_dir),
            "build": str(build_dir),
            "build_dir": str(build_dir),
        }
        for key, value in options.items():
            expanded = self.expander.expand(str(value), ctx, name, version, provider)
            for template_name, template_value in extra_vars.items():
                expanded = expanded.replace("${" + template_name + "}", template_value)
            args.append(f"-D{key}={expanded}")
        return args

    def build_source_package(
        self,
        name: str,
        version: str,
        provider: str,
        ctx: Any,
        configure_source_dir: Path,
        build_dir: Path,
        package_dir: Path,
        cmake_options: dict[str, Any],
        jobs: int | None,
        timeout_seconds: int,
        cpu_semaphore: WeightedSemaphore | None = None,
        cpu_tokens: int = 1,
        phase_callback: Callable[[str, str, Path], None] | None = None,
    ) -> None:
        log_file = self.command_runner.logger.command_log_path(f"{name}-{ctx.variant}-{provider}.log")
        if log_file.exists():
            log_file.unlink()

        configure = ["cmake", "-S", str(configure_source_dir), "-B", str(build_dir)]
        configure.extend(self.cmake_generator_args())
        configure.extend(self.option_args(cmake_options, ctx, name, version, provider, package_dir, configure_source_dir, build_dir))
        if ctx.toolchain_file and "CMAKE_TOOLCHAIN_FILE" not in cmake_options:
            configure.append(f"-DCMAKE_TOOLCHAIN_FILE={ctx.toolchain_file}")
        self.run_phase("Configure", configure, log_file, timeout_seconds, phase_callback)

        build_command = ["cmake", "--build", str(build_dir), "--config", self.expander.cmake_config_name(ctx)]
        if jobs:
            build_command.extend(["-j", str(jobs)])
        if cpu_semaphore is not None:
            cpu_semaphore.acquire(cpu_tokens)
        try:
            self.run_phase("Build", build_command, log_file, timeout_seconds, phase_callback)
        finally:
            if cpu_semaphore is not None:
                cpu_semaphore.release(cpu_tokens)
        self.run_phase(
            "Install",
            ["cmake", "--install", str(build_dir), "--config", self.expander.cmake_config_name(ctx)],
            log_file,
            timeout_seconds,
            phase_callback,
        )

    def run_phase(
        self,
        phase: str,
        command: list[str],
        log_file: Path,
        timeout_seconds: int,
        phase_callback: Callable[[str, str, Path], None] | None,
    ) -> None:
        if phase_callback:
            phase_callback(phase, "running", log_file)
        try:
            self.command_runner.run(command, log_file, timeout_seconds)
        except Exception:
            if phase_callback:
                phase_callback(phase, "failed", log_file)
            raise
        if phase_callback:
            phase_callback(phase, "success", log_file)
