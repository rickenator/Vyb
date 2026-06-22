# Vyb Examples

Runnable Vyb programs demonstrating the current language surface.

```bash
build/vyb examples/<file>.vyb
```

All top-level `.vyb` files in this directory are expected to execute
successfully with the current compiler. Support modules live under
`examples/modules/`. Heavier feature walkthroughs live in `demos/`.

## Examples

| Example | Covers |
|---------|--------|
| `main.vyb` | arithmetic, structs, `Vec<T>`, `match`, strings, recursion, `defer` |
| `sort.vyb` | insertion sort with `Vec<Int>` helper functions |
| `quicksort.vyb` | recursive quicksort returning `Vec<Int>` |
| `stack.vyb` | stack helpers over a struct containing `Vec<Int>` |
| `binary_tree_clean.vyb` | flat tree-node storage and lookup with `Vec<TreeNode>` |
| `vec_filter.vyb` | filtering values into a second vector |
| `vec_max.vyb` | scanning a vector for a maximum value |
| `vec_point_distance.vyb` | iterating over `Vec<Point>` structs |
| `memory_semantics.vyb` | `freedom`, `loc<T>`, and `at(ptr)` |
| `mild_references.vyb` | `our<T>` and `mild<T>` syntax |
| `ffi_puts.vyb` | `extern "C"` plus `freedom`-gated calls |
| `module_import.vyb` | local module import from `examples/modules/` |
| `module_visibility.vyb` | `bundle(...)`, `share(...)`, and selective import aliases |

### Module path demo

This repository also includes a multi-file `--module-path` demo:

```bash
build/vyb examples/module_path_demo/main.vyb --module-path examples/module_path_demo/modules
```

## Known Boundaries

The examples now include runtime `Vec<T>` helper returns for common algorithm
paths. Remaining ownership work is broader: enforcing full move/drop rules for
all owned aggregates and documenting the final borrow/mutation contract.
