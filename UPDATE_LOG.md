# Vyn Implementation Update Log

Tag: `implementation-audit-2026-05-23`
Audit date: 2026-05-23

This log consolidates what still needs to be implemented after scanning the
repository docs, future-feature tests, and relevant compiler/runtime sources.
It is intentionally source-biased: when docs conflict, source code and
expect-fail tests are treated as stronger evidence than optimistic status text.

## Implementation Progress

- 2026-05-23: Implemented first-pass `extern "C" { ... }` block support for
  function signatures. Blocks now parse as declaration groups, semantic analysis
  visits their members, and the LLVM path emits external function declarations.
  Added `test/ffi/extern_c_malloc_free.vyn` as a JIT smoke test for calling C
  `malloc` and `free`.
- 2026-05-23: Extended the JIT FFI path to resolve host process symbols and
  added a narrow `String`-to-C-string call conversion. Added
  `test/ffi/extern_c_puts.vyn` to cover calling libc `puts`.
- 2026-05-23: Restricted direct calls to `extern "C"` functions to
  `freedom { }` blocks and added a negative FFI test for calls outside freedom.
- 2026-05-23: Completed a focused tests/examples/demos repair pass. Semantic
  analysis now preserves resolved function parameter, return, and struct field
  types, and ordinary function calls infer declared return types. Reworked the
  example suite so every `examples/*.vyn` file is runnable with the current
  compiler, added `demos/` for curated language demonstrations, refreshed test
  organization docs, and documented remaining ownership/runtime gaps in
  `reports/TEST_EXAMPLE_DEMO_REVIEW.md` and
  `reports/LANGUAGE_COMPLETION_REVIEW.md`.
- 2026-05-23: Began module finalization by resolving local `.vyn` imports in
  the driver before semantic analysis and codegen. `import nested::module`
  resolves to `nested/module.vyn`, `import name from "./file.vyn"` resolves
  relative file locators, imported declarations are spliced before use, and
  duplicate/circular imports are guarded. Added module runtime tests and a
  runnable example.

## Audit Scope

Docs/status sources reviewed:

- `README.md`, `TODO.md`, `CHANGELOG.md`
- `doc/FEATURE_STATUS.md`, `doc/ROADMAP.md`, `doc/TODO_CURRENT.md`
- Module/FFI docs: `doc/MODULE_FFI_BINARY_ROADMAP.md`, `doc/FFI_DESIGN.md`,
  `doc/bundles_and_sharing.md`, `doc/module_visibility.md`
- Error docs: `doc/ERROR_TRAP.md`, `doc/ERROR_PROPAGATION_DESIGN.md`,
  `doc/ENSURE_IMPLEMENTATION_STATUS.md`, `test/trap/README.md`,
  `test/trap/TEST_RESULTS.md`
- Ownership/memory docs: `doc/OWNERSHIP_MILD.md`, `doc/Memory_Operations.md`,
  `doc/mem_RFC.md`, `test/memory/README.md`, `examples/README.md`
- Aspect/generic docs: `doc/ASPECT_BOUNDS.md`,
  `doc/TRAIT_SYSTEM_DESIGN.md`, `doc/SELF_RESOLUTION_COMPLETE.md`,
  `test/aspect/PHASE_6_ROADMAP.md`
- Lambda, async, string, tuple, Vec, introspection, AST, and test docs under
  `doc/` and `test/`

Source areas checked:

- Parser: `src/parser/*`, `include/vyn/parser/*`
- Semantic analysis: `src/vre/semantic.cpp`, `include/vyn/semantic.hpp`
- LLVM codegen: `src/vre/llvm/*`, `include/vyn/vre/llvm/codegen.hpp`
- Runtime: `src/runtime/*`, `include/vyn/runtime/*`, `runtime/*`
- Future tests: `test/future_features/*.vyn`

## Highest Priority Implementation Backlog

| ID | Area | Priority | What needs to be implemented | Evidence |
|----|------|----------|------------------------------|----------|
| I-001 | Module system | P0 | Real module loading, caching, symbol tables, import resolution, circular dependency detection, `bundle(...)`, `share(...)`, visibility checks, module paths, and stdlib module auto-discovery. | `doc/MODULE_FFI_BINARY_ROADMAP.md`; `src/vre/semantic.cpp` has an empty `visit(ImportDeclaration*)`; `src/vre/llvm/cgen_decl.cpp` treats imports as no-ops. |
| I-002 | FFI | P0 | Continue FFI after first-pass `extern "C"` block support: fuller C type mapping, `repr(C)` structs, variadic calls, `--link` support, explicit `String::as_c_str()`, and broader end-to-end FFI tests. | `doc/FFI_DESIGN.md` says planned; source now supports simple extern C function-signature blocks, host process symbol lookup, freedom-gated direct calls, and a narrow String-to-C-string call path, but still lacks variadic/repr(C)/linker flow. |
| I-003 | Ownership runtime | P0 | Enforce `my<T>` moves, `our<T>` strong ref counts, `their<T>` lifetime/aliasing rules, `view`/`borrow` semantics, and `soft()` conversion rules. | `TODO.md`; `doc/mem_RFC.md`; `src/vre/semantic.cpp` borrow visitor notes missing type/lifetime implementation. |
| I-004 | `mild<T>` weak references | P0 | Implement real control blocks, `weak_count`, released state, valid `grab()` upgrade to `our<T>`, and cleanup when strong/weak counts reach zero. | `doc/OWNERSHIP_MILD.md`; `examples/README.md`; `test/ownership/mild_methods_test.vyn` and examples describe current stubs. |
| I-005 | Error propagation/runtime errors | P0 | Finish cross-function error propagation, construct real `VynError` objects at `fail`, preserve type/data/source location, print detailed untrapped errors, and settle Result-vs-fail/trap design conflict. | `doc/ERROR_PROPAGATION_DESIGN.md`; `test/trap/TEST_RESULTS.md`; `src/runtime/error_handling.cpp` says error structure is not implemented. |
| I-006 | Defer/runtime cleanup | P0 | Decide whether runtime defer/ensure stacks are needed; implement runtime defer stack if `defer` must survive fail/unwind paths. | `src/runtime/error_handling.cpp` has defer/ensure stubs; `src/vre/llvm/cgen_stmt.cpp` stores defers in a codegen stack. |
| I-007 | Aspect completion | P0 | Associated types, aspect objects/dynamic dispatch, aspect inheritance, bounded bind selection precedence, and method monomorphization for generic binds. | `TODO.md`; `doc/ASPECT_BOUNDS.md`; `test/aspect/PHASE_6_ROADMAP.md`; semantic monomorphization has TODO/stub paths. |
| I-008 | Stdlib foundation | P0 | Implement `Option<T>`, decide and implement/document `Result<T,E>`, core aspects (`Display`, `Debug`, `Clone`, `Equatable`, `Comparable`, `Hashable`), `Iterator`, File I/O, maps/sets, and remaining String/Vec helpers. | `TODO.md`; `doc/STRING_IMPLEMENTATION.md`; `test/future_features/test_option_type.vyn`, `test_result_type.vyn`; FFI is a blocker for File I/O. |
| I-009 | Async runtime semantics | P1 | Replace placeholder await behavior with real scheduling, future value storage, suspension/resumption, task spawning, and eventually async I/O integration. | `TODO.md`; `src/vre/llvm/cgen_expr.cpp` awaits with dummy task id and returns the input future; `src/runtime/async_runtime.cpp` stores value support as future work. |
| I-010 | Lambda/closures | P1 | Implement real lambda return type inference, non-void lambda returns, function value/call semantics, closure capture structs, capture extraction, move/mutable captures, generic and async lambdas. | `doc/LAMBDAS.md`; `test/future_features/test_lambda_codegen.vyn`; `src/vre/llvm/cgen_expr.cpp` defaults lambda return type to void and lacks capture handling. |
| I-011 | Pattern matching/select polish | P1 | Struct/tuple destructuring, enum variant patterns, range patterns, guards, exhaustiveness, match-as-expression, and better select type inference. | `TODO.md`; `doc/AST_Roadmap.md`; source warns that complex patterns and select inference are incomplete. |
| I-012 | Enums/sum types | P1 | Implement tagged enum variants with payloads, enum construction, enum methods via bind, pattern matching on variants, and stdlib `Option`/`Result` support. | `test/future_features/test_enum_basic.vyn`; `src/vre/llvm/cgen_decl.cpp` reports enum codegen is not fully implemented. |
| I-013 | Vec correctness/polish | P1 | Add bounds checking to `get`, return the actual popped value from `pop`, implement real `contains`, `concat`, `push_array`, `to_array`, `get_array`, `get_vec`, and improve element type tracking for `Vec<Struct>`. | `src/vre/llvm/cgen_vec.cpp`; `doc/VEC_ITERATION.md` notes `Vec<Struct>`/complex-expression limitations. |
| I-014 | Tuple completion | P1 | Tuple serialization/output, tuple variables, `.0`/`.1` element access, destructuring assignment, and tuple pattern matching. | `test/tuples/README.md`; `TODO.md`. |
| I-015 | Generic/template monomorphization | P1 | Finish template instantiation, AST clone/substitution, constructor inference, nested generics, member template instantiation, and bounds-checked instantiation. | `src/vre/semantic.cpp` has monomorphization stubs; `test/template/generics_examples.vyn`; `doc/SELF_RESOLUTION_COMPLETE.md` lists constructor/Vec issues. |
| I-016 | Introspection completion | P1 | First-class `Type`, type registry initialization, type equality assertions, downcasting/as operator, and `typeof` in wildcard trap handlers. | `TODO.md`; `doc/INTROSPECTION_DESIGN.md`. |
| I-017 | Auto-serialization/metadata edges | P1 | Re-enable/fix main auto-serialization where disabled, handle nested structs and Vec in metadata serialization/deserialization, and dynamic buffer sizing. | `src/main.cpp`; `src/vre/llvm/cgen_decl.cpp`; `runtime/vyn_type_metadata.c`. |
| I-018 | Build optimization pipeline | P2 | Complete LLVM pass pipeline for all optimization levels, add LTO/ThinLTO, bitcode flows, benchmarks, and linker/library flag handling. | `TODO.md`; `doc/MODULE_FFI_BINARY_ROADMAP.md`. |
| I-019 | Developer tooling | P2 | `vyn.toml`, `vyn build` project mode, `vyn test`, package resolution/lockfile, formatter, linter, LSP, REPL, and `vyn doc`. | `TODO.md`; `doc/Development_Guide.md`. |
| I-020 | Networking | P2 | Implement raw socket FFI bindings, `TcpStream`, `UdpSocket`, listener APIs, async socket I/O, and HTTP client after FFI and async runtime are real. | `TODO.md` networking section; depends on I-002 and I-009. |

## Source-Level TODO Hotspots

- `src/vre/semantic.cpp`: borrow/view typing, optional/result typing, template
  monomorphization, generic aspect implementation handling, Vec type validation,
  and richer type checking are incomplete.
- `src/vre/llvm/cgen_expr.cpp`: await is placeholder-level, list
  comprehensions are unimplemented, generic instantiation is TODO, `this`/`super`
  are placeholders, select type inference is incomplete, and lambda codegen lacks
  closure semantics.
- `src/vre/llvm/cgen_stmt.cpp`: legacy try/catch/throw codegen is stubbed or
  obsolete relative to `fail`/`trap`; untrapped `fail` does not build a full
  `VynError`.
- `src/vre/llvm/cgen_vec.cpp`: several Vec methods return placeholders or only
  simulate copies.
- `src/runtime/error_handling.cpp`: untrapped error details, defer stack, and
  ensure stack are stubs.
- `runtime/vyn_type_metadata.c`: dynamic sizing, Vec metadata, and nested struct
  handling are TODOs.

## Documentation Conflicts To Resolve

These should be fixed before using the docs as release guidance.

1. FFI status conflict:
   `doc/FEATURE_STATUS.md` marks `extern "C"` as implemented, while
   `doc/FFI_DESIGN.md`, `doc/MODULE_FFI_BINARY_ROADMAP.md`, and
   `test/future_features/test_ffi_extern_c.vyn` show it as planned/expect-fail.
   Source appears partial, not complete.

2. Error handling status conflict:
   `doc/ERROR_TRAP.md` says core error handling phases are complete, while
   `doc/ERROR_PROPAGATION_DESIGN.md`, `test/trap/TEST_RESULTS.md`, and runtime
   source still show cross-function propagation and full `VynError` construction
   as incomplete.

3. `ensure` meaning conflict:
   `doc/ENSURE_IMPLEMENTATION_STATUS.md` documents block cleanup
   `} ensure -> { ... }` as complete. `test/future_features/test_ensure_statement.vyn`
   and `TODO.md` describe contract-style `ensure condition else fail(...)` as
   unimplemented. These are two different features and need separate names/status.

4. Aspect/Self status conflict:
   `test/aspect/PHASE_6_ROADMAP.md` says Self resolution is partially complete,
   while `doc/SELF_RESOLUTION_COMPLETE.md` says it is complete but still lists
   Vec and constructor inference issues. Update Phase 6 docs to reflect current
   source behavior.

5. Class/OOP direction conflict:
   `doc/WHY_TRAITS_NOT_CLASSES.md` says classes are not planned, while older AST
   and trait design docs still mention classes/inheritance. Decide whether
   classes are removed or legacy parser-only support.

6. Terminology conflict:
   Docs mix `trait`/`impl` with `aspect`/`bind`. The implementation and README
   are mostly `aspect`/`bind`; docs should standardize or explicitly mark old
   names as historical aliases.

7. Syntax conflict:
   Docs mix `fn`, colon-style parameter syntax, `=>` match arms, and
   `<T: Trait>` bounds with newer name-first syntax, `->`, and `<T<Aspect>>`.
   Update examples or mark legacy syntax as deprecated.

8. Production-ready language claims:
   Several docs call Vyn production-ready, but the source audit shows major 1.0
   blockers remain. Release/status docs should use a more precise feature
   matrix and avoid broad production-ready claims until P0 items are complete.

## Suggested Implementation Order

1. Reconcile status docs and future-feature tests so the project has one
   canonical source of truth.
2. Finish a minimal module system and FFI path because those unblock File I/O,
   stdlib modules, networking, and multi-file programs.
3. Complete ownership runtime enforcement, especially `mild<T>`, before expanding
   container/resource APIs that depend on lifecycle correctness.
4. Finish error propagation and real `VynError` construction so `fail`/`trap`
   works across function boundaries and produces useful runtime diagnostics.
5. Complete aspect associated types and iterator design, then implement
   `Iterator`, `Option`, core aspects, and stdlib collections.
6. Complete lambdas, enum/sum types, and pattern matching together because they
   share closure/function-value, variant, and destructuring semantics.
7. Round out developer tooling (`vyn.toml`, `vyn build`, `vyn test`, formatter,
   LSP, REPL) after core language semantics settle.

## Verification Notes

The original implementation audit was source/documentation-focused. The
tests/examples/demos repair pass was behavior-checked with:

- `cmake --build build -j2`
- `python3 test/run_tests.py --test-dir test/new_features --vyn build/vyn --execute-jit`
- `python3 test/run_tests.py --test-dir test/ffi --vyn build/vyn --execute-jit`
- `build/vyn` over every `examples/*.vyn`
- `build/vyn` over every `demos/*.vyn`
