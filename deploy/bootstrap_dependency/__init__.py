"""BAAS dependency bootstrap package."""

from __future__ import annotations

from typing import Any


def main(argv: list[str] | None = None) -> int:
    from .__main__ import main as entry_main

    return entry_main(argv)


def parse_args(argv: list[str] | None = None) -> Any:
    from .__main__ import parse_args as entry_parse_args

    return entry_parse_args(argv)


__all__ = ["main", "parse_args"]
