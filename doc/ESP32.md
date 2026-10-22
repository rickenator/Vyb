# Vyb on ESP32: `vyb --target` (freestanding cross-compilation)

`vyb --target <triple>` lowers a Vyb module for a non-host LLVM target and
writes a plain relocatable object (or `-S` assembly) that the target's own C
toolchain links. It was added to compile the NerdMiner hot path of
[VybMiner](https://github.com/rickenator/VybMiner) straight to an ESP32-S3
(Xtensa LX7) object with Vyb alone; nothing about it is ESP-specific beyond the
target aliases.

```sh
vyb --target xtensa-esp32s3-elf -O3 -c nerd_core.o nerd_core.vyb   # ESP32-S3
vyb --target xtensa-esp32-elf   -O2 -c core.o core.vyb             # ESP32 (LX6)
vyb --target riscv32-esp-elf --mattr +m,+c -c core.o core.vyb      # ESP32-C3 (stock LLVM 18; emits, not yet linked/tested)
vyb --target x86_64-linux-gnu -O3 -c core.o core.vyb               # same code, host object
vyb --target xtensa-esp32s3-elf -S -c core.s core.vyb              # assembly
```

| flag | meaning |
|---|---|
| `--target <triple>` | LLVM triple, or an ESP alias: `xtensa-esp32{,s2,s3}-elf` / `-none-elf` / bare `esp32{,s2,s3}` -> triple `xtensa-esp-unknown-elf` + `-mcpu` |
| `--mcpu <cpu>` | CPU (implied by the aliases) |
| `--mattr <features>` | LLVM feature string |
| `-S`, `--emit-asm` | assembly instead of an object |
| `-c <out>` | output path (default `<input>.o` / `.s`) |

## What "freestanding" means

A `--target` module is compiled like a `--kernel` (NVPTX) module minus the GPU
specifics (`noHostRuntime()` in `codegen.hpp` covers both):

* no `main` wrapper, no JIT; every `share`d function is an exported C-ABI symbol
* no DWARF, no `__vyb_*` intrinsic externs, no type-metadata registry
* **no call-stack frame hooks** (`__vyb_runtime_push/pop_call_frame`), so small
  helper functions inline and cost nothing
* no `core::aspects` auto-import
* the *target's* DataLayout is installed before codegen (32-bit pointers on
  Xtensa), static relocation model, function/data sections on

The compiler then refuses to emit an object that still references the Vyb host
runtime -- e.g. using `println`, `String`, `Vec`, or closures:

```
Freestanding module uses Vyb host-runtime symbols (no runtime exists for xtensa-esp-unknown-elf):
  __vyb_println ...
Error compiling for target: use only scalar/value code ... or call C via FFI
```

So today a `--target` module is scalar/value code (integers, floats, structs
of those, loops, `select`, functions) plus whatever C it calls through FFI. A
runtime port (heap strings/vectors on newlib, FreeRTOS tasks for threads,
lwIP for `network`) is the next step toward running full Vyb programs on the
device; see "Remaining work" below.

## Building vyb with the Xtensa backend

Stock LLVM 18 knows Xtensa only as an MC layer (no codegen, no `esp32s3` CPU);
`vyb --target xtensa-esp32s3-elf` on such a build fails cleanly with
"cpu 'esp32s3' is not valid ... build vyb against Espressif's LLVM fork".
RISC-V ESP parts (C3/C6/H2) can be targeted with stock LLVM 18 (objects emit; not yet exercised in an IDF build).

Espressif's prebuilt `esp-clang` is binaries only (no LLVM headers, libraries
or CMake package), so Vyb links a source build of
[espressif/llvm-project](https://github.com/espressif/llvm-project) at
`esp-18.1.2_20240912` -- the same LLVM major as Vyb's pin, so no API changes
were needed (Vyb's `find_package(LLVM 18.1)` accepts 18.1.2):

```sh
scripts/build-llvm-esp.sh ../llvm-esp18        # ~1 h at -j10; X86 + NVPTX + Xtensa
cmake -S . -B build-xtensa -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DLLVM_DIR=$PWD/../llvm-esp18/install/lib/cmake/llvm \
      -DVYB_USE_QT5=OFF -DVYB_BINDGEN_LIBCLANG=OFF
cmake --build build-xtensa --target vyb
build-xtensa/vyb --target xtensa-esp32s3-elf -O3 -c x.o examples/freestanding/checksum.vyb
```

The resulting vyb is a normal Vyb (JIT, AOT, `--kernel` all unchanged) that
additionally targets Xtensa.

## Linking into ESP-IDF

The object uses the windowed Xtensa ABI that ESP-IDF's GCC uses, so it links
directly. VybMiner's component is the worked example
(`esp32/components/vyb_nerd_core`): it runs `vyb --target xtensa-${IDF_TARGET}-elf`
as a CMake custom command (or uses a checked-in object when no Xtensa vyb is
installed), adds the object to the component library, and uses a linker
fragment to place it in IRAM:

```
[mapping:vyb_nerd_core]
archive: libvyb_nerd_core.a
entries:
    nerd_core (noflash)
```

## Remaining work (not blockers for scalar code)

1. **Runtime for the target**: `String`/`Vec`/`println`/threads on newlib +
   FreeRTOS + lwIP (`runtime/vyb_runtime.c` is POSIX-only today).
2. **Rotate intrinsic**: Vyb has no `rotr`; `(x >> n) | (x << 32-n)` is
   recognised as a funnel shift on x86 but costs extra instructions on Xtensa.
3. **`Int` is 64-bit everywhere**: on 32-bit targets, prefer `UInt32`/`Int32`
   in hot code.
4. Xtensa in upstream LLVM: codegen landed after LLVM 18 and is still
   incomplete for the ESP cores, so the Espressif fork remains the practical
   choice until Vyb moves its LLVM pin.
