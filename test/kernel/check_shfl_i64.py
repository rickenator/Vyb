# reference check for the test_shfl_i64 kernel via the CUDA driver
# usage: python3 check_shfl_i64.py shfl64.ptx
import ctypes as C, struct, sys
ptx = open(sys.argv[1], "rb").read()
for frag in (b"shfl.sync.bfly.b32", b"shfl.sync.down.b32", b"shfl.sync.up.b32", b"shfl.sync.idx.b32"):
    assert frag in ptx, frag
cu = C.CDLL("libcuda.so.1")
def ck(r):
    if r: raise SystemExit("cuda err %d" % r)
ck(cu.cuInit(0)); dev = C.c_int(); ck(cu.cuDeviceGet(C.byref(dev), 0))
ctx = C.c_void_p(); ck(cu.cuCtxCreate_v2(C.byref(ctx), 0, dev))
mod = C.c_void_p(); ck(cu.cuModuleLoadData(C.byref(mod), ptx + b"\0"))
fn = C.c_void_p(); ck(cu.cuModuleGetFunction(C.byref(fn), mod, b"shfl64"))
N = 256
do = C.c_uint64(); ck(cu.cuMemAlloc_v2(C.byref(do), C.c_size_t(32 * N)))
args = (C.c_void_p * 1)(C.cast(C.pointer(do), C.c_void_p))
ck(cu.cuLaunchKernel(fn, N // 64, 1, 1, 64, 1, 1, 0, None, args, None)); ck(cu.cuCtxSynchronize())
ob = C.create_string_buffer(32 * N); ck(cu.cuMemcpyDtoH_v2(ob, do, C.c_size_t(32 * N)))
def val(l):
    v = (l << 40) | (l * 3 + 7) | ((-1 << 62) & (2**64 - 1))
    return (v + 2**63) % 2**64 - 2**63
bad = 0
for g in range(N):
    l = g & 31
    exp = (val(l ^ 5), val(l + 3 if l + 3 < 32 else l), val(l - 2 if l >= 2 else l), val(31 - l))
    got = struct.unpack_from("<4q", ob.raw, 32 * g)
    if got != exp:
        bad += 1
        if bad < 5: print("MISMATCH", g, got, exp)
print("SHFL_I64_CHECK", "PASS" if bad == 0 else "FAIL", "n=%d bad=%d" % (N, bad))
