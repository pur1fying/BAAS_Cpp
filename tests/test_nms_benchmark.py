from __future__ import annotations

import os
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


class NmsBenchmarkIntegrationTests(unittest.TestCase):
    def test_nms_benchmark_matches_opencv_dnn(self) -> None:
        if os.environ.get("BAAS_RUN_NMS_BENCHMARK_TEST") != "1":
            self.skipTest("set BAAS_RUN_NMS_BENCHMARK_TEST=1 to build and run the NMS benchmark integration test")
        if not sys.platform.startswith("win"):
            self.skipTest("the opencv_dnn lock entry currently defines Windows runtime outputs")
        if not shutil.which("cmake"):
            self.skipTest("cmake executable not found")

        repo_root = Path(__file__).resolve().parents[1]
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            workspace = root / ".baas"
            build_dir = root / "build"
            env = os.environ.copy()
            env["BAAS_WORKSPACE_ROOT"] = str(workspace)

            self.run_command(
                [
                    sys.executable,
                    "-m",
                    "deploy.bootstrap_dependency",
                    "--dependencies",
                    "opencv_dnn,benchmark",
                    "--build-type",
                    "Release",
                    "--platform",
                    "windows",
                    "--arch",
                    "x64",
                ],
                repo_root,
                env,
            )
            self.run_command(
                [
                    "cmake",
                    "-S",
                    str(repo_root),
                    "-B",
                    str(build_dir),
                    "-DBUILD_BAAS_NMS_BENCHMARK=ON",
                    "-DCMAKE_BUILD_TYPE=Release",
                    f"-DBAAS_WORKSPACE_ROOT={workspace}",
                    f"-DBAAS_PYTHON_EXECUTABLE={sys.executable}",
                ],
                repo_root,
                env,
            )
            self.run_command(
                [
                    "cmake",
                    "--build",
                    str(build_dir),
                    "--target",
                    "baas_nms_benchmark",
                    "--build-type",
                    "Release",
                ],
                repo_root,
                env,
            )

            executable = self.find_one(build_dir / "bin", "baas_nms_benchmark.exe")
            opencv_runtime = self.find_one(workspace / "dependency" / "opencv_dnn", "opencv_world490.dll")
            shutil.copy2(opencv_runtime, executable.parent / opencv_runtime.name)

            result = self.run_command(
                [
                    str(executable),
                    "--benchmark_min_time=0.001",
                    "--benchmark_repetitions=1",
                ],
                executable.parent,
                env,
            )
            self.assertIn("BAAS NMS matches OpenCV dnn::NMSBoxes", result.stdout)

    def run_command(self, command: list[str], cwd: Path, env: dict[str, str]) -> subprocess.CompletedProcess[str]:
        result = subprocess.run(command, cwd=cwd, env=env, text=True, capture_output=True, check=False)
        if result.returncode != 0:
            self.fail(
                "command failed: "
                + " ".join(command)
                + "\nstdout:\n"
                + result.stdout
                + "\nstderr:\n"
                + result.stderr
            )
        return result

    def find_one(self, root: Path, pattern: str) -> Path:
        matches = list(root.rglob(pattern))
        if len(matches) != 1:
            self.fail(f"expected exactly one {pattern} under {root}, found {len(matches)}")
        return matches[0]


if __name__ == "__main__":
    unittest.main()
