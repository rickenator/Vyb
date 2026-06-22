#!/usr/bin/env python3
"""
Run Vyb demo and example programs.

The runner treats top-level demos/examples as executable entrypoints and checks
example support modules with semantic-only validation.
"""

import argparse
import os
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path


@dataclass(frozen=True)
class ExampleCase:
    path: Path
    mode: str = "run"
    extra_args: tuple[str, ...] = field(default_factory=tuple)


def repo_root() -> Path:
    current = Path(__file__).resolve().parent
    while current != current.parent:
        if (current / "CMakeLists.txt").exists() and (current / "src" / "tests.cpp").exists():
            return current
        current = current.parent
    raise RuntimeError("Could not find Vyb repository root")


def discover_cases(root: Path, run_entrypoints: bool) -> list[ExampleCase]:
    mode = "run" if run_entrypoints else "compile"
    cases: list[ExampleCase] = []

    for path in sorted((root / "demos").glob("*.vyb")):
        cases.append(ExampleCase(path.relative_to(root), mode))

    for path in sorted((root / "examples").glob("*.vyb")):
        cases.append(ExampleCase(path.relative_to(root), mode))

    cases.append(ExampleCase(Path("examples/stdlib_demo/main.vyb"), mode))
    cases.append(
        ExampleCase(
            Path("examples/module_path_demo/main.vyb"),
            mode,
            ("--module-path", "examples/module_path_demo/modules"),
        )
    )

    helper_paths = [
        *sorted((root / "examples" / "modules").glob("**/*.vyb")),
        *sorted((root / "examples" / "module_path_demo" / "modules").glob("**/*.vyb")),
    ]
    for path in helper_paths:
        cases.append(ExampleCase(path.relative_to(root), "semantic"))

    return cases


def command_for_case(vyb: str, case: ExampleCase) -> list[str]:
    cmd = [vyb, str(case.path), *case.extra_args]
    if case.mode == "compile":
        cmd.append("--no-execute")
    elif case.mode == "semantic":
        cmd.append("--semantic-only")
    return cmd


def run_case(root: Path, vyb: str, case: ExampleCase, timeout: float, verbose: bool) -> bool:
    cmd = command_for_case(vyb, case)
    try:
        result = subprocess.run(cmd, cwd=root, capture_output=True, text=True, timeout=timeout)
    except subprocess.TimeoutExpired as exc:
        print(f"FAIL {case.mode}: {case.path} timed out after {timeout:g}s")
        if exc.stdout:
            print(exc.stdout, end="")
        if exc.stderr:
            print(exc.stderr, end="", file=sys.stderr)
        return False

    if result.returncode == 0:
        print(f"PASS {case.mode}: {case.path}")
        if verbose and result.stdout:
            print(result.stdout, end="")
        return True

    print(f"FAIL {case.mode}: {case.path} exited {result.returncode}")
    if result.stdout:
        print(result.stdout, end="")
    if result.stderr:
        print(result.stderr, end="", file=sys.stderr)
    return False


def main() -> int:
    root = repo_root()
    parser = argparse.ArgumentParser(description="Run Vyb demos and examples")
    parser.add_argument("--vyb", default=str(root / "build" / "vyb"), help="Path to the Vyb executable")
    parser.add_argument("--no-run", action="store_true", help="Compile entrypoints with --no-execute instead of running them")
    parser.add_argument("--timeout", type=float, default=10.0, help="Per-entry timeout in seconds")
    parser.add_argument("--verbose", "-v", action="store_true", help="Show stdout for passing examples")
    args = parser.parse_args()

    if not os.path.isfile(args.vyb):
        print(f"Error: Cannot find Vyb executable at {args.vyb}", file=sys.stderr)
        return 1

    cases = discover_cases(root, run_entrypoints=not args.no_run)
    passed = 0
    for case in cases:
        if run_case(root, args.vyb, case, args.timeout, args.verbose):
            passed += 1

    total = len(cases)
    failed = total - passed
    print(f"\nRan {total} demo/example checks")
    print(f"Passed: {passed}")
    print(f"Failed: {failed}")
    return 0 if failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
