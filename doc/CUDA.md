# Vyb CUDA / NVPTX Device Backend

**Status:** Implemented (issue #198) — device lowering + device intrinsics ship behind the
`--kernel` CLI flag. Host-side launch runs through the CUDA driver API via the `freedom`
FFI. Follow-ons: issue #199 (launch argument packing), issue #203 (fp16/bf16 + GGUF q4_0
dequant on device).

**Not authoritative:** this is a design/reference note. The authoritative user-facing
reference is the Programmer's Guide and generated pages under `docs/refman/`. Cross-check
behaviour against `build/vyb`, not this doc.

---

## Overview

Vyb can compile a module as **pure NVPTX device code** and lower it to a PTX artifact that
runs on an NVIDIA GPU. The support is two halves:

1. **Device half (compiler):** `--kernel` lowers a Vyb module to LLVM IR with the
   `nvptx64-nvidia-cuda` target and emits PTX via the in-process NVPTX backend. Device
   code is value/data-parallel only — it has **no host runtime**: no strings, no heap, no
   call-stack, no channel machinery, no ledger/crypto runtime.
2. **Host half (Vyb source):** a hand-written Vyb program uses the CUDA **driver API**
   (not the runtime API) through `freedom` `extern "C"` blocks to load the `.ptx`, launch a
   kernel, and copy results back. No C host is generated; the host is ordinary Vyb source.

```
  fixtures/kernel/*.vyb                      fixtures/cuda/*.ptx   (emitted artifacts)
        |  vyb --kernel ... --ptx out.ptx          |
        v                                          v
   NVPTX IR -> PTX  ----------------------->  cuModuleLoadData / cuLaunchKernel
                                                 (from a Vyb main via freedom FFI)
```

## Build & usage (device half)

```
vyb --kernel axpy.vyb                    # lower as NVPTX, writes axpy.ptx
vyb --kernel axpy_buf.vyb --ptx out.ptx  # explicit output path
vyb --kernel k.vyb --gpu sm_90           # arch override (flag, or $VYB_KERNEL_GPU env)
```

- `--kernel` sets `g_kernel_mode` (main.cpp), which is what makes codegen choose the
  NVPTX triple and suppress the host-runtime `__vyb_*` intrinsic externs.
- GPU arch resolution: `--gpu sm_90` (CLI flag, overrides env), else `$VYB_KERNEL_GPU`, else
  `sm_86` (the P0 feasibility-probe target, RTX-3090 class). Used for both NVPTX lowering and
  the `ptxas` assembly check.
- **1.0 arch baseline**: officially supported compute capabilities are **sm_80–sm_90**
  (Ampere A100/A30/RTX 3xxx, Ada L4/L40/RTX 4xxx, Hopper H100). The default target is
  `sm_86` (RTX 3090, the repo's primary silicon) and that is the **primary** capability —
  everything is validated against it on silicon; other arches in the range are validated at
  the `ptxas` acceptance-gate level (portability gate, no on-GPU execution per arch). Older
  `sm_75` et al. are out of the 1.0 support range.
- PTX defaults to `<source-base>.ptx` next to the input, or `--ptx <path>`.

## Device intrinsics

Available only under kernel mode. The semantic layer treats them as builtins
(`kernelIntrinsicReturnType`, semantic.cpp) so they need no symbol resolution; codegen
lowers them directly (`emitKernelIntrinsic`, cgen_expr.cpp).

Thread / block / grid / lane reads — lower to `llvm.nvvm.read.ptx.sreg.*` (i32, zext to
`Int`):

```
tid_x tid_y tid_z        # thread index within block
blk_x blk_y blk_z        # block index within grid
dim_x dim_y dim_z        # block dimensions (blockDim)
grid_x grid_y            # grid dimensions (gridDim)
lane_id warp_size        # warp lane / warp size
```

Block barrier:

```
kernel_barrier            # bar.sync 0  -> llvm.nvvm.barrier0
```

Device-global load/store — the address is a Vyb `Int` (i64) buffer address, lowered to a
pointer in **addrspace(1)** (global memory):

```
ld_f64 ld_f32 ld_i64 ld_i32 ld_i16 ld_u16 ld_i8 ld_u8   # -> Int/Float/CInt/Int16/... as typed
st_f64 st_f32 st_i64 st_i32 st_i16 st_u16 st_i8 st_u8
```

Wide-from-narrow float loads (fp16 / bf16) widen halves to `Float` (f64); stores narrow
back:

```
ld_f16  ld_bf16
st_f16  st_bf16
```

GGUF q4_0 dequant — a block at `addr` holds `[f16 d][32 x 4-bit signed]`; value(i) =
(nibble−8)·d. This is the 4-bit weight path that makes a ~2.2 GB packed 4B model runnable
without an fp16 mirror:

```
deq_q4_0(addr<Int>, idx<Int>)<Float>
```

Shared memory — a module-wide addrspace(3) buffer (`__vyb_kernel_shared`, 32 KB = 4096
f64; static shared cap is 48 KB on sm_86). Caller bounds-checks:

```
ld_shared_f64(off<Int>)<Float>
st_shared_f64(off<Int>, v<Float>)
```

Global atomics (monotonic) — for KV-cache writes / reductions:

```
atomic_add_f64(addr<Int>, v<Float>)<Float>
atomic_add_i32(addr<Int>, v<CInt>)<CInt>
```

### No host libm in device code (issue #256)

The device module links no libm, so the host math builtins are **rejected** in
`--kernel` mode instead of being emitted as unresolved `.extern .func` calls:

```
exp, sin, cos, tan, log, log2, log10, sqrt, floor, ceil, round, pow
Float abs   # lowers to a fabs libm call
```

Each use is a location-bearing codegen error and the kernel is not emitted (the
`--kernel` driver refuses to write a PTX whose codegen reported errors). Use
pure-arithmetic device math instead — see VybForge `native/kernels/vmath.vyb`
(`vexp`/`vsin`/`vcos` polynomial/Newton forms) — or supply your own device
function. Integer `abs`, `min`/`max` and the arithmetic operators are unaffected
(they lower to plain arithmetic / `select`).

## Kernel vs. helper functions

How a function lowers determines whether it is launchable:

- A **Void-returning top-level function** in kernel mode carries the `nvvm.kernel`
  attribute and the `PTX_Kernel` calling convention (cgen_decl.cpp), so NVPTX emits it as
  a PTX **`.entry`** — that is what makes `cuLaunchKernel` able to run it.
- A **value-returning top-level function** stays a `.visible .func` device helper
  (callable from kernels, not launchable on its own).

All kernel parameters cross the FFI as 8-byte typed values — `Int` buffer addresses, `Int`
counts, `Float` scalars — so the host packs `kernelParams` purely from 64-bit values with
no device staging buffer.

## NVPTX lowering notes

- **No DWARF debug info.** printf-style debug is unavailable on the GPU, and LLVM's NVPTX
  backend emits debug sections whose `labels1 − labels2` expressions require PTX ISA ≥ 7.5
  while it still tags the module `.version 7.1` — `ptxas` rejects that. Debug hooks are
  null-guarded so skipping debug init is safe.
- **No `__vyb_*` externs.** A kernel is pure device code; any reference left to host
  machinery is a bug that must surface (see gate below) rather than choke NVPTX.

## Acceptance gates (compile-time)

- **Host-runtime-free check:** every `__vyb_*` symbol that is a *declaration* (no body) is
  a forbidden reference to host machinery; the module is rejected with the symbol list.
- **P2.4 — symbol presence:** every emitted device function must survive into the PTX
  text result, and there must be at least one. Catches silently-optimized-away or
  mis-named kernels so a host-side launch can't miss its target.
- **P2.2 — ptxas validation:** if `ptxas` is on PATH, the emitted PTX is assembled with
  `--gpu-name=<arch>`; a ptxas rejection is a hard failure, absence of ptxas is a warning.

## Host launch pattern (freedom FFI)

The reference launcher is `fixtures/cuda/launch_fill.vyb`:

1. `cuInit`, `cuDeviceGetCount`/`cuDeviceGet`, `cuCtxCreate_v2`.
2. Read the `.ptx` file (e.g. `fixtures/cuda/fill_kernel.ptx`).
3. `cuModuleLoadData`, `cuModuleGetFunction` (by kernel name).
4. `cuMemAlloc_v2` for device buffers.
5. `cuLaunchKernel`, `cuCtxSynchronize`.
6. `cuMemcpyDtoH_v2` to read results back, then `cuMemFree_v2`.

### The launch-argument pitfall (issue #199)

`cuLaunchKernel`'s `kernelParams` is a `void*[]` whose element *i* points to the buffer
holding argument *i*'s **value**. It is easy to pack it one level short (passing `&slot`
as the array base); then the driver reads `slot`'s content as a *host* pointer and
dereferences a *device* address, SIGSEGVing inside libcuda. The correct pattern:

```
slot<loc<CVoid>> = from<loc<CVoid>>(dptr)   # buffer holding arg0's VALUE
sel<loc<loc<CVoid>>> = loc(slot)             # &slot
pp<loc<CVoid>> = from<loc<CVoid>>(addr(sel)) # pp holds &slot
cuLaunchKernel(hf, ..., loc(pp), ...)        # *kernelParams == &slot
```

## Security note

Kernels are pure value/data-parallel compute. Only integer buffer addresses and raw typed
data cross the FFI to the device; the runtime never stages key/seed material on the GPU.
Any crypto/ledger integration stays host-side.

## Fixtures

| File | Role |
|------|------|
| `fixtures/kernel/axpy.vyb` | Pure-device, zero-runtime scalar sample (no `main`) |
| `fixtures/kernel/axpy_buf.vyb` | Launchable Void 1-D axpy over a flattened buffer |
| `fixtures/kernel/matmul.vyb` | Register-tiled 4x4 tile matmul, no shared memory |
| `fixtures/kernel/p203_verify.vyb` | deq_q4_0 + fp16/bf16 loads on device |
| `fixtures/cuda/launch_fill.vyb` | Host driver-API launcher (reads `fill_kernel.ptx`, expects 42) |
| `fixtures/cuda/*.ptx` | Emitted PTX artifacts |
| `fixtures/cuda/*.vyb.ll` | Generated LLVM IR |
