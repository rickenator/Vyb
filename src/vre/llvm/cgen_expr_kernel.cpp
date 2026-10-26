// SPDX-License-Identifier: Apache-2.0
//
// Kernel-mode device intrinsics (issue #198 P4), extracted verbatim from
// cgen_expr.cpp so the --kernel / NVPTX path lives in its own translation unit
// (#347, checkpoint 1). No behaviour change: this is the same member definition
// declared in include/vyb/vre/llvm/codegen.hpp.

#include "vyb/vre/llvm/codegen.hpp"
#include "vyb/parser/ast.hpp"
#include "vyb/parser/token.hpp"

#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/IntrinsicsNVPTX.h>
#include <llvm/IR/GlobalVariable.h>

using namespace vyb;

// Kernel-mode device intrinsics (issue #198 P4). Lower thread/block/dimension
// reads to NVPTX special-register intrinsics (i32, zext to Int) and device-global
// load/store to addrspace(1) (global) memory access. Only reached under
// g_kernel_mode; a host-mode call to these names falls through to the normal path.
bool LLVMCodegen::emitKernelIntrinsic(vyb::ast::CallExpression* node) {
    auto* kc = dynamic_cast<ast::Identifier*>(node->callee.get());
    if (!kc) return false;
    const std::string& name = kc->name;

    // Thread / block / grid / lane reads -> `llvm.nvvm.read.ptx.sreg.*` (i32, zext).
    auto readSreg = [&](const char* sreg) -> bool {
        llvm::Function* fn = module->getFunction(sreg);
        if (!fn) {
            fn = llvm::Function::Create(
                llvm::FunctionType::get(int32Type, {}, false),
                llvm::Function::ExternalLinkage, sreg, module.get());
        }
        llvm::Value* v = builder->CreateCall(fn, {}, "sreg");
        m_currentLLVMValue = builder->CreateZExt(v, int64Type, "sreg.zext");
        return true;
    };
    if (name == "tid_x") return readSreg("llvm.nvvm.read.ptx.sreg.tid.x");
    if (name == "tid_y") return readSreg("llvm.nvvm.read.ptx.sreg.tid.y");
    if (name == "tid_z") return readSreg("llvm.nvvm.read.ptx.sreg.tid.z");
    if (name == "blk_x") return readSreg("llvm.nvvm.read.ptx.sreg.ctaid.x");
    if (name == "blk_y") return readSreg("llvm.nvvm.read.ptx.sreg.ctaid.y");
    if (name == "blk_z") return readSreg("llvm.nvvm.read.ptx.sreg.ctaid.z");
    if (name == "dim_x") return readSreg("llvm.nvvm.read.ptx.sreg.ntid.x");
    if (name == "dim_y") return readSreg("llvm.nvvm.read.ptx.sreg.ntid.y");
    if (name == "dim_z") return readSreg("llvm.nvvm.read.ptx.sreg.ntid.z");
    if (name == "grid_x") return readSreg("llvm.nvvm.read.ptx.sreg.nctaid.x");
    if (name == "grid_y") return readSreg("llvm.nvvm.read.ptx.sreg.nctaid.y");
    if (name == "lane_id") return readSreg("llvm.nvvm.read.ptx.sreg.laneid");
    if (name == "warp_size") return readSreg("llvm.nvvm.read.ptx.sreg.warpsize");

    // Block-wide barrier (bar.sync 0).
    if (name == "kernel_barrier") {
        llvm::Function* bar = module->getFunction("llvm.nvvm.barrier0");
        if (!bar) bar = llvm::Function::Create(
            llvm::FunctionType::get(llvm::Type::getVoidTy(*context), {}, false),
            llvm::Function::ExternalLinkage, "llvm.nvvm.barrier0", module.get());
        builder->CreateCall(bar, {});
        m_currentLLVMValue = nullptr;
        return true;
    }

    // Recurring helpers: evaluate an argument, build a global pointer.
    auto argv = [&](size_t i) -> llvm::Value* {
        if (i >= node->arguments.size()) return nullptr;
        node->arguments[i]->accept(*this);
        return m_currentLLVMValue;
    };
    auto gptr = [&](llvm::Value* addr) -> llvm::Value* {
        return builder->CreateIntToPtr(addr, llvm::PointerType::get(*context, 1), "gptr");
    };
    llvm::Type* i16Ty = llvm::Type::getInt16Ty(*context);

    // fp16 load: an i16 half at `addr` widened to Vyb Float (f64).
    if (name == "ld_f16" || name == "ld_bf16") {
        llvm::Value* h = builder->CreateLoad(i16Ty, gptr(argv(0)), "h.load");
        if (name == "ld_f16") {
            llvm::Value* halfV = builder->CreateBitCast(h, llvm::Type::getHalfTy(*context), "h.half");
            m_currentLLVMValue = builder->CreateFPExt(halfV, doubleType, "h.ext");
        } else { // bf16 == top 16 bits of an f32
            llvm::Value* wide = builder->CreateZExt(h, int32Type, "bf.wide");
            llvm::Value* shl = builder->CreateShl(wide, llvm::ConstantInt::get(int32Type, 16), "bf.shl");
            llvm::Value* f32 = builder->CreateBitCast(shl, llvm::Type::getFloatTy(*context), "bf.f32");
            m_currentLLVMValue = builder->CreateFPExt(f32, doubleType, "bf.ext");
        }
        return true;
    }
    // fp16 store: Float -> f32 -> half/bf16 -> i16 store.
    if (name == "st_f16" || name == "st_bf16") {
        llvm::Value* a = argv(0);
        llvm::Value* v = argv(1);
        llvm::Value* f = builder->CreateFPTrunc(v, llvm::Type::getFloatTy(*context), "h.f32");
        llvm::Value* enc;
        if (name == "st_f16") {
            llvm::Value* halfV = builder->CreateFPTrunc(f, llvm::Type::getHalfTy(*context), "h.half");
            enc = builder->CreateBitCast(halfV, i16Ty, "h.enc");
        } else {
            // bf16 = round-to-nearest-even on the f32 pattern, NOT a truncation. Add 0x7FFF
            // plus the low mantissa bit (so an exact tie rounds to even) and take the top
            // half. A plain `lshr 16` is off by one ulp wherever the dropped bits are >=
            // half an ulp, which is ~50% of values (Vyb#441).
            llvm::Value* bits = builder->CreateBitCast(f, int32Type, "bf.bits");
            llvm::Value* lsb = builder->CreateAnd(
                builder->CreateLShr(bits, llvm::ConstantInt::get(int32Type, 16)),
                llvm::ConstantInt::get(int32Type, 1), "bf.lsb");
            llvm::Value* bias = builder->CreateAdd(
                llvm::ConstantInt::get(int32Type, 0x7FFF), lsb, "bf.bias");
            llvm::Value* rounded = builder->CreateLShr(
                builder->CreateAdd(bits, bias, "bf.round"),
                llvm::ConstantInt::get(int32Type, 16), "bf.shr");
            // The rounding constant must never touch Inf or NaN: it carries out of the
            // exponent, so a NaN silently becomes Inf (0x7f807fff -> 0x7f80). Inf passes
            // through as its truncated top half; a NaN keeps its sign and gets the canonical
            // quiet bit. Payload propagation on a conversion is implementation-defined (IEEE
            // 754), so this is deliberately a single canonical NaN rather than torch's, and
            // it is the bf16 analogue of what the f16 path gets from cvt.rn.f16.f32.
            llvm::Value* expo = builder->CreateAnd(
                bits, llvm::ConstantInt::get(int32Type, 0x7F800000), "bf.expo");
            llvm::Value* isSpecial = builder->CreateICmpEQ(
                expo, llvm::ConstantInt::get(int32Type, 0x7F800000), "bf.special");
            llvm::Value* mant = builder->CreateAnd(
                bits, llvm::ConstantInt::get(int32Type, 0x007FFFFF), "bf.mant");
            llvm::Value* qbit = builder->CreateSelect(
                builder->CreateICmpNE(mant, llvm::ConstantInt::get(int32Type, 0)),
                llvm::ConstantInt::get(i16Ty, 0x0040), llvm::ConstantInt::get(i16Ty, 0), "bf.qbit");
            llvm::Value* specialEnc = builder->CreateOr(
                builder->CreateTrunc(
                    builder->CreateLShr(bits, llvm::ConstantInt::get(int32Type, 16)), i16Ty, "bf.top"),
                qbit, "bf.special.enc");
            enc = builder->CreateSelect(isSpecial, specialEnc,
                                        builder->CreateTrunc(rounded, i16Ty, "bf.enc"), "bf.store");
        }
        builder->CreateStore(enc, gptr(a));
        m_currentLLVMValue = nullptr;
        return true;
    }

    // GGUF q4_0 dequant (#203): block at `addr` holds `[f16 d][32 x 4-bit signed]`.
    // value(i) = (nibble - 8) * d. Gives Vyb kernels the 4-bit weight path that
    // makes a 4B base model (~2.2 GB packed) runnable without an fp16 mirror.
    if (name == "deq_q4_0") {
        llvm::Value* addr = argv(0);
        llvm::Value* idx = argv(1);          // Int (i64)
        llvm::Value* rawD = builder->CreateLoad(i16Ty, gptr(addr), "q.d.raw");
        llvm::Value* halfD = builder->CreateBitCast(rawD, llvm::Type::getHalfTy(*context), "q.half");
        llvm::Value* d = builder->CreateFPExt(halfD, doubleType, "q.d");   // f64 scale
        llvm::Value* shift = builder->CreateLShr(idx, llvm::ConstantInt::get(int64Type, 1), "q.shr");
        llvm::Value* byteOff = builder->CreateAdd(llvm::ConstantInt::get(int64Type, 2), shift, "q.boff");
        llvm::Value* baddr = builder->CreateAdd(addr, byteOff, "q.baddr");
        llvm::Value* byte = builder->CreateLoad(int8Type, gptr(baddr), "q.b");
        llvm::Value* lo = builder->CreateAnd(byte, llvm::ConstantInt::get(int8Type, 0x0F), "q.niblo");
        llvm::Value* hi = builder->CreateAnd(builder->CreateLShr(byte, llvm::ConstantInt::get(int8Type, 4)),
                                             llvm::ConstantInt::get(int8Type, 0x0F), "q.nibhi");
        llvm::Value* loMask = builder->CreateAnd(builder->CreateTrunc(idx, int32Type),
                                                 llvm::ConstantInt::get(int32Type, 1), "q.low");
        llvm::Value* isLow = builder->CreateICmpEQ(loMask, llvm::ConstantInt::get(int32Type, 0), "q.low.cmp");
        llvm::Value* nib = builder->CreateSelect(isLow, lo, hi, "q.nib");   // i8 0..15
        llvm::Value* s8 = builder->CreateSub(nib, llvm::ConstantInt::get(int8Type, 8), "q.s8");
        llvm::Value* s = builder->CreateSIToFP(s8, doubleType, "q.s");       // -8..7
        m_currentLLVMValue = builder->CreateFMul(s, d, "q.val");
        return true;
    }

    // Kernel-mode shared-memory f64 buffer (a module-wide addrspace(3) global).
    // `ld_shared_f64(off)` / `st_shared_f64(off, v)` index it; caller bounds-checks.
    llvm::GlobalVariable* sharedBuf = module->getNamedGlobal("__vyb_kernel_shared");
    if (name == "ld_shared_f64" || name == "st_shared_f64") {
        if (!sharedBuf) {
            uint64_t shBytes = vyb::g_kernel_shared_bytes ? vyb::g_kernel_shared_bytes : 32768;
            llvm::ArrayType* at = llvm::ArrayType::get(doubleType, shBytes / 8);  // default 32 KB (static cap 48 KB)
            sharedBuf = new llvm::GlobalVariable(*module, at, false,
                llvm::GlobalValue::InternalLinkage, llvm::Constant::getNullValue(at),
                "__vyb_kernel_shared", nullptr,
                llvm::GlobalVariable::NotThreadLocal, /*AddressSpace=*/3);
            sharedBuf->setAlignment(llvm::Align(16));
        }
        llvm::Value* off = argv(0);
        llvm::ArrayType* shTy = llvm::dyn_cast<llvm::ArrayType>(sharedBuf->getValueType());
        llvm::Value* idxs[] = { llvm::ConstantInt::get(int64Type, 0), off };
        llvm::Value* sp = builder->CreateInBoundsGEP(shTy, sharedBuf, idxs, "sh.ptr");
        if (name == "ld_shared_f64") {
            m_currentLLVMValue = builder->CreateLoad(doubleType, sp, "sh.ld");
        } else {
            llvm::Value* v = argv(1);
            builder->CreateStore(v, sp);
            m_currentLLVMValue = nullptr;
        }
        return true;
    }

    // The intrinsic's element type governs the access width, not the operand's
    // Vyb type (Int/Float are i64/f64 in LLVM). Coerce the operand to the element
    // type: st_i32/st_f32 then emit ONE 4-byte st.global.u32/f32 instead of an
    // 8-byte store that clobbers the neighbouring slot, and atomic_add_i32 reads
    // and writes the 4-byte cell with atom.global.add.u32 (#270, #301). An
    // uncoercible value is a hard codegen error.
    auto coerceToElem = [&](llvm::Value* v, llvm::Type* et) -> llvm::Value* {
        if (!v || v->getType() == et) return v;
        llvm::Type* vt = v->getType();
        if (vt->isIntegerTy() && et->isIntegerTy())
            return builder->CreateTruncOrBitCast(v, et, "el.cast");
        if (vt->isFloatingPointTy() && et->isFloatingPointTy())
            return builder->CreateFPCast(v, et, "el.cast");
        if (vt->isFloatingPointTy() && et->isIntegerTy())
            return builder->CreateFPToSI(v, et, "el.cast");
        if (vt->isIntegerTy() && et->isFloatingPointTy())
            return builder->CreateSIToFP(v, et, "el.cast");
        if (vt->isPointerTy() && et->isIntegerTy())
            return builder->CreatePtrToInt(v, et, "el.cast");
        logError(node->loc, name + " cannot use a value of this type.");
        flagHardCodegenError();
        return nullptr;
    };

    // ---- Ursela GPU features (local branch ursela/gpu-features) -------------------------
    // Native-width scalar ops, bit casts, warp shuffles, byte-addressed typed shared memory,
    // 16-byte vector loads and the f16 tensor-core MMA. Every intrinsic coerces its operands to
    // the machine type it needs, so Vyb Int/Float/CInt/UInt32/Float32 arguments all work.
    llvm::Type* f32Ty = llvm::Type::getFloatTy(*context);
    llvm::Type* halfTy = llvm::Type::getHalfTy(*context);
    auto argAs = [&](size_t i, llvm::Type* t) -> llvm::Value* {
        llvm::Value* v = argv(i);
        if (!v) return nullptr;
        if (v->getType() == t) return v;
        // UInt32/CInt -> i32 bit patterns are reinterpretations, never value conversions
        if (v->getType()->isIntegerTy() && t->isIntegerTy())
            return builder->CreateZExtOrTrunc(v, t, "u.cast");
        return coerceToElem(v, t);
    };
    auto sharedPtr = [&](llvm::Value* byteOff) -> llvm::Value* {
        if (!sharedBuf) {
            uint64_t bytes = vyb::g_kernel_shared_bytes ? vyb::g_kernel_shared_bytes : 32768;
            llvm::ArrayType* at = llvm::ArrayType::get(doubleType, bytes / 8);
            sharedBuf = new llvm::GlobalVariable(*module, at, false,
                llvm::GlobalValue::InternalLinkage, llvm::UndefValue::get(at),
                "__vyb_kernel_shared", nullptr,
                llvm::GlobalVariable::NotThreadLocal, /*AddressSpace=*/3);
            sharedBuf->setAlignment(llvm::Align(16));
        }
        llvm::Value* off = byteOff;
        if (off->getType() != int64Type) off = builder->CreateZExtOrTrunc(off, int64Type, "sh.off");
        return builder->CreateInBoundsGEP(int8Type, sharedBuf, off, "shb.ptr");
    };
    auto intrinsicFn = [&](llvm::Intrinsic::ID id, llvm::ArrayRef<llvm::Type*> tys) -> llvm::Function* {
        return llvm::Intrinsic::getDeclaration(module.get(), id, tys);
    };
    // fma_f32(a, b, c) -> Float32 / fma_f64(a, b, c) -> Float: one fused multiply-add.
    if (name == "fma_f32" || name == "fma_f64") {
        llvm::Type* t = (name == "fma_f32") ? f32Ty : doubleType;
        llvm::Value* a = argAs(0, t); llvm::Value* b = argAs(1, t); llvm::Value* c = argAs(2, t);
        if (!a || !b || !c) { m_currentLLVMValue = nullptr; return true; }
        m_currentLLVMValue = builder->CreateCall(intrinsicFn(llvm::Intrinsic::fma, {t}), {a, b, c}, "fma");
        return true;
    }
    // Native f32 math: ex2_f32(x) = 2^x (ex2.approx.ftz.f32), rsqrt_f32(x) (rsqrt.approx.ftz.f32),
    // sqrt_f32(x) (sqrt.rn.f32), max_f32/min_f32(a, b) (max.f32/min.f32). One instruction each, so
    // f32 kernels need not widen to the f64 vmath routines.
    if (name == "ex2_f32" || name == "rsqrt_f32" || name == "sqrt_f32") {
        llvm::Value* a = argAs(0, f32Ty);
        if (!a) { m_currentLLVMValue = nullptr; return true; }
        llvm::Function* f = nullptr;
        if (name == "ex2_f32") f = intrinsicFn(llvm::Intrinsic::nvvm_ex2_approx_ftz_f, {});
        else if (name == "rsqrt_f32") f = intrinsicFn(llvm::Intrinsic::nvvm_rsqrt_approx_ftz_f, {});
        else f = intrinsicFn(llvm::Intrinsic::sqrt, {f32Ty});
        m_currentLLVMValue = builder->CreateCall(f, {a}, "f32m");
        return true;
    }
    if (name == "max_f32" || name == "min_f32") {
        llvm::Value* a = argAs(0, f32Ty); llvm::Value* b = argAs(1, f32Ty);
        if (!a || !b) { m_currentLLVMValue = nullptr; return true; }
        llvm::Intrinsic::ID id = (name == "max_f32") ? llvm::Intrinsic::maxnum : llvm::Intrinsic::minnum;
        m_currentLLVMValue = builder->CreateCall(intrinsicFn(id, {f32Ty}), {a, b}, "f32mm");
        return true;
    }
    // Int8 dot products: dp4a_s32(a, b, c) = c + sum_k s8(a)_k * s8(b)_k (dp4a.s32.s32) and
    // dp4a_us32(a, b, c) with a as four u8 (dp4a.u32.s32). a and b are packed 32-bit words, c and
    // the result are 32-bit ints. vsub4(a, b) is the per-byte wrapping difference (vsub4 / __vsub4),
    // used to apply IQ sign masks: vsub4(g ^ m, m) negates the bytes where m = 0xff.
    if (name == "dp4a_s32" || name == "dp4a_us32" || name == "vsub4") {
        llvm::Value* a = argAs(0, int32Type); llvm::Value* b = argAs(1, int32Type);
        if (!a || !b) { m_currentLLVMValue = nullptr; return true; }
        if (name == "vsub4") {
            // per-byte subtract without carries: ((a | H) - (b & ~H)) ^ ((a ^ ~b) & H), H = 0x80808080
            llvm::Value* H = llvm::ConstantInt::get(int32Type, 0x80808080u);
            llvm::Value* t = builder->CreateSub(builder->CreateOr(a, H), builder->CreateAnd(b, builder->CreateNot(H)));
            llvm::Value* u = builder->CreateAnd(builder->CreateXor(a, builder->CreateNot(b)), H);
            m_currentLLVMValue = builder->CreateXor(t, u, "vsub4");
            return true;
        }
        llvm::Value* cc = argAs(2, int32Type);
        if (!cc) { m_currentLLVMValue = nullptr; return true; }
        llvm::FunctionType* ft = llvm::FunctionType::get(int32Type, {int32Type, int32Type, int32Type}, false);
        const char* txt = (name == "dp4a_s32") ? "dp4a.s32.s32 $0, $1, $2, $3;" : "dp4a.u32.s32 $0, $1, $2, $3;";
        llvm::InlineAsm* ia = llvm::InlineAsm::get(ft, txt, "=r,r,r,r", /*hasSideEffects=*/false);
        m_currentLLVMValue = builder->CreateCall(ft, ia, {a, b, cc}, "dp4a");
        return true;
    }
    // q8_1 activation quantisation: rni_f32(x) rounds to nearest-even int (cvt.rni.s32.f32),
    // sat_s8(i) clamps to [-127, 127], pack_s8x4(a, b, c, d) packs the low bytes into one word
    // (a in byte 0), so a block quantiser is q = pack_s8x4(sat_s8(rni_f32(x_k * inv_d)), ...).
    if (name == "rni_f32") {
        llvm::Value* a = argAs(0, f32Ty);
        if (!a) { m_currentLLVMValue = nullptr; return true; }
        m_currentLLVMValue = builder->CreateCall(intrinsicFn(llvm::Intrinsic::nvvm_f2i_rn, {}), {a}, "rni");
        return true;
    }
    if (name == "sat_s8") {
        llvm::Value* a = argAs(0, int32Type);
        if (!a) { m_currentLLVMValue = nullptr; return true; }
        llvm::Value* hi = llvm::ConstantInt::get(int32Type, 127);
        llvm::Value* lo = llvm::ConstantInt::get(int32Type, -127, true);
        llvm::Value* t = builder->CreateSelect(builder->CreateICmpSGT(a, hi), hi, a);
        m_currentLLVMValue = builder->CreateSelect(builder->CreateICmpSLT(t, lo), lo, t, "sat8");
        return true;
    }
    if (name == "pack_s8x4") {
        llvm::Value* r = nullptr;
        for (unsigned k = 0; k < 4; ++k) {
            llvm::Value* v = argAs(k, int32Type);
            if (!v) { m_currentLLVMValue = nullptr; return true; }
            v = builder->CreateAnd(v, llvm::ConstantInt::get(int32Type, 0xff));
            if (k) v = builder->CreateShl(v, llvm::ConstantInt::get(int32Type, 8 * k));
            r = r ? builder->CreateOr(r, v) : v;
        }
        m_currentLLVMValue = r;
        return true;
    }
    // Bit casts: f32_bits(Float32) -> UInt32, f32_from_bits(UInt32) -> Float32.
    if (name == "f32_bits") {
        llvm::Value* a = argAs(0, f32Ty);
        m_currentLLVMValue = a ? builder->CreateBitCast(a, int32Type, "f.bits") : nullptr;
        return true;
    }
    if (name == "f32_from_bits") {
        llvm::Value* a = argAs(0, int32Type);
        m_currentLLVMValue = a ? builder->CreateBitCast(a, f32Ty, "f.from") : nullptr;
        return true;
    }
    // f16 packing: h2_pack(lo, hi) -> UInt32 (two round-to-nearest halves, lo in bits 0..15),
    // h_bits(x) -> UInt32 (one half, zero-extended), h_lo/h_hi(UInt32) -> Float32 (unpack).
    if (name == "h2_pack") {
        llvm::Value* lo = argAs(0, f32Ty); llvm::Value* hi = argAs(1, f32Ty);
        if (!lo || !hi) { m_currentLLVMValue = nullptr; return true; }
        llvm::Value* l16 = builder->CreateBitCast(builder->CreateFPTrunc(lo, halfTy), i16Ty);
        llvm::Value* h16 = builder->CreateBitCast(builder->CreateFPTrunc(hi, halfTy), i16Ty);
        llvm::Value* r = builder->CreateOr(builder->CreateZExt(l16, int32Type),
            builder->CreateShl(builder->CreateZExt(h16, int32Type), 16), "h2.pack");
        m_currentLLVMValue = r;
        return true;
    }
    if (name == "h_bits") {
        llvm::Value* a = argAs(0, f32Ty);
        if (!a) { m_currentLLVMValue = nullptr; return true; }
        m_currentLLVMValue = builder->CreateZExt(
            builder->CreateBitCast(builder->CreateFPTrunc(a, halfTy), i16Ty), int32Type, "h.bits");
        return true;
    }
    if (name == "h_lo" || name == "h_hi") {
        llvm::Value* a = argAs(0, int32Type);
        if (!a) { m_currentLLVMValue = nullptr; return true; }
        if (name == "h_hi") a = builder->CreateLShr(a, 16);
        llvm::Value* h = builder->CreateBitCast(builder->CreateTrunc(a, i16Ty), halfTy);
        m_currentLLVMValue = builder->CreateFPExt(h, f32Ty, "h.unpack");
        return true;
    }
    // Warp shuffles (full-warp mask): shfl_{xor,down,up,idx}_{f32,u32}(v, lane_arg) and
    // shfl_xor_f64(v, mask) (two 32-bit shuffles of the halves).
    if (name.rfind("shfl_", 0) == 0) {
        std::string rest = name.substr(5);
        size_t us = rest.find('_');
        if (us != std::string::npos) {
            std::string mode = rest.substr(0, us), ty = rest.substr(us + 1);
            llvm::Intrinsic::ID idF = 0, idI = 0;
            int clampC = 0x1f;
            if (mode == "xor") { idF = llvm::Intrinsic::nvvm_shfl_sync_bfly_f32; idI = llvm::Intrinsic::nvvm_shfl_sync_bfly_i32; }
            else if (mode == "down") { idF = llvm::Intrinsic::nvvm_shfl_sync_down_f32; idI = llvm::Intrinsic::nvvm_shfl_sync_down_i32; }
            else if (mode == "up") { idF = llvm::Intrinsic::nvvm_shfl_sync_up_f32; idI = llvm::Intrinsic::nvvm_shfl_sync_up_i32; clampC = 0; }
            else if (mode == "idx") { idF = llvm::Intrinsic::nvvm_shfl_sync_idx_f32; idI = llvm::Intrinsic::nvvm_shfl_sync_idx_i32; }
            if (idF && (ty == "f32" || ty == "u32" || ty == "f64")) {
                llvm::Value* mask = llvm::ConstantInt::get(int32Type, 0xffffffffu);
                llvm::Value* cc = llvm::ConstantInt::get(int32Type, clampC);
                if (ty == "f64") {
                    llvm::Value* v = argAs(0, doubleType);
                    llvm::Value* b = argAs(1, int32Type);
                    if (!v || !b) { m_currentLLVMValue = nullptr; return true; }
                    llvm::Value* bits = builder->CreateBitCast(v, int64Type);
                    llvm::Value* lo = builder->CreateTrunc(bits, int32Type);
                    llvm::Value* hi = builder->CreateTrunc(builder->CreateLShr(bits, 32), int32Type);
                    llvm::Function* f = intrinsicFn(idI, {});
                    llvm::Value* lo2 = builder->CreateCall(f, {mask, lo, b, cc});
                    llvm::Value* hi2 = builder->CreateCall(f, {mask, hi, b, cc});
                    llvm::Value* r = builder->CreateOr(builder->CreateZExt(lo2, int64Type),
                        builder->CreateShl(builder->CreateZExt(hi2, int64Type), 32));
                    m_currentLLVMValue = builder->CreateBitCast(r, doubleType, "shfl.f64");
                    return true;
                }
                bool isF = (ty == "f32");
                llvm::Value* v = argAs(0, isF ? f32Ty : int32Type);
                llvm::Value* b = argAs(1, int32Type);
                if (!v || !b) { m_currentLLVMValue = nullptr; return true; }
                m_currentLLVMValue = builder->CreateCall(intrinsicFn(isF ? idF : idI, {}), {mask, v, b, cc}, "shfl");
                return true;
            }
        }
    }
    // Byte-addressed typed shared memory over the same module buffer as ld/st_shared_f64:
    // ld/st_shared_{u32,f32,u16}(byte_off[, v]); ld_shared_v4_u32(byte_off, i) reads one lane of a
    // 16-byte-aligned vector load (repeated calls on one address CSE into one ld.shared.v4).
    if (name == "ld_shared_u32" || name == "ld_shared_f32" || name == "ld_shared_u16") {
        llvm::Type* t = (name == "ld_shared_f32") ? f32Ty : (name == "ld_shared_u16" ? i16Ty : int32Type);
        llvm::Value* off = argv(0);
        if (!off) { m_currentLLVMValue = nullptr; return true; }
        llvm::Value* v = builder->CreateAlignedLoad(t, sharedPtr(off), llvm::Align(t == i16Ty ? 2 : 4), "shb.ld");
        if (t == i16Ty) v = builder->CreateZExt(v, int32Type);
        m_currentLLVMValue = v;
        return true;
    }
    if (name == "st_shared_u32" || name == "st_shared_f32" || name == "st_shared_u16") {
        llvm::Type* t = (name == "st_shared_f32") ? f32Ty : (name == "st_shared_u16" ? i16Ty : int32Type);
        llvm::Value* off = argv(0);
        llvm::Value* v = argAs(1, t);
        if (!off || !v) { m_currentLLVMValue = nullptr; return true; }
        builder->CreateAlignedStore(v, sharedPtr(off), llvm::Align(t == i16Ty ? 2 : 4));
        m_currentLLVMValue = nullptr;
        return true;
    }
    if (name == "ld_shared_v4_u32" || name == "ld_shared_v2_u32") {
        unsigned n = (name == "ld_shared_v4_u32") ? 4 : 2;
        llvm::Value* off = argv(0);
        llvm::Value* i = argAs(1, int32Type);
        if (!off || !i) { m_currentLLVMValue = nullptr; return true; }
        llvm::Type* vt = llvm::FixedVectorType::get(int32Type, n);
        llvm::Value* vec = builder->CreateAlignedLoad(vt, sharedPtr(off), llvm::Align(4 * n), "shb.vld");
        m_currentLLVMValue = builder->CreateExtractElement(vec, i, "shb.lane");
        return true;
    }
    if (name == "st_shared_v4_u32") {
        llvm::Value* off = argv(0);
        llvm::Value* a = argAs(1, int32Type); llvm::Value* b = argAs(2, int32Type);
        llvm::Value* c = argAs(3, int32Type); llvm::Value* d = argAs(4, int32Type);
        if (!off || !a || !b || !c || !d) { m_currentLLVMValue = nullptr; return true; }
        llvm::Type* vt = llvm::FixedVectorType::get(int32Type, 4);
        llvm::Value* vec = llvm::UndefValue::get(vt);
        vec = builder->CreateInsertElement(vec, a, (uint64_t)0);
        vec = builder->CreateInsertElement(vec, b, (uint64_t)1);
        vec = builder->CreateInsertElement(vec, c, (uint64_t)2);
        vec = builder->CreateInsertElement(vec, d, (uint64_t)3);
        builder->CreateAlignedStore(vec, sharedPtr(off), llvm::Align(16));
        m_currentLLVMValue = nullptr;
        return true;
    }
    // Global native-width loads: ld_u32(addr) -> UInt32 and ld_f32n(addr) -> Float32 (no widening),
    // ld_v4_u32 / ld_v2_u32(addr, i) -> UInt32 (one lane of an aligned 16/8-byte vector load; the
    // repeated loads of one address merge into a single ld.global.v4.u32), ld_v4_f32(addr, i).
    if (name == "ld_u32" || name == "ld_f32n") {
        llvm::Type* t = (name == "ld_u32") ? int32Type : f32Ty;
        llvm::Value* a = argv(0);
        if (!a) { m_currentLLVMValue = nullptr; return true; }
        m_currentLLVMValue = builder->CreateAlignedLoad(t, gptr(a), llvm::Align(4), "gld32");
        return true;
    }
    if (name == "st_f32n" || name == "st_u32") {
        llvm::Type* t = (name == "st_u32") ? int32Type : f32Ty;
        llvm::Value* a = argv(0);
        llvm::Value* v = argAs(1, t);
        if (!a || !v) { m_currentLLVMValue = nullptr; return true; }
        builder->CreateAlignedStore(v, gptr(a), llvm::Align(4));
        m_currentLLVMValue = nullptr;
        return true;
    }
    if (name == "ld_v4_u32" || name == "ld_v2_u32" || name == "ld_v4_f32") {
        unsigned n = (name == "ld_v2_u32") ? 2 : 4;
        llvm::Type* et = (name == "ld_v4_f32") ? f32Ty : int32Type;
        llvm::Value* a = argv(0);
        llvm::Value* i = argAs(1, int32Type);
        if (!a || !i) { m_currentLLVMValue = nullptr; return true; }
        llvm::Type* vt = llvm::FixedVectorType::get(et, n);
        llvm::Value* vec = builder->CreateAlignedLoad(vt, gptr(a), llvm::Align(4 * n), "gvld");
        m_currentLLVMValue = builder->CreateExtractElement(vec, i, "gv.lane");
        return true;
    }
    // Tensor-core MMA, m16n8k16, row.col, f16 inputs, f32 accumulate:
    // mma_16816(a0, a1, a2, a3, b0, b1, c0, c1, c2, c3, i) -> Float32, element i of D.
    // a*/b* are UInt32 half2 fragments (PTX mma fragment layout); c* are Float32 accumulators.
    // The intrinsic is readnone, so the 4 calls that read D[0..3] with the same operands
    // collapse into one mma.sync instruction.
    if (name == "mma_16816") {
        if (node->arguments.size() != 11) {
            logError(node->loc, "mma_16816 takes 11 arguments (a0..a3, b0, b1, c0..c3, i)");
            flagHardCodegenError();
            m_currentLLVMValue = nullptr;
            return true;
        }
        llvm::Type* h2 = llvm::FixedVectorType::get(halfTy, 2);
        std::vector<llvm::Value*> ops;
        for (size_t k = 0; k < 6; ++k) {
            llvm::Value* v = argAs(k, int32Type);
            if (!v) { m_currentLLVMValue = nullptr; return true; }
            ops.push_back(builder->CreateBitCast(v, h2));
        }
        for (size_t k = 6; k < 10; ++k) {
            llvm::Value* v = argAs(k, f32Ty);
            if (!v) { m_currentLLVMValue = nullptr; return true; }
            ops.push_back(v);
        }
        llvm::Value* i = argAs(10, int32Type);
        if (!i) { m_currentLLVMValue = nullptr; return true; }
        llvm::Function* f = intrinsicFn(llvm::Intrinsic::nvvm_mma_m16n8k16_row_col_f32_f32, {});
        llvm::Value* r = builder->CreateCall(f, ops, "mma");
        // element select: D is a 4-float struct; pick by (constant or runtime) index
        llvm::Value* d0 = builder->CreateExtractValue(r, 0);
        llvm::Value* d1 = builder->CreateExtractValue(r, 1);
        llvm::Value* d2 = builder->CreateExtractValue(r, 2);
        llvm::Value* d3 = builder->CreateExtractValue(r, 3);
        llvm::Value* z = llvm::ConstantInt::get(int32Type, 0);
        llvm::Value* s01 = builder->CreateSelect(builder->CreateICmpEQ(i, z), d0, d1);
        llvm::Value* s23 = builder->CreateSelect(builder->CreateICmpEQ(i, llvm::ConstantInt::get(int32Type, 2)), d2, d3);
        m_currentLLVMValue = builder->CreateSelect(
            builder->CreateICmpULT(i, llvm::ConstantInt::get(int32Type, 2)), s01, s23, "mma.d");
        return true;
    }

    // Global atomics (monotonic) — KV-cache writes / reductions.
    if (name == "atomic_add_f64" || name == "atomic_add_i32") {
        bool isI32 = (name == "atomic_add_i32");
        llvm::Type* el = isI32 ? int32Type : doubleType;
        llvm::Value* a = argv(0);
        llvm::Value* v = argv(1);
        v = coerceToElem(v, el);
        if (!v) { m_currentLLVMValue = nullptr; return true; }
        llvm::AtomicRMWInst::BinOp op = isI32 ? llvm::AtomicRMWInst::Add : llvm::AtomicRMWInst::FAdd;
        llvm::Value* r = builder->CreateAtomicRMW(op, gptr(a), v,
            llvm::MaybeAlign(), llvm::AtomicOrdering::Monotonic, llvm::SyncScope::System);
        m_currentLLVMValue = r;
        return true;
    }

    // Device-global load/store: address is a Vyb Int (i64) buffer address -> global.
    if (name.rfind("ld_", 0) == 0 || name.rfind("st_", 0) == 0) {
        llvm::Type* elemTy = nullptr;   // memory element type
        llvm::Type* retTy = nullptr;    // Vyb-surface result type
        if (name == "ld_f64" || name == "st_f64") { elemTy = doubleType; retTy = doubleType; }
        else if (name == "ld_f32" || name == "st_f32") {
            elemTy = llvm::Type::getFloatTy(*context); retTy = doubleType;
        }
        else if (name == "ld_i64" || name == "st_i64") { elemTy = int64Type; retTy = int64Type; }
        else if (name == "ld_i32" || name == "st_i32") { elemTy = int32Type; retTy = int32Type; }
        else if (name == "ld_i8" || name == "st_i8" || name == "ld_u8" || name == "st_u8") {
            elemTy = int8Type; retTy = int8Type;
        }
        else if (name == "ld_i16" || name == "st_i16" || name == "ld_u16" || name == "st_u16") {
            elemTy = i16Ty; retTy = i16Ty;
        }
        else { return false; }

        llvm::Value* addr = node->arguments.empty() ? nullptr : (node->arguments[0]->accept(*this), m_currentLLVMValue);
        if (!addr) return true;
        llvm::Value* gp = builder->CreateIntToPtr(
            addr, llvm::PointerType::get(*context, 1), "gptr");
        if (name.rfind("st_", 0) == 0) {
            if (node->arguments.size() < 2) return true;
            node->arguments[1]->accept(*this);
            // Store at the intrinsic's element width (see coerceToElem above).
            llvm::Value* v = coerceToElem(m_currentLLVMValue, elemTy);
            if (!v) { m_currentLLVMValue = nullptr; return true; }
            builder->CreateStore(v, gp);
            m_currentLLVMValue = nullptr;
        } else {
            llvm::Value* loaded = builder->CreateLoad(elemTy, gp, "gld");
            if (retTy != elemTy)
                loaded = builder->CreateFPExt(loaded, retTy, "ld.ext");
            m_currentLLVMValue = loaded;
        }
        return true;
    }
    return false;
}
