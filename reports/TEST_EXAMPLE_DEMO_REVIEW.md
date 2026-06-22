# Tests, Examples, And Demos Review

Date: 2026-05-23

## Inventory

| Area | Count | Status |
|------|------:|--------|
| Tests | 671 `.vyb` files | Organized by feature area; docs updated |
| Examples | 12 top-level `.vyb` programs | All execute with `build/vyb examples/<file>.vyb` |
| Demos | 4 `.vyb` files | Added runnable demos under `demos/` |

## Fixes Applied

- Fixed semantic resolution so `Vec<T>` function parameters, function return
  annotations, and struct fields store the resolved `VecType`.
- Added ordinary function-call return typing, including recursive calls, so
  expressions like `sorted = insert_sorted(...)` receive their declared return
  type.
- Restored algorithm and stack tests to runtime execution after rebasing onto
  current `main`, which includes `Vec<T>` deep-copy handling for function
  parameters and returns.
- Expanded examples to cover recursive quicksort, insertion sort helper
  returns, and struct-backed stack helpers at runtime.
- Added local module import coverage for same-directory, nested-path, and
  `from "./file.vyb"` module resolution.
- Added demos for control flow, collections, aspects, and FFI freedom blocks.

## Verification

```bash
cmake --build build -j2
python3 test/run_tests.py --test-dir test/new_features --vyb build/vyb --execute-jit
python3 test/run_tests.py --test-dir test/ffi --vyb build/vyb --execute-jit
python3 test/run_tests.py --test-dir test/modules --vyb build/vyb --execute-jit
for f in examples/*.vyb; do build/vyb "$f"; done
for f in demos/*.vyb; do build/vyb "$f"; done
```

All commands above passed during this review after running the examples and
demos sequentially.

## Remaining Test Debt

- Full-suite execution still needs triage by directory. `test/new_features`,
  `test/ffi`, and `test/modules` are the current verified runtime suites from
  this pass.
- `test/debug/` contains scratch/regression files and should be split into
  runnable regression tests versus archived repros.
- Generated artifacts and backup files under `test/` should be removed from
  version control in a separate cleanup.
