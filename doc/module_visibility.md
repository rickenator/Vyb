# Module Resolution and Visibility Notes

This document summarizes the current source-level module resolver behavior
around `import`/`smuggle`, `bundle(...)`, and `share(...)`.

## ModuleRegistry model

`ModuleRegistry` (in `include/vyb/module_registry.hpp` / `src/module_registry.cpp`)
owns module loading metadata:

- Canonical module key (normalized absolute source path)
- Source path and parsed AST root
- Imported module keys
- Resolution state: `Unresolved`, `Parsing`, `Resolved`, `Failed`
- Original import spelling used for diagnostics

Imports are resolved before semantic analysis/codegen, and imported declarations
are spliced into the importing module with visibility filtering.

## Search path precedence

For path imports like `import a::b::c`, resolver search order is:

1. Directory of the importing file
2. `--module-path <dir>` (repeatable, CLI order)
3. `VYB_MODULE_PATH` (colon-separated)
4. Auto-discovered stdlib root

For locator imports like `import name from "./relative.vyb"`, resolution keeps
the current relative-file behavior from the importing file.

### Path convention

For each search root and `a::b::c`, resolver tries:

1. `<root>/a/b/c.vyb`
2. `<root>/a/b/c/mod.vyb`

## Stdlib auto-discovery

Stdlib root is detected in this order:

1. `VYB_STDLIB` environment variable (if set)
2. Relative to compiler executable:
   - `<exe_dir>/../stdlib`
   - `<exe_dir>/stdlib`

The discovered stdlib root is appended to module search paths automatically.

## Stdlib prelude behavior

Current behavior is explicit-only (no auto-import). See `doc/stdlib_layout.md`
for the canonical stdlib layout and prelude module paths.

## Diagnostics

Module-resolution failures now distinguish:

- file-not-found (includes all tried candidate paths)
- parse error inside imported module
- circular import chain
- duplicate symbol introduced during import splice

All include original import spelling and importer source location.