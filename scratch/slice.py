#!/usr/bin/env python3
"""Move a cohesive seam out of a monolith translation unit into a new TU (#347).

Usage:
  slice.py <src.cpp> <new.cpp> <cmake_anchor> <name1>[,name2,...] [--dry]

Chunks are located by top-level definition signatures: the definition whose
signature line contains one of the names, plus every definition that follows it
until the first definition that belongs to another seam. Trailing comment/blank
runs are left behind (they document the next definition), so the move stays
"verbatim plus a header".

The tool prints the planned region and, unless --dry, rewrites both files and
inserts the new TU next to the source file in CMakeLists.txt.
"""
import re, sys

KW = re.compile(r'^(if|while|for|switch|else|do|try|catch|namespace|extern|template|return)\b')

def defs(lines):
    """Candidate definition starts at column 0.

    A signature may wrap across lines (`static IntAssignCheck f(ast::TypeNode* a,\n
    ast::Expression* b) {`), so the declaration is followed until its parentheses
    balance; the statement is a definition start only if a `{` follows (a `;` means a
    declaration/typedef, which is still a boundary worth stopping at, so both count).
    Catching wrapped signatures matters: a missed definition is a missed boundary, and
    the move then sweeps unrelated code into the new TU.
    """
    out = []
    for i, l in enumerate(lines):
        if not l or l[0] in ' \t#/}':
            continue
        if KW.match(l):
            continue
        # follow the declaration until parentheses balance, then look for the opener
        depth, k, text = 0, i, ''
        while k < len(lines) and k < i + 6:
            text += lines[k].strip() + ' '
            depth += lines[k].count('(') - lines[k].count(')')
            if depth <= 0 and ('(' in text):
                break
            k += 1
        rest = text[text.rfind(')'):] if ')' in text else text
        rest = rest.split('//')[0].strip()
        if not (rest.startswith(')') and ('{' in rest or ';' in rest)):
            continue
        out.append(i)
    return out

def chunk_end(lines, dlist, i):
    """End (exclusive) of the definition starting at dlist[i]."""
    stop = dlist[i + 1] if i + 1 < len(dlist) else len(lines)
    # trim back over blank/comment lines so the next definition keeps its doc block
    end = stop
    while end > dlist[i] + 1 and (lines[end - 1].strip() == '' or lines[end - 1].lstrip().startswith('//')):
        end -= 1
    return end

def main():
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    dry = '--dry' in sys.argv
    src, new, anchor, names = args[0], args[1], args[2], args[3].split(',')
    text = open(src).read()
    lines = text.split('\n')
    dlist = defs(lines)
    picks = set()
    for i, d in enumerate(dlist):
        sig = lines[d].split('(')[0]
        for n in names:
            # a pick may name the argument list (`visit(ast::AspectDeclaration`) when the
            # bare name is not unique; then match the whole signature line
            if n in lines[d] if '(' in n else n in sig:
                picks.add(i)
                break
    if not picks:
        print('no definition matched:', names); return 1
    spans = []
    for i in sorted(picks):
        spans.append((dlist[i], chunk_end(lines, dlist, i)))
    # merge touching/overlapping spans so a definition is never emitted twice
    merged = []
    for s, e in spans:
        if merged and s <= merged[-1][1]:
            merged[-1] = (merged[-1][0], max(merged[-1][1], e))
        else:
            merged.append((s, e))
    body = []
    for s, e in merged:
        body += lines[s:e]
    print(f'{src}: moving {len(merged)} chunk(s), {len(body)} lines -> {new}')
    for s, e in merged:
        print(f'  {s+1}..{e}: {lines[s][:80]}')
    if dry:
        return 0
    banner = [
        '// SPDX-License-Identifier: Apache-2.0',
        '//',
        f'// Seam extracted from {src} (#347): the member definitions below are moved',
        '// VERBATIM -- no reformatting, no renames -- so the split stays behaviour-neutral.',
        '// Everything that is file-local (static helpers, macros) travels with them or is',
        '// promoted into the shared internal header, never duplicated.',
        '',
    ]
    incs = []
    for l in lines:
        if l.startswith('#include') and l not in incs:
            incs.append(l)
        if l.strip().startswith('static ') and '(' in l:  # stop scanning at code
            break
    banner += incs + ['', 'namespace vyb {', '']
    out = banner + body + ['', '} // namespace vyb', '']
    open(new, 'w').write('\n'.join(out))
    drop = set()
    for s, e in merged:
        drop.update(range(s, e))
    rest = [l for i, l in enumerate(lines) if i not in drop]
    open(src, 'w').write('\n'.join(rest))
    cm = open(anchor).read()
    if f'src/vre/{new.split("/")[-1]}' not in cm:
        cm = cm.replace(src, src + '\n    ' + new, 1)
        open(anchor, 'w').write(cm)
    print('done')
    return 0

sys.exit(main())
