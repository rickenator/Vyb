# Vyn Language Completion Review

Date: 2026-05-23

Vyn is usable, but not complete. The remaining completion work is now explicit:

| Area | Missing Work |
|------|--------------|
| Ownership runtime | Correct move/clone/drop behavior for `Vec<T>` and structs containing owned data; eliminate shallow-copy double frees |
| Borrowing | Complete `their<T>`/`borrow` semantics outside ad hoc cases, including mutable borrow validation |
| FFI | Add `repr(C)` structs, ABI validation, varargs policy, header/library linking, and platform symbol tests |
| Modules | Implement `import`/`smuggle`/package resolution beyond documented syntax |
| Error model | Finish typed `fail`/`trap` propagation and integrate it with runtime/AOT paths |
| Sum types | Complete enums, `Option<T>`, and `Result<T,E>` as first-class runtime features |
| Generics/aspects | Finish bounded generic validation and monomorphized method dispatch across modules |
| Lambdas/closures | Complete closure capture typing and codegen |
| Async/concurrency | Complete async lowering and runtime scheduling beyond syntax experiments |
| Standard library | Stabilize collections, strings, I/O, filesystem, process, networking, and math contracts |
| Tooling | Full test-suite classification, package metadata, formatter/linter, diagnostics cleanup |

Priority order:

1. Ownership runtime for owned aggregates and `Vec<T>`.
2. Borrowing semantics for `their<T>` and mutation boundaries.
3. Module/package implementation.
4. FFI ABI completion.
5. Sum types and typed error propagation.

The immediate blocker for richer examples is `Vec<T>` ownership. Semantic
coverage exists, but runtime examples avoid by-value `Vec<T>` helper returns
until moves or deep clones are implemented.
