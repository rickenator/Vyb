# Vyb Demos

These demos are intended to run with the current compiler and runtime:

```bash
build/vyb demos/<file>.vyb
```

The demos are intentionally compact; richer runtime examples, including
`Vec<T>` helper-return algorithms and local module imports, live in `examples/`.

| Demo | Covers |
|------|--------|
| `control_flow.vyb` | recursion, arithmetic, `match`, `defer` |
| `collections.vyb` | `Vec<Int>`, push, iteration, `len` |
| `structs_aspects.vyb` | structs, aspects, `bind`, method dispatch |
| `ffi_freedom.vyb` | `extern "C"` declarations called only inside `freedom` |
| `finalization_targets.vyb` | borrow scope checks, typed `fail`/`trap`, and C ABI aliases |
