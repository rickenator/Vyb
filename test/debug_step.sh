#!/usr/bin/env bash
# #390: validate the DWARF debugger integration end-to-end.
#
# DWARF emission has existed for a while, but nothing proved a debugger could
# actually stop on a *Vyb source line*. This script builds a fixture with debug
# info, sets a breakpoint by Vyb source line, steps, and reads a local -- under
# gdb, and under lldb when it is installed. Both run in batch mode so the check
# works headless in CI.
#
# The breakpoint line is not hard-coded here: the fixture carries a `# bp:` marker
# on the statement to stop at, and the script reads the line number out of it, so
# the two cannot drift apart.
#
# Usage: ./test/debug_step.sh            (from the repo root, or anywhere)
#        VYB=build-asan/vyb ./test/debug_step.sh   to use a different compiler

set -euo pipefail

root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"

VYB=${VYB:-build/vyb}
export VYB_STDLIB=${VYB_STDLIB:-stdlib}

fixture=test/units/test_debug_stepping.vyb
exe=./test_output_debug_stepping
expected_value=42
expected_output="total = $expected_value"

fail() { echo "FAIL: $*" >&2; exit 1; }

[ -x "$VYB" ] || fail "compiler not found at $VYB (build it first, or set VYB=)"

bp_line=$(grep -n '# bp:' "$fixture" | head -1 | cut -d: -f1 || true)
[ -n "${bp_line:-}" ] || fail "no '# bp:' marker in $fixture"

cleanup() {
    # The AOT link step drops its own runtime objects at the repo root; the debug
    # script must not leave them behind (ci.yml's AOT step cleans up the same way).
    rm -f "$exe" "$exe.o" "$exe.ll" \
          vyb_runtime.o vyb_type_metadata.o error_handling.o intrinsics.o
}
trap cleanup EXIT

# A stale object/exe from an earlier run can make the driver skip work it should
# redo, so start from a clean slate.
cleanup

echo "== debug stepping fixture =="
echo "   fixture   : $fixture"
echo "   breakpoint: $fixture:$bp_line"

# -O0 so the local is readable at the breakpoint (an optimizer is free to
# discard it); the optimized build is exercised by the suite as usual.
"$VYB" "$fixture" --build "$exe" -O0 >/dev/null || fail "AOT build failed"
[ -x "$exe" ] || fail "no executable produced at $exe"

"$exe" >/dev/null || fail "fixture did not exit cleanly"

# ---------------------------------------------------------------- gdb --------
if command -v gdb >/dev/null 2>&1; then
    out=$(gdb -batch -nx -iex 'set debuginfod enabled off' \
              -ex "break $fixture:$bp_line" \
              -ex run \
              -ex next \
              -ex "print partial" \
              -ex continue "$exe" 2>&1) || true

    echo "$out" | grep -q "at $(basename "$fixture"):$bp_line" \
        || { echo "$out" | tail -20; fail "gdb did not stop at $(basename "$fixture"):$bp_line"; }
    echo "$out" | grep -qE '\$[0-9]+ = 42' \
        || { echo "$out" | tail -20; fail "gdb could not read the local 'partial' after stepping"; }
    echo "$out" | grep -q "$expected_output" \
        || { echo "$out" | tail -20; fail "gdb run did not print '$expected_output'"; }
    echo "   gdb  : OK (stopped at the Vyb statement, stepped, read a local)"
else
    echo "   gdb  : not installed -- skipped"
fi

# --------------------------------------------------------------- lldb --------
if command -v lldb >/dev/null 2>&1; then
    out=$(lldb --batch \
              -o "breakpoint set --file $(basename "$fixture") --line $bp_line" \
              -o run \
              -o next \
              -o "frame variable partial" \
              -o continue "$exe" 2>&1) || true

    echo "$out" | grep -q "$(basename "$fixture"):$bp_line" \
        || { echo "$out" | tail -20; fail "lldb did not stop at $fixture:$bp_line"; }
    echo "$out" | grep -qE 'partial = 42' \
        || { echo "$out" | tail -20; fail "lldb could not read the local 'partial' after stepping"; }
    echo "$out" | grep -q "$expected_output" \
        || { echo "$out" | tail -20; fail "lldb run did not print '$expected_output'"; }
    echo "   lldb : OK (stopped at the Vyb statement, stepped, read a local)"
else
    echo "   lldb : not installed -- skipped"
fi

echo "debugger integration OK"
