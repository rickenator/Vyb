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

**Per-module identity for spliced declarations (#432).** The splice flattens a module's
declarations into the importer's namespace, so two modules may contribute the same name.
The rule:

- A declaration **private** to its declaring module (not in that module's `share` set) is
  an implementation detail: the splice renames it to a per-module name (`__pv_<module>`_
  `<name>`, keyed on the *declaring* module so it keeps one identity across import hops)
  and rewrites the module's own references to it. Two modules may therefore each define a
  private `hexval`, and each module's internal callers bind to their own.
- A declaration whose **shared** name is already in the importer's scope — because an
  earlier import (or a carried dependency) contributed it — also takes a per-module
  identity: **the first module to contribute a shared name keeps the plain one**, and a
  later module's declaration of that name becomes module-local. So `import b;` of a module
  that shares a name with an already-imported module compiles and runs, and each module's
  internal calls still resolve to its own definition.

Before this, the same root cause produced two different failures: a whole-module import
**refused to compile** (`Duplicate symbol after splice: '<name>'`) even when the program
never named the symbol, and a subset import that carried a private helper **silently bound
one module's internal call to another module's same-named definition** (standalone B
printed `2`, while A+subset-B printed `1`). A genuine duplicate that still cannot be
resolved reports the import site *and both defining modules*.

Fixtures: `test/modules/test_import_private_name_identity.vyb` (whole import, private-name
collision), `test_import_private_name_binding.vyb` (subset import, the silent mis-binding),
`test_import_shared_name_shadowing.vyb` (two modules exporting the same name), with
`test/modules/privname_lib/{pna,pnb,pnc,pnd}.vyb`.

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
