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
            llvm::Value* bits = builder->CreateBitCast(f, int32Type, "bf.bits");
            enc = builder->CreateTrunc(builder->CreateLShr(bits, llvm::ConstantInt::get(int32Type, 16)), i16Ty, "bf.enc");
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
            llvm::ArrayType* at = llvm::ArrayType::get(doubleType, 4096);  // 32 KB (static shared cap is 48KB on sm_86)
            sharedBuf = new llvm::GlobalVariable(*module, at, false,
                llvm::GlobalValue::InternalLinkage, llvm::Constant::getNullValue(at),
                "__vyb_kernel_shared", nullptr,
                llvm::GlobalVariable::NotThreadLocal, /*AddressSpace=*/3);
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
