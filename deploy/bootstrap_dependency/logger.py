from __future__ import annotations

import sys
import threading
import re
from dataclasses import dataclass
from dataclasses import field
from datetime import datetime
from pathlib import Path
from typing import TextIO


@dataclass
class DependencyLogger:
    log_dir: Path | None
    out: TextIO = field(default_factory=lambda: sys.stdout)
    err: TextIO = field(default_factory=lambda: sys.stderr)
    _progress_width: int = 0
    _lock: threading.Lock = field(default_factory=threading.Lock, repr=False)

    @classmethod
    def create(cls, repo_root: Path) -> "DependencyLogger":
        timestamp = datetime.now().strftime("%Y-%m-%d_%H.%M.%S")
        log_dir = repo_root / "output" / "log" / "dependency" / timestamp
        log_dir.mkdir(parents=True, exist_ok=True)
        return cls(log_dir=log_dir)

    @classmethod
    def stderr_only(cls) -> "DependencyLogger":
        return cls(log_dir=None)

    def info(self, message: str) -> None:
        self._write(message, self.out)

    def error(self, message: str) -> None:
        self._write(message, self.err)

    def progress(self, message: str) -> None:
        with self._lock:
            self._progress_width = max(self._progress_width, len(message))
            self.out.write("\r" + message.ljust(self._progress_width))
            self.out.flush()

    def clear_progress(self) -> None:
        with self._lock:
            self._clear_progress_unlocked()

    def command_log_path(self, name: str) -> Path:
        if self.log_dir is None:
            raise ValueError("dependency logger has no log directory")
        self.log_dir.mkdir(parents=True, exist_ok=True)
        return self.log_dir / name

    def dependency_log_path(self, *parts: str) -> Path | None:
        if self.log_dir is None:
            return None
        safe_parts = [self.safe_name(part) for part in parts if part]
        if not safe_parts:
            safe_parts = ["dependency"]
        self.log_dir.mkdir(parents=True, exist_ok=True)
        return self.log_dir / ("-".join(safe_parts) + ".log")

    def detail(self, log_file: Path | None, message: str) -> None:
        if log_file is None:
            return
        log_file.parent.mkdir(parents=True, exist_ok=True)
        with log_file.open("a", encoding="utf-8") as log:
            log.write(message + "\n")

    def safe_name(self, value: str) -> str:
        cleaned = re.sub(r"[^A-Za-z0-9_.-]+", "-", value.strip())
        return cleaned.strip("-") or "item"

    def _write(self, message: str, stream: TextIO) -> None:
        with self._lock:
            if stream is self.out:
                self._clear_progress_unlocked()
            stream.write(message + "\n")
            stream.flush()
            if self.log_dir is not None:
                self.log_dir.mkdir(parents=True, exist_ok=True)
                with (self.log_dir / "bootstrap.log").open("a", encoding="utf-8") as log:
                    log.write(message + "\n")

    def _clear_progress_unlocked(self) -> None:
        if self._progress_width <= 0:
            return
        self.out.write("\r" + (" " * self._progress_width) + "\r")
        self.out.flush()
        self._progress_width = 0
