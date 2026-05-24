# Vyn Examples

Runnable Vyn programs demonstrating the current language surface.

```bash
build/vyn examples/<file>.vyn
```

All top-level `.vyn` files in this directory are expected to execute
successfully with the current compiler. Support modules live under
`examples/modules/`. Heavier feature walkthroughs live in `demos/`.

## Examples

| Example | Covers |
|---------|--------|
| `main.vyn` | arithmetic, structs, `Vec<T>`, `match`, strings, recursion, `defer` |
| `sort.vyn` | insertion sort with `Vec<Int>` helper functions |
| `quicksort.vyn` | recursive quicksort returning `Vec<Int>` |
| `stack.vyn` | stack helpers over a struct containing `Vec<Int>` |
| `binary_tree_clean.vyn` | flat tree-node storage and lookup with `Vec<TreeNode>` |
| `vec_filter.vyn` | filtering values into a second vector |
| `vec_max.vyn` | scanning a vector for a maximum value |
| `vec_point_distance.vyn` | iterating over `Vec<Point>` structs |
| `memory_semantics.vyn` | `freedom`, `loc<T>`, and `at(ptr)` |
| `mild_references.vyn` | `our<T>` and `mild<T>` syntax |
| `ffi_puts.vyn` | `extern "C"` plus `freedom`-gated calls |
| `module_import.vyn` | local module import from `examples/modules/` |
| `module_visibility.vyn` | `bundle(...)`, `share(...)`, and selective import aliases |

### Module path demo

This repository also includes a multi-file `--module-path` demo:

```bash
build/vyn examples/module_path_demo/main.vyn --module-path examples/module_path_demo/modules
```

## Known Boundaries

The examples now include runtime `Vec<T>` helper returns for common algorithm
paths. Remaining ownership work is broader: enforcing full move/drop rules for
all owned aggregates and documenting the final borrow/mutation contract.
