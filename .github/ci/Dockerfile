# CI toolchain image (#484) — everything the hosted jobs used to `apt-get install` on
# every run, baked once. No job touches the network for dependencies any more, and the
# flaky part goes away with it (the apt step was measured at 30s on a good mirror, 5.2
# and 10.8 minutes on a slow one, and once hung for 20 minutes and needed a cancel +
# rerun).
#
# Every package here is either needed by the build, invoked by a step, or present for a
# parity reason -- a silently different environment changes which tests run:
#   - libssl-dev and libncursesw5-dev are NOT in the old apt step, but the ubuntu-latest
#     runner image has both, which is why CI logs "OpenSSL: 3.0.13 (tls module enabled)"
#     and "ncursesw found". Leaving them out would drop the tls/curses modules.
#   - libsqlite3-dev: test/bindgen/test_sqlite_query.vyb runs (not skips) in CI, so the
#     runner image provides libsqlite3; this image must too.
#   - ptxas comes from NVIDIA's CUDA repo (cuda-nvcc-12-6), NOT from Ubuntu's
#     nvidia-cuda-toolkit: that package drags in libnvidia-compute-535/580 (the userspace
#     driver, ~1 GB) and with it libcuda.so.1 -- which would make the six CUDA tests try
#     to run on a GPU-less runner instead of skipping. cuda-nvcc is ptxas+nvvm only, and
#     the compiler already searches /usr/local/cuda-12.6/bin/ptxas.
#   - zlib1g-dev and libzstd-dev: LLVM 18's CMake exports ZLIB::ZLIB and
#     zstd::libzstd_shared, and the compiler links them; without the dev packages the
#     link dies with "cannot find -lZLIB::ZLIB / -lzstd::libzstd_shared".
#   - libSDL3 is deliberately absent (one test declares itself skippable without it).
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive \
    LANG=C.UTF-8 \
    # the jobs set CCACHE_DIR themselves (${{ github.workspace }}/.ccache); this is the
    # default for anyone running the image by hand.
    CCACHE_DIR=/root/.cache/ccache

# LLVM 18 is the compiler's build dependency (LLVM_DIR=/usr/lib/llvm-18/cmake); catch2,
# gdb and lldb serve the suite and the debugger-integration step; python3 is used by the
# release job (no CI step needs it, so the jobs no longer set up a toolchain python).
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential cmake ccache git ca-certificates curl gnupg \
        llvm-18-dev libclang-18-dev \
        libssl-dev libncursesw5-dev libsqlite3-dev \
        zlib1g-dev libzstd-dev \
        catch2 gdb lldb \
        python3 \
    && rm -rf /var/lib/apt/lists/*

# ptxas for the kernel job's arch/acceptance gate (issue #198), from NVIDIA's own repo so
# the driver userspace stays out. The bare `ptxas` the job runs needs it on PATH; the
# versioned path is what the compiler probes.
RUN curl -fsSLo /tmp/cuda-keyring.deb \
        https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2404/x86_64/cuda-keyring_1.1-1_all.deb \
    && dpkg -i /tmp/cuda-keyring.deb \
    && rm -f /tmp/cuda-keyring.deb \
    && apt-get update \
    && apt-get install -y --no-install-recommends cuda-nvcc-12-6 \
    && ln -sf /usr/local/cuda-12.6/bin/ptxas /usr/local/bin/ptxas \
    && rm -rf /var/lib/apt/lists/*

# Refuse to publish an image that contradicts the notes above. A build that fails here is
# cheaper than a CI run that quietly tests a different environment.
RUN set -eux; \
    ptxas --version | tail -1; \
    /usr/local/cuda-12.6/bin/ptxas --version | tail -1; \
    clang-18 --version | head -1; \
    cmake --version | head -1; \
    gdb --version | head -1; \
    test -f /usr/lib/x86_64-linux-gnu/libssl.so; \
    test -f /usr/lib/x86_64-linux-gnu/libncursesw.so; \
    test -f /usr/lib/x86_64-linux-gnu/libsqlite3.so; \
    test -f /usr/lib/x86_64-linux-gnu/libzstd.so; \
    test -f /usr/lib/x86_64-linux-gnu/libz.so; \
    if ldconfig -p | grep -q 'libcuda\.so\.1'; then \
      echo "libcuda.so.1 is present: the CUDA tests would run instead of skipping" >&2; exit 1; \
    fi; \
    if ldconfig -p | grep -q 'libSDL3\.so'; then \
      echo "libSDL3.so is present: a test that must skip would run" >&2; exit 1; \
    fi; \
    echo "parity checks passed"
