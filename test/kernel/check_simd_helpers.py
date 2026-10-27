# reference check for test_simd_helpers kernel via the CUDA driver (not in any inference path)
# usage: python3 check_simd_helpers.py simdh.ptx   (also asserts the PTX uses ld.global.nc.v4)
import ctypes as C, random, struct, sys
ptx = open(sys.argv[1], "rb").read()
assert b"ld.global.nc.v4" in ptx, "expected ld.global.nc.v4 in PTX"
cu = C.CDLL("libcuda.so.1")
def ck(r):
    if r: raise SystemExit("cuda err %d" % r)
ck(cu.cuInit(0)); dev = C.c_int(); ck(cu.cuDeviceGet(C.byref(dev), 0))
ctx = C.c_void_p(); ck(cu.cuCtxCreate_v2(C.byref(ctx), 0, dev))
mod = C.c_void_p(); ck(cu.cuModuleLoadData(C.byref(mod), ptx + b"\0"))
fn = C.c_void_p(); ck(cu.cuModuleGetFunction(C.byref(fn), mod, b"simdh"))
N = 4096; random.seed(2)
rows = [(random.getrandbits(32), random.getrandbits(32), random.getrandbits(32), random.randint(-2**20, 2**20)) for _ in range(N)]
F = [struct.unpack("f", struct.pack("f", random.uniform(-100, 100)))[0] for _ in range(N)]
wb = b"".join(struct.pack("<IIIi", *r) for r in rows); fb = struct.pack("<%df" % N, *F)
def alloc(n):
    p = C.c_uint64(); ck(cu.cuMemAlloc_v2(C.byref(p), C.c_size_t(n))); return p
dw, dw2, df, do = alloc(len(wb)), alloc(len(wb)), alloc(len(fb)), alloc(16 * N)
ck(cu.cuMemcpyHtoD_v2(dw, wb, C.c_size_t(len(wb)))); ck(cu.cuMemcpyHtoD_v2(dw2, wb, C.c_size_t(len(wb))))
ck(cu.cuMemcpyHtoD_v2(df, fb, C.c_size_t(len(fb))))
n = C.c_int64(N)
args = (C.c_void_p * 5)(*[C.cast(C.pointer(v), C.c_void_p) for v in (dw, dw2, df, do, n)])
ck(cu.cuLaunchKernel(fn, N // 128, 1, 1, 128, 1, 1, 0, None, args, None)); ck(cu.cuCtxSynchronize())
ow = C.create_string_buffer(len(wb)); oo = C.create_string_buffer(16 * N)
ck(cu.cuMemcpyDtoH_v2(ow, dw, C.c_size_t(len(wb)))); ck(cu.cuMemcpyDtoH_v2(oo, do, C.c_size_t(16 * N)))
s8 = lambda w, k: ((w >> (8*k)) & 255) - (256 if (w >> (8*k)) & 128 else 0)
f32 = lambda v: struct.unpack("f", struct.pack("f", v))[0]
bad = 0
for gi, (g, x, nib, c) in enumerate(rows):
    m, s, ws, r = struct.unpack_from("<IiiI", ow.raw, 16 * gi)
    em = sum((0xff << (8*k)) for k in range(4) if (nib >> k) & 1)
    es = c + sum((-1 if (nib >> k) & 1 else 1) * s8(g, k) * s8(x, k) for k in range(4))
    w0 = gi // 32 * 32
    ews = sum(rows[j][3] for j in range(w0, w0 + 32))
    ews = (ews + 2**31) % 2**32 - 2**31
    er = rows[gi // 4][gi % 4] & 0xffffffff
    fs, fm, fl, _ = struct.unpack_from("<4f", oo.raw, 16 * gi)
    # butterfly order: v += shfl(v, 16), 8, 4, 2, 1 -- emulate exactly in f32
    v = F[w0:w0 + 32]
    for off in (16, 8, 4, 2, 1):
        v = [f32(v[l] + v[l ^ off]) for l in range(32)]
    efs = v[gi - w0]; efm = max(F[w0:w0 + 32]); efl = F[gi // 4 * 4 + gi % 4]
    if (m, s, ws, r) != (em, es, ews, er) or (fs, fm, fl) != (efs, efm, efl):
        bad += 1
        if bad < 5: print("MISMATCH", gi, (hex(m), s, ws, hex(r), fs, fm, fl), (hex(em), es, ews, hex(er), efs, efm, efl))
print("SIMD_HELPERS_CHECK", "PASS" if bad == 0 else "FAIL", "n=%d bad=%d" % (N, bad))
