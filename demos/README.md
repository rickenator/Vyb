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
| `collections_demo.vyb` | `Vec<Int>`, push, iteration, `len`, `import collections` |
| `structs_aspects.vyb` | structs, aspects, `bind`, method dispatch |
| `ffi_freedom.vyb` | `extern "C"` declarations called only inside `freedom` |
| `ffi_http_get/` | external URL request through C FFI and `curl` |
| `ffi_native_3d/` | native GLFW/OpenGL scene launched through C FFI |
| `finalization_targets.vyb` | borrow scope checks, typed `fail`/`trap`, and C ABI aliases |
| `VybWeb/` | native Qt5 browser package (`vyb build demos/VybWeb` → `target/vybweb`; stdlib `qt` + `QWebEngineView`), starts on Google, with back/forward/reload, an address bar, Go, and Quit |
| `legit_smuggle/` | the signed-ledger smuggling model (#204): a `priv/` package consumed through `smuggle`, a `consumer/` package that verifies the persisted receipt against the package's live legitimization token, and `run_legit_smuggle.sh` which seals a receipt, tampers a copy, and shows both outcomes (`target/` output and `*.receipt` artifacts are gitignored) |
