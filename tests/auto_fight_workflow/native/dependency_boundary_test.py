#!/usr/bin/env python3
"""Build BAAS workflow headers and Core with runtime files physically absent."""

import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path


def run(command):
    result = subprocess.run(command, text=True, capture_output=True)
    if result.returncode:
        raise RuntimeError(result.stdout + result.stderr)


def main():
    root, build = (Path(arg).resolve() for arg in sys.argv[1:3])
    compiler, generator, include_list, configuration, make_program, source_list = sys.argv[3:]
    json_include = next(Path(value) for value in include_list.split("|")
                        if (Path(value) / "nlohmann/json.hpp").is_file())
    public = root / "apps/BAAS/include/module/auto_fight/workflow"
    private = root / "apps/BAAS/src/module/auto_fight/workflow"
    # Copy compiled C++ sources, excluding CMake's generated .rule metadata.
    sources = [Path(value) for value in source_list.split("|") if Path(value).suffix == ".cpp"]
    deferred_headers = {"Definition.h", "RuleCatalog.h", "SchemaService.h"}
    public_headers = [path for path in public.glob("*.h") if path.name not in deferred_headers]
    private_headers = list(private.glob("*.h"))
    headers = public_headers + private_headers
    for path in sources + headers:
        text = path.read_text(encoding="utf-8")
        for include in re.findall(r'^#include\s+[<"]([^>"]+)', text, re.M):
            assert not any(word in include for word in ("opencv", "onnx", "BAASConfig", "BAASLogger")), (path, include)
            if include.startswith("module/"):
                assert include.startswith("module/auto_fight/workflow/"), (path, include)
        if path.suffix == ".h":
            assert "#pragma once" not in text
            assert "#ifndef BAAS_CPP_" in text
    parser_header = (public / "WorkflowParser.h").read_text(encoding="utf-8")
    assert "filesystem" not in parser_header and "FileParser" not in parser_header
    with tempfile.TemporaryDirectory(prefix="core-isolation-", dir=build) as directory:
        isolated = Path(directory).resolve()
        assert isolated.is_relative_to(build)
        target_include = isolated / "include/module/auto_fight/workflow"
        target_include.mkdir(parents=True)
        for path in public_headers:
            shutil.copy2(path, target_include / path.name)
        shutil.copy2(root / "include/core_defines.h", isolated / "include/core_defines.h")
        shutil.copytree(json_include / "nlohmann", isolated / "include/nlohmann")
        target_src = isolated / "src"
        target_src.mkdir()
        for path in sources + private_headers:
            shutil.copy2(path, target_src / path.name)
        probes = []
        for i, path in enumerate(sorted(target_include.glob("*.h"))):
            probe = isolated / ("header_" + str(i) + ".cpp")
            probe.write_text('#define ERROR 0\n#include "module/auto_fight/workflow/' + path.name + '"\n', encoding="utf-8")
            probes.append(probe.name)
        cpp = " ".join('src/' + p.name for p in target_src.glob("*.cpp"))
        cmake = (
            "cmake_minimum_required(VERSION 3.22)\nproject(CoreIsolation LANGUAGES CXX)\n"
            "set(CMAKE_CXX_STANDARD 20)\nset(CMAKE_CXX_STANDARD_REQUIRED ON)\n"
            "add_library(core STATIC " + cpp + ")\n"
            "target_include_directories(core PUBLIC include)\n"
            "add_library(headers OBJECT " + " ".join(probes) + ")\n"
            "target_include_directories(headers PRIVATE include)\n"
            "if(MSVC)\n target_compile_options(core PRIVATE /utf-8)\n"
            " target_compile_options(headers PRIVATE /utf-8)\nendif()\n"
        )
        (isolated / "CMakeLists.txt").write_text(cmake, encoding="utf-8")
        run(["cmake", "-S", str(isolated), "-B", str(isolated / "out"), "-G", generator,
             "-DCMAKE_CXX_COMPILER=" + compiler, "-DCMAKE_BUILD_TYPE=" + configuration,
             "-DCMAKE_MAKE_PROGRAM=" + make_program])
        run(["cmake", "--build", str(isolated / "out"), "--config", configuration, "--parallel", "2"])
    print("Core and every public header compile without BAAS runtime files or libraries.")


if __name__ == "__main__":
    main()
