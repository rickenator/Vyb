# VyB Language Completion Review

Date: 2026-05-23

VyB is usable, but not complete. The remaining completion work is now explicit:

| Area | Missing Work |
|------|--------------|
| Ownership runtime | Finish full move/drop enforcement for owned aggregates; current `Vec<T>` function parameter/return cases have runtime coverage |
| Borrowing | Complete `their<T>`/`borrow` semantics outside ad hoc cases, including mutable borrow validation |
| FFI | Add `repr(C)` structs, ABI validation, varargs policy, header/library linking, and platform symbol tests |
| Modules | Local `.vyb` imports now resolve and splice declarations; package metadata, bundle visibility, aliases/specifiers, and remote `smuggle` locators remain |
| Error model | Finish typed `fail`/`trap` propagation and integrate it with runtime/AOT paths |
| Sum types | Complete enums, `Option<T>`, and `Result<T,E>` as first-class runtime features |
| Generics/aspects | Finish bounded generic validation and monomorphized method dispatch across modules |
| Lambdas/closures | Complete closure capture typing and codegen |
| Async/concurrency | Complete async lowering and runtime scheduling beyond syntax experiments |
| Standard library | Stabilize collections, strings, I/O, filesystem, process, networking, and math contracts |
| Tooling | Full test-suite classification, package metadata, formatter/linter, diagnostics cleanup |

Priority order:

1. Ownership runtime for owned aggregates beyond the currently verified `Vec<T>` helper-return paths.
2. Borrowing semantics for `their<T>` and mutation boundaries.
3. Module/package implementation beyond local file imports.
4. FFI ABI completion.
5. Sum types and typed error propagation.

The previous immediate blocker, by-value `Vec<T>` helper returns, is now covered
by runtime tests and examples. The remaining ownership work is the broader
language contract: move/drop enforcement, aggregate ownership, and precise
borrow mutation boundaries.
