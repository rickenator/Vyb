# Tests, Examples, And Demos Review

Date: 2026-05-23

## Inventory

| Area | Count | Status |
|------|------:|--------|
| Tests | 671 `.vyn` files | Organized by feature area; docs updated |
| Examples | 12 top-level `.vyn` programs | All execute with `build/vyn examples/<file>.vyn` |
| Demos | 4 `.vyn` files | Added runnable demos under `demos/` |

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
  `from "./file.vyn"` module resolution.
- Added demos for control flow, collections, aspects, and FFI freedom blocks.

## Verification

```bash
cmake --build build -j2
python3 test/run_tests.py --test-dir test/new_features --vyn build/vyn --execute-jit
python3 test/run_tests.py --test-dir test/ffi --vyn build/vyn --execute-jit
python3 test/run_tests.py --test-dir test/modules --vyn build/vyn --execute-jit
for f in examples/*.vyn; do build/vyn "$f"; done
for f in demos/*.vyn; do build/vyn "$f"; done
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
