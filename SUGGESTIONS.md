# SUGGESTIONS for Vyn 1.0

*A forward-looking digest of concrete next steps to reach a stable, polished 1.0 release.*

---

## Executive Summary

Vyn is in solid shape. The core is working: name-first syntax, Vec, String, structs, generics, aspects, ownership keywords, JIT + AOT code generation. The test suite passes at ~97% for the focused test suite (100% for `test/new_features`). The language has a clear, distinctive identity.

This document organizes remaining work into themed sprints with concrete, actionable items.

---

## Recent Wins (this session)

- **Quicksort works** — Fixed double-free bug in Vec parameter passing. Vec function parameters now receive a deep copy of the data so callee and caller each own independent storage.
- **Struct + borrow + Vec mutations work** — Fixed two bugs:
  1. Use-after-free in semantic analysis (`handleVecMethodCallOnMember` stored raw pointer into a temporary `VecType`).
  2. Vec method calls through chained member expressions (`s.items.push()`) now correctly get a *pointer* to the field rather than a loaded copy, so mutations persist.
- **Stack Data Structure test passes** (was segfaulting).
- **Test suite: 94.6% → 100%** for `test/new_features`.

---

## Sprint 1: Consistency & Polish (Priority: HIGH)

### 1.1 Silence DEBUG output in release mode
Nearly every file in `src/vre/` is saturated with `std::cout << "DEBUG: ..."` and `std::cerr << "DEBUG: ..."` statements that make program output nearly unreadable. These should be gated behind a compile-time or runtime flag.

**Action**: Add a `#define VYN_DEBUG_CODEGEN 0` / `1` toggle (or `--verbose` CLI flag) and wrap all DEBUG prints accordingly. This single change would make the language feel dramatically more professional.

### 1.2 Silence parser noise
The parser emits `[PEEK]`, `[CONSUME]`, `[EXPECT]` for every token. This is useful during development but should be off by default.

**Action**: Gate parser tracing behind `--verbose-parser` or a global flag that defaults to off. The infrastructure already exists (`g_suppress_all_parser_debug_output`); make it the default.

### 1.3 Consolidate duplicate Doc files
`doc/` has overlapping/redundant documents (`ROADMAP.md` vs `TODO_CURRENT.md` vs top-level `TODO.md`; multiple ownership design docs). A reader can't tell which is current.

**Action**: Keep `TODO.md` (living document), `doc/FEATURE_STATUS.md`, and `CHANGELOG.md`. Archive or delete the rest into `doc/archive/`. Add a `doc/README.md` index.

### 1.4 Canonical syntax examples in all docs
Some docs still show `unsafe` instead of `freedom`, or use old `:` syntax for struct fields instead of `<Type>`.

**Action**: Audit and update all `*.md` files for syntax consistency.

---

## Sprint 2: Core Language Completeness (Priority: HIGH)

### 2.1 Enum codegen
Enums are fully parsed and represented in the AST but codegen is not implemented beyond C-style integer enums.

**Gap**: No pattern matching on enum variants, no associated data.

**Action**: Implement enum codegen as tagged unions. Emit an LLVM struct `{ i32 tag, <payload> }`. Enable `match` on enum variants.

### 2.2 Multi-value return (fix `direct_return.vyn`)
Multi-value returns parse and type-check but codegen fails for certain patterns (notably `return a, b` where variables rather than literals are returned).

**Action**: Fix the `{T, ptr}` struct return encoding for multi-value non-error returns. This is a focused fix in `cgen_stmt.cpp`.

### 2.3 Closures / lambda capture codegen
Lambdas can be defined and called inline, but captured variables from outer scopes are not reliably stored in the closure struct.

**Action**: Generate a capture struct for each lambda that closes over outer variables. Store references or copies (depending on ownership) in the struct. Pass the struct as the first argument to the lambda function.

### 2.4 `match` exhaustiveness and guards
`match` works for literal patterns but lacks:
- **Exhaustiveness checking** (warn/error if a `?` wildcard is missing and patterns don't cover all cases)
- **Guard conditions** (`pattern if expr -> ...`)
- **Destructuring** (struct field patterns)

**Action**: Add exhaustiveness check in semantic analysis. Add guard syntax to the parser.

### 2.5 `for (item in vec)` mutation
Currently `for (item in vec)` iterates a copy of each element. Mutation of `item` doesn't write back. This is expected for value semantics but should be documented clearly.

**Action**: Document the copy semantics. Consider `for (ref item in vec)` or `for (borrow item in vec)` syntax for mutable iteration.

---

## Sprint 3: Ownership Enforcement (Priority: HIGH)

### 3.1 Semantic enforcement of `my<T>` uniqueness
Variables of type `my<T>` can currently be aliased freely without compiler error.

**Action**: Track move state in the semantic analyzer. After a `my<T>` value is passed to a function (moved), mark the original as uninitialized. Reject subsequent uses.

### 3.2 `their<T>` lifetime scope enforcement
Borrowed references (`their<T>`) should not outlive the borrowee. Currently this is not checked.

**Action**: At minimum, add a lifetime scope depth check: a `their<T>` reference cannot be returned from a function unless the referent is also returned or is 'static.

### 3.3 `our<T>` reference counting runtime
The runtime ref-count infrastructure exists (control blocks are created), but decrement and drop logic is incomplete.

**Action**: Generate ref-count decrement at scope exit for `our<T>` variables. Call the destructor (or free) when the strong count reaches zero.

### 3.4 `mild<T>` weak reference upgrade
`mild<T>.grab()` exists in the AST and semantic analyzer but the codegen for runtime weak-pointer upgrade (atomically check `object_freed`, increment strong count, return `our<T>?`) is missing.

**Action**: Implement `grab()` codegen: atomically read `object_freed` flag, CAS increment `strong_count` if alive, return the control block pointer; otherwise return null.

---

## Sprint 4: Standard Library (Priority: MEDIUM)

### 4.1 `HashMap<K, V>`
No HashMap yet. This is one of the most-wanted standard library items.

**Action**: Implement as an intrinsic (backed by C `malloc`/open-addressing hash table), or expose a simple FFI-based hash map. Even a basic linear-probe implementation would unblock many use cases.

### 4.2 File I/O
No file I/O. `println` and `print` work but there's no way to read files.

**Action**: Add `File::open(path)`, `File::read_to_string()`, `File::write()` as intrinsics backed by C `fopen`/`fread`/`fwrite`.

### 4.3 `Option<T>` / nullable types
There's `nil` in the language concept but no `Option<T>` type. Nullable returns use raw `nil` which isn't type-safe.

**Action**: Add `Option<T>` as a built-in type with `Some(value)` and `None` constructors. Make `nil` an alias for `None`. Enable `match (opt) { Some(v) -> ..., None -> ... }`.

### 4.4 `Result<T, E>` / error type
The fail/trap system exists but there's no typed `Result<T, E>`.

**Action**: Define `Result<T, E>` as a built-in tagged union. Wire `fail` to construct `Err(e)`. This unifies with existing error propagation.

### 4.5 String methods: `split`, `join`, `format`
Common string operations missing:
- `str.split(delimiter)` → `Vec<String>`
- `String::join(vec, separator)` → `String`
- `String::format("template {}", val)` → `String`

---

## Sprint 5: Module System (Priority: MEDIUM)

### 5.1 Single-file module resolution
`import` and `smuggle` parse correctly but the compiler ignores them at codegen time.

**Action**: Implement single-file module loading: resolve `import path::to::module` to `path/to/module.vyn` relative to the source root. Parse, analyze, and codegen the imported module in the same LLVM module before the main file.

### 5.2 Namespace isolation
Once module loading works, imported symbols need to live in their own namespace to avoid name collisions.

**Action**: Prefix all symbols from imported modules with their module path when emitting LLVM IR names.

### 5.3 Circular import detection
Prevent infinite loops in module resolution.

---

## Sprint 6: FFI (Priority: MEDIUM)

### 6.1 `extern "C"` function declarations
The AST has `ExternStatement` but codegen is minimal.

**Action**: Complete `extern "C"` codegen: declare the function in the LLVM module with C calling convention. Allow Vyn code to call it without any wrapper.

### 6.2 C struct interop
Allow `freedom` blocks to access C-layout structs.

**Action**: Support `struct` with `#[repr(C)]` attribute (or a `repr<C>` type modifier) that lays out fields in C order without padding insertion.

---

## Sprint 7: Tooling (Priority: MEDIUM)

### 7.1 LSP server (`vyn-lsp`)
A Language Server would unlock IDE support (VS Code, Neovim, etc.) including:
- Go-to-definition
- Hover types
- Completion
- Inline error diagnostics

**Action**: Start with a basic `textDocument/hover` and `textDocument/publishDiagnostics` implementation using the existing semantic analyzer.

### 7.2 Formatter (`vynfmt`)
A canonical formatter reduces style debates and enforces consistency.

**Action**: Implement a pretty-printer that visits the AST and emits canonical Vyn syntax with consistent indentation and spacing.

### 7.3 Package manager (`vyn.toml`)
Zero toolchain usability without a package manager.

**Action**: Define `vyn.toml` schema. Implement `vyn add`, `vyn build`, `vyn test` subcommands. At minimum, support local-path dependencies before network fetching.

### 7.4 REPL
A REPL enables quick experimentation.

**Action**: Wire the JIT backend to a readline loop. Each expression or statement entered should be compiled and immediately executed.

---

## Sprint 8: Test Coverage (Priority: MEDIUM)

### 8.1 Fix pre-existing test failures
- `test/basic/test_basic_functionality.vyn`: empty file — add a proper test or delete.
- `test/basic/circular_struct.vyn`: document expected behavior (should fail gracefully).
- `test/basic/direct_return.vyn`: fix multi-value codegen (see Sprint 2.2).

### 8.2 Add ownership tests
No tests validate `my<T>` move semantics, `our<T>` ref-counting, or `their<T>` lifetime enforcement. These are critical for correctness.

### 8.3 Add FFI tests
No tests exercise `extern "C"` declarations or `freedom` block pointer arithmetic.

### 8.4 Add error propagation tests
The `fail`/`trap` system has partial coverage. Add tests for:
- Propagating errors across multiple function calls
- Trapping errors with specific types
- Stack traces on untrapped errors

---

## Consistency Issues to Fix (Miscellaneous)

### Syntax consistency
- **Vec methods**: `pop()` removes the last element but there's no `peek()` (non-removing last element). Add `Vec::last()` or `Vec::peek()`.
- **String indexing**: `str.char_at(i)` vs `str[i]` — pick one and document it.
- **Struct construction**: support both `Point { x = 1, y = 2 }` and positional `Point(1, 2)` or document that only named-field syntax is supported.

### Implementation consistency
- **`borrow` prefix vs `borrow()` function call**: both syntaxes exist. The canonical form (per `Canonical_Reference_Syntax.md`) is `borrow(expr)`. The prefix form `borrow expr` also works. Pick one as *the* form and document the other as deprecated/discouraged.
- **`their<T>` as pointer**: `their<T>` compiles to a pointer-to-T in LLVM, which is correct. But the semantic analyzer sometimes fails to dereference through it for nested member access. Audit all places where `their<T>` fields are accessed transitively.
- **Vec cleanup in `return`**: the current heuristic (check if returned identifier matches a Vec variable) works for direct returns but may miss returns via expressions. Consider a more robust "transfer ownership on return" pass.

---

## Architecture Suggestions

### A. Separate semantic analysis from AST mutation
The semantic analyzer currently mutates AST nodes (sets `node->type`, `expressionTypes[node]`, etc.). This creates fragile dependencies between semantic passes. Consider:
- Using an immutable AST + a separate `TypeTable` (map from node ID → type)
- Avoiding raw pointer storage in `expressionTypes` (use stable IDs or shared_ptr)

### B. IR optimization as a separate phase
Currently IR optimization is applied inline during JIT setup. Consider separating it into an explicit `optimize(module)` step that can be controlled per-compilation-target.

### C. Error recovery in parsing
The parser currently throws on the first error. Adding error recovery (synchronize to the next statement/declaration boundary) would allow reporting multiple errors per file, greatly improving the developer experience.

---

## 1.0 Definition

A Vyn 1.0 release should satisfy:

1. **Core language stable**: all syntax in `doc/` works correctly with no unexpected panics or segfaults.
2. **Test suite ≥ 99%**: the focused `test/new_features` suite and a new `test/conformance` suite both pass.
3. **Silent by default**: no DEBUG output in normal mode; `--verbose` shows internals.
4. **Module system works**: at least local single-file imports resolve and execute.
5. **Ownership enforcement in semantic analysis**: `my<T>` moves are checked; `their<T>` cannot outlive its borrowee in basic cases.
6. **Standard library usable**: Vec, String, HashMap, File I/O, Option<T>.
7. **Documentation complete**: `README.md`, `doc/FEATURE_STATUS.md`, and a Vyn Language Reference are accurate and up-to-date.
8. **Tooling**: `vyn build` (AOT), `vyn run` (JIT), `vyn test` subcommands.

---

*Last updated: 2026-02-22*
