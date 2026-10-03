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

**What counts as a module's own declaration.** The registry seeds each module's
resolvable scope and its name→owner map from its top-level declarations, one name per
statement via `declarationName`. An `extern "C" { ... }` block is a namespace statement
named `__extern_C`, so `collectDeclarationNames` recurses into its members: the functions
such a block declares belong to the module that writes them, exactly like top-level
functions (#442). Without that, a module could not resolve an FFI entry point it declared
itself once any other module in the program — typically an imported binding — declared the
same name: the owner map attributed the name elsewhere and the namespace gate hid it from
the declarer (`Undefined identifier: <name>`). An FFI declaration names a process-global C
symbol, so several modules may declare the same one; each resolves its own. Note the shape
that masks the bug: *importing the colliding name* grants it into the importer's scope, so
the failure only appears when the importer declares its own extern without importing that
name (`test/modules/test_import_ffi_sibling.vyb`).

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
