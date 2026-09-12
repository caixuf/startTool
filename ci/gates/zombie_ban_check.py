#!/usr/bin/env python3
"""Ban reintroduction of retired Python flowboard_server (replaced by flowmond).

Scans the repo for:
  - orphaned flowboard_server*.pyc
  - source/docs/scripts referencing flowboard_server as a runnable path

Skips: tools/flowboard/ (live frontend), build/, vendor/, node_modules/,
and this gate file itself.
"""

from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

SELF = Path(__file__).resolve()


def skip_dir(path: Path) -> bool:
    parts = set(path.parts)
    if parts & {".git", "node_modules", "vendor", "third_party"}:
        return True
    if any(p.startswith("build") for p in path.parts):
        return True
    rel = path.relative_to(ROOT).as_posix()
    if rel.startswith("tools/flowboard/"):
        return True
    return False


def main() -> int:
    errors: list[str] = []

    for pyc in ROOT.rglob("flowboard_server*.pyc"):
        if skip_dir(pyc.parent):
            # still ban pycache zombies under tools/
            if "flowboard_server" in pyc.name and "__pycache__" in pyc.parts:
                if "tools/flowboard" not in pyc.as_posix():
                    errors.append(f"orphaned bytecode: {pyc.relative_to(ROOT)}")
            continue
        errors.append(f"orphaned bytecode: {pyc.relative_to(ROOT)}")

    # Explicitly check common leftover location
    leftover = ROOT / "tools" / "__pycache__"
    if leftover.is_dir():
        for pyc in leftover.glob("flowboard_server*.pyc"):
            errors.append(f"orphaned bytecode: {pyc.relative_to(ROOT)}")

    patterns = ("*.md", "*.py", "*.sh", "*.ps1", "*.yml", "*.yaml", "*.mjs", "*.js", "*.html")
    for pattern in patterns:
        for path in ROOT.rglob(pattern):
            if path.resolve() == SELF:
                continue
            if skip_dir(path):
                continue
            rel = path.relative_to(ROOT).as_posix()
            # Historical CI comment is OK if it only says "replaced".
            try:
                text = path.read_text(errors="ignore")
            except OSError:
                continue
            if "flowboard_server.py" in text or "tools/flowboard_server" in text:
                errors.append(f"{rel}: banned runnable reference to flowboard_server")

    uniq = sorted(set(errors))
    if uniq:
        for e in uniq:
            print(f"::error::{e}")
        print("zombie-ban-gate FAILED (flowboard_server is retired; use flowmond).")
        return 1
    print("✓ zombie ban OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
