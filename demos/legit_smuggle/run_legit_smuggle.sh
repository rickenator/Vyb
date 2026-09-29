#!/usr/bin/env bash
# run_legit_smuggle.sh — Vyb smuggle-safe import demo.
#
# Seals a legitimization receipt for the privileged `priv` package (same
# identity + tip-token as priv::priv_token), then builds+runs the consumer
# that `smuggle`s the package. Two runs:
#   1. honest receipt  -> SMUGGLE-SAFE: PASS (package consumed)
#   2. tampered receipt-> SMUGGLE-SAFE: REFUSED (package NOT consumed)
#
# This is the Vyb half of the chain: the import gate (smuggle, #204) is
# reinforced by a chain-integrity + tip-token cross-check before the smuggled
# package is consumed.
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VYB="${VYB:-$root/../../build/vyb}"
export VYB_STDLIB="$root/../../stdlib"

echo "== Vyb smuggle-safe import (chain legitimacy gate) =="

# 1. seal the honest receipt for priv (same facts as priv::priv_token)
"$VYB" "$root/seal_recept.vyb" "$root/priv.receipt"

# Build the consumer (smuggle of the privileged package)
( cd "$root/consumer" && "$VYB" build >/dev/null )
exe="$root/consumer/target/legit-smuggle"

# Honest receipt: seal the same identity priv::priv_token seals, serialized.
HONEST="$root/priv.receipt"
export SMUGGLE_RECEIPT="$HONEST"
echo
echo "-- honest receipt --"
out1="$( "$exe" )"
echo "$out1"

# Tampered receipt: flip the sealed version.
TAMPERED="$root/priv.receipt.tampered"
"$VYB" "$root/seal_recept.vyb" --tamper "$HONEST" "$TAMPERED"
export SMUGGLE_RECEIPT="$TAMPERED"
echo
echo "-- tampered receipt --"
refused=1
out2="$( "$exe" )" || refused=0
echo "$out2"

echo
if grep -q "SMUGGLE-SAFE: PASS" <<<"$out1" && grep -q "SMUGGLE-SAFE: REFUSED" <<<"$out2"; then
    echo "SMUGGLE-LEGIT: PASS (honest accepted, tampered refused)"
    exit 0
fi
echo "SMUGGLE-LEGIT: FAIL (expected PASS then REFUSED)"
exit 1
