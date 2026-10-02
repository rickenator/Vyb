#!/usr/bin/env bash
# vybenv_smoke.sh -- SOURCEME_VYB resolves the checkout and exports the pair
# every Vyb consumer needs (#424). Runs in clean shells (env -i) so nothing the
# calling environment already exports can mask a resolution bug.
#
# Usage: test/vybenv_smoke.sh   (from the checkout root or anywhere)
set -u
HERE="$(cd "$(dirname "$0")" && pwd -P)"
ROOT="$(cd "$HERE/.." && pwd -P)"
SM="$ROOT/SOURCEME_VYB"
pass=0; fail=0
ok()   { echo "  ok   $1"; pass=$((pass+1)); }
bad()  { echo "  FAIL $1"; fail=$((fail+1)); }
check() { # check <label> <expected> <actual>
    if [ "$2" = "$3" ]; then ok "$1"; else bad "$1 (expected '$2', got '$3')"; fi
}

[ -f "$SM" ] || { echo "FAIL: $SM missing"; exit 1; }

# 1. Clean bash, cwd=/ -- self-locating.
out=$(cd / && env -i /bin/bash -c ". '$SM'; printf '%s|%s|%s' \"\$VYBHOME\" \"\$VYB\" \"\$VYB_STDLIB\"")
check "clean bash from / resolves the checkout" "$ROOT|$ROOT/build/vyb|$ROOT/stdlib" "$out"

# 2. bash, cwd elsewhere -- still self-locating.
out=$(cd /tmp && env -i /bin/bash -c ". '$SM'; printf '%s' \"\$VYBHOME\"")
check "clean bash from /tmp resolves the checkout" "$ROOT" "$out"

# 3. A VYB pointing at the compiler binary derives the home.
out=$(env -i VYB="$ROOT/build/vyb" /bin/bash -c ". '$SM'; printf '%s' \"\$VYBHOME\"")
check "VYB=<checkout>/build/vyb derives the home" "$ROOT" "$out"

# 4. A VYB_BIN pointing at the build directory derives the home.
out=$(env -i VYB_BIN="$ROOT/build" /bin/bash -c ". '$SM'; printf '%s' \"\$VYBHOME\"")
check "VYB_BIN=<checkout>/build derives the home" "$ROOT" "$out"

# 5. An explicit VYBHOME wins over a stale VYB.
out=$(env -i VYBHOME="$ROOT" VYB=/tmp/elsewhere/build/vyb /bin/bash -c ". '$SM'; printf '%s|%s' \"\$VYBHOME\" \"\$VYB\"")
check "VYBHOME wins over a stale VYB" "$ROOT|$ROOT/build/vyb" "$out"

# 6. Idempotent: sourcing twice is the same as once.
out=$(env -i /bin/bash -c ". '$SM'; . '$SM'; printf '%s|%s' \"\$VYBHOME\" \"\$VYB\"")
check "idempotent: sourcing twice gives the same values" "$ROOT|$ROOT/build/vyb" "$out"

# 7. A strictly POSIX shell with VYBHOME set works.
if [ -x /bin/sh ]; then
    out=$(cd / && env -i VYBHOME="$ROOT" /bin/sh -c ". '$SM'; printf '%s|%s' \"\$VYBHOME\" \"\$VYB_STDLIB\"")
    check "POSIX sh with VYBHOME set works" "$ROOT|$ROOT/stdlib" "$out"

    # 8. ... and without a hint it fails loudly instead of exporting a wrong home.
    out=$(cd / && env -i /bin/sh -c ". '$SM' 2>/dev/null; printf '%s' \"\${VYBHOME:-unset}\"")
    check "POSIX sh without a hint exports nothing" "unset" "$out"
fi

# 9. The documented consumer contract: source it, then the three are the checkout's.
out=$(env -i /bin/bash -c ". '$SM'; [ -d \"\$VYB_STDLIB\" ] && echo pair-ok")
check "VYB_STDLIB exists after sourcing" "pair-ok" "$out"

echo "vybenv smoke: $pass/$((pass+fail)) checks passed"
[ "$fail" -eq 0 ] || exit 1
