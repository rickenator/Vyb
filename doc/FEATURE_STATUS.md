# Vyn Feature Status (v0.5.0)

This document tracks the implementation status of Vyn language features.

Legend: ✅ Implemented | 🚧 Partial / Stubbed | 📋 Planned

---

## Module System

| Feature | Status | Notes |
|---------|--------|-------|
| `import <path>` | ✅ | Parses module path and resolves local `.vyn` files (`::` or `.` separated) |
| `import <path> as <alias>` | 🚧 | Whole-module alias parses; namespace binding still planned |
| `import <path>::{symbol as alias}` | ✅ | Selective import specifiers filter and rename imported declarations |
| `import <path> from "<locator>"` | ✅ | Locator string parsed and stored in AST |
| `smuggle <path> from "<locator>"` | ✅ | Locator string parsed and stored in AST |
| `smuggle <path> as <alias>` | ✅ | Alias binding at parse level |
| `ImportKind` (TrustedImport / Smuggle) | ✅ | Captured in AST `ImportDeclaration.kind` |
| `from` keyword | ✅ | Lexed as `KEYWORD_FROM`; also valid in `from<T>(addr)` freedom-block expressions |
| Module resolution (load files) | ✅ | Local files are loaded and spliced before semantic analysis/codegen |
| Local path loading (`from "./..."`) | ✅ | Relative locators resolve from the importing file |
| `bundle(...)` visibility | ✅ | Source-level directives are enforced by the local resolver |
| `share(...)` exports | ✅ | `share(all)` and bundle-scoped shares export declarations/imports |
| `smuggle` visibility bypass | ✅ | Smuggled imports bypass share/bundle checks |
| URL/Git fetching (`from "github.com/..."`) | 📋 | v0.6.x |
| Module cycle detection | ✅ | Circular local imports are rejected |
| Symbol re-export | ✅ | `share(...)` before an import re-exports selected imported declarations |

## Println / Output

| Feature | Status | Notes |
|---------|--------|-------|
| `println(x)` for string types | ✅ | Extracts `char*` from String struct |
| `println(x)` for Int | ✅ | Auto-converts via `__vyn_int_to_string` |
| `println(x)` for Float | ✅ | Auto-converts via `__vyn_float_to_string` |
| `println(x)` for Bool | ✅ | Auto-converts via `__vyn_bool_to_string` |
| `println(x)` for Vec/arrays | ✅ | Array serialization |
| `println(x)` for structs | ✅ | JSON/generic serialization |
| `print(x)` (no newline) | ✅ | Same auto-stringify as println |
| `println(a, b, c, ...)` multiple args | ✅ | Space-separated; all args formatted into one output call |
| `println_int()` / `println_bool()` variants | 🚧 | Still present for compatibility; prefer `println()` |

## String Concatenation

| Feature | Status | Notes |
|---------|--------|-------|
| `String + String` | ✅ | Direct struct concat |
| `String + Int` | ✅ | Auto-coercion via `generateMixedStringConcatenation` |
| `String + Float` | ✅ | Auto-coercion |
| `String + Bool` | ✅ | Auto-coercion |
| `Int + String` | ✅ | Auto-coercion |
| String literal `+` non-string | ✅ | String struct detection handles this case |

## Core Language

| Feature | Status | Notes |
|---------|--------|-------|
| Functions (name-first syntax) | ✅ | |
| Structs | ✅ | |
| Enums | ✅ | C-like integer enums: variants map to sequential `i64` constants; `Enum::Variant` access works; tagged unions (data variants) planned for v0.6 |
| Generics (monomorphization) | ✅ | |
| Aspect/Bind polymorphism | ✅ | |
| Ownership: `my`, `our`, `their`, `mild` | 🚧 | Syntax/runtime pieces plus lexical borrow enforcement; full move/drop checker still planned |
| `freedom` blocks + `loc<T>` raw pointers | ✅ | |
| `match` / `select` expressions | ✅ | |
| `defer` | ✅ | |
| `fail` / `trap` error system | ✅ | Includes typed `fail<T>(value)`, typed traps, wildcard/multi-type trap parsing |
| `async` / `await` | 🚧 | Runtime stub |
| `Vec<T>` | ✅ | |
| String methods | ✅ | |
| Math intrinsics | ✅ | |
| `typeof` / `typename` | ✅ | |
| Templates | ✅ | |

## Vec<T>

| Feature | Status | Notes |
|---------|--------|-------|
| `Vec::new()` | ✅ | Heap-allocated dynamic array |
| `Vec::push()` | ✅ | Append element |
| `Vec::pop()` | ✅ | Remove last element |
| `Vec::len()` | ✅ | Returns element count |
| `Vec::get()` | ✅ | Index access |
| `Vec::contains()` | ✅ | Fixed: now emits correct LLVM comparison loop (was hardcoded `false`) |
| `Vec::push()` on borrowed struct fields | ✅ | Fixed: `s.items.push(val)` where `s<their<T>>` now mutates in-place |
| Vec parameter deep copy | ✅ | Vec parameters receive an independent copy on function entry (fixes double-free in recursive algorithms) |
| `Vec::map()` / `filter()` / `reduce()` | 📋 | Requires lambda codegen + Iterator aspect |
| `for (item in vec)` iteration | ✅ | Compiler-generated loop with break/continue |

## Lambdas / Closures

| Feature | Status | Notes |
|---------|--------|-------|
| Lambda parsing `\|x, y\| -> expr` | ✅ | |
| Lambda parsing `\|x<Int>\| -> { block }` | ✅ | |
| Capture detection (semantic) | ✅ | |
| Type inference on lambda body | ✅ | |
| Indirect call from local variable | ✅ | `localLambdaTypes` map; return type coercion working |
| Full closure struct codegen | 📋 | Capture extraction + struct allocation |
| Move capture (`my<T>` into closure) | 📋 | |
| `our<T>` shared capture | 📋 | |



| Feature | Status | Notes |
|---------|--------|-------|
| LLVM IR codegen | ✅ | |
| JIT execution | ✅ | |
| AOT native executable | ✅ | `--build` flag |
| Multi-file compilation | 📋 | v0.5.x (module resolution) |
| `extern "C"` FFI | ✅ | Extern blocks parse/codegen, freedom-gated calls, C ABI scalar/pointer aliases |

---

*Last updated: v0.5.1 (2026-02-22)*
