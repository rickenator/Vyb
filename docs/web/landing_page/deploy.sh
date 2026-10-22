#!/usr/bin/env bash
# deploy.sh — copy the Vyb landing page to the vyb-lang.org document root.
#
#   ./deploy.sh            # upload, then verify the live site
#   ./deploy.sh --check    # verify only (no ssh, no upload)
#
# The site source is this directory: index.html (generated from README.md) plus
# the logo and icons. A GitHub push does NOT publish — these files are copied to
# the web host, which is separate from the repository.
#
# Host: anivizac@69.89.24.177 (Bluehost), ssh on port 2222. The vyb-lang.org
# document root is an addon-domain directory under public_html; set DEST if you
# know its name, otherwise the default below is used and the post-upload check
# tells you whether it landed.
#
# Auth: this machine holds ~/.ssh/id_ed25519_aniviza, which the host does NOT
# yet accept, and there is no SSH password to type — only the cPanel login.
# Two ways in:
#   * authorize that key once: cPanel -> SSH Access -> Manage SSH Keys ->
#     Import Key (paste the PUBLIC half of ~/.ssh/id_ed25519_aniviza.pub), then
#     Manage -> Authorize. After that this script asks for nothing.
#   * or let it fall through to the password prompt (works when the cPanel
#     password doubles as the SSH password, which it often does on this host);
#     without an authorized key uploads are better done through cPanel ->
#     File Manager (upload the files, no ssh needed).
#
# Two ssh quirks are handled below: offering every key in the agent makes the
# host drop the connection with "Too many authentication failures" before the
# password is ever tried, so exactly one identity is offered (IdentitiesOnly +
# an explicit -i); and the host's ssh key changed, so the stale entry in
# ~/.ssh/known_hosts is bypassed rather than trusted.
set -euo pipefail
cd "$(dirname "$0")"

HOST="anivizac@69.89.24.177"
PORT=2222
DEST="${DEST:-public_html/vyb-lang.org/}"
SITE="https://vyb-lang.org/"
# A string that exists ONLY in the current page — the live check greps for it,
# so a stale upload (or a wrong document root) cannot pass as success.
TOKEN='1314'

# Files that make up the live root. Everything the page references must be here:
# the og:/twitter: image (/vyb.png) was missing on the host, so social previews
# 404'd until it was added to this list.
FILES=(index.html vyb.png favicon.ico favicon-16x16.png favicon-32x32.png apple-touch-icon.png)

KEY="$HOME/.ssh/id_ed25519_aniviza"
KEY_OPT=()
[ -f "$KEY" ] && KEY_OPT=(-i "$KEY")
SSH_OPTS=(-P "$PORT" "${KEY_OPT[@]}" -o IdentitiesOnly=yes
          -o UserKnownHostsFile="$HOME/.ssh/known_hosts.vyb-lang"
          -o StrictHostKeyChecking=yes
          -o PreferredAuthentications=publickey,password,keyboard-interactive
          -o NumberOfPasswordPrompts=3)

check_live() {
  echo "==> verifying $SITE (expecting token '$TOKEN')"
  local body
  body=$(curl -fsS "$SITE") || { echo "FAIL: $SITE did not answer"; return 1; }
  case "$body" in
    *"$TOKEN"*) echo "OK: live page carries '$TOKEN' ($(printf %s "$body" | wc -c) bytes)";;
    *) echo "FAIL: live page does not carry '$TOKEN' — stale upload or wrong DEST ($DEST)"; return 1;;
  esac
  local p
  for p in vyb.png favicon.ico favicon-32x32.png apple-touch-icon.png; do
    printf '  %-22s %s\n' "$p" "$(curl -s -o /dev/null -w '%{http_code} %{size_download}B' "$SITE$p")"
  done
  # The social-preview image must resolve, not merely exist in this directory.
  if curl -fsS -o /dev/null "$SITE"vyb.png; then echo "OK: og:image /vyb.png resolves"; else echo "FAIL: og:image /vyb.png 404s"; return 1; fi
}

if [ "${1:-}" = "--check" ]; then check_live; exit $?; fi

for f in "${FILES[@]}"; do
  [ -f "$f" ] || { echo "missing $f — refusing to upload"; exit 1; }
done

echo "==> deploying ${FILES[*]} to ${HOST}:${DEST}"
scp "${SSH_OPTS[@]}" "${FILES[@]}" "${HOST}:${DEST}"
echo "==> sent"
check_live
echo
echo "If the check FAILED with a wrong-DEST message, find the real root in cPanel"
echo "(Domains -> vyb-lang.org -> Document Root) and re-run:  DEST='<root>/' ./deploy.sh"
