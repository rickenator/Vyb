# Vyn Demos

These demos are intended to run with the current compiler and runtime:

```bash
build/vyn demos/<file>.vyn
```

The demos are intentionally compact; richer runtime examples, including
`Vec<T>` helper-return algorithms and local module imports, live in `examples/`.

| Demo | Covers |
|------|--------|
| `control_flow.vyn` | recursion, arithmetic, `match`, `defer` |
| `collections.vyn` | `Vec<Int>`, push, iteration, `len` |
| `structs_aspects.vyn` | structs, aspects, `bind`, method dispatch |
| `ffi_freedom.vyn` | `extern "C"` declarations called only inside `freedom` |
