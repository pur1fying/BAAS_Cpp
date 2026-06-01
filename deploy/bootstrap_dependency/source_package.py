from __future__ import annotations

import hashlib
import http.client
import json
import os
import shutil
import tarfile
import time
import urllib.error
import urllib.parse
import urllib.request
import zipfile
from pathlib import Path
from typing import Any

from .logger import DependencyLogger
from .repository import is_placeholder, provider_path
from .utils import SafeFilesystem


def entry_error_prefix(name: str) -> str:
    return f"dependency: {name};"


class SourcePackageResolver:
    def __init__(self, filesystem: SafeFilesystem, logger: DependencyLogger, archive_download_retry_cnt: int | None = None) -> None:
        self.filesystem = filesystem
        self.logger = logger
        self.archive_download_retry_cnt = archive_download_retry_cnt

    def archive_urls(self, provider_data: dict[str, Any]) -> list[str]:
        urls = provider_data.get("urls")
        if isinstance(urls, list):
            return [str(url) for url in urls if str(url).strip()]
        url = str(provider_data.get("url", "")).strip()
        return [url] if url else []

    def archive_file_name(self, name: str, version: str, provider_data: dict[str, Any]) -> str:
        if provider_data.get("file_name"):
            return str(provider_data["file_name"])
        urls = self.archive_urls(provider_data)
        if urls:
            parsed = urllib.parse.urlparse(urls[0])
            suffix = Path(parsed.path).name
            if suffix:
                return suffix
        return f"{name}-{version}.archive"

    def verify_sha256(self, path: Path, expected: str) -> bool:
        if not path.exists():
            return False
        return self.sha256_file(path).lower() == expected.lower()

    def sha256_file(self, path: Path) -> str:
        digest = hashlib.sha256()
        with path.open("rb") as file:
            for chunk in iter(lambda: file.read(1024 * 1024), b""):
                digest.update(chunk)
        return digest.hexdigest()

    def download_attempts(self) -> int:
        value = self.archive_download_retry_cnt or os.environ.get("BAAS_DEPENDENCY_ARCHIVE_DOWNLOAD_RETRY_CNT") or "3"
        try:
            return max(1, int(value))
        except (TypeError, ValueError):
            return 3

    def download_timeout(self, provider_data: dict[str, Any]) -> int:
        value = provider_data.get("timeout") or os.environ.get("BAAS_DOWNLOAD_TIMEOUT", "120")
        try:
            return max(1, int(value))
        except (TypeError, ValueError):
            return 120

    def archive_lock_stale_seconds(self) -> int:
        value = os.environ.get("BAAS_DEPENDENCY_ARCHIVE_LOCK_STALE_SECONDS", "1800")
        try:
            return max(1, int(value))
        except (TypeError, ValueError):
            return 1800

    def archive_lock_timeout_seconds(self) -> int:
        value = os.environ.get("BAAS_DEPENDENCY_ARCHIVE_LOCK_TIMEOUT_SECONDS", "3600")
        try:
            return max(1, int(value))
        except (TypeError, ValueError):
            return 3600

    def archive_lock_poll_seconds(self) -> float:
        value = os.environ.get("BAAS_DEPENDENCY_ARCHIVE_LOCK_POLL_SECONDS", "1.0")
        try:
            return max(0.01, float(value))
        except (TypeError, ValueError):
            return 1.0

    def format_size(self, size: int) -> str:
        value = float(size)
        for unit in ("B", "KiB", "MiB", "GiB"):
            if value < 1024.0 or unit == "GiB":
                return f"{value:.1f} {unit}" if unit != "B" else f"{int(value)} {unit}"
            value /= 1024.0
        return f"{value:.1f} GiB"

    def format_download_size(self, size: int) -> str:
        return f"{size / (1024 * 1024):.1f}MB"

    def dependency_download_label(self, name: str, version: str) -> str:
        return name

    def download_progress_text(self, label: str, downloaded: int, total: int) -> str:
        total_text = self.format_download_size(total) if total > 0 else "unknown"
        return f"{self.format_download_size(downloaded)} / {total_text} （ {label} ）"

    def report_download_progress(self, progress_log: Path | None, label: str, downloaded: int, total: int) -> None:
        message = "    " + self.download_progress_text(label, downloaded, total)
        self.logger.info(message)
        self.logger.detail(progress_log, message)

    def archive_lock_dir(self, archive_path: Path) -> Path:
        return archive_path.with_name(archive_path.name + ".lock")

    def archive_tmp_path(self, archive_path: Path) -> Path:
        return archive_path.with_suffix(archive_path.suffix + ".tmp")

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

    def write_archive_lock_owner(self, lock_dir: Path, archive_name: str) -> None:
        owner = {
            "pid": os.getpid(),
            "created_at": time.time(),
            "archive": archive_name,
        }
        (lock_dir / "owner.json").write_text(json.dumps(owner, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    def read_archive_lock_owner(self, lock_dir: Path) -> dict[str, Any]:
        try:
            owner = json.loads((lock_dir / "owner.json").read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError):
            return {}
        return owner if isinstance(owner, dict) else {}

    def archive_lock_is_stale(self, lock_dir: Path) -> bool:
        stale_seconds = self.archive_lock_stale_seconds()
        owner = self.read_archive_lock_owner(lock_dir)
        try:
            created_at = float(owner.get("created_at", lock_dir.stat().st_mtime))
        except (OSError, TypeError, ValueError):
            created_at = time.time()
        if time.time() - created_at < stale_seconds:
            return False
        try:
            pid = int(owner.get("pid", 0))
        except (TypeError, ValueError):
            pid = 0
        return not self.process_exists(pid)

    def log_download_wait(self, download_log: Path | None, archive_name: str) -> None:
        message = f"download wait: {archive_name} is being downloaded by another bootstrap process"
        self.logger.info(message)
        self.logger.detail(download_log, message)

    def acquire_archive_lock(self, archive_path: Path, download_log: Path | None) -> Path:
        lock_dir = self.archive_lock_dir(archive_path)
        started_at = time.monotonic()
        last_wait_log = 0.0
        while True:
            try:
                lock_dir.mkdir()
                try:
                    self.write_archive_lock_owner(lock_dir, archive_path.name)
                except OSError:
                    shutil.rmtree(lock_dir, ignore_errors=True)
                    raise
                return lock_dir
            except FileExistsError:
                if self.archive_lock_is_stale(lock_dir):
                    message = f"download stale lock removed: {lock_dir}"
                    self.logger.info(message)
                    self.logger.detail(download_log, message)
                    shutil.rmtree(lock_dir, ignore_errors=True)
                    continue

                now = time.monotonic()
                if now - started_at >= self.archive_lock_timeout_seconds():
                    raise TimeoutError(f"timed out waiting for download lock: {lock_dir}")
                if last_wait_log == 0.0 or now - last_wait_log >= 10.0:
                    self.log_download_wait(download_log, archive_path.name)
                    last_wait_log = now
                time.sleep(self.archive_lock_poll_seconds())

    def release_archive_lock(self, lock_dir: Path) -> None:
        shutil.rmtree(lock_dir, ignore_errors=True)

    def response_content_length(self, headers: Any) -> int:
        total_value = headers.get("Content-Length", "")
        try:
            return int(total_value) if total_value else 0
        except ValueError:
            return 0

    def download_to_temp(self, url: str, tmp_path: Path, provider_data: dict[str, Any]) -> None:
        self.download_to_temp_with_log(url, tmp_path, provider_data, None)

    def download_to_temp_with_log(self, url: str, tmp_path: Path, provider_data: dict[str, Any], progress_log: Path | None, label: str = "archive") -> None:
        request = urllib.request.Request(url, headers={"User-Agent": "BAAS_Cpp dependency bootstrap"})
        timeout = self.download_timeout(provider_data)
        chunk_size = 256 * 1024
        started_at = time.monotonic()
        downloaded = 0
        completed = False

        self.logger.detail(progress_log, f"$ download {url}")
        with urllib.request.urlopen(request, timeout=timeout) as response, tmp_path.open("wb") as out:
            total = self.response_content_length(response.headers)
            progress_step = max(1024 * 1024, total // 100 if total > 0 else 1024 * 1024)
            last_progress_bytes = 0
            last_progress_at = started_at

            while True:
                chunk = response.read(chunk_size)
                if not chunk:
                    break
                out.write(chunk)
                downloaded += len(chunk)

                now = time.monotonic()
                if downloaded - last_progress_bytes >= progress_step or now - last_progress_at >= 1.0:
                    self.report_download_progress(progress_log, label, downloaded, total)
                    last_progress_bytes = downloaded
                    last_progress_at = now

            if downloaded != last_progress_bytes or downloaded == 0:
                self.report_download_progress(progress_log, label, downloaded, total)
            completed = True

        if completed:
            elapsed = max(0.001, time.monotonic() - started_at)
            message = f"    download complete: {self.format_size(downloaded)} in {elapsed:.1f}s （ {label} ）"
            self.logger.info(message)
            self.logger.detail(progress_log, message)

    def download_log_path(self, name: str, version: str) -> Path | None:
        return self.logger.dependency_log_path(name, version, "download")

    def downloaded_file(self, name: str, version: str, provider_data: dict[str, Any], downloads_dir: Path) -> Path:
        expected_sha = str(provider_data.get("sha256", ""))
        if is_placeholder(expected_sha):
            raise ValueError(f"dependency {name} provider is missing locked sha256")

        download_log = self.download_log_path(name, version)
        download_label = self.dependency_download_label(name, version)
        downloads_dir.mkdir(parents=True, exist_ok=True)
        archive_path = downloads_dir / self.archive_file_name(name, version, provider_data)
        if archive_path.exists() and self.verify_sha256(archive_path, expected_sha):
            message = f"download cache hit: {archive_path} （ {download_label} ）"
            self.logger.info(message)
            self.logger.detail(download_log, message)
            return archive_path

        urls = self.archive_urls(provider_data)
        if not urls:
            raise ValueError(f"failed to download archive for {name}: no URL configured")

        lock_dir = self.acquire_archive_lock(archive_path, download_log)
        tmp_path = self.archive_tmp_path(archive_path)
        try:
            if archive_path.exists() and self.verify_sha256(archive_path, expected_sha):
                message = f"download cache hit: {archive_path} （ {download_label} ）"
                self.logger.info(message)
                self.logger.detail(download_log, message)
                return archive_path
            if archive_path.exists():
                message = f"download cache sha256 mismatch: {archive_path}; redownloading （ {download_label} ）"
                self.logger.info(message)
                self.logger.detail(download_log, message)
                archive_path.unlink()

            errors: list[str] = []
            attempts = self.download_attempts()
            for url in urls:
                message = f"download start: {url} （ {download_label} ）"
                self.logger.info(message)
                self.logger.detail(download_log, message)
                for attempt in range(1, attempts + 1):
                    tmp_path.unlink(missing_ok=True)
                    try:
                        self.download_to_temp_with_log(url, tmp_path, provider_data, download_log, download_label)
                        actual_sha = self.sha256_file(tmp_path)
                        if actual_sha.lower() != expected_sha.lower():
                            tmp_path.unlink(missing_ok=True)
                            message = f"{entry_error_prefix(name)} source: {url}; expected sha256: {expected_sha}; actual sha256: {actual_sha}"
                            errors.append(message)
                            self.logger.info(f"    {message}")
                            self.logger.detail(download_log, f"    {message}")
                            if attempt < attempts:
                                retry = f"Retry: {attempt}/{attempts - 1}"
                                self.logger.info(retry)
                                self.logger.detail(download_log, retry)
                            continue
                        tmp_path.replace(archive_path)
                        return archive_path
                    except (OSError, TimeoutError, http.client.IncompleteRead, urllib.error.URLError) as exc:
                        tmp_path.unlink(missing_ok=True)
                        message = f"{url} attempt {attempt}/{attempts}: {type(exc).__name__}: {exc}"
                        errors.append(message)
                        if attempt < attempts:
                            retry = f"Retry: {attempt}/{attempts - 1} after {type(exc).__name__}: {exc}"
                            self.logger.info(retry)
                            self.logger.detail(download_log, retry)
                        else:
                            failure = f"    download failed: {type(exc).__name__}: {exc}"
                            self.logger.info(failure)
                            self.logger.detail(download_log, failure)
            raise ValueError(f"failed to download archive for {name}: " + "; ".join(errors))
        finally:
            try:
                tmp_path.unlink(missing_ok=True)
            except OSError as exc:
                self.logger.detail(download_log, f"    warning: failed to remove temporary download file {tmp_path}: {exc}")
            self.release_archive_lock(lock_dir)

    def source_archive(self, name: str, version: str, provider_data: dict[str, Any], downloads_dir: Path) -> Path:
        return self.downloaded_file(name, version, provider_data, downloads_dir)

    def local_archive(self, name: str, provider_data: dict[str, Any]) -> Path:
        path_value = provider_path(provider_data)
        if not path_value:
            env_name = provider_data.get("path_env") or provider_data.get("path_var")
            raise ValueError(f"dependency {name} requires environment variable {env_name}")
        archive_path = Path(path_value).resolve()
        expected_sha = str(provider_data.get("sha256", ""))
        if not archive_path.exists():
            raise ValueError(f"local archive does not exist: {archive_path}")
        if is_placeholder(expected_sha):
            raise ValueError(f"dependency {name} local archive provider is missing locked sha256")
        actual_sha = self.sha256_file(archive_path)
        if actual_sha.lower() != expected_sha.lower():
            raise ValueError(
                f"{entry_error_prefix(name)} source: {archive_path}; expected sha256: {expected_sha}; actual sha256: {actual_sha}"
            )
        return archive_path

    def local_source_dir(self, name: str, provider_data: dict[str, Any]) -> Path:
        path_value = provider_path(provider_data)
        if not path_value:
            env_name = provider_data.get("path_env") or provider_data.get("path_var")
            raise ValueError(f"dependency {name} requires environment variable {env_name}")
        source_dir = Path(path_value).resolve()
        if not (source_dir / "CMakeLists.txt").exists():
            raise ValueError(f"local source directory must contain CMakeLists.txt: {source_dir}")
        return source_dir

    def prepare_source(self, name: str, entry: Any, provider_data: dict[str, Any]) -> Path:
        data = entry.to_dict() if hasattr(entry, "to_dict") else dict(entry)
        provider_type = data["provider_type"]
        if provider_type == "local_source":
            return self.local_source_dir(name, provider_data)
        if provider_type == "local_archive":
            return self.local_archive(name, provider_data)
        return self.source_archive(name, data["version"], provider_data, Path(data["downloads"]))

    def prepare_download(self, name: str, entry: Any, provider_data: dict[str, Any]) -> Path:
        data = entry.to_dict() if hasattr(entry, "to_dict") else dict(entry)
        provider_type = data["provider_type"]
        if provider_type == "local_archive":
            return self.local_archive(name, provider_data)
        return self.downloaded_file(name, data["version"], provider_data, Path(data["downloads"]))

    def extract_archive(self, archive_path: Path, provider_data: dict[str, Any], source_dir: Path, build_root: Path) -> None:
        archive_type = str(provider_data.get("archive_type", "tar.gz"))
        strip_prefix = str(provider_data.get("strip_prefix", ""))
        temp_dir = source_dir.parent / (source_dir.name + ".extract")
        self.filesystem.safe_rmtree(temp_dir, build_root)
        self.filesystem.safe_rmtree(source_dir, build_root)
        temp_dir.mkdir(parents=True, exist_ok=True)

        if archive_type in {"tar.gz", "tgz"}:
            with tarfile.open(archive_path, "r:gz") as archive:
                try:
                    archive.extractall(temp_dir, filter="data")
                except TypeError:
                    archive.extractall(temp_dir)
        elif archive_type == "zip":
            with zipfile.ZipFile(archive_path) as archive:
                archive.extractall(temp_dir)
        else:
            raise ValueError(f"unsupported archive_type for source dependency: {archive_type}")

        extracted_root = temp_dir / strip_prefix if strip_prefix else None
        if extracted_root is None or not extracted_root.exists():
            children = [path for path in temp_dir.iterdir() if path.name != source_dir.name]
            if len(children) != 1:
                raise ValueError(f"cannot determine extracted source root in {temp_dir}")
            extracted_root = children[0]
        source_dir.parent.mkdir(parents=True, exist_ok=True)
        shutil.move(str(extracted_root), str(source_dir))
        self.filesystem.safe_rmtree(temp_dir, build_root)

    def copy_stage_items(self, source_dir: Path, package_dir: Path, items: list[dict[str, Any]]) -> None:
        for item in items:
            source_pattern = str(item.get("from", "")).strip()
            target_value = str(item.get("to", "")).strip()
            if not source_pattern or not target_value:
                raise ValueError("staging copy items require non-empty from and to fields")

            matches = list(source_dir.glob(source_pattern)) if any(char in source_pattern for char in "*?[]") else [source_dir / source_pattern]
            if not matches:
                raise ValueError(f"staging source does not exist: {source_dir / source_pattern}")

            target_root = package_dir / target_value
            for source_path in matches:
                if source_path.is_dir():
                    if len(matches) == 1:
                        destination = target_root
                    else:
                        destination = target_root / source_path.name
                    shutil.copytree(source_path, destination, dirs_exist_ok=True)
                else:
                    destination = target_root
                    if len(matches) > 1 or target_value.endswith("/") or target_value.endswith("\\") or not destination.suffix:
                        destination = target_root / source_path.name
                    destination.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copy2(source_path, destination)

    def copy_single_file(self, file_path: Path, package_dir: Path, target_path: str) -> None:
        destination = package_dir / target_path
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(file_path, destination)
