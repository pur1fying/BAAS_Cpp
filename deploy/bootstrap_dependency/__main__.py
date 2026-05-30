from __future__ import annotations

import argparse
import json
import sys

from .bootstrapper import BootstrapService, clean
from .cmake import CMakeBuilder
from .cmake_index import CMakeDependencyIndex
from .concurrency import ResourcePlanner as BootstrapResourcePlanner
from .context import BootstrapContext
from .json_store import JsonStore
from .logger import DependencyLogger
from .locks import LockRepository
from .outputs import OutputResolver, OutputValidator
from .planner import DependencyPlanner, PlanBuilder, ResourcePlanner
from .repository import PathResolver, RepoPaths, StateStore
from .reporter import Reporter
from .source_package import SourcePackageResolver
from .templating import TemplateExpander
from .utils import CommandRunner, SafeFilesystem


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Plan, verify, or bootstrap BAAS dependencies.")
    parser.add_argument("--all", action="store_true", help="Plan all dependencies and resources.")
    parser.add_argument("--dependency", "--dependencies", dest="dependency", help="Comma-separated dependency names, or all.")
    parser.add_argument("--deps", dest="dependency", help=argparse.SUPPRESS)
    parser.add_argument("--resources", help="Comma-separated resource names, or all.")
    parser.add_argument("--provider", help="Override provider for selected dependencies.")
    parser.add_argument("--config", default="Release", choices=("Debug", "Release", "debug", "release"))
    parser.add_argument("--platform", choices=("windows", "linux", "macos", "android", "Windows", "Linux", "MacOS", "Android"))
    parser.add_argument("--arch", help="Target architecture, for example x64 or arm64.")
    parser.add_argument("--android-abi", help="Android ABI, for example arm64-v8a.")
    parser.add_argument("--compiler-id", help="Compiler id used in the variant and fingerprint.")
    parser.add_argument("--compiler-version", help="Compiler version used in the fingerprint.")
    parser.add_argument("--toolchain-file", help="Toolchain file path used in the fingerprint.")
    parser.add_argument("--print-plan", action="store_true", help="Print dependency/resource plan.")
    parser.add_argument("--verify-only", action="store_true", help="Check state and required outputs only.")
    parser.add_argument("--clean", help="Clean one or more dependency/resource package roots under BAAS_LOCAL_ROOT.")
    parser.add_argument("--verbose", action="store_true", help="Print required output groups.")
    parser.add_argument(
        "--jobs",
        default="auto",
        help="CMake build parallelism per dependency. Pass N for cmake --build ... -j N, or auto.",
    )
    parser.add_argument("--download-jobs", default="auto", help="Concurrent artifact downloads across dependencies, or auto.")
    parser.add_argument("--stage-jobs", default="auto", help="Parallelism for lightweight unpack/stage/prebuilt/header packaging work, or auto.")
    parser.add_argument("--cpu-budget", default="auto", help="CPU budget used to derive auto job counts, or auto.")
    parser.add_argument("--command-timeout", type=int, default=600, help="Seconds before an external configure/build/package command is stopped.")
    parser.add_argument(
        "--archive-download-retry-cnt",
        dest="archive_download_retry_cnt",
        type=int,
        help="Global archive download attempts per URL. Overrides BAAS_DEPENDENCY_ARCHIVE_DOWNLOAD_RETRY_CNT.",
    )
    parser.add_argument("--retry-cnt", dest="archive_download_retry_cnt", type=int, help=argparse.SUPPRESS)
    return parser.parse_args(argv)


class BootstrapApp:
    def __init__(self, repo_paths: RepoPaths | None = None) -> None:
        self.repo_paths = repo_paths

    def run(self, args: argparse.Namespace) -> int:
        logger: DependencyLogger | None = None
        try:
            repo_paths = self.repo_paths or RepoPaths.discover()
            logger = DependencyLogger.create(repo_paths.root)
            json_store = JsonStore()
            expander = TemplateExpander()
            path_resolver = PathResolver(expander)
            output_resolver = OutputResolver(expander)
            output_validator = OutputValidator()
            cmake_index = CMakeDependencyIndex()
            filesystem = SafeFilesystem()
            state_store = StateStore(json_store)
            lock_repo = LockRepository(repo_paths, json_store)
            dependency_planner = DependencyPlanner(output_resolver, output_validator, path_resolver, state_store)
            resource_planner = ResourcePlanner(output_resolver, output_validator, path_resolver, state_store)
            plan_builder = PlanBuilder(lock_repo, state_store, dependency_planner, resource_planner)
            reporter = Reporter(logger)
            ctx = BootstrapContext.from_args(args, repo_paths)
            resource_plan = BootstrapResourcePlanner().compute(args)

            if args.clean:
                code = clean(args, ctx, filesystem, logger)
                if code == 0:
                    cmake_index.remove(ctx)
                return code
            if args.verify_only:
                code = reporter.verify_only(args, ctx, repo_paths, plan_builder)
                if code == 0:
                    index_path = cmake_index.write(ctx, plan_builder.build(args, ctx))
                    logger.info(f"wrote CMake dependency index: {index_path}")
                return code
            if args.print_plan or (not args.all and not args.dependency and not args.resources):
                return reporter.print_plan(args, ctx, repo_paths, plan_builder)

            command_runner = CommandRunner(logger)
            source_resolver = SourcePackageResolver(filesystem, logger, archive_download_retry_cnt=args.archive_download_retry_cnt)
            cmake_builder = CMakeBuilder(command_runner, expander)
            bootstrap_service = BootstrapService(
                plan_builder,
                lock_repo,
                source_resolver,
                cmake_builder,
                state_store,
                output_validator,
                filesystem,
                reporter,
                expander,
                logger,
                resource_plan,
            )
            code = bootstrap_service.run(args, ctx)
            if code == 0:
                index_path = cmake_index.write(ctx, plan_builder.build(args, ctx))
                logger.info(f"wrote CMake dependency index: {index_path}")
            return code
        except (KeyError, ValueError, OSError, json.JSONDecodeError) as exc:
            (logger or DependencyLogger.stderr_only()).error(f"error: {exc}")
            return 2
        except KeyboardInterrupt:
            (logger or DependencyLogger.stderr_only()).error("interrupted by user")
            return 130


def main(argv: list[str] | None = None) -> int:
    return BootstrapApp().run(parse_args(argv))


if __name__ == "__main__":
    sys.exit(main())
