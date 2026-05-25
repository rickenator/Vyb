# stdlib_demo

Demonstrates stdlib module discovery with an explicit `import core::option`.

## Run

From the repository root:

```bash
# Option 1: explicit stdlib root
VYN_STDLIB=stdlib ./build/vyn examples/stdlib_demo/main.vyn

# Option 2: executable-relative discovery (works from this repo layout)
./build/vyn examples/stdlib_demo/main.vyn
```

Expected output: `42`
