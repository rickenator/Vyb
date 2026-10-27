# reference check for the test_atomics_bits kernel via the CUDA driver
# usage: python3 check_atomics_bits.py atombits.ptx
import ctypes as C, random, struct, sys
ptx = open(sys.argv[1], "rb").read()
for frag in (b"vote.sync.ballot.b32", b"popc.b32", b".or.b32", b".and.b32", b".xor.b32",
             b".add.u64", b".xor.b64", b".add.f32", b".max.s32"):
    assert frag in ptx, frag
cu = C.CDLL("libcuda.so.1")
def ck(r):
    if r: raise SystemExit("cuda err %d" % r)
ck(cu.cuInit(0)); dev = C.c_int(); ck(cu.cuDeviceGet(C.byref(dev), 0))
ctx = C.c_void_p(); ck(cu.cuCtxCreate_v2(C.byref(ctx), 0, dev))
mod = C.c_void_p(); ck(cu.cuModuleLoadData(C.byref(mod), ptx + b"\0"))
fn = C.c_void_p(); ck(cu.cuModuleGetFunction(C.byref(fn), mod, b"atombits"))
N = 4096; random.seed(5)
X = [random.getrandbits(32) for _ in range(N - 4)] + [0, 0xFFFFFFFF, 1 << 31, 1]
X[100] = X[100] | 0xFFFF0000   # keep the and-accumulator from collapsing too early to be useful
def alloc(n):
    p = C.c_uint64(); ck(cu.cuMemAlloc_v2(C.byref(p), C.c_size_t(n))); return p
dx, da, do = alloc(4 * N), alloc(40), alloc(16 * N)
ck(cu.cuMemcpyHtoD_v2(dx, struct.pack("<%dI" % N, *X), C.c_size_t(4 * N)))
acc0 = struct.pack("<IIIiqqf4x", 0, 0xFFFFFFFF, 0, -1, 0, 0, 0.0)
ck(cu.cuMemcpyHtoD_v2(da, acc0, C.c_size_t(40)))
args = (C.c_void_p * 3)(*[C.cast(C.pointer(v), C.c_void_p) for v in (dx, da, do)])
ck(cu.cuLaunchKernel(fn, N // 128, 1, 1, 128, 1, 1, 0, None, args, None)); ck(cu.cuCtxSynchronize())
ob = C.create_string_buffer(16 * N); ck(cu.cuMemcpyDtoH_v2(ob, do, C.c_size_t(16 * N)))
ab = C.create_string_buffer(40); ck(cu.cuMemcpyDtoH_v2(ab, da, C.c_size_t(40)))
bad = 0
for i, v in enumerate(X):
    w = i // 32 * 32
    eb = sum(((X[w + k] & 1) << k) for k in range(32))
    ctz = 32 if v == 0 else (v & -v).bit_length() - 1
    gb, = struct.unpack_from("<I", ob.raw, 16 * i)
    gp, = struct.unpack_from("<q", ob.raw, 16 * i + 8)
    if gb != eb or gp != bin(v).count("1") * 64 + ctz:
        bad += 1
        if bad < 5: print("MISMATCH", i, hex(v), hex(gb), hex(eb), gp)
o_, a_, x_ = 0, 0xFFFFFFFF, 0
s64, x64 = 0, 0
for v in X:
    o_ |= v; a_ &= v; x_ ^= v; s64 += v * 4097; x64 ^= v << 20
s64 = (s64 + 2**63) % 2**64 - 2**63; x64 = (x64 + 2**63) % 2**64 - 2**63
got = struct.unpack("<IIIiqqf4x", ab.raw)
exp = (o_, a_, x_, max(v >> 1 for v in X), s64, x64, float(N))
if got != exp:
    bad += 1; print("ACC MISMATCH", got, exp)
print("ATOMICS_BITS_CHECK", "PASS" if bad == 0 else "FAIL", "n=%d bad=%d" % (N, bad))
