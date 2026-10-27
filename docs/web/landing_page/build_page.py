#!/usr/bin/env python3
"""build_page.py -- render README.md into the body + table of contents of the
vyb-lang.org landing page (index.html).

The page is two layers and only one of them is generated:

  shell   hand-maintained: <head> metadata, topbar, hero, footer, scripts.
          Everything outside the BODY/TOC markers.
  body    generated from ../../README.md by this script, between
          <!-- BODY:BEGIN --> / <!-- BODY:END --> and the sidebar table of
          contents between <!-- TOC:BEGIN --> / <!-- TOC:END -->.

That keeps the page's feature list, version and test count equal to the
README's: edit the README, then run

    python3 build_page.py            # rewrite index.html in place
    python3 build_page.py --check    # exit 1 if index.html is out of date

Rendering rules (they match how GitHub renders the README):
  * GFM tables and fenced code; CommonMark for everything else (a fenced block
    indented inside a list item stays inside that item -- python-markdown
    breaks those and produced stray <p><code> blocks on the live page).
  * every h2/h3/h4 gets a GitHub-style slug id (lowercase, punctuation dropped,
    spaces -> '-', duplicates get -1, -2, ...) plus a '#' anchor link.
  * repo-relative links are rewritten to github.com/rickenator/Vyb/blob/main/
    (or /tree/main/ for a directory) so they resolve for a reader with no
    checkout; external links open in a new tab.
  * the sidebar TOC is rebuilt from the h2/h3 headings in document order.

Requires markdown-it-py (pip install markdown-it-py); no other dependency.
"""

from __future__ import annotations

import argparse
import html
import os
import re
import sys

from markdown_it import MarkdownIt

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(HERE, "..", "..", ".."))
README = os.path.join(REPO, "README.md")
PAGE = os.path.join(HERE, "index.html")

BLOB = "https://github.com/rickenator/Vyb/blob/main/"
TREE = "https://github.com/rickenator/Vyb/tree/main/"

BODY_BEGIN = "<!-- BODY:BEGIN (generated from README.md by build_page.py -- do not edit) -->"
BODY_END = "<!-- BODY:END -->"
TOC_BEGIN = "<!-- TOC:BEGIN (generated from README.md headings by build_page.py -- do not edit) -->"
TOC_END = "<!-- TOC:END -->"

# README's own banner: the vybsplash generation note and the logo <img>, which
# the page's hero already carries. Dropped so the page does not repeat them.
DROP = re.compile(r"^\s*(<!--.*-->|<img\s[^>]*>)\s*$")


def slugify(text: str) -> str:
    """GitHub-style heading slug: lowercase, punctuation dropped, spaces -> '-'.

    '✅ Binary Executable Generation' -> 'binary-executable-generation'
    'Projects (`vyb.toml`)'           -> 'projects-vybtoml'
    '1.1 Purpose & Audience'          -> '11-purpose-audience'
    """
    t = html.unescape(text).lower()
    t = re.sub(r"[^a-z0-9\s-]", "", t)
    t = re.sub(r"[\s-]+", "-", t.strip())
    return t.strip("-")


def strip_tags(fragment: str) -> str:
    return html.unescape(re.sub(r"<[^>]+>", "", fragment))


LINK = re.compile(r"<a([^>]*)>(.*?)</a>", re.S)


def rewrite_links(body: str) -> str:
    def fix(m: re.Match[str]) -> str:
        attrs, text = m.group(1), m.group(2)
        hm = re.search(r'href="([^"]*)"', attrs)
        if not hm:
            return m.group(0)
        href = hm.group(1)
        if href.startswith(("#", "mailto:", "data:")):
            return m.group(0)
        if href.startswith(("http://", "https://")):
            url = href
        else:
            # repo-relative: split off any #fragment, resolve blob vs tree
            path, _, frag = href.partition("#")
            local = os.path.normpath(os.path.join(REPO, path))
            url = (TREE if os.path.isdir(local) else BLOB) + path.lstrip("./")
            if frag:
                url += "#" + frag
        rest = re.sub(r'\s*(?:href|target|rel)="[^"]*"', "", attrs)
        return f'<a{rest} href="{url}" rel="noopener noreferrer" target="_blank">{text}</a>'

    return LINK.sub(fix, body)


def render_body(markdown_text: str) -> tuple[str, list[tuple[int, str, str]]]:
    """Render README markdown -> (html body, [(level, id, text), ...])."""
    lines = [ln for ln in markdown_text.splitlines() if not DROP.match(ln)]
    md = MarkdownIt("commonmark").enable("table").enable("strikethrough")
    body = md.render("\n".join(lines) + "\n")

    toc: list[tuple[int, str, str]] = []
    seen: dict[str, int] = {}

    def add_heading(m: re.Match[str]) -> str:
        level, inner = int(m.group(1)), m.group(2)
        text = strip_tags(inner).strip()
        base = slug = slugify(text)
        while slug in seen:
            seen[base] += 1
            slug = f"{base}-{seen[base]}"
        seen[slug] = 0
        label = html.escape(text, quote=True).replace("&#x27;", "&#x27;")
        if level in (2, 3):
            toc.append((level, slug, text))
        return (
            f'<h{level} id="{slug}">{inner}'
            f'<a aria-label="Link to {label}" class="heading-anchor" href="#{slug}">#</a>'
            f"</h{level}>"
        )

    body = re.sub(r"<h([234])>(.*?)</h\1>", add_heading, body, flags=re.S)
    body = rewrite_links(body)
    return body, toc


def render_toc(toc: list[tuple[int, str, str]]) -> str:
    return "\n".join(
        f'<a class="toc-h{level}" href="#{slug}">{html.escape(text)}</a>' for level, slug, text in toc
    )


def splice(page: str, body: str, toc: str) -> str:
    def between(text: str, begin: str, end: str, payload: str) -> str:
        start = text.index(begin) + len(begin)
        stop = text.index(end)
        return text[:start] + "\n" + payload + "\n" + text[stop:]

    page = between(page, TOC_BEGIN, TOC_END, toc)
    return between(page, BODY_BEGIN, BODY_END, body)


def main() -> int:
    ap = argparse.ArgumentParser(description="render README.md into index.html")
    ap.add_argument("--check", action="store_true", help="verify index.html is current; do not write")
    args = ap.parse_args()

    with open(README, encoding="utf-8") as f:
        readme = f.read()
    with open(PAGE, encoding="utf-8") as f:
        page = f.read()

    body, toc = render_body(readme)
    updated = splice(page, body, render_toc(toc))

    if args.check:
        if updated != page:
            print("index.html is STALE -- run python3 build_page.py", file=sys.stderr)
            return 1
        print(f"index.html is current ({len(toc)} TOC entries)")
        return 0

    if updated != page:
        with open(PAGE, "w", encoding="utf-8") as f:
            f.write(updated)
        print(f"wrote {PAGE} ({len(body):,} bytes body, {len(toc)} TOC entries)")
    else:
        print(f"{PAGE} already current ({len(toc)} TOC entries)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
