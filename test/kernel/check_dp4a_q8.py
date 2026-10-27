# reference check for test_dp4a_q8 kernel via the CUDA driver (not in any inference path)
import ctypes as C, random, struct, sys
cu = C.CDLL("libcuda.so.1")
def ck(r):
    if r: raise SystemExit("cuda err %d" % r)
ck(cu.cuInit(0)); dev = C.c_int(); ck(cu.cuDeviceGet(C.byref(dev), 0))
ctx = C.c_void_p(); ck(cu.cuCtxCreate_v2(C.byref(ctx), 0, dev))
mod = C.c_void_p(); ptx = open(sys.argv[1], "rb").read() + b"\0"
ck(cu.cuModuleLoadData(C.byref(mod), ptx))
fn = C.c_void_p(); ck(cu.cuModuleGetFunction(C.byref(fn), mod, b"dp4aq8"))
N = 4096; random.seed(1)
W = [random.getrandbits(32) for _ in range(3 * N)]
rows = []
for g in range(N):
    a, b, m = W[3*g], W[3*g+1], random.choice([0, 0xffffffff, 0xff00ff00, 0x00ff00ff, 0xff000000])
    c = random.randint(-2**20, 2**20)
    rows.append((a, b, c, m))
X = [random.uniform(-60, 60) if k % 7 else random.choice([0.2, -0.2, 0.6, 1.0, -1.0]) for k in range(4 * N)]
X = [struct.unpack("f", struct.pack("f", v))[0] for v in X]
wb = b"".join(struct.pack("<IIiI", a, b, c, m) for a, b, c, m in rows)
xb = struct.pack("<%df" % (4 * N), *X)
def alloc(n):
    p = C.c_uint64(); ck(cu.cuMemAlloc_v2(C.byref(p), C.c_size_t(n))); return p
dw, dx, dq = alloc(len(wb)), alloc(len(xb)), alloc(4 * N)
ck(cu.cuMemcpyHtoD_v2(dw, wb, C.c_size_t(len(wb)))); ck(cu.cuMemcpyHtoD_v2(dx, xb, C.c_size_t(len(xb))))
n = C.c_int64(N)
args = (C.c_void_p * 4)(*[C.cast(C.pointer(v), C.c_void_p) for v in (dw, dx, dq, n)])
ck(cu.cuLaunchKernel(fn, N // 128, 1, 1, 128, 1, 1, 0, None, args, None)); ck(cu.cuCtxSynchronize())
ow = C.create_string_buffer(len(wb)); oq = C.create_string_buffer(4 * N)
ck(cu.cuMemcpyDtoH_v2(ow, dw, C.c_size_t(len(wb)))); ck(cu.cuMemcpyDtoH_v2(oq, dq, C.c_size_t(4 * N)))
s8 = lambda w, k: ((w >> (8*k)) & 255) - (256 if (w >> (8*k)) & 128 else 0)
u8 = lambda w, k: (w >> (8*k)) & 255
bad = 0
for g, (a, b, c, m) in enumerate(rows):
    rs, ru, rv, _ = struct.unpack_from("<iiII", ow.raw, 16 * g)
    es = c + sum(s8(a, k) * s8(b, k) for k in range(4))
    eu = c + sum(u8(a, k) * s8(b, k) for k in range(4))
    ev = sum((((u8(a ^ m, k) - u8(m, k)) & 255) << (8*k)) for k in range(4))
    q = struct.unpack_from("<I", oq.raw, 4 * g)[0]
    eq = 0
    for k in range(4):
        t = round(X[4*g+k] * 2.5)  # Python round = half-to-even, like cvt.rni; f32 mul then round
        t = round(struct.unpack("f", struct.pack("f", X[4*g+k] * 2.5))[0])
        t = max(-127, min(127, t)); eq |= (t & 255) << (8*k)
    if (rs, ru, rv, q) != (es, eu, ev, eq):
        bad += 1
        if bad < 5: print("MISMATCH", g, (rs, ru, hex(rv), hex(q)), (es, eu, hex(ev), hex(eq)))
print("DP4A_Q8_CHECK", "PASS" if bad == 0 else "FAIL", "n=%d bad=%d" % (N, bad))
