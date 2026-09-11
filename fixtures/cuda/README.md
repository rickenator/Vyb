# CUDA PTX fixtures — intentional golden artifacts

These `.ptx` files are deliberate, version-controlled test fixtures for the
on-silicon CUDA/JIT path (`vyb --kernel` NVPTX lowering + `cuModuleLoadData`),
NOT transient build residue.

- `fill_kernel.ptx` — loaded by `test/ffi/test_cuda_launch.vyb` and
  `test/ffi/test_cuda_module_load_209.vyb`.
- `axpy_buf.ptx` — loaded by `test/ffi/test_cuda_axpy.vyb`.
- `matmul.ptx` — loaded by `test/ffi/test_cuda_matmul.vyb`.
- `p203_verify.ptx` — loaded by `test/ffi/test_cuda_p203.vyb`.

They are checked in so the CUDA tests are reproducible on GPU-bearing CI
without a live `ptxas` step at test time. Keep them — do not "clean" them up.
