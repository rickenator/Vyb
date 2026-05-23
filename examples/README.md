# Vyn Examples

Runnable Vyn programs demonstrating the current language surface.

```bash
build/vyn examples/<file>.vyn
```

All files in this directory are expected to execute successfully with the
current compiler. Heavier feature walkthroughs live in `demos/`.

## Examples

| Example | Covers |
|---------|--------|
| `main.vyn` | arithmetic, structs, `Vec<T>`, `match`, strings, recursion, `defer` |
| `sort.vyn` | compare-and-swap sorting without by-value Vec returns |
| `quicksort.vyn` | quicksort partition logic over a local `Vec<Int>` |
| `stack.vyn` | stack behavior using `Vec<Int>` directly |
| `binary_tree_clean.vyn` | flat tree-node storage and lookup with `Vec<TreeNode>` |
| `vec_filter.vyn` | filtering values into a second vector |
| `vec_max.vyn` | scanning a vector for a maximum value |
| `vec_point_distance.vyn` | iterating over `Vec<Point>` structs |
| `memory_semantics.vyn` | `freedom`, `loc<T>`, and `at(ptr)` |
| `mild_references.vyn` | `our<T>` and `mild<T>` syntax |
| `ffi_puts.vyn` | `extern "C"` plus `freedom`-gated calls |

## Known Boundaries

The current examples avoid returning or passing `Vec<T>` by value in runtime
algorithm helpers. Semantic tests cover those forms, but runtime execution still
needs completed Vec move/clone ownership semantics to prevent shallow-copy
double frees.
