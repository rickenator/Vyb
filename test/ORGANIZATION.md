# Test Directory Organization

Current `.vyn` test count: 671.

| Path | Count | Purpose |
|------|------:|---------|
| `test/units/` | 230 | Legacy and unit-level language checks |
| `test/trap/` | 52 | `fail`/`trap`/`panic` behavior |
| `test/debug/` | 48 | Debugging and regression scratch tests |
| `test/parser/` | 43 | Parser syntax coverage |
| `test/basic/` | 39 | Basic runtime and semantic behavior |
| `test/new_features/` | 39 | Current integrated feature tests |
| `test/aspect/` | 28 | Aspect/bind system coverage |
| `test/vectors/` | 21 | Vec method and typing coverage |
| `test/vec_for/` | 17 | Vec and range loop behavior |
| `test/tuples/` | 14 | Tuple and multiple-return coverage |
| `test/future_features/` | 14 | Syntax and designs not yet complete |
| `test/string/` | 15 | String methods and literals |
| `test/template/` | 11 | Template/generic behavior |
| `test/select_match/` | 11 | `match` and `select` |
| `test/ownership/` | 11 | Ownership syntax and lifecycle experiments |
| `test/arrays/` | 7 | Fixed array behavior |
| `test/range_for/` | 7 | Range loop variants |
| `test/introspection/` | 6 | `typeof` and `typename` |
| `test/math/` | 6 | Math intrinsics |
| `test/stack_trace/` | 6 | Stack trace diagnostics |
| `test/types/` | 6 | Primitive and sized types |
| `test/memory/` | 5 | Ownership and memory examples |
| `test/stdlib/` | 5 | Error and stdlib sketches |
| `test/syntax/` | 4 | Canonical syntax checks |
| `test/tree_structures/` | 4 | Tree structure examples |
| `test/modules/` | 4 | Local module import resolution |
| `test/compilation/` | 3 | Compilation pipeline examples |
| `test/ffi/` | 3 | `extern "C"` and `freedom` FFI checks |
| `test/async/` | 2 | Async syntax/runtime experiments |
| `test/json/` | 2 | JSON conversion checks |
| top-level `.vyn` tests | 6 | Legacy focused smoke tests |

`test/future_features/` is intentionally allowed to contain incomplete language
designs. Tests outside that directory should either execute successfully or be
marked `@parse-only` / `@semantic-only` when they cover a feature whose runtime
is not complete yet.
