#!/bin/sh
# build-llvm-esp.sh -- build Espressif's LLVM 18 fork as an LLVM *library*
# install that Vyb can link, adding the Xtensa backend (ESP32, ESP32-S2,
# ESP32-S3 cores) next to Vyb's usual X86 + NVPTX. See doc/ESP32.md.
#
#   scripts/build-llvm-esp.sh [workdir]        (default: ../llvm-esp18)
#   cmake -S . -B build-xtensa -DLLVM_DIR=<workdir>/install/lib/cmake/llvm \
#         -DVYB_USE_QT5=OFF -DVYB_BINDGEN_LIBCLANG=OFF -DCMAKE_BUILD_TYPE=Release
#   cmake --build build-xtensa --target vyb
#
# Cost on a 12-thread desktop: ~1.5 GB shallow clone, ~1 h build at -j10, a few
# GB of disk for the build tree. Espressif's prebuilt esp-clang ships only
# binaries (clang/llc/lld + libLTO), no LLVM headers/libs/CMake package, which
# is why this builds from source. Xtensa is an *experimental* target in the
# fork, hence LLVM_EXPERIMENTAL_TARGETS_TO_BUILD.
set -eu
W="${1:-$(dirname "$0")/../../llvm-esp18}"
TAG="${ESP_LLVM_TAG:-esp-18.1.2_20240912}"
JOBS="${JOBS:-$(nproc)}"
mkdir -p "$W" && cd "$W"
[ -d src ] || git clone --depth 1 --branch "$TAG" https://github.com/espressif/llvm-project.git src
CC_=${CC:-$(command -v clang-18 || command -v clang || command -v cc)}
CXX_=${CXX:-$(command -v clang++-18 || command -v clang++ || command -v c++)}
LAUNCH=""
command -v ccache >/dev/null && LAUNCH="-DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache"
cmake -G Ninja -S src/llvm -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX="$PWD/install" \
  -DCMAKE_C_COMPILER="$CC_" -DCMAKE_CXX_COMPILER="$CXX_" $LAUNCH \
  -DLLVM_TARGETS_TO_BUILD="X86;NVPTX" \
  -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD=Xtensa \
  -DLLVM_ENABLE_PROJECTS="" \
  -DLLVM_ENABLE_RTTI=ON -DLLVM_ENABLE_ASSERTIONS=OFF \
  -DLLVM_INCLUDE_TESTS=OFF -DLLVM_INCLUDE_BENCHMARKS=OFF -DLLVM_INCLUDE_EXAMPLES=OFF \
  -DLLVM_INCLUDE_DOCS=OFF -DLLVM_ENABLE_BINDINGS=OFF -DLLVM_ENABLE_LIBXML2=OFF \
  -DLLVM_INSTALL_UTILS=ON -DLLVM_PARALLEL_LINK_JOBS=2
cmake --build build -j "$JOBS"
cmake --install build
echo "LLVM_DIR=$PWD/install/lib/cmake/llvm"
