#!/usr/bin/env python3
"""Promote file-local helpers out of a freshly extracted TU into the internal header.

Usage: promote.py <src.cpp> <newTU.cpp> <symbol>[,symbol...]

For each symbol <src.cpp> still calls, copy its definition from <newTU.cpp> into
include/vyb/vre/semantic_internal.hpp as `inline` and delete it from the TU: one
definition, visible to every TU, symbol visibility unchanged from the old
file-local `static`. Run BEFORE the build; the header must already include what the
promoted code needs (<set>/<string>/<algorithm>/...).

Identifier matching is exact (`\bname\s*\(`) so `intCanonicalName` does not match
`intCanonicalNameForName` -- a substring match promotes the wrong definition and
leaves a duplicate behind.
"""
import re, sys

src, new, names = sys.argv[1], sys.argv[2], sys.argv[3].split(',')
hdr = 'include/vyb/vre/semantic_internal.hpp'
src_text = open(src).read()
lines = open(new).read().split('\n')


def brace_end(lines, start):
    depth = 0
    for k in range(start, len(lines)):
        depth += lines[k].count('{') - lines[k].count('}')
        if depth == 0:
            return k + 1
    raise SystemExit('unbalanced braces starting at line %d' % (start + 1))


def calls(text, name):
    return re.search(r'\b' + re.escape(name) + r'\s*\(', text) is not None


def find_def(lines, name):
    pat = re.compile(r'\b' + re.escape(name) + r'\s*\(')
    for i, l in enumerate(lines):
        if not l.startswith('static '):
            continue
        if pat.search(l):
            j = i
            while j > 0 and (lines[j - 1].lstrip().startswith('//') or lines[j - 1].strip() == ''):
                j -= 1
            return j, brace_end(lines, i)
    raise SystemExit('definition not found in TU: ' + name)


promoted, spans = [], []
for n in names:
    if not calls(src_text, n):
        print('  keep in TU (no callers left in %s): %s' % (src, n))
        continue
    s, e = find_def(lines, n)
    block = '\n'.join(lines[s:e]).replace('static ', 'inline ', 1)
    promoted.append(block)
    spans.append((s, e))
    print('  promote: %s (lines %d..%d)' % (n, s + 1, e))

if promoted:
    h = open(hdr).read()
    pos = h.rindex('\n} // namespace vyb')
    h = h[:pos] + '\n\n' + '\n\n'.join(promoted) + '\n' + h[pos:]
    open(hdr, 'w').write(h)
    drop = set()
    for s, e in spans:
        drop.update(range(s, e))
    open(new, 'w').write('\n'.join(l for i, l in enumerate(lines) if i not in drop))
print('promoted %d symbol(s)' % len(promoted))
