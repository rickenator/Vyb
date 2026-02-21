## Consistency Work

1. Treat i32/i64 as internal only (surface types are Int/Float/Bool/String/UInt).
2. Standardize tests/examples to use auto-stringifying `println` and Java-like string concatenation.
3. Reconcile docs vs runtime regarding `println_int`/`println_bool` intrinsics.

### Action Items

- Fix or move non-Vyn *.vyn fixtures (e.g. extracted tests containing C++ snippets).
- Pick a single canonical test runner.