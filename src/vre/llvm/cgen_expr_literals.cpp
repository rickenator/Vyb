// SPDX-License-Identifier: Apache-2.0
//
// Literal lowering (integer / float / bool / string / nil), extracted verbatim
// from cgen_expr.cpp (#347, checkpoint 2). Same member definitions declared in
// include/vyb/vre/llvm/codegen.hpp; no behaviour change.

#include "vyb/vre/llvm/codegen.hpp"
#include "vyb/parser/ast.hpp"

#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>

using namespace vyb;

void LLVMCodegen::visit(vyb::ast::IntegerLiteral *node) {
    if (node->isUnsigned) {
        m_currentLLVMValue = llvm::ConstantInt::get(*context, llvm::APInt(64, node->uvalue, false));
    } else {
        m_currentLLVMValue = llvm::ConstantInt::get(*context, llvm::APInt(64, node->value, true));
    }
}

void LLVMCodegen::visit(vyb::ast::FloatLiteral *node) {
    m_currentLLVMValue = llvm::ConstantFP::get(*context, llvm::APFloat(node->value));
}

void LLVMCodegen::visit(vyb::ast::BooleanLiteral *node) {
    m_currentLLVMValue = llvm::ConstantInt::get(*context, llvm::APInt(1, node->value));
}

void LLVMCodegen::visit(vyb::ast::StringLiteral *node) {
    llvm::Type* int8PtrType = llvm::PointerType::get(*context, 0);
    llvm::Type* int64Type = llvm::Type::getInt64Ty(*context);
    llvm::StructType* stringStructType = llvm::StructType::get(*context, {int8PtrType, int64Type});
    llvm::Constant* lenValue = llvm::ConstantInt::get(int64Type, node->value.length());

    if (currentFunction) {
        // Inside a function - use CreateGlobalStringPtr and build a runtime string struct.
        llvm::Value* strPtr = builder->CreateGlobalStringPtr(node->value);
        llvm::Value* stringStruct = llvm::UndefValue::get(stringStructType);
        stringStruct = builder->CreateInsertValue(stringStruct, strPtr, 0, "str.ptr");
        stringStruct = builder->CreateInsertValue(stringStruct, lenValue, 1, "str.len");
        m_currentLLVMValue = stringStruct;
        return;
    }

    // Global scope - emit a constant global string and return a constant String struct.
    llvm::Constant* stringConstant = llvm::ConstantDataArray::getString(*context, node->value, true);
    llvm::GlobalVariable* globalString = new llvm::GlobalVariable(
        *module,
        stringConstant->getType(),
        true,
        llvm::GlobalValue::PrivateLinkage,
        stringConstant,
        ".str"
    );

    std::vector<llvm::Constant*> indices = {
        llvm::ConstantInt::get(int64Type, 0),
        llvm::ConstantInt::get(int64Type, 0)
    };

    llvm::Constant* strPtr = llvm::ConstantExpr::getGetElementPtr(
        stringConstant->getType(),
        globalString,
        indices
    );

    m_currentLLVMValue = llvm::ConstantStruct::get(stringStructType, {strPtr, lenValue});
}

void LLVMCodegen::visit(vyb::ast::NilLiteral* node) {
    // Nil is a polymorphic null pointer. For now, default to i8*.
    // Type inference or context should ideally provide a more specific pointer type.
    if (m_currentLLVMType && m_currentLLVMType->isPointerTy()) {
        // If we have a specific pointer type from context, use it
        m_currentLLVMValue = llvm::ConstantPointerNull::get(
            llvm::cast<llvm::PointerType>(m_currentLLVMType));
    } else {
        // Default to i8* if no specific type is known
        m_currentLLVMValue = llvm::ConstantPointerNull::get(
            llvm::PointerType::getUnqual(int8Type));
    }
}
