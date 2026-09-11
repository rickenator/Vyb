#!/bin/bash
# Conformance for the smuggle channel: vyb mod install (issue #175, Phase 2).
#
# Exercises the offline channel core against the test/remote_modules/ fixture:
# path:install materializes .vybmod/<name>/ (+ relative-import sibling), writes
# vyb.lock with the correct sha256, and registers a path dependency in vyb.toml;
# a matching @sha256: pin installs, a mismatching pin is a HARD error.
set -euo pipefail
cd "$(dirname "$0")/.."   # repo root (this script lives under test/)
ROOT=$(pwd)
VYB="$ROOT/build/vyb"
FIXTURE="$ROOT/fixtures/remote_modules/calc"
OKSHA="f062b009d931797dba09cf0559bab33bc4da2f7c109fd78ae5db55600372afdb"

WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
pass=0; fail=0
ok(){ echo "  ok: $1"; pass=$((pass+1)); }
bad(){ echo "FAIL: $1"; fail=$((fail+1)); }

printf '[package]\nname = "app"\nversion = "0.1.0"\n' > "$WORK/vyb.toml"
cd "$WORK"

# 1. Plain path install materializes module + relative-import sibling.
"$VYB" mod install "path:$FIXTURE" >/dev/null 2>"$WORK/e1"
[ $? -eq 0 ] && [ -f .vybmod/calc/mod.vyb ] && [ -f .vybmod/calc/strutil.vyb ] \
  && cmp -s .vybmod/calc/mod.vyb "$FIXTURE/mod.vyb" \
  && cmp -s .vybmod/calc/strutil.vyb "$FIXTURE/strutil.vyb" \
  && ok "path install materializes module + sibling" || bad "path install materialize"

# 2. vyb.lock records the correct sha256.
grep -q "sha256 = \"$OKSHA\"" vyb.lock && ok "vyb.lock sha256 matches fixture INDEX" \
  || bad "vyb.lock sha256"

# 3. vyb.toml gains the path dependency.
grep -q 'calc = { path = ".vybmod/calc" }' vyb.toml && ok "vyb.toml registers path dep" \
  || bad "vyb.toml dep"

# 4. A matching pin installs cleanly.
if "$VYB" mod install "path:$FIXTURE@sha256:$OKSHA" >/dev/null 2>"$WORK/e4"; then
  ok "matching @sha256 pin installs"
else bad "matching pin"; fi

# 5. A mismatching pin is a HARD error (exit 1) naming both hexes.
if "$VYB" mod install "path:$FIXTURE@sha256:deadbeef" >/dev/null 2>"$WORK/e5"; then
  bad "mismatched pin (expected failure)"
elif grep -q "sha256 mismatch" "$WORK/e5" && grep -q "deadbeef" "$WORK/e5" && grep -q "$OKSHA" "$WORK/e5"; then
  ok "mismatched pin hard-errors with both hexes"
else
  bad "mismatched pin (wrong message)"
fi

# 6. Re-install is idempotent (single dependency entry).
[ "$(grep -c 'calc = { path' vyb.toml)" -eq 1 ] && ok "re-install idempotent" || bad "idempotent"

# 7. Posted signed bindings verify against the pinned publisher key (issue #198:
#    any committed INDEX.json + .sig must stay valid under the official key).
for post in sqlite cuda; do
  if out=$("$VYB" mod verify-signed "$ROOT/bindings/$post/INDEX.json" 2>&1); then
    printf '%s' "$out" | grep -q "SIGNED VERIFY OK" && ok "$post signed INDEX verifies" \
      || bad "$post verify-signed (unexpected output)"
  else
    bad "$post verify-signed (exit $?)"
  fi
done

# ===== #204 P2: package-level freedom/trust boundary on install =====
GPU="$ROOT/fixtures/remote_modules/gpu"   # vyb.toml: boundary=freedom + caps
clean_gpu(){ rm -f "$WORK"/vyb.lock "$WORK"/.vybmod/gpu -r 2>/dev/null || true; }
clean_gpu

# 8. A privileged package (declares [mod] capabilities/freedom) must NOT install
#    silently without a trust acceptance (non-interactive: no --yes, no env).
if "$VYB" mod install "path:$GPU" >/dev/null 2>"$WORK/e8" && [ -f .vybmod/gpu/mod.vyb ]; then
  bad "privileged install without acceptance (expected refusal)"
elif grep -q "privileged" "$WORK/e8" && grep -q "cuda-driver" "$WORK/e8"; then
  ok "privileged install refused without acceptance (lists capabilities)"
else
  bad "privileged refusal (wrong message)"
fi

# 9. --yes accepts: installs AND records {source, sha256, capabilities} in vyb.lock.
clean_gpu
if "$VYB" mod install "path:$GPU" --yes >/dev/null 2>"$WORK/e9" && [ -f .vybmod/gpu/mod.vyb ]; then
  if grep -q "gpu = { source = \"path:$GPU\"" vyb.lock \
     && grep -q "capabilities = \[ \"ffi\", \"cuda-driver\", \"nvptx\" \]" vyb.lock \
     && grep -q "freedom = true" vyb.lock; then
    ok "--yes installs privileged pkg + records source/sha256/capabilities (+ freedom)"
  else
    bad "--yes installed but vyb.lock misses trust record"
  fi
else
  bad "--yes privileged install"
fi

# 10. Re-install after acceptance is silent (decision pinned in vyb.lock) — the
#     repro/build is non-interactive and deterministic.
if "$VYB" mod install "path:$GPU" >/dev/null 2>"$WORK/e10" && [ -f .vybmod/gpu/mod.vyb ]; then
  grep -q "privileged" "$WORK/e10" && bad "re-install re-prompted (should be silent)" \
    || ok "re-install of accepted privileged pkg is silent (pinned in vyb.lock)"
else
  bad "re-install after acceptance"
fi

echo
echo "REMOTE-IMPORT $([ $fail -eq 0 ] && echo PASS || echo FAIL) ($([ $fail -eq 0 ] && echo all "$pass" || echo "$fail of $((pass+fail)) failed"))"
exit $([ $fail -eq 0 ] && echo 0 || echo 1)
