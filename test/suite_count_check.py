#!/usr/bin/env python3
"""#226 suite-count drift check.

Verifies that every documented suite-size number (in README.md and the refman
PROGRAMMERS_GUIDE.md) equals the number of `.vyb` files the canonical harness
actually discovers (test/run_tests.py's `test_dir.glob("**/*.vyb")`).

Exit 0 = all documented counts match the discovered suite.
Exit 1 = drift: prints the mismatched documents/numbers for humans and CI.

Wired into CI so a fixture add/remove can no longer silently desync the README /
refman from the real suite count.
"""

import pathlib
import re
import sys


def find_root() -> pathlib.Path:
    d = pathlib.Path(__file__).resolve().parent
    for cand in (d, d.parent, d.parent.parent):
        if (cand / "CMakeLists.txt").exists() and (cand / "test" / "run_tests.py").exists():
            return cand
    raise SystemExit("cannot locate Vyb repo root from " + str(d))


def discovered_count(root: pathlib.Path) -> int:
    test_dir = root / "test"
    return len(list(test_dir.glob("**/*.vyb")))


def documented_counts(path: pathlib.Path):
    """Yield every integer appearing as a `.vyb tests` total or N/N success ratio."""
    text = path.read_text(errors="ignore")
    counts = set()
    # "1144 `.vyb` tests", "1144 .vyb tests", "full suite (1144 tests)"
    for m in re.finditer(r"(\d+)[ ]*(?:\.vyb[ ]*)?tests", text):
        counts.add(int(m.group(1)))
    # "100% (1144/1144)"
    for m in re.finditer(r"(\d+)/(\d+)", text):
        if m.group(1) == m.group(2):
            counts.add(int(m.group(1)))
    return counts


def main() -> int:
    root = find_root()
    actual = discovered_count(root)
    targets = [
        root / "README.md",
        root / "docs" / "refman" / "PROGRAMMERS_GUIDE.md",
    ]
    problems = []
    for t in targets:
        if not t.exists():
            continue
        for n in sorted(documented_counts(t)):
            if n != actual:
                problems.append(f"{t.relative_to(root)}: documents {n} .vyb tests, "
                                f"harness discovers {actual}")

    if problems:
        print("suite-count drift (#226):", file=sys.stderr)
        for p in problems:
            print("  - " + p, file=sys.stderr)
        print(f"  canonical = {actual} (.vyb files under test/)", file=sys.stderr)
        return 1
    print(f"suite-count --check OK ({actual} .vyb tests in docs match harness)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
