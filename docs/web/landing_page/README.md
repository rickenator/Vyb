# vyb-lang.org — the deployed project site

`index.html` in this directory **is** the live root page of
<https://vyb-lang.org/>. A GitHub push does not publish it: the page is copied
to the web host (see *Deploy* below). The site is separate from the repo, so a
change lands in the repository and in production independently.

## What is in the page

Two layers, and only one of them is generated:

* **Shell** (hand-maintained, at the top and bottom of `index.html`): `<head>`
  metadata — description, canonical, Open Graph, Twitter card, JSON-LD — the
  topbar, the hero with the inline logo, the footer, and the copy-button script.
* **Body** (generated): the whole "Vyb Programming Guide" section is a render of
  the repository [`README.md`](../../README.md), plus the sidebar table of
  contents built from the body's `h2`/`h3` headings. Heading ids are
  GitHub-style slugs with the `#` anchor links the stylesheet styles, and
  repo-relative links are rewritten to GitHub `blob/main` URLs so they work for
  a reader who has no checkout.

That is why the page's feature list, version, test count and platform claims are
the README's: **update the README first**, then regenerate the body. The page's
non-README facts (site name, canonical host, Aniviza attribution) live in the
shell only.

## Regenerating the body from README.md

1. Render `README.md` (GitHub-flavoured: tables, fenced code, `**bold**`).
2. Give every heading an id (lowercase, punctuation dropped, spaces → `-`,
   duplicates get `-1`, `-2`) and append the `#` anchor link.
3. Rewrite relative repo links to `https://github.com/rickenator/Vyb/blob/main/<path>`
   (directories → `tree/main/`).
4. Rebuild the sidebar TOC from the `h2`/`h3` headings in document order
   (`class="toc-h2"` / `toc-h3`).
5. Keep the shell untouched, then re-check: every TOC link and every heading
   anchor must resolve to an existing id, no duplicate ids, and the new version
   and test count must appear in the page text.

The regeneration done for v0.7.7 used a throwaway Python helper outside the
repo; nothing in the tree depends on it. The repo-consistent follow-up, matching
how `tools/refman.py` was replaced, is a Vyb tool (e.g. `tools/landing_page.vyb`)
that emits this body — until then the steps above are the manual recipe.

## Deploy

```bash
./deploy.sh            # upload + verify the live site
./deploy.sh --check    # verify only (no ssh)
```

* Host: `anivizac@69.89.24.177` (Bluehost), ssh on port 2222.
* `DEST` defaults to `public_html/vyb-lang.org/` but the real document root is
  an addon-domain directory that has **not** been confirmed from this machine;
  override it if the check reports the wrong root (`--check` greps the live page
  for the current test-count token, so a stale upload or a wrong root cannot
  pass as success).
* Auth: `~/.ssh/id_ed25519_aniviza` exists here but is **not yet authorised** on
  the host, and there is no SSH password on this machine — only the cPanel
  login. Authorise the key once in cPanel (SSH Access → Manage SSH Keys →
  Import the `.pub` half → Authorize), or use cPanel → File Manager to upload
  the files without ssh.
* Two host quirks the script handles: offering every key in the ssh agent gets
  the connection dropped with "Too many authentication failures" (so exactly one
  identity is offered), and the host's ssh host key changed since the stale
  `known_hosts` entry, so the script bypasses that entry rather than trusting
  it.
* Every asset the page references must be uploaded with it. `/vyb.png` (the
  `og:image` / `twitter:image`) was **missing** from the live root and 404'd, so
  social previews showed no image; the script now sends it and asserts it
  resolves after upload. Case matters on the host even where the authoring
  machine is case-insensitive.
* The live root is a distinct directory from `aniviza.com`'s document root
  (`/favicon.ico` differs between the two, and `/bdas.html` 404s here), so the
  Aniviza deploy script does not publish this page.
