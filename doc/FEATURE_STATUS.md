# Vyn Feature Status (v0.5.0)

This document tracks the implementation status of Vyn language features.

Legend: ✅ Implemented | 🚧 Partial / Stubbed | 📋 Planned

---

## Module System

| Feature | Status | Notes |
|---------|--------|-------|
| `import <path>` | ✅ | Parses module path (`::` or `.` separated) |
| `import <path> as <alias>` | ✅ | Alias binding at parse level |
| `import <path> from "<locator>"` | ✅ | Locator string parsed and stored in AST |
| `smuggle <path> from "<locator>"` | ✅ | Locator string parsed and stored in AST |
| `smuggle <path> as <alias>` | ✅ | Alias binding at parse level |
| `ImportKind` (TrustedImport / Smuggle) | ✅ | Captured in AST `ImportDeclaration.kind` |
| `from` keyword | ✅ | Lexed as `KEYWORD_FROM`; also valid in `from<T>(addr)` freedom-block expressions |
| Module resolution (load files) | 🚧 | Pre-codegen resolution stub; imports are no-ops at codegen |
| Local path loading (`from "./..."`) | 📋 | v0.5.x |
| URL/Git fetching (`from "github.com/..."`) | 📋 | v0.6.x |
| Module cycle detection | 📋 | v0.5.x |
| Symbol re-export | 📋 | v0.6.x |

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
| Enums | ✅ | |
| Generics (monomorphization) | ✅ | |
| Aspect/Bind polymorphism | ✅ | |
| Ownership: `my`, `our`, `their`, `mild` | ✅ | |
| `freedom` blocks + `loc<T>` raw pointers | ✅ | |
| `match` / `select` expressions | ✅ | |
| `defer` | ✅ | |
| `fail` / `trap` error system | ✅ | |
| `async` / `await` | 🚧 | Runtime stub |
| `Vec<T>` | ✅ | |
| String methods | ✅ | |
| Math intrinsics | ✅ | |
| `typeof` / `typename` | ✅ | |
| Templates | ✅ | |

## Compilation Pipeline

| Feature | Status | Notes |
|---------|--------|-------|
| LLVM IR codegen | ✅ | |
| JIT execution | ✅ | |
| AOT native executable | ✅ | `--build` flag |
| Multi-file compilation | 📋 | v0.5.x (module resolution) |
| `extern "C"` FFI | ✅ | |

---

*Last updated: v0.5.0*
