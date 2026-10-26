# reference check for test_imma_s8 via the CUDA driver (not in any inference path)
import ctypes as C, random, struct, sys
cu = C.CDLL("libcuda.so.1")
def ck(r):
    if r: raise SystemExit("cuda err %d" % r)
ck(cu.cuInit(0)); dev = C.c_int(); ck(cu.cuDeviceGet(C.byref(dev), 0))
ctx = C.c_void_p(); ck(cu.cuCtxCreate_v2(C.byref(ctx), 0, dev))
mod = C.c_void_p(); ck(cu.cuModuleLoadData(C.byref(mod), open(sys.argv[1], "rb").read() + b"\0"))
fn = C.c_void_p(); ck(cu.cuModuleGetFunction(C.byref(fn), mod, b"imma"))
def alloc(n):
    p = C.c_uint64(); ck(cu.cuMemAlloc_v2(C.byref(p), C.c_size_t(n))); return p
bad = 0
for trial in range(20):
    random.seed(trial)
    A = [random.randint(-128, 127) for _ in range(512)]; B = [random.randint(-128, 127) for _ in range(256)]
    da, db, dd = alloc(512), alloc(256), alloc(1024)
    ck(cu.cuMemcpyHtoD_v2(da, struct.pack("512b", *A), C.c_size_t(512)))
    ck(cu.cuMemcpyHtoD_v2(db, struct.pack("256b", *B), C.c_size_t(256)))
    args = (C.c_void_p * 3)(*[C.cast(C.pointer(v), C.c_void_p) for v in (da, db, dd)])
    ck(cu.cuLaunchKernel(fn, 1, 1, 1, 32, 1, 1, 0, None, args, None)); ck(cu.cuCtxSynchronize())
    o = C.create_string_buffer(1024); ck(cu.cuMemcpyDtoH_v2(o, dd, C.c_size_t(1024)))
    D = struct.unpack("256i", o.raw)
    for r in range(16):
        for c in range(8):
            e32 = r * 8 + c + sum(A[r * 32 + k] * B[c * 32 + k] for k in range(32))
            e16 = r * 8 + c + sum(A[r * 32 + k] * B[c * 32 + k] for k in range(16))
            if D[r * 8 + c] != e32 or D[128 + r * 8 + c] != e16:
                bad += 1
                if bad < 5: print("MISMATCH", trial, r, c, D[r * 8 + c], e32, D[128 + r * 8 + c], e16)
print("IMMA_S8_CHECK", "PASS" if bad == 0 else "FAIL", "trials=20 bad=%d" % bad)
