from __future__ import annotations

import os
import shutil
import subprocess
import threading
import time
from pathlib import Path

from .logger import DependencyLogger


class SafeFilesystem:
    def ensure_inside(self, child: Path, parent: Path) -> None:
        child_resolved = child.resolve()
        parent_resolved = parent.resolve()
        try:
            child_resolved.relative_to(parent_resolved)
        except ValueError as exc:
            raise ValueError(f"refusing to operate outside BAAS_LOCAL_ROOT: {child_resolved}") from exc

    def safe_rmtree(self, path: Path, allowed_root: Path) -> None:
        self.ensure_inside(path, allowed_root)
        if path.resolve() == allowed_root.resolve():
            raise ValueError(f"refusing to remove root directory: {path}")
        if path.exists():
            shutil.rmtree(path)


class CommandRunner:
    def __init__(self, logger: DependencyLogger) -> None:
        self.logger = logger

    def terminate_process_tree(self, process: subprocess.Popen[str]) -> None:
        if os.name == "nt":
            subprocess.run(
                ["taskkill", "/F", "/T", "/PID", str(process.pid)],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
                check=False,
            )
        else:
            process.kill()

    def process_environment(self) -> dict[str, str]:
        env = os.environ.copy()
        if os.name == "nt":
            self.add_msvc_environment(env)
        return env

    def add_msvc_environment(self, env: dict[str, str]) -> None:
        cl_path = shutil.which("cl") or shutil.which("cl.exe")
        if not cl_path:
            return

        cl_exe = Path(cl_path).resolve()
        msvc_root = None
        for parent in cl_exe.parents:
            if (parent / "include").is_dir() and (parent / "lib" / "x64").is_dir():
                msvc_root = parent
                break
        if msvc_root is None:
            return

        sdk_root, sdk_version = self.find_windows_sdk()
        include_paths = [msvc_root / "include"]
        lib_paths = [msvc_root / "lib" / "x64"]
        path_prefixes = [cl_exe.parent]

        if sdk_root and sdk_version:
            sdk_include = sdk_root / "Include" / sdk_version
            sdk_lib = sdk_root / "Lib" / sdk_version
            include_paths.extend(
                [
                    sdk_include / "ucrt",
                    sdk_include / "um",
                    sdk_include / "shared",
                    sdk_include / "winrt",
                    sdk_include / "cppwinrt",
                ]
            )
            lib_paths.extend([sdk_lib / "ucrt" / "x64", sdk_lib / "um" / "x64"])
            path_prefixes.append(sdk_root / "bin" / sdk_version / "x64")

        self.prepend_env_paths(env, "INCLUDE", include_paths)
        self.prepend_env_paths(env, "LIB", lib_paths)
        self.prepend_env_paths(env, "PATH", path_prefixes)

    def find_windows_sdk(self) -> tuple[Path | None, str]:
        candidates = [
            Path(os.environ.get("WindowsSdkDir", "")),
            Path("D:/Windows Kits/10"),
            Path("C:/Program Files (x86)/Windows Kits/10"),
        ]
        for root in candidates:
            if not root or not root.exists():
                continue
            lib_root = root / "Lib"
            if not lib_root.exists():
                continue
            versions = sorted(
                (path.name for path in lib_root.iterdir() if (path / "um" / "x64" / "kernel32.lib").exists()),
                reverse=True,
            )
            if versions:
                return root, versions[0]
        return None, ""

    def prepend_env_paths(self, env: dict[str, str], key: str, paths: list[Path]) -> None:
        existing = env.get(key, "")
        values = [str(path) for path in paths if path.exists()]
        if existing:
            values.append(existing)
        env[key] = os.pathsep.join(values)

    def run(self, command: list[str], log_file: Path, timeout_seconds: int) -> None:
        log_file.parent.mkdir(parents=True, exist_ok=True)
        self.logger.info("run " + " ".join(command))
        with log_file.open("a", encoding="utf-8") as log:
            log.write("$ " + " ".join(command) + "\n")
            log.flush()
            creationflags = subprocess.CREATE_NEW_PROCESS_GROUP if os.name == "nt" else 0
            process = subprocess.Popen(
                command,
                stdin=subprocess.DEVNULL,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                errors="replace",
                bufsize=1,
                creationflags=creationflags,
                env=self.process_environment(),
            )

            def stream_output() -> None:
                if process.stdout is None:
                    return
                try:
                    for line in process.stdout:
                        log.write(line)
                        log.flush()
                finally:
                    process.stdout.close()

            output_thread = threading.Thread(target=stream_output, daemon=True)
            output_thread.start()
            try:
                deadline = time.monotonic() + timeout_seconds
                while True:
                    try:
                        return_code = process.wait(timeout=0.2)
                        break
                    except subprocess.TimeoutExpired as exc:
                        if time.monotonic() >= deadline:
                            self.terminate_process_tree(process)
                            output_thread.join(timeout=5)
                            raise RuntimeError(f"command timed out after {timeout_seconds} seconds; see log: {log_file}") from exc
            except KeyboardInterrupt:
                self.logger.info("command interrupted by user; terminating process tree...")
                self.terminate_process_tree(process)
                output_thread.join(timeout=5)
                raise
            output_thread.join(timeout=5)
        if return_code != 0:
            raise RuntimeError(f"command failed with exit code {return_code}; see log: {log_file}")
