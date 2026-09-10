#!/usr/bin/env python3
"""End-to-end smoke test for `vyb repl` (issue #154 / Testing & Tooling).

Spawns `vyb repl` with a scripted stdin session and asserts the program-output
stdout lines. Verifies: bare-expression auto-display, persistent declarations
(including a multi-line function), persistent variables, error recovery (a bad
line is reported and does not kill the session), and `:clear`.

The REPL writes prompts to stderr when stdin is not a TTY, so stdout carries only
evaluated program output — this makes the assertions deterministic and lets the
test gate hosted CI.

Usage: python3 test/repl_smoke.py [path-to-vyb]   (defaults to build/vyb then build-ci/vyb)
"""

import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(HERE)


def find_exe():
    for cand in sys.argv[1:]:
        if os.path.exists(cand):
            return os.path.abspath(cand)
    for cand in ("build/vyb", "build-ci/vyb"):
        p = os.path.join(REPO, cand)
        if os.path.exists(p):
            return os.path.abspath(p)
    raise SystemExit("no built `vyb` binary found (build/vyb or build-ci/vyb); pass a path")


SESSION = "\n".join([
    "6 + 6",                      # bare expression -> 12
    "f(x<Int>, y<Int>)<Int> -> {",  # multi-line function declaration (persisted)
    "    return x * y",
    "}",
    "f(6, 7)",                    # 42
    "v = 5",                      # variable assignment (persisted)
    "v + 2",                      # 7
    ":type v",                    # ":type v = Int"
    "broken_expr_zz",             # error -> reported, session continues
    "9 - 4",                      # 5
    ":clear",
    "9 - 4",                      # 5 (fresh session after clear)
    ":quit",
    "",
])

EXPECTED_STDOUT = ["12", "42", "7", ":type v = Int", "5", "5"]


def main():
    exe = find_exe()
    env = dict(os.environ)
    env["VYB_STDLIB"] = os.path.join(REPO, "stdlib")
    p = subprocess.run([exe, "repl"], input=SESSION.encode(),
                       stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=env, timeout=300)
    out = p.stdout.decode().strip().splitlines()
    err = p.stderr.decode()
    if out != EXPECTED_STDOUT:
        print("REPL smoke FAILED")
        print("  expected stdout: %r" % EXPECTED_STDOUT)
        print("  actual stdout:   %r" % out)
        print("  stderr: %r" % err[:2000])
        return 1
    # error recovery: the broken line must have produced an error report
    if "broken_expr_zz" not in err and "rror" not in err:
        print("REPL smoke FAILED: expected an error report for the broken line")
        return 1
    print("REPL smoke: %d/%d checks passed" % (len(EXPECTED_STDOUT), len(EXPECTED_STDOUT)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
