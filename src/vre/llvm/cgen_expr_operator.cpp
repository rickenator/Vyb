// SPDX-License-Identifier: Apache-2.0
//
// Seam extracted from src/vre/llvm/cgen_expr.cpp (#347): the member definitions below
// are moved VERBATIM -- no reformatting, no renames -- so the split stays
// behaviour-neutral. Helpers that other code still calls are promoted into
// include/vyb/vre/semantic_internal.hpp as `inline`, never duplicated.

#include "vyb/vre/llvm/codegen.hpp"
#include "vyb/parser/ast.hpp"
#include "vyb/vre/thread_boundary.hpp"
#include "vyb/parser/token.hpp" // For TokenType in BinaryExpression
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/DerivedTypes.h> // For PointerType, StructType
#include <llvm/ADT/APFloat.h>   // For APFloat in FloatLiteral
#include <llvm/ADT/APInt.h>     // For APInt in IntegerLiteral
#include <regex>                // For regex in MemberExpression loaded value handling

namespace vyb {

void LLVMCodegen::visit(vyb::ast::UnaryExpression *node) {
    node->operand->accept(*this);
    llvm::Value *operandValue = m_currentLLVMValue;

    if (!operandValue) {
        logError(node->operand->loc, "Operand for unary expression is null.");
        m_currentLLVMValue = nullptr;
        return;
    }

    switch (node->op.type) {
        case vyb::TokenType::MINUS: // Reverted to vyb::TokenType::MINUS
            if (operandValue->getType()->isFloatingPointTy()) {
                m_currentLLVMValue = builder->CreateFNeg(operandValue, "fnegtmp");
            } else if (operandValue->getType()->isIntegerTy()) {
                m_currentLLVMValue = builder->CreateNeg(operandValue, "negtmp");
            } else {
                logError(node->loc, "Unary minus operator can only be applied to integer or float types.");
                m_currentLLVMValue = nullptr;
            }
            break;
        case vyb::TokenType::BANG: // Reverted to vyb::TokenType::BANG
            // Logical NOT: typically (operand == 0) for integers, or fcmp one for floats
             if (operandValue->getType()->isIntegerTy(1)) { // Already a boolean
                m_currentLLVMValue = builder->CreateNot(operandValue, "nottmp");
            } else if (operandValue->getType()->isIntegerTy()) { // Other integers
                m_currentLLVMValue = builder->CreateICmpEQ(operandValue, llvm::ConstantInt::get(operandValue->getType(), 0), "icmpeqtmp");
            } else if (operandValue->getType()->isFloatingPointTy()) {
                m_currentLLVMValue = builder->CreateFCmpOEQ(operandValue, llvm::ConstantFP::get(operandValue->getType(), 0.0), "fcmpoeqtmp");
            } else {
                logError(node->loc, "Logical NOT operator can only be applied to boolean, integer or float types.");
                m_currentLLVMValue = nullptr;
            }
            break;
        case vyb::TokenType::TILDE: // Reverted to vyb::TokenType::TILDE
            // Bitwise NOT: one's complement of an integer
            if (operandValue->getType()->isIntegerTy()) {
                m_currentLLVMValue = builder->CreateNot(operandValue, "bwnottmp");
            } else {
                logError(node->loc, "Unary '~' operator can only be applied to integer types.");
                m_currentLLVMValue = nullptr;
            }
            break;
        case vyb::TokenType::KEYWORD_AWAIT: {
            // `await expr` as a bare statement (parser wraps it as a UnaryExpression
            // with an AWAIT token). Delegate to the await-expression path so the
            // event loop is actually driven. A String result is discarded here and
            // must release its single owned reference (the slot was already freed
            // inside the await path).
            auto awaitExpr = std::make_unique<ast::AwaitExpression>(node->loc,
                std::move(node->operand));
            visit(awaitExpr.get());
            if (m_currentLLVMValue && isVybStringStructType(m_currentLLVMValue->getType())) {
                releaseStringValue(m_currentLLVMValue);
            }
            break;
        }
        default:
            logError(node->loc, "Unsupported unary operator.");
            m_currentLLVMValue = nullptr;
            break;
    }
}
void LLVMCodegen::visit(vyb::ast::BinaryExpression *node) {
    // Native `T?` default: `optional else default`. Handled first (before
    // visiting operands) so the default is only evaluated when the optional is
    // absent. An eager `CreateSelect` would run the default's expression on the
    // success path too -- wrong for side-effecting defaults such as a failable
    // call or an `open(...)`. We branch on presence, visit the default only in
    // the absent block, then merge with a phi.
    if (node->op.type == TokenType::KEYWORD_ELSE) {
        if (!node->left || !node->right) {
            logError(node->loc, "Malformed 'else' expression.");
            m_currentLLVMValue = nullptr;
            return;
        }
        node->left->accept(*this);
        llvm::Value* L = m_currentLLVMValue;
        if (!L || !L->getType()->isStructTy() ||
            llvm::cast<llvm::StructType>(L->getType())->getNumElements() < 2) {
            logError(node->loc, "'else' operator requires an optional (T?) left value.");
            m_currentLLVMValue = nullptr;
            return;
        }
        llvm::Value* present = builder->CreateExtractValue(L, 1, "opt.present");
        llvm::Value* payload = builder->CreateExtractValue(L, 0, "opt.payload");

        llvm::Function* parentFn = builder->GetInsertBlock()->getParent();
        llvm::BasicBlock* presentBB = llvm::BasicBlock::Create(*context, "opt.else.present", parentFn);
        llvm::BasicBlock* defaultBB = llvm::BasicBlock::Create(*context, "opt.else.default", parentFn);
        llvm::BasicBlock* mergeBB  = llvm::BasicBlock::Create(*context, "opt.else.merge", parentFn);

        builder->CreateCondBr(present, presentBB, defaultBB);

        builder->SetInsertPoint(presentBB);
        builder->CreateBr(mergeBB);

        builder->SetInsertPoint(defaultBB);
        node->right->accept(*this);
        llvm::Value* defaultVal = m_currentLLVMValue;
        if (!defaultVal || defaultVal->getType() != payload->getType()) {
            defaultVal = tryCast(defaultVal, payload->getType(), node->loc);
            if (!defaultVal) {
                logError(node->loc, "'else' default value is not assignable to the optional payload type.");
                m_currentLLVMValue = nullptr;
                return;
            }
        }
        llvm::BasicBlock* defaultSrcBB = builder->GetInsertBlock();
        builder->CreateBr(mergeBB);

        builder->SetInsertPoint(mergeBB);
        llvm::PHINode* result = builder->CreatePHI(payload->getType(), 2, "opt.else.result");
        result->addIncoming(payload, presentBB);
        result->addIncoming(defaultVal, defaultSrcBB);
        m_currentLLVMValue = result;
        return;
    }

    node->left->accept(*this);
    llvm::Value *L = m_currentLLVMValue;
    vyb::ast::TypeNode* leftTypeNode = typeOfNode(node->left).get(); // Get AST type of left operand

    node->right->accept(*this);
    llvm::Value *R = m_currentLLVMValue;
    vyb::ast::TypeNode* rightTypeNode = typeOfNode(node->right).get(); // Get AST type of right operand

    if (!L || !R) {
        logError(node->loc, "One or both operands of binary expression are null.");
        m_currentLLVMValue = nullptr;
        return;
    }

    // An optional compared against a `null`/`nil` literal is a presence check:
    // the absent optional equals `null`. `opt == null` <=> !present;
    // `opt != null` <=> present. Without this, the optional's `{ T, i1 }` struct
    // is compared against the null literal's `i8*`, which abort()s in LLVM
    // (ICmpInst type mismatch). Non-optional pointers keep the plain null test.
    // This must run before any pointer/int swap below so operands are in their
    // original order.
    {
        bool leftNil  = dynamic_cast<ast::NilLiteral*>(node->left.get()) != nullptr;
        bool rightNil = dynamic_cast<ast::NilLiteral*>(node->right.get()) != nullptr;
        if ((node->op.type == vyb::TokenType::EQEQ || node->op.type == vyb::TokenType::NOTEQ) &&
            leftNil != rightNil) {
            llvm::Value* other = leftNil ? R : L;
            if (isOptionalStructType(other->getType())) {
                llvm::Value* opt = other;
                llvm::Value* present = builder->CreateExtractValue(opt, 1, "optcmp.null.present");
                m_currentLLVMValue = (node->op.type == vyb::TokenType::EQEQ)
                    ? builder->CreateNot(present, "optcmp.null.isnull")
                    : present;
                return;
            }
            // A plain value compared against null is only meaningful for
            // pointers; anything else used to abort() inside LLVM with an
            // ICmpInst type mismatch, so turn it into a clean diagnostic.
            if (!other->getType()->isPointerTy()) {
                logError(node->loc, "cannot compare a non-pointer, non-optional value against 'null'");
                m_currentLLVMValue = nullptr;
                return;
            }
        }
    }

    bool isFloatOp = L->getType()->isFloatingPointTy() || R->getType()->isFloatingPointTy();

    if (L->getType()->isFloatingPointTy() && R->getType()->isIntegerTy()) {
        R = builder->CreateSIToFP(R, L->getType(), "sitofptmp");
        isFloatOp = true;
    } else if (R->getType()->isFloatingPointTy() && L->getType()->isIntegerTy()) {
        L = builder->CreateSIToFP(L, R->getType(), "sitofptmp");
        isFloatOp = true;
    } else if (L->getType()->isIntegerTy() && R->getType()->isIntegerTy() && L->getType() != R->getType()) {
        // Handle integer width mismatches (e.g., i32 vs i64)
        // Coerce to the smaller width to preserve variable precision
        llvm::IntegerType* leftIntType = llvm::cast<llvm::IntegerType>(L->getType());
        llvm::IntegerType* rightIntType = llvm::cast<llvm::IntegerType>(R->getType());

        if (leftIntType->getBitWidth() < rightIntType->getBitWidth()) {
            // Left is smaller, truncate right to match left
            R = builder->CreateTrunc(R, L->getType(), "inttrunctmp");
        } else {
            // Right is smaller, truncate left to match right
            L = builder->CreateTrunc(L, R->getType(), "inttrunctmp");
        }
    } else if (L->getType()->isFloatingPointTy() && R->getType()->isFloatingPointTy() &&
               L->getType() != R->getType()) {
        // #312: two floating-point operands of different widths. An unannotated
        // float literal always lowers as an LLVM `double` (see
        // visit(ast::FloatLiteral)), so `x<Float32> == 2.5` reached `CreateFCmpOEQ`
        // as `fcmp oeq float %x, double 2.5` -- invalid IR that fails module
        // verification while the compiler still exits 0. `Float64` was unaffected
        // because it already lowers to `double`.
        //
        // Width rule: a float LITERAL is retyped to the width of the value it is
        // compared against, which keeps the declared width of the variable (`2.5`
        // is exactly representable in `float`). When NEITHER operand is a literal
        // both sides are live stored values, so promote to the WIDER width --
        // silently truncating a stored value would change the comparison's result.
        bool leftIsLiteral = dynamic_cast<vyb::ast::FloatLiteral*>(node->left.get()) != nullptr;
        bool rightIsLiteral = dynamic_cast<vyb::ast::FloatLiteral*>(node->right.get()) != nullptr;

        if (leftIsLiteral != rightIsLiteral) {
            llvm::Value* literal = leftIsLiteral ? L : R;
            llvm::Type* target = leftIsLiteral ? R->getType() : L->getType();
            llvm::Value* retyped = (literal->getType()->getScalarSizeInBits() >
                                    target->getScalarSizeInBits())
                                       ? builder->CreateFPTrunc(literal, target, "fptrunctmp")
                                       : builder->CreateFPExt(literal, target, "fpexttmp");
            if (leftIsLiteral) L = retyped; else R = retyped;
        } else if (L->getType()->getScalarSizeInBits() >= R->getType()->getScalarSizeInBits()) {
            R = builder->CreateFPExt(R, L->getType(), "fpexttmp");
        } else {
            L = builder->CreateFPExt(L, R->getType(), "fpexttmp");
        }
    } else if (L->getType()->isPointerTy() && R->getType()->isIntegerTy()) {
        // Pointer arithmetic (e.g. ptr + int)
        // We need to extract the appropriate type information for CreateGEP
        leftTypeNode = typeOfNode(node->left).get();
        // No additional changes needed with leftTypeNode - already set above
    } else if (R->getType()->isPointerTy() && L->getType()->isIntegerTy()) {
        // Pointer arithmetic (e.g. int + ptr)
        std::swap(L,R); // Put pointer on the left
        leftTypeNode = typeOfNode(node->right).get(); // Pointer is now L, so use right's AST type
    }

    // Native optional `T?` equality (`==` / `!=`, e.g. `a == Int?()`): compare
    // presence and, when both present, the payloads. This previously fell into the
    // 2-element-struct "string" path and emitted an invalid memcmp; route it to a
    // proper optional comparison. Ordering on optionals is rejected in semantic.cpp.
    if ((node->op.type == vyb::TokenType::EQEQ || node->op.type == vyb::TokenType::NOTEQ) &&
        isOptionalStructType(L->getType()) && isOptionalStructType(R->getType())) {
        m_currentLLVMValue = generateOptionalEquality(L, R, node->op.type);
        return;
    }

    // Tagged-union enum equality (`==` / `!=`): compare the i64 tag and, when
    // tags match, the matched variant's payload fields via a switch (structs
    // cannot be fed into ICmp directly — see generateTaggedEnumEquality, #181).
    if ((node->op.type == vyb::TokenType::EQEQ || node->op.type == vyb::TokenType::NOTEQ) &&
        L->getType()->isStructTy() && R->getType()->isStructTy() &&
        L->getType() == R->getType()) {
        const TaggedEnumInfo* enInfo = findTaggedEnum(L->getType());
        if (enInfo) {
            m_currentLLVMValue = generateTaggedEnumEquality(L, R, node->op.type, *enInfo);
            return;
        }
    }

    switch (node->op.type) {
        case vyb::TokenType::PLUS: // Reverted to vyb::TokenType::PLUS
            if (isFloatOp) {
                m_currentLLVMValue = builder->CreateFAdd(L, R, "faddtmp");
                break;  // Exit the case after creating FAdd
            }

            // Handle non-float operations
            {
                if (verbose) {
                    VYB_CDBG << "DEBUG PLUS: leftTypeNode=" << (leftTypeNode ? "yes" : "null")
                              << ", rightTypeNode=" << (rightTypeNode ? "yes" : "null") << std::endl;
                    VYB_CDBG << "DEBUG PLUS: Checking LLVM types for string detection..." << std::endl;
                }

                // Check for String struct types: { ptr, len }
                bool leftIsStringStruct = false;
                bool rightIsStringStruct = false;

                if (L->getType()->isStructTy()) {
                    llvm::StructType* structType = llvm::cast<llvm::StructType>(L->getType());
                    if (structType->getNumElements() == 2 && structType->getElementType(0)->isPointerTy()) {
                        leftIsStringStruct = true;
                    }
                }
                if (R->getType()->isStructTy()) {
                    llvm::StructType* structType = llvm::cast<llvm::StructType>(R->getType());
                    if (structType->getNumElements() == 2 && structType->getElementType(0)->isPointerTy()) {
                        rightIsStringStruct = true;
                    }
                }

                // Handle String + String concatenation
                if (leftIsStringStruct && rightIsStringStruct) {
                    VYB_CDBG << "DEBUG PLUS: String + String concatenation detected" << std::endl;

                    // Define String struct type: { ptr: *i8, len: i64 }
                    std::vector<llvm::Type*> strFields = {
                        llvm::PointerType::get(*context, 0),
                        llvm::Type::getInt64Ty(*context)
                    };
                    llvm::StructType* strStructType = llvm::StructType::get(*context, strFields, false);

                    // Extract fields from left string
                    llvm::Value* str1Data = builder->CreateExtractValue(L, 0, "str1.data");
                    llvm::Value* str1Len = builder->CreateExtractValue(L, 1, "str1.len");

                    // Extract fields from right string
                    llvm::Value* str2Data = builder->CreateExtractValue(R, 0, "str2.data");
                    llvm::Value* str2Len = builder->CreateExtractValue(R, 1, "str2.len");

                    // Calculate new length
                    llvm::Value* newLen = builder->CreateAdd(str1Len, str2Len, "str.new_len");

                    // Allocate new buffer (+1 for null terminator)
                    llvm::Value* allocSize = builder->CreateAdd(newLen,
                        llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 1), "str.alloc_size");

                    llvm::FunctionType* mallocType = llvm::FunctionType::get(
                        llvm::PointerType::get(*context, 0),
                        {llvm::Type::getInt64Ty(*context)},
                        false
                    );
                    llvm::Function* mallocFunc = module->getFunction("malloc");
                    if (!mallocFunc) {
                        mallocFunc = llvm::Function::Create(mallocType, llvm::Function::ExternalLinkage, "malloc", module.get());
                    }
                    llvm::Value* newData = builder->CreateCall(mallocFunc, {allocSize}, "str.new_data");

                    // Copy first string
                    llvm::FunctionType* memcpyType = llvm::FunctionType::get(
                        llvm::PointerType::get(*context, 0),
                        {llvm::PointerType::get(*context, 0), llvm::PointerType::get(*context, 0), llvm::Type::getInt64Ty(*context)},
                        false
                    );
                    llvm::Function* memcpyFunc = module->getFunction("memcpy");
                    if (!memcpyFunc) {
                        memcpyFunc = llvm::Function::Create(memcpyType, llvm::Function::ExternalLinkage, "memcpy", module.get());
                    }
                    builder->CreateCall(memcpyFunc, {newData, str1Data, str1Len});

                    // Copy second string at offset
                    llvm::Value* offset = builder->CreateGEP(llvm::Type::getInt8Ty(*context), newData, str1Len, "str.offset");
                    builder->CreateCall(memcpyFunc, {offset, str2Data, str2Len});

                    // Add null terminator
                    llvm::Value* nullTermPos = builder->CreateGEP(llvm::Type::getInt8Ty(*context), newData, newLen, "str.null_pos");
                    builder->CreateStore(llvm::ConstantInt::get(llvm::Type::getInt8Ty(*context), 0), nullTermPos);

                    // Track this freshly-allocated buffer in the string registry so
                    // __vyb_string_free can reclaim it safely (and binding-level
                    // cleanup at scope end or overwrite can too).
                    builder->CreateCall(getOrCreateVybStringRegisterFunction(), {newData});

                    // If an operand was itself a freshly-allocated owned String temp
                    // (e.g. a nested concat or `.to_string()`), it has now been fully
                    // consumed by the copy above, so it can be reclaimed.
                    if (exprProducesOwnedStringTemp(node->left.get())) {
                        builder->CreateCall(getOrCreateVybStringFreeFunction(), {str1Data});
                    }
                    if (exprProducesOwnedStringTemp(node->right.get())) {
                        builder->CreateCall(getOrCreateVybStringFreeFunction(), {str2Data});
                    }

                    // Create new String struct
                    llvm::Value* resultStr = llvm::UndefValue::get(strStructType);
                    resultStr = builder->CreateInsertValue(resultStr, newData, 0, "str.result_data");
                    resultStr = builder->CreateInsertValue(resultStr, newLen, 1, "str.result_len");

                    m_currentLLVMValue = resultStr;
                    break;
                }

                // Handle String struct + non-string concatenation (e.g. "Hello" + 1000)
                if (leftIsStringStruct || rightIsStringStruct) {
                    m_currentLLVMValue = generateMixedStringConcatenation(L, R, leftTypeNode, rightTypeNode, node->loc,
                                  exprProducesOwnedStringTemp(node->left.get()),
                                  exprProducesOwnedStringTemp(node->right.get()));
                    if (!m_currentLLVMValue) {
                        logError(node->loc, "Failed to generate mixed string concatenation");
                        return;
                    }
                    break;
                }

                // First check for old-style string types using LLVM types directly (more reliable)
                bool leftIsString = (L->getType() == int8PtrType);
                bool rightIsString = (R->getType() == int8PtrType);

                // If at least one operand is a string, treat as string concatenation
                if (leftIsString || rightIsString) {
                    if (verbose) {
                        VYB_CDBG << "DEBUG PLUS: Detected string concatenation (leftIsString="
                                  << leftIsString << ", rightIsString=" << rightIsString << ")" << std::endl;
                    }
                    m_currentLLVMValue = generateMixedStringConcatenation(L, R, leftTypeNode, rightTypeNode, node->loc,
                                  exprProducesOwnedStringTemp(node->left.get()),
                                  exprProducesOwnedStringTemp(node->right.get()));
                    if (!m_currentLLVMValue) {
                        logError(node->loc, "Failed to generate mixed string concatenation");
                        return;
                    }
                    break;
                }
            }

            // Check for string concatenation - either pure string + string or mixed types with string
            if (leftTypeNode && rightTypeNode) {
                // Resolve type aliases to get base type names
                std::string leftBaseName = resolveTypeAliasToBaseName(leftTypeNode);
                std::string rightBaseName = resolveTypeAliasToBaseName(rightTypeNode);

                // Check if either operand is a string (including string literals)
                bool leftIsString = (L->getType() == int8PtrType) || (leftBaseName == "String") ||
                                   (leftTypeNode->getCategory() == vyb::ast::TypeNode::Category::IDENTIFIER &&
                                    dynamic_cast<vyb::ast::TypeName*>(leftTypeNode) &&
                                    dynamic_cast<vyb::ast::TypeName*>(leftTypeNode)->identifier &&
                                    (dynamic_cast<vyb::ast::TypeName*>(leftTypeNode)->identifier->name == "String" ||
                                     dynamic_cast<vyb::ast::TypeName*>(leftTypeNode)->identifier->name == "string"));

                bool rightIsString = (R->getType() == int8PtrType) || (rightBaseName == "String") ||
                                    (rightTypeNode->getCategory() == vyb::ast::TypeNode::Category::IDENTIFIER &&
                                     dynamic_cast<vyb::ast::TypeName*>(rightTypeNode) &&
                                     dynamic_cast<vyb::ast::TypeName*>(rightTypeNode)->identifier &&
                                     (dynamic_cast<vyb::ast::TypeName*>(rightTypeNode)->identifier->name == "String" ||
                                      dynamic_cast<vyb::ast::TypeName*>(rightTypeNode)->identifier->name == "string"));

                // If at least one operand is a string, treat as string concatenation with auto-conversion
                if (leftIsString || rightIsString) {
                    m_currentLLVMValue = generateMixedStringConcatenation(L, R, leftTypeNode, rightTypeNode, node->loc,
                                  exprProducesOwnedStringTemp(node->left.get()),
                                  exprProducesOwnedStringTemp(node->right.get()));
                    if (!m_currentLLVMValue) {
                        logError(node->loc, "Failed to generate mixed string concatenation");
                        return;
                    }
                }
                // Check for pointer arithmetic
                else if (L->getType()->isPointerTy() && R->getType()->isIntegerTy() && leftTypeNode) {
                    vyb::ast::TypeNode* pointeeAstType = nullptr;

                    // Try to get pointee type from different sources
                    if (auto ptrAstNode = dynamic_cast<vyb::ast::PointerType*>(leftTypeNode)) {
                        pointeeAstType = ptrAstNode->pointeeType.get();
                    } else if (auto arrayAstNode = dynamic_cast<vyb::ast::ArrayType*>(leftTypeNode)) {
                        pointeeAstType = arrayAstNode->elementType.get();
                    } else if (auto typeName = dynamic_cast<vyb::ast::TypeName*>(leftTypeNode)) {
                        // Check if it's a loc<T> type
                        if (typeName->identifier->name == "loc" && !typeName->genericArgs.empty()) {
                            pointeeAstType = typeName->genericArgs[0].get();
                        }
                    }

                    if (pointeeAstType) {
                        llvm::Type* pointeeType = codegenType(pointeeAstType);
                        if (pointeeType) {
                            m_currentLLVMValue = builder->CreateGEP(pointeeType, L, R, "ptraddtmp");
                        } else {
                            logError(node->left->loc, "Could not determine LLVM pointee type for pointer addition from AST type: " + leftTypeNode->toString());
                            m_currentLLVMValue = nullptr;
                        }
                    } else {
                        // If we can't determine the pointee type from AST, use int64 as a fallback
                        if (verbose) {
                            logWarning(node->left->loc, "Pointer operand for addition lacks specific pointee type information. Using i64 as fallback pointee type.");
                        }
                        m_currentLLVMValue = builder->CreateGEP(int64Type, L, R, "ptraddtmp_fallback");
                    }
                }
                // Regular integer/numeric addition
                else {
                    m_currentLLVMValue = builder->CreateAdd(L, R, "addtmp");
                }
            }
            // Fallback to regular addition if no type info
            else {
                m_currentLLVMValue = builder->CreateAdd(L, R, "addtmp");
            }
            break;
        case vyb::TokenType::MINUS: // Reverted to vyb::TokenType::MINUS
            if (isFloatOp) m_currentLLVMValue = builder->CreateFSub(L, R, "fsubtmp");
            else if (L->getType()->isPointerTy() && R->getType()->isPointerTy()){
                 // Pointer subtraction (ptr - ptr) gives an integer distance
                 L = builder->CreatePtrToInt(L, int64Type, "ptrtointtmp_l");
                 R = builder->CreatePtrToInt(R, int64Type, "ptrtointtmp_r");
                 llvm::Value* diffBytes = builder->CreateSub(L, R, "subtmp");
                 m_currentLLVMValue = diffBytes;
            }
            else if (L->getType()->isPointerTy() && leftTypeNode) { // ptr - int
                 vyb::ast::TypeNode* pointeeAstType = nullptr;

                 // Try to get pointee type from different sources
                 if (auto ptrAstNode = dynamic_cast<vyb::ast::PointerType*>(leftTypeNode)) {
                    pointeeAstType = ptrAstNode->pointeeType.get();
                 } else if (auto arrayAstNode = dynamic_cast<vyb::ast::ArrayType*>(leftTypeNode)) {
                    pointeeAstType = arrayAstNode->elementType.get();
                 } else if (auto typeName = dynamic_cast<vyb::ast::TypeName*>(leftTypeNode)) {
                    // Check if it's a loc<T> type
                    if (typeName->identifier->name == "loc" && !typeName->genericArgs.empty()) {
                        pointeeAstType = typeName->genericArgs[0].get();
                    }
                 }

                 if (pointeeAstType) {
                    llvm::Type* pointeeType = codegenType(pointeeAstType);
                    if (pointeeType) {
                        m_currentLLVMValue = builder->CreateGEP(pointeeType, L, builder->CreateNeg(R), "ptrsubtmp");
                    } else {
                         logError(node->left->loc, "Could not determine LLVM pointee type for pointer subtraction from AST type: " + leftTypeNode->toString());
                         m_currentLLVMValue = nullptr;
                    }
                 } else {
                     // If we can't determine the pointee type from AST, use int64 as a fallback
                     // This is common in test cases with opaque pointers
                     if (verbose) {
                         logWarning(node->left->loc, "Pointer operand for subtraction lacks specific pointee type information. Using i64 as fallback pointee type.");
                     }
                     m_currentLLVMValue = builder->CreateGEP(int64Type, L, builder->CreateNeg(R), "ptrsubtmp_fallback");
                 }
            }
            else m_currentLLVMValue = builder->CreateSub(L, R, "subtmp");
            break;
        case vyb::TokenType::MULTIPLY: // Reverted to vyb::TokenType::MULTIPLY
            if (isFloatOp) m_currentLLVMValue = builder->CreateFMul(L, R, "fmultmp");
            else m_currentLLVMValue = builder->CreateMul(L, R, "multmp");
            break;
        case vyb::TokenType::DIVIDE: // Reverted to vyb::TokenType::DIVIDE
            if (isFloatOp) m_currentLLVMValue = builder->CreateFDiv(L, R, "fdivtmp");
            else m_currentLLVMValue = builder->CreateSDiv(L, R, "sdivtmp");
            break;
        case vyb::TokenType::MODULO: // Reverted to vyb::TokenType::MODULO
             if (isFloatOp) m_currentLLVMValue = builder->CreateFRem(L, R, "fremtmp");
             else m_currentLLVMValue = builder->CreateSRem(L, R, "sremtmp");
            break;
        // Comparison operators
        case vyb::TokenType::EQEQ: // Reverted to vyb::TokenType::EQEQ
            // Check for String comparison first
            if (isVybStringStructType(L->getType()) && isVybStringStructType(R->getType())) {
                m_currentLLVMValue = generateStringComparison(L, R, vyb::TokenType::EQEQ);
                break;
            }
            if (isFloatOp) m_currentLLVMValue = builder->CreateFCmpOEQ(L, R, "fcmpoeqtmp");
            else m_currentLLVMValue = builder->CreateICmpEQ(L, R, "icmpeqtmp");
            break;
        case vyb::TokenType::NOTEQ: // Reverted to vyb::TokenType::NOTEQ
            // Check for String comparison first
            if (isVybStringStructType(L->getType()) && isVybStringStructType(R->getType())) {
                m_currentLLVMValue = generateStringComparison(L, R, vyb::TokenType::NOTEQ);
                break;
            }
            if (isFloatOp) m_currentLLVMValue = builder->CreateFCmpONE(L, R, "fcmponeqtmp");
            else m_currentLLVMValue = builder->CreateICmpNE(L, R, "icmpneqtmp");
            break;
        case vyb::TokenType::LT: // Reverted to vyb::TokenType::LT
            // Check for String comparison first
            if (isVybStringStructType(L->getType()) && isVybStringStructType(R->getType())) {
                m_currentLLVMValue = generateStringComparison(L, R, vyb::TokenType::LT);
                break;
            }
            if (isFloatOp) m_currentLLVMValue = builder->CreateFCmpOLT(L, R, "fcmpltmp");
            else m_currentLLVMValue = builder->CreateICmpSLT(L, R, "icmpslttmp");
            break;
        case vyb::TokenType::LTEQ: // Reverted to vyb::TokenType::LTEQ:
            // Check for String comparison first
            if (isVybStringStructType(L->getType()) && isVybStringStructType(R->getType())) {
                m_currentLLVMValue = generateStringComparison(L, R, vyb::TokenType::LTEQ);
                break;
            }
            if (isFloatOp) m_currentLLVMValue = builder->CreateFCmpOLE(L, R, "fcmpletmp");
            else m_currentLLVMValue = builder->CreateICmpSLE(L, R, "icmpsletmp");
            break;
        case vyb::TokenType::GT: // Reverted to vyb::TokenType::GT
            // Check for String comparison first
            if (isVybStringStructType(L->getType()) && isVybStringStructType(R->getType())) {
                m_currentLLVMValue = generateStringComparison(L, R, vyb::TokenType::GT);
                break;
            }
            if (isFloatOp) m_currentLLVMValue = builder->CreateFCmpOGT(L, R, "fcmpgtmp");
            else m_currentLLVMValue = builder->CreateICmpSGT(L, R, "icmpsgttmp");
            break;
        case vyb::TokenType::GTEQ: // Reverted to vyb::TokenType::GTEQ:
            // Check for String comparison first
            if (isVybStringStructType(L->getType()) && isVybStringStructType(R->getType())) {
                m_currentLLVMValue = generateStringComparison(L, R, vyb::TokenType::GTEQ);
                break;
            }
            if (isFloatOp) m_currentLLVMValue = builder->CreateFCmpOGE(L, R, "fcmpgetmp");
            else m_currentLLVMValue = builder->CreateICmpSGE(L, R, "icmpsgetmp");
            break;
        // Bitwise operators (integer operands only)
        case vyb::TokenType::AMPERSAND:
            m_currentLLVMValue = builder->CreateAnd(L, R, "bwandtmp");
            break;
        case vyb::TokenType::PIPE:
            m_currentLLVMValue = builder->CreateOr(L, R, "bwortmp");
            break;
        case vyb::TokenType::CARET:
            m_currentLLVMValue = builder->CreateXor(L, R, "bwxortmp");
            break;
        case vyb::TokenType::LSHIFT:
            m_currentLLVMValue = builder->CreateShl(L, R, "shltmp");
            break;
        case vyb::TokenType::RSHIFT:
            {
                // Signed Int types use an arithmetic (sign-extending) shift;
                // UInt* types use a logical shift.
                bool isUnsigned = false;
                if (leftTypeNode) {
                    if (auto tn = dynamic_cast<ast::TypeName*>(leftTypeNode)) {
                        if (tn->identifier) {
                            const std::string& n = tn->identifier->name;
                            isUnsigned = (n == "UInt8" || n == "UInt16" || n == "UInt32" || n == "UInt64");
                        }
                    }
                }
                if (isUnsigned) m_currentLLVMValue = builder->CreateLShr(L, R, "lshrtmp");
                else m_currentLLVMValue = builder->CreateAShr(L, R, "ashrtmp");
            }
            break;
        // Logical operators (short-circuiting needs careful handling with basic blocks)
        // For simplicity, this example evaluates both sides. Proper logical ops need control flow.
        case vyb::TokenType::AND: // Reverted to vyb::TokenType::AND
             m_currentLLVMValue = builder->CreateAnd(L, R, "andtmp"); // Bitwise AND, assumes L and R are i1
            break;
        case vyb::TokenType::OR: // Reverted to vyb::TokenType::OR
            m_currentLLVMValue = builder->CreateOr(L, R, "ortmp"); // Bitwise OR, assumes L and R are i1
            break;
        default:
            logError(node->loc, "Unsupported binary operator.");
            m_currentLLVMValue = nullptr;
            break;
    }
}

} // namespace vyb
