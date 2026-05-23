# Vyn Demos

These demos are intended to run with the current compiler and runtime:

```bash
build/vyn demos/<file>.vyn
```

The demos avoid by-value `Vec<T>` helper returns until Vec ownership move/clone
semantics are completed.

| Demo | Covers |
|------|--------|
| `control_flow.vyn` | recursion, arithmetic, `match`, `defer` |
| `collections.vyn` | `Vec<Int>`, push, iteration, `len` |
| `structs_aspects.vyn` | structs, aspects, `bind`, method dispatch |
| `ffi_freedom.vyn` | `extern "C"` declarations called only inside `freedom` |
