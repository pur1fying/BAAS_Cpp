from __future__ import annotations

import json
import os
import shutil
import time
from pathlib import Path
from typing import Any


class ProcessDirectoryLock:
    def __init__(
        self,
        lock_dir: Path,
        *,
        owner: dict[str, Any],
        env_prefix: str,
        wait_message: str,
        stale_message: str,
        timeout_message: str,
        logger: Any = None,
        default_stale_seconds: int = 1800,
        default_timeout_seconds: int = 3600,
        default_poll_seconds: float = 1.0,
    ) -> None:
        self.lock_dir = lock_dir
        self.owner = dict(owner)
        self.env_prefix = env_prefix
        self.wait_message = wait_message
        self.stale_message = stale_message
        self.timeout_message = timeout_message
        self.logger = logger
        self.default_stale_seconds = default_stale_seconds
        self.default_timeout_seconds = default_timeout_seconds
        self.default_poll_seconds = default_poll_seconds
        self.acquired = False

    def env_int(self, suffix: str, default: int) -> int:
        value = os.environ.get(f"{self.env_prefix}_{suffix}")
        try:
            return max(1, int(value)) if value is not None else default
        except (TypeError, ValueError):
            return default

    def env_float(self, suffix: str, default: float) -> float:
        value = os.environ.get(f"{self.env_prefix}_{suffix}")
        try:
            return max(0.01, float(value)) if value is not None else default
        except (TypeError, ValueError):
            return default

    def stale_seconds(self) -> int:
        return self.env_int("STALE_SECONDS", self.default_stale_seconds)

    def timeout_seconds(self) -> int:
        return self.env_int("TIMEOUT_SECONDS", self.default_timeout_seconds)

    def poll_seconds(self) -> float:
        return self.env_float("POLL_SECONDS", self.default_poll_seconds)

    def log(self, message: str) -> None:
        if self.logger is not None:
            self.logger.info(message)

    def owner_path(self) -> Path:
        return self.lock_dir / "owner.json"

    def write_owner(self) -> None:
        payload = {
            "pid": os.getpid(),
            "created_at": time.time(),
            **self.owner,
        }
        self.owner_path().write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    def read_owner(self) -> dict[str, Any]:
        try:
            owner = json.loads(self.owner_path().read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError):
            return {}
        return owner if isinstance(owner, dict) else {}

    def process_exists(self, pid: int) -> bool:
        if pid <= 0:
            return False
        if os.name == "nt":
            import ctypes

            kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
            process_query_limited_information = 0x1000
            handle = kernel32.OpenProcess(process_query_limited_information, False, pid)
            if handle:
                kernel32.CloseHandle(handle)
                return True
            return ctypes.get_last_error() == 5
        try:
            os.kill(pid, 0)
        except ProcessLookupError:
            return False
        except PermissionError:
            return True
        except OSError:
            return False
        return True

    def is_stale(self) -> bool:
        owner = self.read_owner()
        try:
            created_at = float(owner.get("created_at", self.lock_dir.stat().st_mtime))
        except (OSError, TypeError, ValueError):
            created_at = time.time()
        if time.time() - created_at < self.stale_seconds():
            return False
        try:
            pid = int(owner.get("pid", 0))
        except (TypeError, ValueError):
            pid = 0
        return not self.process_exists(pid)

    def acquire(self) -> "ProcessDirectoryLock":
        self.lock_dir.parent.mkdir(parents=True, exist_ok=True)
        started_at = time.monotonic()
        last_wait_log = 0.0
        while True:
            try:
                self.lock_dir.mkdir()
                try:
                    self.write_owner()
                except OSError:
                    shutil.rmtree(self.lock_dir, ignore_errors=True)
                    raise
                self.acquired = True
                return self
            except FileExistsError:
                if self.is_stale():
                    self.log(f"{self.stale_message}: {self.lock_dir}")
                    shutil.rmtree(self.lock_dir, ignore_errors=True)
                    continue

                now = time.monotonic()
                if now - started_at >= self.timeout_seconds():
                    raise TimeoutError(f"{self.timeout_message}: {self.lock_dir}")
                if last_wait_log == 0.0 or now - last_wait_log >= 10.0:
                    self.log(self.wait_message)
                    last_wait_log = now
                time.sleep(self.poll_seconds())

    def release(self) -> None:
        if self.acquired:
            shutil.rmtree(self.lock_dir, ignore_errors=True)
            self.acquired = False

    def __enter__(self) -> "ProcessDirectoryLock":
        return self.acquire()

    def __exit__(self, exc_type: Any, exc: Any, tb: Any) -> None:
        self.release()
