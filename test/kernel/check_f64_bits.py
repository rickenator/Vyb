# reference check for test_f64_bits kernel via the CUDA driver (not in any inference path)
# usage: python3 check_f64_bits.py f64b.ptx
import ctypes as C, random, struct, sys
ptx = open(sys.argv[1], "rb").read()
assert b"mov.b64" in ptx or b"ld.global" in ptx
cu = C.CDLL("libcuda.so.1")
def ck(r):
    if r: raise SystemExit("cuda err %d" % r)
ck(cu.cuInit(0)); dev = C.c_int(); ck(cu.cuDeviceGet(C.byref(dev), 0))
ctx = C.c_void_p(); ck(cu.cuCtxCreate_v2(C.byref(ctx), 0, dev))
mod = C.c_void_p(); ck(cu.cuModuleLoadData(C.byref(mod), ptx + b"\0"))
fn = C.c_void_p(); ck(cu.cuModuleGetFunction(C.byref(fn), mod, b"f64b"))
N = 4096; random.seed(3)
X = [random.choice([-1, 1]) * 10 ** random.uniform(-30, 30) for _ in range(N - 4)] + [0.0, -0.0, 1.0, 2.0 ** -1060]
xb = struct.pack("<%dd" % N, *X)
def alloc(n):
    p = C.c_uint64(); ck(cu.cuMemAlloc_v2(C.byref(p), C.c_size_t(n))); return p
dx, do = alloc(len(xb)), alloc(24 * N)
ck(cu.cuMemcpyHtoD_v2(dx, xb, C.c_size_t(len(xb))))
n = C.c_int64(N)
args = (C.c_void_p * 3)(*[C.cast(C.pointer(v), C.c_void_p) for v in (dx, do, n)])
ck(cu.cuLaunchKernel(fn, N // 128, 1, 1, 128, 1, 1, 0, None, args, None)); ck(cu.cuCtxSynchronize())
ob = C.create_string_buffer(24 * N); ck(cu.cuMemcpyDtoH_v2(ob, do, C.c_size_t(24 * N)))
bad = 0
for i, x in enumerate(X):
    b, = struct.unpack("<q", struct.pack("<d", x))
    h = abs(b) // 2 * (1 if b >= 0 else -1)      # Vyb Int division truncates toward zero
    q = (6910469410427058089 - h + 2**63) % 2**64 - 2**63   # i64 wraparound
    eb, er, es = b, struct.pack("<d", x), struct.pack("<q", q)
    gb, = struct.unpack_from("<q", ob.raw, 24 * i)
    if gb != eb or ob.raw[24*i+8:24*i+16] != er or ob.raw[24*i+16:24*i+24] != es:
        bad += 1
        if bad < 5: print("MISMATCH", i, x, gb, eb)
print("F64_BITS_CHECK", "PASS" if bad == 0 else "FAIL", "n=%d bad=%d" % (N, bad))
