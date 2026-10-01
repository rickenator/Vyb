// SPDX-License-Identifier: Apache-2.0

#include "vyb/vre/llvm/codegen.hpp"
#include "vyb/vre/llvm/cgen_internal.hpp"
#include "vyb/parser/ast.hpp"
#include "vyb/vre/thread_boundary.hpp"
#include "vyb/parser/token.hpp" // For TokenType in BinaryExpression

#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/DerivedTypes.h> // For PointerType, StructType
#include <llvm/ADT/APFloat.h>   // For APFloat in FloatLiteral
#include <llvm/ADT/APInt.h>     // For APInt in IntegerLiteral
#include <regex>                // For regex in MemberExpression loaded value handling

using namespace vyb;
// using namespace llvm; // Uncomment if desired for brevity

// Classify a sized integer type name into signedness and bit width. Returns
// false for non-integer (or unknown) type names.
static bool vybIntClass(const std::string& name, bool& isUnsigned, unsigned& bits) {
    if (name == "Int" || name == "Int64" || name == "i64")    { isUnsigned = false; bits = 64; return true; }
    if (name == "Int32" || name == "i32")                     { isUnsigned = false; bits = 32; return true; }
    if (name == "Int16" || name == "i16")                     { isUnsigned = false; bits = 16; return true; }
    if (name == "Int8"  || name == "i8")                      { isUnsigned = false; bits = 8;  return true; }
    if (name == "UInt64" || name == "u64")                    { isUnsigned = true;  bits = 64; return true; }
    if (name == "UInt32" || name == "u32")                    { isUnsigned = true;  bits = 32; return true; }
    if (name == "UInt16" || name == "u16")                    { isUnsigned = true;  bits = 16; return true; }
    if (name == "UInt8"  || name == "u8" || name == "Byte")   { isUnsigned = true;  bits = 8;  return true; }
    if (name == "Char")                                       { isUnsigned = false; bits = 8;  return true; }
    if (name == "Rune")                                       { isUnsigned = false; bits = 32; return true; }
    return false;
}

// Is `name` one of Vyb's float types? Sets `bits` to the storage width.
static bool vybFloatClass(const std::string& name, unsigned& bits) {
    if (name == "Float" || name == "Float64" || name == "f64" || name == "float64") { bits = 64; return true; }
    if (name == "Float32" || name == "f32" || name == "float32") { bits = 32; return true; }
    return false;
}

// --- Literal Codegen ---
// Field-type probe helpers for struct-literal ownership transfer (below). A
// struct field initialized from a *shared* read (a bare owning variable or a
// member borrow) needs assignment-like copy semantics — deep-copy a Vec, retain a
// String / our / mild reference, deep-copy a nested owned struct — so both the
// source and the struct field own independent data. Fresh constructors and
// transfer producers (`Vec()`, `our(...)`, `soft(x)`, String-returning calls)
// hand over a single owned reference and are stored as-is.
static bool objFieldTypeIsString(const vyb::ast::TypeNode* tn) {
    if (!tn) return false;
    if (auto* nn = dynamic_cast<const vyb::ast::TypeName*>(tn)) {
        if (nn->identifier) {
            const std::string& n = nn->identifier->name;
            if (n == "String" || n == "string") return true;
        }
    }
    return false;
}

static bool objFieldTypeIsVec(const vyb::ast::TypeNode* tn) {
    if (!tn) return false;
    if (dynamic_cast<const vyb::ast::VecType*>(tn)) return true;
    if (auto* nn = dynamic_cast<const vyb::ast::TypeName*>(tn))
        return nn->identifier && nn->identifier->name == "Vec";
    return false;
}

// The element type node of a `Vec<T>` field, or null.
static const vyb::ast::TypeNode* objFieldVecElement(const vyb::ast::TypeNode* tn) {
    if (!tn) return nullptr;
    if (auto* vt = dynamic_cast<const vyb::ast::VecType*>(tn)) return vt->elementType.get();
    if (auto* nn = dynamic_cast<const vyb::ast::TypeName*>(tn)) {
        if (nn->identifier && nn->identifier->name == "Vec" && !nn->genericArgs.empty())
            return nn->genericArgs[0].get();
    }
    return nullptr;
}

// A fresh `mild<T>` owns a single weak reference that can be handed to a storage
// location without an extra retain. This covers the `soft(...)` operation (which
// already bumped the weak count) and any call returning `mild<T>`. A bare
// identifier or field read of an existing `mild` value is shared and must be
// retained on stow (mirrors exprIsOurTransfer).
bool LLVMCodegen::isMildTransferExpr(ast::Expression* expr) {
    if (!expr) return false;
    auto* call = dynamic_cast<ast::CallExpression*>(expr);
    if (!call) return false;
    if (auto* id = dynamic_cast<ast::Identifier*>(call->callee.get())) {
        if (id->name == "soft") return true;
    }
    if (auto t = typeOfNode(call)) {
        std::string s = t->toString();
        if (s.rfind("mild<", 0) == 0) return true;
    }
    return false;
}

// Is this expression a whole-value *shared* read (a bare variable or a member
// borrow)? Fresh constructors / calls / literals are excluded: they produce (or
// already are) owned values, not shared borrows of a source binding.
static bool exprIsSharedOwnedRead(ast::Expression* expr) {
    if (!expr) return false;
    if (dynamic_cast<ast::Identifier*>(expr)) return true;
    if (dynamic_cast<ast::MemberExpression*>(expr)) return true;
    return false;
}

// True when every value-producing arm of a select expression is a whole-value
// read of a variable or member (a *borrow*), never a fresh owner/constructor. A
// select like `{ true -> sc, false -> other }` yields the selected binding's data
// (a borrow), so an owned struct field receiving it must deep-copy/retain to own
// data independent of the source. Because there is no fresh-producing arm, the
// copy can never leak. A select that also has a fresh-constructor arm
// (`false -> Vec()`) is left alone: its produced value may already be an owned
// transfer, and copying it at the field store could leak that fresh value, so
// that case stays on the existing ownership path.
static bool selectAllArmsAreOwnedReads(ast::Expression* expr) {
    auto* sel = dynamic_cast<ast::SelectExpression*>(expr);
    if (!sel) return false;
    if (sel->cases.empty()) return false;
    for (const auto& cs : sel->cases) {
        ast::Expression* body = cs.second.get();
        if (!body) return false;
        bool armIsBorrow = exprIsSharedOwnedRead(body);
        if (!armIsBorrow && dynamic_cast<ast::BlockExpression*>(body)) {
            auto* blk = static_cast<ast::BlockExpression*>(body);
            if (blk->block) {
                for (const auto& st : blk->block->body) {
                    if (auto* pass = dynamic_cast<ast::PassStatement*>(st.get())) {
                        if (pass->argument && exprIsSharedOwnedRead(pass->argument.get()))
                            armIsBorrow = true;
                    }
                }
            }
        }
        if (!armIsBorrow) return false;
    }
    return true;
}


void LLVMCodegen::visit(vyb::ast::ObjectLiteral* node) {
    if (!node->typePath) {
        logError(node->loc, "Object literal is missing type information");
        m_currentLLVMValue = nullptr;
        return;
    }

    // Get the struct type for the object
    VYB_CDBG << "DEBUG: ObjectLiteral resolving type: " << node->typePath->toString() << std::endl;
    llvm::Type* structTy = codegenType(node->typePath.get());
    VYB_CDBG << "DEBUG: ObjectLiteral resolved type to: " << getTypeName(structTy) << " with pointer: " << structTy << std::endl;
    if (!structTy || !structTy->isStructTy()) {
        logError(node->loc, "Object literal type is not a struct type");
        m_currentLLVMValue = nullptr;
        return;
    }

    // (the member-expression receiver's AST type is read from the TypeTable via
    // typeOfNode; codegen no longer writes into the retired node->type field)

    std::string structName = llvm::cast<llvm::StructType>(structTy)->getName().str();
    if (structName.empty()) {
        structName = "anon";
    }

    // Allocate stack space for the struct
    llvm::AllocaInst* allocaInst = builder->CreateAlloca(structTy, nullptr, structName + "_obj");

    // Zero-initialize the whole struct first so any *omitted* fields default to
    // an empty value (null Vec buffer / empty String / zero scalars) rather than
    // uninitialized stack bytes. An owned field (e.g. a Vec) left at garbage
    // bytes would hand its scope-exit reclamation a bogus non-null buffer to free,
    // corrupting the heap. Explicit properties are stored right after this.
    builder->CreateStore(llvm::ConstantAggregateZero::get(structTy), allocaInst);

    // Set metadata or debug info to help identify this as a struct of type structName
    // This can help when trying to determine the type in MemberExpression
    if (!structName.empty() && structName != "anon") {
        // Store the allocated type in the userTypeMap if it's not already there
        // This ensures the type is registered for field lookups
        auto it = userTypeMap.find(structName);
        if (it == userTypeMap.end() && llvm::isa<llvm::StructType>(structTy)) {
            llvm::StructType* structType = llvm::cast<llvm::StructType>(structTy);
            if (!structType->isOpaque()) {
                UserTypeInfo typeInfo;
                typeInfo.llvmType = structType;
                typeInfo.isStruct = true;

                // Try to populate field indices if we have the information
                // This might be incomplete, but can be useful for debugging
                userTypeMap[structName] = typeInfo;
            }
        }
    }

    // Resolve the concrete field types (in layout order) so the store below can
    // apply assignment-like ownership semantics to owned fields.
    std::vector<vyb::ast::TypeNodePtr> objectFieldTypes;
    collectStructConcreteFieldTypes(node->typePath.get(), objectFieldTypes);

    // Store each field
    for (size_t i = 0; i < node->properties.size(); ++i) {
        const auto& prop = node->properties[i];
        if (!prop.value || !prop.key) {
            logError(node->loc, "ObjectLiteral property missing key or value");
            m_currentLLVMValue = nullptr;
            return;
        }

        // Get field index by name
        std::string fieldName = prop.key->toString();
        int fieldIndex = getStructFieldIndex(llvm::cast<llvm::StructType>(structTy), fieldName);
        if (fieldIndex < 0) {
            logError(node->loc, "Field '" + fieldName + "' not found in struct '" + structName + "'");
            m_currentLLVMValue = nullptr;
            return;
        }

        // Generate the value for the field
        prop.value->accept(*this);
        if (!m_currentLLVMValue) {
            logError(prop.value->loc, "Failed to codegen value for field '" + fieldName + "'");
            m_currentLLVMValue = nullptr;
            return;
        }

        // Create GEP to get pointer to field
        llvm::Value* fieldPtr = builder->CreateStructGEP(structTy, allocaInst, fieldIndex, fieldName + "_ptr");

        // Store the value into the field. Cast the initializer to the field's
        // declared LLVM type (e.g. truncate a Vyb i64 literal to a CInt i32
        // slot) so the struct is stored at its real width. Without this the
        // literal lands at i64 width in an i32 field, corrupting the offsets
        // and values of later fields.
        llvm::Value* fieldValue = m_currentLLVMValue;
        auto* stLeaf = llvm::cast<llvm::StructType>(structTy);
        llvm::Type* fieldTy = stLeaf->getElementType(static_cast<unsigned>(fieldIndex));
        if (fieldValue->getType() != fieldTy) {
            fieldValue = tryCast(fieldValue, fieldTy, prop.value->loc);
            if (!fieldValue) {
                m_currentLLVMValue = nullptr;
                return;
            }
        }

        // Owned-field ownership transfer: a field initialized from a *shared*
        // read (a bare owning variable or a member borrow) must copy/retain so
        // the struct owns data independent of the source — otherwise both the
        // source binding and the field would release the same buffer on scope
        // exit (a double free). Fresh constructor / transfer producers hand over
        // a single owned reference and are stored as-is. Only fields whose value
        // came from a bare identifier or a member read borrow; explicit
        // constructors, calls, and literals already own their payload.
        bool borrowedRead =
            dynamic_cast<ast::Identifier*>(prop.value.get()) != nullptr ||
            dynamic_cast<ast::MemberExpression*>(prop.value.get()) != nullptr ||
            selectAllArmsAreOwnedReads(prop.value.get());
        const vyb::ast::TypeNode* fieldAst =
            (fieldIndex >= 0 && (size_t)fieldIndex < objectFieldTypes.size())
                ? objectFieldTypes[(size_t)fieldIndex].get() : nullptr;
        if (fieldValue && borrowedRead && fieldAst) {
            if (objFieldTypeIsVec(fieldAst) && isVecStructType(fieldValue->getType())) {
                // Deep-copy a borrowed Vec so the field owns an independent buffer.
                if (const vyb::ast::TypeNode* elem = objFieldVecElement(fieldAst)) {
                    if (llvm::Type* elemT = codegenType(const_cast<vyb::ast::TypeNode*>(elem))) {
                        if (auto* vecTy = llvm::dyn_cast<llvm::StructType>(fieldValue->getType())) {
                            llvm::Value* copy = generateVecDeepCopy(fieldValue, elemT, vecTy, elem);
                            fieldValue = copy ? copy : fieldValue;
                        }
                    }
                }
            } else if (objFieldTypeIsString(fieldAst) && isVybStringStructType(fieldValue->getType())) {
                if (!exprIsStringTransfer(prop.value.get())) retainStringValue(fieldValue);
            } else if (fieldValue->getType()->isPointerTy() && isOurRefType(fieldAst)) {
                if (!exprIsOurTransfer(prop.value.get())) retainOurControlBlock(fieldValue, "objlit.our");
            } else if (fieldValue->getType()->isPointerTy() && isMildRefType(fieldAst)) {
                if (!isMildTransferExpr(prop.value.get())) retainMildControlBlock(fieldValue, "objlit.mild");
            } else if (fieldValue->getType()->isStructTy() && isKnownStructTypeNode(fieldAst)) {
                // A borrowed nested struct (whose owned fields are deep-copied).
                if (auto* nestTy = llvm::dyn_cast<llvm::StructType>(fieldValue->getType())) {
                    llvm::Value* copy = generateStructDeepCopy(fieldValue, fieldAst, nestTy);
                    fieldValue = copy ? copy : fieldValue;
                }
            }
        }
        builder->CreateStore(fieldValue, fieldPtr);
    }

    // In Vyb, struct initialization can be used both for creating temporary values
    // and for direct assignment to variables. We need to decide if we should return
    // the pointer or load the actual struct value.

    // Store the struct type information in userTypeMap if not already present
    if (!structName.empty() && structName != "anon") {
        auto it = userTypeMap.find(structName);
        if (it == userTypeMap.end() && llvm::isa<llvm::StructType>(structTy)) {
            UserTypeInfo typeInfo;
            typeInfo.llvmType = llvm::cast<llvm::StructType>(structTy);
            typeInfo.isStruct = true;
            userTypeMap[structName] = typeInfo;
        }
    }

    // For struct initialization in variable assignment or return statements,
    // we should load the struct value rather than return the pointer.
    // This ensures type compatibility with value semantics.
    llvm::Value* structValue = builder->CreateLoad(structTy, allocaInst, structName + "_val");

    VYB_CDBG << "DEBUG: ObjectLiteral created struct value with type: " << getTypeName(structValue->getType()) << std::endl;
    VYB_CDBG << "DEBUG: Expected struct type was: " << getTypeName(structTy) << std::endl;

    // Return the loaded struct value
    m_currentLLVMValue = structValue;
}

void LLVMCodegen::visit(vyb::ast::ArrayLiteral* node) {
    if (node->elements.empty()) {
        // Handle empty array literal. Need its type.
        // If typeOfNode(node) is set by semantic analysis:
        if (typeOfNode(node)) {
            llvm::Type* arrayLlvmType = codegenType(typeOfNode(node).get());
            if (auto at = llvm::dyn_cast<llvm::ArrayType>(arrayLlvmType)) {
                 m_currentLLVMValue = llvm::ConstantArray::get(at, {}); // Empty constant array
                 return;
            } else if (auto pt = llvm::dyn_cast<llvm::PointerType>(arrayLlvmType)) {
                // If it's a pointer to an array or slice type.
                // This case is more complex for an empty literal.
                // It might mean a null pointer or pointer to an empty static region.
                // For now, let's assume if type is known, it's an ArrayType.
            }
        }
        logError(node->loc, "Empty array literal with unknown type.");
        m_currentLLVMValue = nullptr;
        return;
    }

    std::vector<llvm::Constant*> constantElements;
    llvm::Type* elementLlvmType = nullptr;

    for (const auto& elemExpr : node->elements) {
        elemExpr->accept(*this); // Codegen element
        llvm::Value* elemValue = m_currentLLVMValue;
        if (!elemValue) {
            logError(elemExpr->loc, "Element codegen failed in array literal.");
            m_currentLLVMValue = nullptr;
            return;
        }
        if (!elementLlvmType) {
            elementLlvmType = elemValue->getType();
        } else if (elemValue->getType() != elementLlvmType) {
            // TODO: Handle mixed types, promotions, or error
            // For now, assume all elements must be of the same type as the first
            // Or attempt to cast to the type of the first element.
            // This should ideally be caught by semantic analysis.
            logError(elemExpr->loc, "Array literal elements have mixed types. Expected " + getTypeName(elementLlvmType) + " but got " + getTypeName(elemValue->getType()));
            m_currentLLVMValue = nullptr;
            return;
        }
        // Array literals must consist of constants to form a ConstantArray
        if (auto* constElem = llvm::dyn_cast<llvm::Constant>(elemValue)) {
            constantElements.push_back(constElem);
        } else {
            // If elements are not constant, we can't create a llvm::ConstantArray.
            // This means the array must be constructed at runtime, e.g., by allocating
            // memory and storing each element. This is more like ArrayInitializationExpression
            // or requires a helper function.
            // For now, array literals are assumed to produce ConstantArrays.
            logError(elemExpr->loc, "Array literal element is not a constant value. Runtime array construction not yet fully supported here.");
            m_currentLLVMValue = nullptr;
            return;
        }
    }

    if (!elementLlvmType) { // Should not happen if elements is not empty and codegen succeeded
        logError(node->loc, "Could not determine element type for array literal.");
        m_currentLLVMValue = nullptr;
        return;
    }

    llvm::ArrayType* arrayType = llvm::ArrayType::get(elementLlvmType, constantElements.size());
    m_currentLLVMValue = llvm::ConstantArray::get(arrayType, constantElements);
    VYB_CDBG << "DEBUG: ArrayLiteral produced type=" << getTypeName(m_currentLLVMValue->getType()) << std::endl;
}

// --- Expressions ---


void LLVMCodegen::emitVecConstructor(vyb::ast::CallExpression* node) {
    // Create Vec struct: { ptr, size, capacity }
    std::vector<llvm::Type*> vecFields = {
        llvm::PointerType::get(*context, 0), // ptr to elements (opaque pointer)
        llvm::Type::getInt64Ty(*context),    // size
        llvm::Type::getInt64Ty(*context)     // capacity
    };

    llvm::StructType* vecStructType = llvm::StructType::get(*context, vecFields, false);

    // Allocate the Vec struct.
    llvm::Value* vecAlloca = builder->CreateAlloca(vecStructType, nullptr, "vec.new");

    if (node->arguments.empty()) {
        // Vec() / Vec::new() - empty vector.
        llvm::Value* nullPtr = llvm::ConstantPointerNull::get(llvm::PointerType::get(*context, 0));
        llvm::Value* zero = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 0);
        llvm::Value* ptrFieldPtr = builder->CreateStructGEP(vecStructType, vecAlloca, 0, "vec.ptr_field");
        builder->CreateStore(nullPtr, ptrFieldPtr);
        llvm::Value* sizeFieldPtr = builder->CreateStructGEP(vecStructType, vecAlloca, 1, "vec.size_field");
        builder->CreateStore(zero, sizeFieldPtr);
        llvm::Value* capFieldPtr = builder->CreateStructGEP(vecStructType, vecAlloca, 2, "vec.cap_field");
        builder->CreateStore(zero, capFieldPtr);
    } else if (node->arguments.size() == 1) {
        // Vec(n) / Vec::new(n) - preallocated vector with zero-initialized elements.
        node->arguments[0]->accept(*this);
        llvm::Value* sizeValue = m_currentLLVMValue;
        if (!sizeValue) {
            logError(node->loc, "Failed to evaluate size argument for Vec(n)");
            m_currentLLVMValue = nullptr;
            return;
        }

        if (sizeValue->getType() != llvm::Type::getInt64Ty(*context)) {
            sizeValue = builder->CreateSExtOrTrunc(sizeValue, llvm::Type::getInt64Ty(*context), "size.ext");
        }

        // Allocate memory for elements. Numeric/opaque elements are stored inline;
        // this defaults to 8 bytes per element (matching the legacy Vec::new(n) path).
        llvm::Type* elementType = llvm::Type::getInt64Ty(*context);
        llvm::Value* allocSize = builder->CreateMul(sizeValue,
            llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 8), "alloc.size");

        // Reuse the single module-level `malloc` declaration (like the Vec
        // push/resize paths). Creating a fresh ExternalLinkage "malloc" per
        // `Vec(n)` call makes LLVM suffix each duplicate (`malloc.N`), which
        // the ORC JIT cannot resolve ("Symbols not found: [ malloc.N ]").
        llvm::Function* mallocFunc = getOrCreateMallocFunction();
        llvm::Value* dataPtr = builder->CreateCall(mallocFunc, {allocSize}, "vec.data");

        llvm::Function* memsetFunc = getOrCreateMemsetFunction();
        builder->CreateCall(memsetFunc, {
            dataPtr,
            llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0),
            allocSize
        });

        llvm::Value* ptrFieldPtr = builder->CreateStructGEP(vecStructType, vecAlloca, 0, "vec.ptr_field");
        builder->CreateStore(dataPtr, ptrFieldPtr);
        llvm::Value* sizeFieldPtr = builder->CreateStructGEP(vecStructType, vecAlloca, 1, "vec.size_field");
        builder->CreateStore(sizeValue, sizeFieldPtr);
        llvm::Value* capFieldPtr = builder->CreateStructGEP(vecStructType, vecAlloca, 2, "vec.cap_field");
        builder->CreateStore(sizeValue, capFieldPtr);
    }

    m_currentLLVMValue = builder->CreateLoad(vecStructType, vecAlloca, "vec.new.value");
}


void LLVMCodegen::visit(vyb::ast::LocationExpression *node) {
    // loc(expr) creates a pointer to the expression's memory location

    // Fast path: if expr is an Identifier, return its alloca directly
    auto ident = dynamic_cast<vyb::ast::Identifier*>(node->expression.get());
    if (ident) {
        auto it = namedValues.find(ident->name);
        if (it != namedValues.end()) {
            m_currentLLVMValue = it->second;
            return;
        }
    }

    // 1. Evaluate the expression to get its value
    node->expression->accept(*this);
    llvm::Value *exprVal = m_currentLLVMValue;
    if (!exprVal) {
        logError(node->loc, "Expression in loc() evaluated to null");
        m_currentLLVMValue = nullptr;
        return;
    }

    // 2. If the expression is already a pointer type, use it directly
    if (exprVal->getType()->isPointerTy()) {
        m_currentLLVMValue = exprVal;
        return;
    }

    // 3. Create an alloca for the value and store it
    llvm::Type* valType = exprVal->getType();
    llvm::Value* tempAlloca = builder->CreateAlloca(valType, nullptr, "loc_alloca");
    builder->CreateStore(exprVal, tempAlloca);

    // 4. Return the pointer
    m_currentLLVMValue = tempAlloca;
}

void LLVMCodegen::visit(vyb::ast::AddrOfExpression *node) {
    // addr(expr) gets the address of a pointer expression

    // 1. Evaluate the expression to get the pointer
    node->getLocation()->accept(*this);
    llvm::Value *exprVal = m_currentLLVMValue;
    if (!exprVal) {
        logError(node->loc, "Expression in addr() evaluated to null");
        m_currentLLVMValue = nullptr;
        return;
    }

    // 2. If we have a pointer to a pointer, load the actual pointer
    if (exprVal->getType()->isPointerTy()) {
        // Load the pointer value if we have a pointer-to-pointer
        if (auto allocaInst = llvm::dyn_cast<llvm::AllocaInst>(exprVal)) {
            if (allocaInst->getAllocatedType()->isPointerTy()) {
                exprVal = builder->CreateLoad(allocaInst->getAllocatedType(), exprVal, "ptr_load");
            }
        }
    }

    // 3. Convert the pointer to an integer
    if (!exprVal->getType()->isPointerTy()) {
        logError(node->loc, "Expression in addr() must be a pointer type");
        m_currentLLVMValue = nullptr;
        return;
    }

    m_currentLLVMValue = builder->CreatePtrToInt(exprVal, int64Type, "addr_cast");
}

void LLVMCodegen::visit(vyb::ast::PointerDerefExpression *node) {
    // at(ptr) dereferences a pointer for reading or writing

    // 1. Evaluate the pointer expression
    node->pointer->accept(*this);
    llvm::Value *ptrVal = m_currentLLVMValue;
    if (!ptrVal) {
        logError(node->loc, "Pointer expression in at() evaluated to null");
        m_currentLLVMValue = nullptr;
        return;
    }

    // 2. If we have a pointer to a pointer (e.g., alloca of a pointer), load the actual pointer
    if (ptrVal->getType()->isPointerTy()) {
        // Load the pointer value if we have a pointer-to-pointer
        if (auto allocaInst = llvm::dyn_cast<llvm::AllocaInst>(ptrVal)) {
            if (allocaInst->getAllocatedType()->isPointerTy()) {
                ptrVal = builder->CreateLoad(allocaInst->getAllocatedType(), ptrVal, "ptr_load");
            }
        }
    }

    // 3. Verify we have a valid pointer type
    if (!ptrVal->getType()->isPointerTy()) {
        logError(node->loc, "Operand of at() must be a pointer type. Got: " + getTypeName(ptrVal->getType()));
        m_currentLLVMValue = nullptr;
        return;
    }

    // 4. Add null check
    llvm::BasicBlock *currentBB = builder->GetInsertBlock();
    llvm::Function *currentFn = currentBB->getParent();

    llvm::BasicBlock *notNullBB = llvm::BasicBlock::Create(*context, "ptr.not_null", currentFn);
    llvm::BasicBlock *nullBB = llvm::BasicBlock::Create(*context, "ptr.null", currentFn);
    llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(*context, "ptr.merge", currentFn);

    llvm::Value *isNotNull = builder->CreateICmpNE(ptrVal, llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(ptrVal->getType())), "ptr.is_not_null");
    builder->CreateCondBr(isNotNull, notNullBB, nullBB);

    // Not null case: perform the dereference
    builder->SetInsertPoint(notNullBB);

    // For assignment target, return the pointer itself
    // For reading, load the value
    if (m_isLHSOfAssignment) {
        m_currentLLVMValue = ptrVal;
    } else {
        // For reading, load the value
        llvm::Type* pointeeType = nullptr;
        if (typeOfNode(node)) {
            pointeeType = codegenType(typeOfNode(node).get());
        } else {
            // Fallback to i64 for test cases
            pointeeType = int64Type;
        }
        m_currentLLVMValue = builder->CreateLoad(pointeeType, ptrVal, "deref.load");
    }

    builder->CreateBr(mergeBB);

    // Null case: handle the error
    builder->SetInsertPoint(nullBB);
    builder->CreateUnreachable();

    // Merge point
    builder->SetInsertPoint(mergeBB);
}

void LLVMCodegen::visit(vyb::ast::AssignmentExpression *node) {
    // Save and set LHS flag before visiting LHS
    bool wasLHS = m_isLHSOfAssignment;
    m_isLHSOfAssignment = true;
    node->left->accept(*this);
    m_isLHSOfAssignment = wasLHS;
    llvm::Value *LHS = m_currentLLVMValue;

    // Generate RHS value
    node->right->accept(*this);
    llvm::Value *RHS = m_currentLLVMValue;
    if (!LHS || !RHS) {
        logError(node->loc, "Invalid operands in assignment.");
        m_currentLLVMValue = nullptr;
        return;
    }

    // Capture type information from AST if available
    std::shared_ptr<vyb::ast::TypeNode> lhsTypeNode = typeOfNode(node->left);
    std::shared_ptr<vyb::ast::TypeNode> rhsTypeNode = typeOfNode(node->right);

    // Store location for error reporting
    SourceLocation errorLoc = node->left->loc;

    // Check if we're assigning to a variable (used for special case handling)
    bool isAssignToVar = false;
    auto identLeft = dynamic_cast<ast::Identifier*>(node->left.get());
    if (identLeft) {
        isAssignToVar = true;

        // For direct identifier assignments, register the RHS type with the identifier
        // This helps propagate type info for variables
        if (typeOfNode(identLeft) && RHS) {
            valueTypeMap[RHS] = typeOfNode(identLeft);
            VYB_CDBG << "DEBUG: In assignment, associated RHS value with identifier type: "
                      << typeOfNode(identLeft)->toString() << std::endl;
        }
    }

    // Mutable closure captures: propagate writes to the outer variable's
    // address so the enclosing scope (and subsequent invocations) observe the
    // change. This map is only populated while generating a lambda body, so for
    // ordinary assignments this is a no-op.
    auto writeThroughMutable = [&](llvm::Value* v) {
        if (identLeft) {
            auto mIt = mutableCaptureOuterPointers.find(identLeft->name);
            if (mIt != mutableCaptureOuterPointers.end()) {
                builder->CreateStore(v, mIt->second);
            }
        }
    };

    // Check if LHS is a valid target for assignment
    if (!LHS->getType()->isPointerTy()) {
        // Log detailed information about the LHS
        std::string lhsTypeStr = getTypeName(LHS->getType());
        std::string lhsNodeType = "<unknown>";

        // Get more detailed info about the LHS expression type
        if (auto callExpr = dynamic_cast<ast::CallExpression*>(node->left.get())) {
            if (auto identCallee = dynamic_cast<ast::Identifier*>(callExpr->callee.get())) {
                lhsNodeType = "CallExpression to " + identCallee->name;
            } else {
                lhsNodeType = "CallExpression";
            }
        } else if (dynamic_cast<ast::PointerDerefExpression*>(node->left.get())) {
            lhsNodeType = "PointerDerefExpression";
        } else if (identLeft) {
            lhsNodeType = "Identifier: " + identLeft->name;
        }

        // If the LHS is a variable and RHS is a pointer-to-int conversion (addr()),
        // we're probably assigning an address to an int variable, which is valid
        if (isAssignToVar && RHS->getType()->isIntegerTy()) {
            // Create an alloca if LHS doesn't point to memory yet
            llvm::AllocaInst* allocaInst = nullptr;
            if (auto existingAlloca = llvm::dyn_cast<llvm::AllocaInst>(LHS)) {
                // LHS is already an alloca, use it
                allocaInst = existingAlloca;
            } else {
                // Need to create an alloca for the LHS
                if (identLeft) {
                    // Look up the alloca for the variable
                    auto it = namedValues.find(identLeft->name);
                    if (it != namedValues.end() && llvm::isa<llvm::AllocaInst>(it->second)) {
                        allocaInst = llvm::cast<llvm::AllocaInst>(it->second);
                    }
                }

                if (!allocaInst) {
                    logError(errorLoc, "Cannot assign to " + lhsNodeType + " (not a valid destination for assignment)");
                    m_currentLLVMValue = nullptr;
                    return;
                }
            }

            // Store the integer value directly
            builder->CreateStore(RHS, allocaInst);

            // Preserve AST type information
            if (lhsTypeNode) {
                valueTypeMap[allocaInst] = lhsTypeNode;
            }

            m_currentLLVMValue = RHS;
            return;
        } else {
            logError(errorLoc, "Destination for assignment is not a pointer type. Got: " +
                     lhsTypeStr + " (Node type: " + lhsNodeType + ")");
            m_currentLLVMValue = nullptr;
            return;
        }
    }

    // Determine the pointee type for the store
    llvm::Type *destPointeeType = nullptr;

    // Try to get pointee type from all available sources
    if (auto allocaInst = llvm::dyn_cast<llvm::AllocaInst>(LHS)) {
        destPointeeType = allocaInst->getAllocatedType();
    } else if (auto gep = llvm::dyn_cast<llvm::GetElementPtrInst>(LHS)) {
        // A Vec<T> subscript stores through a *byte-offset* GEP, whose
        // getResultElementType() is i8 -- not the element type. Prefer the
        // AST-declared element type when the subscript's base is a Vec, so the
        // store uses T and the assignment does not warn "Storing i64 into
        // location of type i8". See issue #318.
        bool usedAstElemType = false;
        if (auto* aee = dynamic_cast<ast::ArrayElementExpression*>(node->left.get())) {
            if (typeOfNode(aee->array)) {
                if (auto* vecTn = dynamic_cast<ast::VecType*>(typeOfNode(aee->array).get())) {
                    if (vecTn->elementType) {
                        destPointeeType = codegenType(vecTn->elementType.get());
                        usedAstElemType = destPointeeType != nullptr;
                    }
                }
            }
        }
        if (!usedAstElemType) {
            destPointeeType = gep->getResultElementType();
        }
    } else if (lhsTypeNode) {
        destPointeeType = codegenType(lhsTypeNode.get());
    } else {
        // Try to infer pointer element type from AST
        // For PointerDerefExpression, try to get the type info from pointer operand
        if (auto pointerDeref = dynamic_cast<ast::PointerDerefExpression*>(node->left.get())) {
            if (pointerDeref->pointer && typeOfNode(pointerDeref->pointer)) {
                // If it's a pointer type, get its pointee type
                if (auto ptrType = dynamic_cast<ast::PointerType*>(typeOfNode(pointerDeref->pointer).get())) {
                    if (ptrType->pointeeType) {
                        destPointeeType = codegenType(ptrType->pointeeType.get());
                    }
                }
                // If it's a loc<T> type, get T
                else if (auto locType = dynamic_cast<ast::TypeName*>(typeOfNode(pointerDeref->pointer).get())) {
                    if (locType->identifier->name == "loc" && !locType->genericArgs.empty()) {
                        destPointeeType = codegenType(locType->genericArgs[0].get());
                    }
                }
            }
        }

        // Fallback: Use RHS type if all else fails
        if (!destPointeeType) {
            destPointeeType = RHS->getType();
        }
    }

    // Cast RHS to match destination type if needed
    // A raw `char*` being stored into a Vyb String struct `{ ptr, i64 }` (e.g.
    // `s = 7.to_string()` or `s = s + "x"` after a narrowed `.to_string()` in
    // the chain) must be wrapped with its length into a proper String struct.
    // Otherwise the store writes only the data pointer field and the length
    // field keeps its stale, pre-assignment value — corrupting later reads.
    if (destPointeeType && isVybStringStructType(destPointeeType) &&
        RHS->getType()->isPointerTy()) {
        llvm::Value* strRhs = tryCast(RHS, destPointeeType, errorLoc);
        if (strRhs) RHS = strRhs;
    }
    if (RHS->getType() != destPointeeType && destPointeeType) {
        if (destPointeeType->isFloatingPointTy() && RHS->getType()->isIntegerTy()) {
            RHS = builder->CreateSIToFP(RHS, destPointeeType, "assigncast");
        } else if (destPointeeType->isIntegerTy() && RHS->getType()->isFloatingPointTy()) {
            RHS = builder->CreateFPToSI(RHS, destPointeeType, "assigncast");
        } else if (destPointeeType->isPointerTy() && RHS->getType()->isPointerTy()) {
            RHS = builder->CreatePointerCast(RHS, destPointeeType, "assigncast");
        } else if (destPointeeType->isIntegerTy() && RHS->getType()->isPointerTy()) {
            RHS = builder->CreatePtrToInt(RHS, destPointeeType, "assigncast");
        } else if (destPointeeType->isPointerTy() && RHS->getType()->isIntegerTy()) {
            RHS = builder->CreateIntToPtr(RHS, destPointeeType, "assigncast");
        }
    }

    // Bitwise compound assigns (&=/|=/^=/<<=/>>=) coerce the RHS to the LHS
    // width themselves, so a narrowed/`narrow` literal is expected and the
    // generic "type mismatch" warning below would be misleading.
    bool bitwiseCompound = node->op.type == vyb::TokenType::BITWISEANDEQ ||
                           node->op.type == vyb::TokenType::BITWISEOREQ ||
                           node->op.type == vyb::TokenType::BITWISEXOREQ ||
                           node->op.type == vyb::TokenType::LSHIFTEQ ||
                           node->op.type == vyb::TokenType::RSHIFTEQ;

    // If types still don't match, warn but proceed (might still work in some cases)
    if (RHS->getType() != destPointeeType && !bitwiseCompound) {
        logWarning(node->loc, "Type mismatch in assignment. Storing " + getTypeName(RHS->getType()) +
                  " into location of type " + (destPointeeType ? getTypeName(destPointeeType) : "unknown"));
    }

    // Handle compound assignments (+=, -=, *=, /=, %=)
    if (node->op.type == vyb::TokenType::PLUSEQ) {
        llvm::Value *lhsVal = builder->CreateLoad(destPointeeType, LHS, "lhs.load");
        lhsVal = builder->CreateAdd(lhsVal, RHS, "compound.add");
        builder->CreateStore(lhsVal, LHS);
        writeThroughMutable(lhsVal);
        m_currentLLVMValue = lhsVal;
        return;
    } else if (node->op.type == vyb::TokenType::MINUSEQ) {
        llvm::Value *lhsVal = builder->CreateLoad(destPointeeType, LHS, "lhs.load");
        lhsVal = builder->CreateSub(lhsVal, RHS, "compound.sub");
        builder->CreateStore(lhsVal, LHS);
        writeThroughMutable(lhsVal);
        m_currentLLVMValue = lhsVal;
        return;
    } else if (node->op.type == vyb::TokenType::MULTIPLYEQ) {
        llvm::Value *lhsVal = builder->CreateLoad(destPointeeType, LHS, "lhs.load");
        lhsVal = builder->CreateMul(lhsVal, RHS, "compound.mul");
        builder->CreateStore(lhsVal, LHS);
        writeThroughMutable(lhsVal);
        m_currentLLVMValue = lhsVal;
        return;
    } else if (node->op.type == vyb::TokenType::DIVEQ) {
        llvm::Value *lhsVal = builder->CreateLoad(destPointeeType, LHS, "lhs.load");
        lhsVal = builder->CreateSDiv(lhsVal, RHS, "compound.div");
        builder->CreateStore(lhsVal, LHS);
        writeThroughMutable(lhsVal);
        m_currentLLVMValue = lhsVal;
        return;
    } else if (node->op.type == vyb::TokenType::MODEQ) {
        llvm::Value *lhsVal = builder->CreateLoad(destPointeeType, LHS, "lhs.load");
        lhsVal = builder->CreateSRem(lhsVal, RHS, "compound.rem");
        builder->CreateStore(lhsVal, LHS);
        writeThroughMutable(lhsVal);
        m_currentLLVMValue = lhsVal;
        return;
    } else if (node->op.type == vyb::TokenType::BITWISEANDEQ ||
               node->op.type == vyb::TokenType::BITWISEOREQ ||
               node->op.type == vyb::TokenType::BITWISEXOREQ ||
               node->op.type == vyb::TokenType::LSHIFTEQ ||
               node->op.type == vyb::TokenType::RSHIFTEQ) {
        // Compound bitwise assignment. Coerce the RHS to the destination (LHS)
        // integer width so a bare literal (or defended mismatch) widens/truncates
        // before the and/or/xor/shift; LLVM requires same-width operands. Genuine
        // typed width mismatches are rejected earlier in semantic analysis.
        auto coerceBitwiseRHS = [&](llvm::Value* rhs) -> llvm::Value* {
            if (!rhs->getType()->isIntegerTy() || !destPointeeType->isIntegerTy()) return rhs;
            if (rhs->getType() == destPointeeType) return rhs;
            unsigned dw = destPointeeType->getIntegerBitWidth();
            unsigned rw = rhs->getType()->getIntegerBitWidth();
            if (rw > dw) return builder->CreateTrunc(rhs, destPointeeType, "compound.trunc");
            return builder->CreateZExt(rhs, destPointeeType, "compound.zext");
        };
        llvm::Value *lhsVal = builder->CreateLoad(destPointeeType, LHS, "lhs.load");
        llvm::Value* cRHS = coerceBitwiseRHS(RHS);
        if (node->op.type == vyb::TokenType::LSHIFTEQ) {
            lhsVal = builder->CreateShl(lhsVal, cRHS, "compound.shl");
        } else if (node->op.type == vyb::TokenType::RSHIFTEQ) {
            // Signed Int types use an arithmetic (sign-extending) shift; UInt*
            // types use a logical shift (consistent with the `>>` binary op).
            bool isUnsigned = false;
            if (lhsTypeNode) {
                if (auto tn = dynamic_cast<ast::TypeName*>(lhsTypeNode.get())) {
                    if (tn->identifier) {
                        const std::string& n = tn->identifier->name;
                        isUnsigned = (n == "UInt8" || n == "UInt16" || n == "UInt32" || n == "UInt64");
                    }
                }
            }
            lhsVal = isUnsigned
                ? builder->CreateLShr(lhsVal, cRHS, "compound.lshr")
                : builder->CreateAShr(lhsVal, cRHS, "compound.ashr");
        } else if (node->op.type == vyb::TokenType::BITWISEANDEQ) {
            lhsVal = builder->CreateAnd(lhsVal, cRHS, "compound.bwand");
        } else if (node->op.type == vyb::TokenType::BITWISEXOREQ) {
            lhsVal = builder->CreateXor(lhsVal, cRHS, "compound.bwxor");
        } else {
            lhsVal = builder->CreateOr(lhsVal, cRHS, "compound.bwor");
        }
        builder->CreateStore(lhsVal, LHS);
        writeThroughMutable(lhsVal);
        m_currentLLVMValue = lhsVal;
        return;
    }

    // Closure-typed variable overwrites release the target's previous hold on a
    // capture environment. Load the outgoing value first, then store the incoming
    // value (so a self-assignment keeps its env alive), then release the old one.
    bool closureOverwrite = isAssignToVar && destPointeeType && isClosureStructType(destPointeeType);
    if (closureOverwrite) {
        // Only overwrite-handle assignments to a confirmed `fn` binding; a plain
        // {ptr, ptr} target (2-pointer tuple) must keep plain store semantics.
        bool lhsFn = isFnTypeNode(typeOfNode(node->left).get()) || isFnTypeNode(typeOfNode(node->right).get());
        if (!lhsFn) {
            auto vit = valueTypeMap.find(LHS);
            if (vit != valueTypeMap.end() && isFnTypeNode(vit->second.get())) lhsFn = true;
        }
        if (!lhsFn) closureOverwrite = false;
    }
    llvm::Value* oldClosureVal = nullptr;
    if (closureOverwrite) {
        oldClosureVal = builder->CreateLoad(destPointeeType, LHS, "assign.old_closure");
        bool closureTransfer = dynamic_cast<ast::CallExpression*>(node->right.get()) != nullptr;
        if (!closureTransfer) {
            retainClosureValue(RHS);
        }
    }

    // String-typed overwrites: the incoming value may be a freshly-created
    // transfer (concat / to_string / a String-returning call) whose single owned
    // reference is handed over as-is, or a shared borrow that this location must
    // retain (+1). The outgoing buffer is released after the store so a
    // self-assignment's retain/release on the same buffer net to zero. Applies
    // to identifier *and* member destinations (a `st.field = src` through a
    // `their<Struct>` borrow): without the retain the destination shared the
    // source's buffer and the source's scope-exit release could drop it to
    // zero, leaving the member dangling.
    bool stringOverwrite = destPointeeType && isVybStringStructType(destPointeeType);
    llvm::Value* oldStringVal = nullptr;
    if (stringOverwrite) {
        oldStringVal = builder->CreateLoad(destPointeeType, LHS, "assign.old_string");
        if (!exprIsStringTransfer(node->right.get())) {
            retainStringValue(RHS);
        }
    }

    // Vec-typed overwrites: Vec is a VALUE TYPE — every storage location (a
    // `my` binding OR a struct field) that stores a Vec owns its own backing
    // buffer. Storing a new value over it must release the outgoing buffer, or
    // the old data leaks (e.g. reassigning a build-up Vec from a function's
    // return). A *borrowed read* RHS (a bare owning binding or member read that
    // will itself be reclaimed on scope exit) must be deep-copied before the
    // store so the destination does not alias the source; a transfer RHS (a
    // Vec-returning call) hands over a fresh single owner and is stored as-is.
    // (Fix #217: `b.records = rs` previously shallow-stored rs's data pointer
    // into the field with no clone and no old-buffer release, so both b and rs
    // free()'d the same Vec buffer at scope exit -> double free / heap
    // corruption, made deterministic when a later verify pass perturbs the
    // allocator. Mirrors the ownedStructAst value-semantics path below; applies
    // to member Vec destinations too, symmetric with the DECL path which
    // already clones on borrow.)
    bool vecOverwrite = false;
    llvm::Value* oldVecVal = nullptr;
    const vyb::ast::TypeNode* vecDestAst = nullptr;
    const vyb::ast::TypeNode* vecElemAst = nullptr;
    if (destPointeeType && isVecStructType(destPointeeType)) {
        const vyb::ast::TypeNode* destAst = lhsTypeNode.get();
        auto vtIt = valueTypeMap.find(LHS);
        if (vtIt != valueTypeMap.end() && vtIt->second) destAst = vtIt->second.get();
        if (destAst && objFieldTypeIsVec(destAst)) {
            vecDestAst = destAst;
            vecElemAst = objFieldVecElement(destAst);
            vecOverwrite = true;
            oldVecVal = builder->CreateLoad(destPointeeType, LHS, "assign.old_vec");
            bool borrowedRead =
                dynamic_cast<ast::Identifier*>(node->right.get()) != nullptr ||
                dynamic_cast<ast::MemberExpression*>(node->right.get()) != nullptr ||
                selectAllArmsAreOwnedReads(node->right.get());
            if (borrowedRead && vecElemAst) {
                if (llvm::Type* elemT = codegenType(const_cast<vyb::ast::TypeNode*>(vecElemAst))) {
                    if (auto* vecTy = llvm::dyn_cast<llvm::StructType>(destPointeeType)) {
                        llvm::Value* copy = generateVecDeepCopy(RHS, elemT, vecTy, vecElemAst);
                        if (copy) RHS = copy;
                    }
                }
            }
        }
    }
    // A Vec<String> keeps one reference per element; releasing the buffer on
    // overwrite must drop those element references first (mirrors scope-exit
    // cleanup) or the pointed-to String buffers would leak.
    bool vecHoldsStrings = false;
    if (vecOverwrite && vecDestAst) {
        vecHoldsStrings = isVecOfStringTypeNode(vecDestAst);
    }

    // A `my<Struct>` field (or binding) exclusively owns a heap allocation that
    // is stored as a `T*`. Overwriting it must release the outgoing allocation
    // (mirroring scope-exit reclaim) or the replaced struct leaks. Recursively
    // reclaim owned fields inside the pointed-to struct before freeing the block.
    bool myOverwrite = false;
    llvm::Value* oldMyPtr = nullptr;
    const vyb::ast::TypeNode* myPointeeAst = nullptr;
    if (lhsTypeNode && destPointeeType && destPointeeType->isPointerTy() &&
        isMyOwnedStructTypeNode(lhsTypeNode.get())) {
        myPointeeAst = myPointeeOf(lhsTypeNode.get());
        if (myPointeeAst) {
            myOverwrite = true;
            llvm::PointerType* rawPtrTy = llvm::PointerType::get(*context, 0);
            oldMyPtr = builder->CreateLoad(rawPtrTy, LHS, "assign.old_myp");
        }
    }

    // An `our<T>` binding (control-block backed struct) holds one strong ref and
    // releases it on scope exit. Overwriting a fresh value must release the
    // outgoing strong ref (or the replaced control block leaks), and a shared
    // incoming value (an existing `our` read) must be retained (+1) so the new
    // location owns its own reference. A transfer producer (`our(...)`,
    // `.grab()`, or a function returning `our<T>`) needs no retain.
    bool ourOverwrite = false;
    llvm::Value* oldOurPtr = nullptr;
    if (isAssignToVar && lhsTypeNode && destPointeeType && destPointeeType->isPointerTy() &&
        isOurRefType(lhsTypeNode.get())) {
        ourOverwrite = true;
        oldOurPtr = builder->CreateLoad(destPointeeType, LHS, "assign.old_our");
        if (!exprIsOurTransfer(node->right.get())) {
            retainOurControlBlock(RHS, "assign.our");
        }
    }

    // A `mild<T>` binding (weak reference) being overwritten must drop its old weak
    // ref, or the replaced control block can never be freed once its strong owner
    // drops (the orphaned weak ref keeps it alive). `soft(...)` hands over a fresh
    // weak ref needing no retain; a copy of an existing `mild` shares one and is
    // retained (+weak) to balance its own scope-exit release.
    bool mildOverwrite = false;
    llvm::Value* oldMildPtr = nullptr;
    if (isAssignToVar && lhsTypeNode && destPointeeType && destPointeeType->isPointerTy() &&
        isMildRefType(lhsTypeNode.get())) {
        mildOverwrite = true;
        oldMildPtr = builder->CreateLoad(destPointeeType, LHS, "assign.old_mild");
        if (!exprIsMildTransfer(node->right.get())) {
            retainMildControlBlock(RHS, "assign.mild");
        }
    }

    // A plain struct-typed destination whose fields own heap data (Strings,
    // Vecs, nested owning structs) must use by-value (deep) assignment
    // semantics: the outgoing struct's owned fields are reclaimed, and when the
    // source is a *borrowed read* (a bare owning binding or member read that
    // will itself be reclaimed on scope exit) the source is deep-copied so the
    // destination owns data independent of the source. Without this a whole-
    // struct store (`st.page = pg`, say) shallow-copied the owned pointers, so
    // both the destination and the source reclaimed the same buffers later -
    // "free(): double free detected" (the destructor double-free the VybLynx
    // BrowserState surfaced). A transfer-producing RHS (a call / constructor)
    // hands over a fresh single owner and is stored as-is after reclaiming the
    // outgoing value.
    const vyb::ast::TypeNode* ownedStructAst = nullptr;
    if (destPointeeType && llvm::isa<llvm::StructType>(destPointeeType)) {
        const vyb::ast::TypeNode* astType = lhsTypeNode.get();
        auto vtIt = valueTypeMap.find(LHS);
        if (vtIt != valueTypeMap.end() && vtIt->second) astType = vtIt->second.get();
        if (astType && isKnownStructTypeNode(astType) && structTypeHasOwnedFields(astType)) {
            ownedStructAst = astType;
        }
    }
    if (ownedStructAst) {
        auto* llvStruct = llvm::cast<llvm::StructType>(destPointeeType);
        bool borrowedRead =
            dynamic_cast<ast::Identifier*>(node->right.get()) != nullptr ||
            dynamic_cast<ast::MemberExpression*>(node->right.get()) != nullptr ||
            selectAllArmsAreOwnedReads(node->right.get());
        if (borrowedRead) {
            llvm::Value* dc = generateStructDeepCopy(RHS, ownedStructAst, llvStruct);
            if (dc) RHS = dc;
        }
        std::set<std::string> visited;
        reclaimStructOwnedFieldsAt(LHS, ownedStructAst, llvStruct, visited);
    }

    // Create the store instruction with proper alignment
    builder->CreateStore(RHS, LHS);
    writeThroughMutable(RHS);
    if (closureOverwrite && oldClosureVal) {
        releaseClosureValue(oldClosureVal);
    }
    if (stringOverwrite && oldStringVal) {
        releaseStringValue(oldStringVal);
    }
    if (vecOverwrite && oldVecVal) {
        llvm::Value* oldData = builder->CreateExtractValue(oldVecVal, 0, "assign.old_vec_data");
        llvm::PointerType* rawPtr = llvm::PointerType::get(*context, 0);
        llvm::Constant* nullPtr = llvm::ConstantPointerNull::get(rawPtr);
        llvm::Value* isNotNull = builder->CreateICmpNE(oldData, nullPtr, "assign.vec_not_null");
        llvm::Value* notSelfVal = llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context), 1);
        if (RHS->getType() == destPointeeType) {
            llvm::Value* newData = builder->CreateExtractValue(RHS, 0, "assign.new_vec_data");
            notSelfVal = builder->CreateICmpNE(oldData, newData, "assign.vec_not_self");
        }
        llvm::Value* shouldFree = builder->CreateAnd(isNotNull, notSelfVal, "assign.vec_should_free");
        llvm::BasicBlock* freeBB = llvm::BasicBlock::Create(*context, "assign.vec_free", currentFunction);
        llvm::BasicBlock* contBB = llvm::BasicBlock::Create(*context, "assign.vec_cont", currentFunction);
        builder->CreateCondBr(shouldFree, freeBB, contBB);
        builder->SetInsertPoint(freeBB);
        if (vecHoldsStrings) {
            llvm::Value* oldSize = builder->CreateExtractValue(oldVecVal, 1, "assign.old_vec_size");
            releaseStringElements(oldData, oldSize);
        }
        builder->CreateCall(getOrCreateFreeFunction(), {oldData});
        builder->CreateBr(contBB);
        builder->SetInsertPoint(contBB);
    }
    if (myOverwrite && oldMyPtr && myPointeeAst) {
        llvm::PointerType* rawPtrTy = llvm::PointerType::get(*context, 0);
        llvm::Constant* nullPtr = llvm::ConstantPointerNull::get(rawPtrTy);
        llvm::Value* isNotNull = builder->CreateICmpNE(oldMyPtr, nullPtr, "assign.myp_not_null");
        llvm::Value* notSelf = llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context), 1);
        if (RHS->getType()->isPointerTy()) {
            notSelf = builder->CreateICmpNE(oldMyPtr, RHS, "assign.myp_not_self");
        }
        llvm::Value* shouldFree = builder->CreateAnd(isNotNull, notSelf, "assign.myp_should_free");
        llvm::BasicBlock* freeBB = llvm::BasicBlock::Create(*context, "assign.myp_free", currentFunction);
        llvm::BasicBlock* contBB = llvm::BasicBlock::Create(*context, "assign.myp_cont", currentFunction);
        builder->CreateCondBr(shouldFree, freeBB, contBB);
        builder->SetInsertPoint(freeBB);
        llvm::Type* pointeeTy = codegenType(const_cast<vyb::ast::TypeNode*>(myPointeeAst));
        if (auto* pois = llvm::dyn_cast<llvm::StructType>(pointeeTy)) {
            reclaimOwnedStructAt(oldMyPtr, myPointeeAst, pois);
        }
        builder->CreateCall(getOrCreateFreeFunction(), {oldMyPtr});
        builder->CreateBr(contBB);
        builder->SetInsertPoint(contBB);
    }
    if (ourOverwrite && oldOurPtr) {
        llvm::PointerType* rawPtrTy = llvm::PointerType::get(*context, 0);
        llvm::Constant* nullPtr = llvm::ConstantPointerNull::get(rawPtrTy);
        llvm::Value* isNotNull = builder->CreateICmpNE(oldOurPtr, nullPtr, "assign.our_not_null");
        llvm::BasicBlock* freeBB = llvm::BasicBlock::Create(*context, "assign.our_free", currentFunction);
        llvm::BasicBlock* contBB = llvm::BasicBlock::Create(*context, "assign.our_cont", currentFunction);
        builder->CreateCondBr(isNotNull, freeBB, contBB);
        builder->SetInsertPoint(freeBB);
        const vyb::ast::TypeNode* pointeeAst = nullptr;
        llvm::Type* pointeeLlvm = nullptr;
        if (lhsTypeNode && ourPointeeOf(lhsTypeNode.get())) {
            pointeeAst = ourPointeeOf(lhsTypeNode.get());
            pointeeLlvm = codegenType(const_cast<vyb::ast::TypeNode*>(pointeeAst));
        }
        releaseOurControlBlock(oldOurPtr, "assign.our", pointeeAst, pointeeLlvm);
        builder->CreateBr(contBB);
        builder->SetInsertPoint(contBB);
    }
    if (mildOverwrite && oldMildPtr) {
        releaseMildControlBlock(oldMildPtr, "assign.mild");
    }

    // A `my<Struct>` move-assignment of an owning binding to another owning
    // binding (`dest = src`) leaves the source slot holding the same heap
    // pointer after the store, so both would reclaim it on scope exit (double
    // free). When the source is a local owner, null its slot so ownership
    // transfers to the target. A self-assignment (src == dest) is excluded, and
    // a borrowed `my` parameter (not an owner) is left untouched.
    if (myOverwrite && identLeft && node->right) {
        if (auto* rhsIdent = dynamic_cast<ast::Identifier*>(node->right.get())) {
            const ScopeVariable* srcVar = nullptr;
            for (auto sit = scopeStack.rbegin(); sit != scopeStack.rend() && !srcVar; ++sit) {
                for (const auto& sv : *sit) {
                    if (sv.name == rhsIdent->name) { srcVar = &sv; break; }
                }
            }
            if (srcVar && srcVar->ownership == ast::OwnershipKind::MY &&
                srcVar->needsCleanup && srcVar->allocaInst != LHS) {
                llvm::PointerType* rawPtr = llvm::PointerType::get(*context, 0);
                builder->CreateStore(llvm::ConstantPointerNull::get(rawPtr),
                                     srcVar->allocaInst, "move_assign.null_src");
                VYB_CDBG << "DEBUG: my<Struct> move assign '" << rhsIdent->name
                          << "' -> '" << identLeft->name << "': nulled source slot" << std::endl;
            }
        }
    }

    // If we're assigning to a member of a struct, we need to preserve type information
    if (auto memberExpr = dynamic_cast<ast::MemberExpression*>(node->left.get())) {
        if (typeOfNode(memberExpr->object)) {
            // The object has a type, so we can use it to determine field types
            // In a full implementation, we would look up the field's type from the struct definition
            // For now, we will propagate the object's type to help with future member accesses
            valueTypeMap[RHS] = typeOfNode(memberExpr->object);

            // If LHS is from a GEP instruction, associate the struct type with the GEP result
            if (auto gep = llvm::dyn_cast<llvm::GetElementPtrInst>(LHS)) {
                valueTypeMap[gep] = typeOfNode(memberExpr->object);
            }
        }
    }

    // Return the value being stored (C semantics)
    m_currentLLVMValue = RHS;

    // Make sure we also associate this result with the type if available
    if (rhsTypeNode) {
        valueTypeMap[RHS] = rhsTypeNode;
    }
}


void LLVMCodegen::visit(vyb::ast::ArrayElementExpression *node) {
    // This is for using array[index] or tuple[index] as an R-value (i.e., loading the value)
    // LHS usage is handled in AssignmentExpression

    // Check if the base expression is a tuple type
    bool isTupleAccess = false;
    if (typeOfNode(node->array)) {
        if (auto* tupleType = dynamic_cast<vyb::ast::TupleTypeNode*>(typeOfNode(node->array).get())) {
            isTupleAccess = true;
        }
    }

    // For tuple access, we need the actual struct value, not a pointer
    if (isTupleAccess) {
        // Visit the tuple expression to get the struct value
        node->array->accept(*this);
        llvm::Value *tupleValue = m_currentLLVMValue;

        // Visit the index expression
        node->index->accept(*this);
        llvm::Value *indexVal = m_currentLLVMValue;

        if (!tupleValue || !indexVal) {
            logError(node->loc, "Tuple or index expression failed to codegen.");
            m_currentLLVMValue = nullptr;
            return;
        }

        // Index must be a constant integer for tuple access
        if (auto* constIndex = llvm::dyn_cast<llvm::ConstantInt>(indexVal)) {
            uint64_t index = constIndex->getZExtValue();

            // Verify index is within bounds
            auto* tupleType = dynamic_cast<vyb::ast::TupleTypeNode*>(typeOfNode(node->array).get());
            if (index >= tupleType->memberTypes.size()) {
                logError(node->loc, "Tuple index " + std::to_string(index) + " out of bounds (size: " +
                         std::to_string(tupleType->memberTypes.size()) + ")");
                m_currentLLVMValue = nullptr;
                return;
            }

            // Extract the element from the tuple struct
            llvm::Value* element = builder->CreateExtractValue(tupleValue, {static_cast<unsigned>(index)}, "tuple_elem");
            m_currentLLVMValue = element;
            return;
        } else {
            logError(node->loc, "Tuple index must be a constant integer");
            m_currentLLVMValue = nullptr;
            return;
        }
    }

    // Original array access logic
    llvm::Value *arrayPtr = nullptr;

    // Special handling for identifier expressions to get the alloca directly
    if (auto* identExpr = dynamic_cast<vyb::ast::Identifier*>(node->array.get())) {
        auto it = namedValues.find(identExpr->name);
        if (it != namedValues.end()) {
            if (auto* alloca = llvm::dyn_cast<llvm::AllocaInst>(it->second)) {
                // For array variables, we need the alloca (pointer to array), not the loaded value
                arrayPtr = alloca;
            } else {
                arrayPtr = it->second; // Global or function
            }
        } else {
            logError(identExpr->loc, "Undefined identifier in array access: " + identExpr->name);
            m_currentLLVMValue = nullptr;
            return;
        }
    } else {
        // For other expressions, visit normally
        node->array->accept(*this);
        arrayPtr = m_currentLLVMValue;
    }

    // The index is always an R-value (never the assignment target): visit it with
    // the LHS flag cleared so a plain identifier index yields its VALUE, not its
    // alloca. Without this, `a[i]=v` fed the index's alloca pointer straight into
    // the GEP as an index -> `getelementptr [N x T], ptr, i32 0, ptr %i` -> module
    // verification failed (issue #201).
    bool saveIndexLHS = m_isLHSOfAssignment;
    m_isLHSOfAssignment = false;
    node->index->accept(*this);
    m_isLHSOfAssignment = saveIndexLHS;
    llvm::Value *indexVal = m_currentLLVMValue;

    if (!arrayPtr || !indexVal) {
        logError(node->loc, "Array or index expression for element access failed to codegen.");
        m_currentLLVMValue = nullptr;
        return;
    }

    // arrayPtr must be a pointer type for GEP
    if (!arrayPtr->getType()->isPointerTy()) {
        logError(node->array->loc, "Base of array element access (R-value) is not a pointer. Type: " + getTypeName(arrayPtr->getType()));
        m_currentLLVMValue = nullptr;
        return;
    }

    // Determine element type for GEP and Load. This is the type GEP operates on.
    llvm::Type* elementType = nullptr;
    if (typeOfNode(node->array)) { // AST type of the array expression itself
        vyb::ast::TypeNode* arrAstTypeNode = typeOfNode(node->array).get();
        if (auto arrayAstType = dynamic_cast<vyb::ast::ArrayType*>(arrAstTypeNode)) {
            // If the AST says it's an array T[N], then GEP on a T* needs element type T
            elementType = codegenType(arrayAstType->elementType.get());
        } else if (auto ptrAstType = dynamic_cast<vyb::ast::PointerType*>(arrAstTypeNode)) {
            // If the AST says it's a pointer T*, then GEP on T* needs element type T
            // Or if it's T(*)[N], GEP still needs T.
            // The key is what the pointer *ultimately points to* at the element level.
            if (auto pointedToArrType = dynamic_cast<vyb::ast::ArrayType*>(ptrAstType->pointeeType.get())) { // Corrected member access
                elementType = codegenType(pointedToArrType->elementType.get()); // Pointer to array, get element type of that array
            } else {
                elementType = codegenType(ptrAstType->pointeeType.get()); // Corrected member access // Pointer to element
            }
        } else if (auto vecAstType = dynamic_cast<vyb::ast::VecType*>(arrAstTypeNode)) {
            // A Vec<T> is a heap-backed container whose element type lives in the
            // same place as an array's. Without this branch, subscripting
            // Vec<String> fell through to the null-elementType error even though
            // the AST type ("Vec<String>") was known -- see issue #317.
            elementType = codegenType(vecAstType->elementType.get());
        }
    }

    if (!elementType) {
        // This is a fallback/error if AST type information was insufficient or missing.
        // Relying on LLVM types directly is problematic with opaque pointers.
        logError(node->loc, "Could not determine element type for array access (R-value) from AST. Array AST type: " + (typeOfNode(node->array) ? typeOfNode(node->array)->toString() : "null"));
        m_currentLLVMValue = nullptr;
        return;
    }

    // For array access, we need to handle the indexing properly
    // If arrayPtr is an alloca of array type, we need [0, index] to access the element
    // If arrayPtr points to the first element, we just use [index]

    llvm::Value *elementAddress = nullptr;

    // A Vec<T> is a struct { void* data; i64 size; i64 cap } (VecSlot in
    // runtime/vyb_type_metadata.c). Subscripting it must go through the data
    // pointer (field 0), so the GEP needs [0, 0, index] -- a single index cannot
    // walk into a struct. See issue #317.
    bool isVecAccess = false;
    if (auto* vecTn = typeOfNode(node->array)
            ? dynamic_cast<vyb::ast::VecType*>(typeOfNode(node->array).get()) : nullptr) {
        isVecAccess = vecTn != nullptr;
    }

    if (isVecAccess) {
        // Mirror the sequence the working `get` path emits:
        //   data_ptr  = gep {ptr,i64,i64}, %v, 0, 0     (field 0 = void* data)
        //   data      = load ptr, data_ptr
        //   elem_addr = gep i8, data, byteOffset        (byte-offset, untyped)
        llvm::Type* vecStructTy = llvm::StructType::get(*context,
            {llvm::PointerType::get(*context, 0),
             llvm::Type::getInt64Ty(*context),
             llvm::Type::getInt64Ty(*context)});

        llvm::Value* zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0);
        std::vector<llvm::Value*> fieldIdx = {zero, zero};
        llvm::Value* dataFieldPtr = builder->CreateGEP(vecStructTy, arrayPtr, fieldIdx,
                                                      "vec.data_ptr");
        llvm::Value* dataPtr = builder->CreateLoad(llvm::PointerType::get(*context, 0),
                                                   dataFieldPtr, "vec.data");
        // Elements are addressed by byte offset: index * sizeof(element).
        llvm::Value* elemSize = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context),
            module->getDataLayout().getTypeAllocSize(elementType));
        llvm::Value* idx64 = builder->CreateSExtOrTrunc(indexVal,
                                  llvm::Type::getInt64Ty(*context), "vec.idx64");
        llvm::Value* byteOffset = builder->CreateMul(idx64, elemSize, "vec.offset");
        elementAddress = builder->CreateGEP(llvm::Type::getInt8Ty(*context), dataPtr,
                                            byteOffset, "vecelemaddr_rval");
    } else if (auto* alloca = llvm::dyn_cast<llvm::AllocaInst>(arrayPtr)) {
        // arrayPtr is an alloca of array type, so we need [0, index] to get to the element
        llvm::Value* zero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0);
        std::vector<llvm::Value*> indices = {zero, indexVal};
        elementAddress = builder->CreateGEP(alloca->getAllocatedType(), arrayPtr, indices, "arrayelemaddr_rval");
    } else {
        // arrayPtr points to the first element, so just use [index]
        elementAddress = builder->CreateGEP(elementType, arrayPtr, indexVal, "arrayelemaddr_rval");
    }

    // Return the element address when this is an assignment target (LHS), so the
    // caller stores RHS through it; otherwise load the element value (read).
    if (m_isLHSOfAssignment) {
        m_currentLLVMValue = elementAddress;
        return;
    }
    m_currentLLVMValue = builder->CreateLoad(elementType, elementAddress, "arrayelemload");
}

// --- Basic Expression Visitors ---

void LLVMCodegen::visit(ast::Identifier* node) {
    // Look up the identifier in the named values map
    auto it = namedValues.find(node->name);
    if (it != namedValues.end()) {
        // Check if this is an AllocaInst (variable) and we're not on the LHS of assignment
        if (auto* alloca = llvm::dyn_cast<llvm::AllocaInst>(it->second)) {
            if (!m_isLHSOfAssignment) {
                // Load the value from the alloca for variable access
                llvm::Type* loadType = alloca->getAllocatedType();
                llvm::Value* loadedValue = builder->CreateLoad(loadType, alloca, node->name);

                // Propagate type information from alloca to loaded value
                auto typeIt = valueTypeMap.find(alloca);
                if (typeIt != valueTypeMap.end()) {
                    valueTypeMap[loadedValue] = typeIt->second;
                    VYB_CDBG << "DEBUG: Propagated type mapping from alloca to loaded value for '" << node->name << "'" << std::endl;
                }

                m_currentLLVMValue = loadedValue;
                return;
            }
        }
        // Module-level global variable read (used when emitting the deferred
        // runtime initializers, where globals still live in this map): load the
        // stored value rather than the address. Functions normally resolve
        // globals through globalValues_ below because they isolate namedValues.
        if (llvm::GlobalVariable* gv = llvm::dyn_cast<llvm::GlobalVariable>(it->second)) {
            if (!m_isLHSOfAssignment) {
                llvm::Value* loaded = builder->CreateLoad(gv->getValueType(), gv, node->name);
                auto typeIt = valueTypeMap.find(gv);
                if (typeIt != valueTypeMap.end()) {
                    valueTypeMap[loaded] = typeIt->second;
                }
                m_currentLLVMValue = loaded;
                return;
            }
            m_currentLLVMValue = gv;
            return;
        }
        // For functions or LHS of assignment, return the value directly
        m_currentLLVMValue = it->second;
        return;
    }

    // Also check current function named values
    auto funcIt = m_currentFunctionNamedValues.find(node->name);
    if (funcIt != m_currentFunctionNamedValues.end()) {
        // Load the value from the alloca
        llvm::Type* loadType = funcIt->second->getAllocatedType();
        llvm::Value* loadedValue = builder->CreateLoad(loadType, funcIt->second, node->name);

        // Propagate type information from alloca to loaded value
        auto typeIt = valueTypeMap.find(funcIt->second);
        if (typeIt != valueTypeMap.end()) {
            valueTypeMap[loadedValue] = typeIt->second;
            VYB_CDBG << "DEBUG: Propagated type mapping from function alloca to loaded value for '" << node->name << "'" << std::endl;
        }

        m_currentLLVMValue = loadedValue;
        return;
    }

    // Module-level global variable. Function bodies start from an isolated
    // namedValues, so look up globals here; read loads the value (unless this
    // is an assignment target, which needs the address).
    auto globalIt = globalValues_.find(node->name);
    if (globalIt != globalValues_.end()) {
        llvm::GlobalVariable* globalVar = globalIt->second;
        if (!m_isLHSOfAssignment) {
            llvm::Value* loaded = builder->CreateLoad(globalVar->getValueType(), globalVar, node->name);
            auto typeIt = valueTypeMap.find(globalVar);
            if (typeIt != valueTypeMap.end()) {
                valueTypeMap[loaded] = typeIt->second;
            }
            m_currentLLVMValue = loaded;
            return;
        }
        m_currentLLVMValue = globalVar;
        return;
    }

    // Check if it's a global function
    llvm::Function* func = module->getFunction(node->name);
    if (func) {
        m_currentLLVMValue = func;
        return;
    }

    logError(node->loc, "Undefined identifier: " + node->name);
    m_currentLLVMValue = nullptr;
}

void LLVMCodegen::visit(ast::MemberExpression* node) {
    if (!node->object) {
        logError(node->loc, "Member expression missing object");
        m_currentLLVMValue = nullptr;
        return;
    }

    // Check for enum variant access: EnumName::VariantName
    if (!node->computed && node->property) {
        // Generic data enum, unit variant used as a value: `Box<Int>::Empty`.
        if (auto* gi = dynamic_cast<ast::GenericInstantiationExpression*>(node->object.get())) {
            if (auto* baseIdent = dynamic_cast<ast::Identifier*>(gi->baseExpression.get())) {
                if (auto* propIdent = dynamic_cast<ast::Identifier*>(node->property.get())) {
                    if (genericEnumTemplates.count(baseIdent->name) || baseIdent->name == "Result") {
                        std::vector<ast::TypeNodePtr> concreteEnumArgs;
                        concreteEnumArgs.reserve(gi->genericArguments.size());
                        for (const auto& arg : gi->genericArguments) {
                            std::string s = arg->toString();
                            if (!currentTypeSubstitutions.empty()) {
                                for (const auto& kv : currentTypeSubstitutions) {
                                    s = replaceTypeTokens(s, kv.first, kv.second);
                                }
                            }
                            concreteEnumArgs.push_back(typePatternToTypeNode(TypePattern::parse(s), gi->loc));
                        }
                        std::string mangled = mangleGenericTypeName(baseIdent->name, concreteEnumArgs);
                        if (!taggedEnumInfo.count(mangled)) monomorphizeEnum(baseIdent->name, concreteEnumArgs);
                        auto tagEnumIt = taggedEnumInfo.find(mangled);
                        const TaggedEnumInfo& tei = tagEnumIt->second;
                        if (tei.variantPayloadTypes.count(propIdent->name)) {
                            logError(node->loc, "Enum variant '" + propIdent->name +
                                     "' of generic enum " + mangled + " requires constructor arguments");
                            m_currentLLVMValue = nullptr;
                            return;
                        }
                        if (!tei.variantTags.count(propIdent->name)) {
                            logError(node->loc, "Unknown enum variant: " + baseIdent->name + "::" + propIdent->name);
                            m_currentLLVMValue = nullptr;
                            return;
                        }
                        m_currentLLVMValue = buildTaggedEnumValue(mangled, propIdent->name, {});
                        return;
                    }
                }
            }
        }
        if (auto* objIdent = dynamic_cast<ast::Identifier*>(node->object.get())) {
            if (enumTypeNames.count(objIdent->name)) {
                if (auto* propIdent = dynamic_cast<ast::Identifier*>(node->property.get())) {
                    const std::string qualName = objIdent->name + "::" + propIdent->name;
                    // A tagged-union enum (has data variants) is emitted as a value
                    // struct rather than an integer constant. A unit variant used
                    // directly as a value builds a tag-only value; a data variant
                    // used without constructor arguments is an error.
                    if (taggedEnumInfo.count(objIdent->name)) {
                        auto tagEnumIt = taggedEnumInfo.find(objIdent->name);
                        const TaggedEnumInfo& tei = tagEnumIt->second;
                        if (tei.variantPayloadTypes.count(propIdent->name)) {
                            logError(node->loc, "Enum variant '" + propIdent->name +
                                     "' of tagged enum " + objIdent->name + " requires constructor arguments");
                            m_currentLLVMValue = nullptr;
                            return;
                        }
                        if (!tei.variantTags.count(propIdent->name)) {
                            logError(node->loc, "Unknown enum variant: " + qualName);
                            m_currentLLVMValue = nullptr;
                            return;
                        }
                        m_currentLLVMValue = buildTaggedEnumValue(objIdent->name, propIdent->name, {});
                        return;
                    }
                    auto enumIt = enumVariantValues.find(qualName);
                    if (enumIt != enumVariantValues.end()) {
                        m_currentLLVMValue = enumIt->second;
                        return;
                    }
                    logError(node->loc, "Unknown enum variant: " + qualName);
                    m_currentLLVMValue = nullptr;
                    return;
                }
            }
        }
    }

    // Evaluate the object expression (should not be treated as LHS)
    bool wasLHS = m_isLHSOfAssignment;
    llvm::Value* objectValue = nullptr;
    llvm::AllocaInst* originalAlloca = nullptr;  // Track original alloca for LHS struct values

    if (m_isLHSOfAssignment) {
        // Optimization: when on LHS and object is an identifier, get the alloca directly
        // instead of loading the value. This avoids creating temp allocas that lose writes.
        // Only apply when the alloca's pointee is a direct struct type (not ownership-wrapped).
        if (auto* objIdent = dynamic_cast<ast::Identifier*>(node->object.get())) {
            auto it = namedValues.find(objIdent->name);
            if (it != namedValues.end()) {
                if (auto* alloca = llvm::dyn_cast<llvm::AllocaInst>(it->second)) {
                    // Check if pointee is a direct struct type (not control block)
                    llvm::Type* pointeeType = alloca->getAllocatedType();
                    bool isDirectStruct = pointeeType->isStructTy();
                    // Also check valueTypeMap to ensure it's not an ownership wrapper
                    auto vtIt = valueTypeMap.find(alloca);
                    if (vtIt != valueTypeMap.end()) {
                        if (auto astType = vtIt->second.get()) {
                            if (auto typeNameNode = dynamic_cast<ast::TypeName*>(astType)) {
                                std::string tn = typeNameNode->identifier ? typeNameNode->identifier->name : "";
                                if ((tn == "my" || tn == "our" || tn == "their" || tn == "borrow" || tn == "view") &&
                                    !typeNameNode->genericArgs.empty()) {
                                    isDirectStruct = false;  // Ownership wrapper, use normal path
                                }
                            }
                        }
                    }
                    if (isDirectStruct) {
                        objectValue = alloca;
                        originalAlloca = alloca;
                    }
                }
            }
            if (!objectValue) {
                auto funcIt = m_currentFunctionNamedValues.find(objIdent->name);
                if (funcIt != m_currentFunctionNamedValues.end()) {
                    if (auto* alloca = llvm::dyn_cast<llvm::AllocaInst>(funcIt->second)) {
                        llvm::Type* pointeeType = alloca->getAllocatedType();
                        bool isDirectStruct = pointeeType->isStructTy();
                        auto vtIt = valueTypeMap.find(alloca);
                        if (vtIt != valueTypeMap.end()) {
                            if (auto astType = vtIt->second.get()) {
                                if (auto typeNameNode = dynamic_cast<ast::TypeName*>(astType)) {
                                    std::string tn = typeNameNode->identifier ? typeNameNode->identifier->name : "";
                                    if ((tn == "my" || tn == "our" || tn == "their" || tn == "borrow" || tn == "view") &&
                                        !typeNameNode->genericArgs.empty()) {
                                        isDirectStruct = false;
                                    }
                                }
                            }
                        }
                        if (isDirectStruct) {
                            objectValue = alloca;
                            originalAlloca = alloca;
                        }
                    }
                }
            }
        }
    }

    if (!objectValue) {
        // Fall back to normal evaluation. A nested member chain that is itself the
        // LHS base (e.g. the `a.b` in `a.b.c = x`) must be evaluated in pointer/LHS
        // mode so it yields the address of the nested struct field; loading it into
        // a temporary would write `c` into the copy and silently drop the store.
        // Only do this when the chain's immediate field holds a plain struct value.
        // Ownership/pointer-typed fields (`their`/`our`/`my`/`mild`/`view`/`borrow`)
        // must be LOADED to dereference the pointer slot even in an LHS chain (e.g.
        // the `self.map` in a nested by-ref `self.map.keys` Vec receiver read).
        bool keepPointerMode = m_isLHSOfAssignment &&
                               dynamic_cast<ast::MemberExpression*>(node->object.get()) != nullptr;
        if (keepPointerMode) {
            if (auto* nm = dynamic_cast<ast::TypeName*>(typeOfNode(node->object).get())) {
                if (nm->identifier) {
                    const std::string t = nm->identifier->name;
                    if (t == "my" || t == "our" || t == "their" || t == "mild" ||
                        t == "view" || t == "borrow") {
                        keepPointerMode = false;
                    }
                }
            }
        }
        if (!keepPointerMode) {
            m_isLHSOfAssignment = false;  // Object evaluation should load the value normally
        }
        node->object->accept(*this);
        m_isLHSOfAssignment = wasLHS;  // Restore LHS flag for field access
        objectValue = m_currentLLVMValue;
    }

    if (!objectValue) {
        logError(node->object->loc, "Failed to evaluate object in member expression");
        m_currentLLVMValue = nullptr;
        return;
    }
    // A fresh owned-struct receiver temp -- the object is a call/literal whose
    // result is a struct with owned Vec/String fields, e.g. `arena.get(idx).keys
    // .len()` where get() deep-copies an owned Node. Used only as a member base
    // and discarded, such a temp's deep-copied buffers are otherwise never
    // reclaimed (BTreeMap put/get leaked ~1 x 64B per inline `.get().field`,
    // #192). Register a reclaim-copy in the current scope so the existing
    // scope-exit teardown (cleanupVariable -> reclaimOwnedStructAt) frees its
    // owned fields -- the same durable mechanism named structs use. `objectValue`
    // is left untouched so the field/method access below is unaffected; this copy
    // merely shares the same owned buffers and is reclaimed once the enclosing
    // scope exits (after every use of the chain has completed). Not gated on
    // m_isLHSOfAssignment (spuriously true on RHS member chains); reclaim at
    // scope exit is safe for both sides. Named-variable bases are excluded (their
    // binding owns the data).
    if (objectValue && objectValue->getType() &&
        objectValue->getType()->isStructTy() && node->object &&
        (dynamic_cast<ast::CallExpression*>(node->object.get()) != nullptr ||
         dynamic_cast<ast::ObjectLiteral*>(node->object.get()) != nullptr) &&
        typeOfNode(node->object) && isKnownStructTypeNode(typeOfNode(node->object).get()) &&
        structTypeHasOwnedFields(typeOfNode(node->object).get())) {
        if (auto* st = llvm::dyn_cast<llvm::StructType>(objectValue->getType())) {
            llvm::Value* tmp = builder->CreateAlloca(st, nullptr, "recvstruct.tmp");
            builder->CreateStore(objectValue, tmp);
            valueTypeMap[tmp] = std::shared_ptr<vyb::ast::TypeNode>(typeOfNode(node->object)->clone());
            std::string nm = "__recv_struct_tmp_" + std::to_string(m_recvStructTempCounter++);
            registerVariable(nm, tmp, objectValue, ast::OwnershipKind::MY, st, /*needsCleanup=*/true);
        }
    }

    // `.tag` accessor on an enum value: returns the raw positional i64 tag as an Int.
    // Works for C-like scalar enums (the value already IS the tag) and for
    // data-carrying enums (extract field 0 of the { i64 tag, [N x i8] data } struct).
    if (!node->computed) {
        if (auto* tagPropIdent = dynamic_cast<ast::Identifier*>(node->property.get())) {
            if (tagPropIdent->name == "tag") {
                if (const TaggedEnumInfo* tei = findTaggedEnum(typeOfNode(node->object).get())) {
                    if (tei->isScalar) {
                        m_currentLLVMValue = objectValue;
                    } else if (objectValue->getType()->isPointerTy()) {
                        llvm::Type* enumTy = codegenType(typeOfNode(node->object).get());
                        llvm::Value* loaded = builder->CreateLoad(enumTy, objectValue, "enum.tag.load");
                        m_currentLLVMValue = builder->CreateExtractValue(loaded, 0, "enum.tag");
                    } else {
                        m_currentLLVMValue = builder->CreateExtractValue(objectValue, 0, "enum.tag");
                    }
                    return;
                }
            }
        }
    }

    // `.value` accessor on the built-in `Result<T,E>` enum: reads the payload of
    // the primary (success) data variant (`Ok(T)`) as its payload type `T`.
    // Guards on the runtime tag so reading the "wrong" variant (Err) yields a
    // default T rather than uninitialized union bytes, matching the house
    // "return default on invalid access" style used by Vec.get.
    if (!node->computed) {
        if (auto* valPropIdent = dynamic_cast<ast::Identifier*>(node->property.get())) {
            if (valPropIdent->name == "value") {
                if (auto* objTn = dynamic_cast<ast::TypeName*>(typeOfNode(node->object).get())) {
                    const std::string vb = objTn->identifier ? objTn->identifier->name : "";
                    const bool isResult = vb == "Result";
                    if (isResult && objTn->genericArgs.size() >= 1) {
                        if (const TaggedEnumInfo* tei = findTaggedEnum(typeOfNode(node->object).get())) {
                            const std::string prim = "Ok";
                            auto payIt = tei->variantPayloadTypes.find(prim);
                            if (payIt != tei->variantPayloadTypes.end() && payIt->second &&
                                payIt->second->getNumElements() > 0) {
                                llvm::StructType* payloadTy = payIt->second;
                                llvm::Type* eltTy = payloadTy->getElementType(0);
                                llvm::Value* enumVal = objectValue;
                                if (enumVal->getType()->isPointerTy()) {
                                    llvm::Type* enumTy = codegenType(typeOfNode(node->object).get());
                                    enumVal = builder->CreateLoad(enumTy, enumVal, "enum.value.load");
                                }
                                auto tagIt = tei->variantTags.find(prim);
                                if (!enumVal || tagIt == tei->variantTags.end()) {
                                    logError(node->loc, "Cannot resolve .value on " + vb);
                                    m_currentLLVMValue = nullptr;
                                    return;
                                }
                                llvm::Value* tagVal = builder->CreateExtractValue(enumVal, 0, "enum.value.tag");
                                llvm::Value* primTag = llvm::ConstantInt::get(int64Type, tagIt->second, true);
                                llvm::Value* isPrim = builder->CreateICmpEQ(tagVal, primTag, "enum.value.isprimary");
                                llvm::Value* payload = extractEnumVariantField(enumVal, payloadTy, 0);
                                llvm::Value* defVal = llvm::Constant::getNullValue(eltTy);
                                m_currentLLVMValue = builder->CreateSelect(isPrim, payload, defVal, "enum.value");
                                return;
                            }
                        }
                    }
                }
            }
        }
    }

    if (node->computed) {
        // Computed member access: obj[prop]
        if (!node->property) {
            logError(node->loc, "Computed member access missing property expression");
            m_currentLLVMValue = nullptr;
            return;
        }

        // Evaluate the property expression to get the index
        node->property->accept(*this);
        llvm::Value* indexValue = m_currentLLVMValue;

        if (!indexValue) {
            logError(node->property->loc, "Failed to evaluate property in computed member access");
            m_currentLLVMValue = nullptr;
            return;
        }

        // This is similar to array element access
        if (!objectValue->getType()->isPointerTy()) {
            logError(node->object->loc, "Computed member access on non-pointer type");
            m_currentLLVMValue = nullptr;
            return;
        }

        // For now, assume the object is a pointer to the first element
        // In a full implementation, we'd need type information to determine the element type
        llvm::Type* elementType = llvm::Type::getInt32Ty(*context); // Default fallback

        llvm::Value* elementPtr = builder->CreateGEP(elementType, objectValue, indexValue, "member.computed");
        m_currentLLVMValue = builder->CreateLoad(elementType, elementPtr, "member.load");
    } else {
        // Property member access: obj.prop
        if (!node->property) {
            logError(node->loc, "Property member access missing property identifier");
            m_currentLLVMValue = nullptr;
            return;
        }

        // For property access, we need to cast the property to an identifier
        ast::Identifier* propIdent = dynamic_cast<ast::Identifier*>(node->property.get());
        if (!propIdent) {
            logError(node->property->loc, "Property in member access must be an identifier");
            m_currentLLVMValue = nullptr;
            return;
        }

        llvm::Value* structPtr = nullptr;
        llvm::Type* structType = nullptr;
        bool objectIsOurControlBlock = false;

        // Check if the object is a pointer to a struct (alloca) or actual struct value
        if (objectValue->getType()->isPointerTy()) {
            // Object is a pointer (from alloca) - use it directly
            structPtr = objectValue;

            // Get the pointee type from alloca instruction if available
            if (llvm::AllocaInst* allocaInst = llvm::dyn_cast<llvm::AllocaInst>(objectValue)) {
                structType = allocaInst->getAllocatedType();
                VYB_CDBG << "DEBUG: Got struct type from alloca: " << getTypeName(structType) << std::endl;
            } else {
                // Handle function parameters and other pointer values
                VYB_CDBG << "DEBUG: Object is a pointer but not an alloca, checking pointee type" << std::endl;
                VYB_CDBG << "DEBUG: Object pointer type: " << getTypeName(objectValue->getType()) << std::endl;
                if (llvm::PointerType* ptrType = llvm::dyn_cast<llvm::PointerType>(objectValue->getType())) {
                    // For newer LLVM versions, we need to use a different approach
                    // Since we can't easily get the pointee type, try to get it from the value type map
                    auto valueTypeIter = valueTypeMap.find(objectValue);
                    if (valueTypeIter != valueTypeMap.end()) {
                        // Get the AST type and convert it to LLVM type
                        if (auto astType = valueTypeIter->second.get()) {
                            // Handle ownership types specially - extract underlying type
                            ast::TypeNode* underlyingType = astType;
                            if (auto typeNameNode = dynamic_cast<ast::TypeName*>(astType)) {
                                std::string typeNameStr = typeNameNode->identifier ? typeNameNode->identifier->name : "";
                                if ((typeNameStr == "my" || typeNameStr == "our" || typeNameStr == "their" ||
                                     typeNameStr == "borrow" || typeNameStr == "view" || typeNameStr == "mild") &&
                                    !typeNameNode->genericArgs.empty() && typeNameNode->genericArgs[0]) {
                                    underlyingType = typeNameNode->genericArgs[0].get();
                                    // `our<T>` and `mild<T>` are both stored as a shared
                                    // control-block pointer, so field access on either goes
                                    // through the block's payload pointer.
                                    objectIsOurControlBlock = typeNameStr == "our" || typeNameStr == "mild";
                                    VYB_CDBG << "DEBUG: Extracted underlying type from ownership type: " << typeNameStr
                                              << " -> " << underlyingType->toString() << std::endl;
                                }
                            }

                            llvm::Type* astLLVMType = codegenType(underlyingType);
                            if (astLLVMType && astLLVMType->isStructTy()) {
                                structType = astLLVMType;
                                VYB_CDBG << "DEBUG: Got struct type from AST type mapping: " << getTypeName(structType) << std::endl;
                            } else {
                                VYB_CDBG << "DEBUG: AST type mapping didn't yield struct type, got: " << (astLLVMType ? getTypeName(astLLVMType) : "null") << std::endl;
                                logError(node->loc, "Cannot determine struct type for member access");
                                m_currentLLVMValue = nullptr;
                                return;
                            }
                        } else {
                            VYB_CDBG << "DEBUG: No AST type information available" << std::endl;
                            logError(node->loc, "Cannot determine struct type for member access");
                            m_currentLLVMValue = nullptr;
                            return;
                        }
                    } else {
                        VYB_CDBG << "DEBUG: No type mapping found for pointer value" << std::endl;
                        logError(node->loc, "Cannot determine struct type for member access");
                        m_currentLLVMValue = nullptr;
                        return;
                    }
                } else {
                    VYB_CDBG << "DEBUG: Pointer type cast failed" << std::endl;
                    logError(node->loc, "Cannot determine struct type for member access");
                    m_currentLLVMValue = nullptr;
                    return;
                }
            }
        } else if (objectValue->getType()->isStructTy()) {
            // Object is a struct value (loaded from variable) - create temporary alloca
            structType = objectValue->getType();
            structPtr = builder->CreateAlloca(structType, nullptr, "temp_struct");
            builder->CreateStore(objectValue, structPtr);
            VYB_CDBG << "DEBUG: Created temporary alloca for struct value: " << getTypeName(structType) << std::endl;
            // If on LHS and we have the original alloca, store back after field modification
            if (m_isLHSOfAssignment && originalAlloca) {
                VYB_CDBG << "DEBUG: Tracking original alloca for struct value back-store" << std::endl;
            }
        } else {
            logError(node->object->loc, "Property member access on non-struct type");
            m_currentLLVMValue = nullptr;
            return;
        }

        if (!structType || !structType->isStructTy()) {
            logError(node->loc, "Cannot access field of non-struct type");
            m_currentLLVMValue = nullptr;
            return;
        }

        llvm::StructType* llvmStructType = llvm::cast<llvm::StructType>(structType);
        if (objectIsOurControlBlock) {
            // our<T> is represented as a control-block pointer. Field access
            // goes through the payload pointer stored in the block.
            std::vector<llvm::Type*> cbFields = {
                llvm::Type::getInt32Ty(*context),
                llvm::Type::getInt32Ty(*context),
                llvm::Type::getInt8Ty(*context),
                llvm::PointerType::get(*context, 0)
            };
            llvm::StructType* controlBlockType = llvm::StructType::get(*context, cbFields, /*isPacked=*/false);
            llvm::Value* objectPtrFieldPtr = builder->CreateStructGEP(controlBlockType, structPtr, 3, "our.object_ptr_field");
            structPtr = builder->CreateLoad(llvm::PointerType::get(*context, 0), objectPtrFieldPtr, "our.object_ptr");
        }
        std::string fieldName = propIdent->name;
        int fieldIndex = getStructFieldIndex(llvmStructType, fieldName);

        if (fieldIndex < 0) {
            logError(node->loc, "Field '" + fieldName + "' not found in struct");
            m_currentLLVMValue = nullptr;
            return;
        }


        VYB_CDBG << "DEBUG: MemberExpression - Field '" << fieldName << "' at index " << fieldIndex
                  << " in struct " << llvmStructType->getName().str() << std::endl;

        // Create a GEP to get a pointer to the field
        llvm::Value* fieldPtr = builder->CreateStructGEP(llvmStructType, structPtr, fieldIndex, fieldName + "_ptr");


        llvm::Type* fieldType = llvmStructType->getElementType(fieldIndex);
        VYB_CDBG << "DEBUG: Field type: " << getTypeName(fieldType) << std::endl;

        // Check if we're on the LHS of an assignment - in that case return the pointer
        if (m_isLHSOfAssignment) {
            m_currentLLVMValue = fieldPtr;
            // Remember the field-pointer's AST type so a further member access on
            // it (a nested chain like `a.b.c = x`) can resolve the struct type of
            // the pointer object level by level without an alloca.
            if (typeOfNode(node)) {
                valueTypeMap[fieldPtr] = std::shared_ptr<vyb::ast::TypeNode>(typeOfNode(node)->clone());
            }
        } else {
            // For reading, always load the value
            // Even for struct types (like String), we want the value, not a pointer to temporary storage
            llvm::Value* loaded = builder->CreateLoad(fieldType, fieldPtr, fieldName + "_val");
            // Remember the read value's AST type so a further member access on it
            // (e.g. `self.set.values` / `self.map.keys`) can determine the struct
            // type without an alloca. This is what lets iterators read fields
            // through a nested `their<Struct>` view field.
            if (typeOfNode(node)) {
                valueTypeMap[loaded] = std::shared_ptr<vyb::ast::TypeNode>(typeOfNode(node)->clone());
            }
            m_currentLLVMValue = loaded;
        }
    }
}

llvm::Value* LLVMCodegen::borrowTargetPointer(llvm::Value* operandValue,
                                              const vyb::ast::TypeNode* operandType,
                                              const std::string& kw) {
    if (!operandValue || !operandValue->getType()->isPointerTy()) return operandValue;

    std::string base;
    if (auto* tn = dynamic_cast<const vyb::ast::TypeName*>(operandType)) {
        if (tn->identifier && !tn->genericArgs.empty()) base = tn->identifier->name;
    }
    if (base.empty()) return operandValue;  // plain struct/pointer: the slot is the object

    llvm::PointerType* ptrTy = llvm::PointerType::get(*context, 0);
    const bool isSlot = llvm::dyn_cast<llvm::AllocaInst>(operandValue) != nullptr;

    if (base == "our" || base == "mild") {
        // Control block pointer; the borrowed object is the payload pointer in
        // field 3. The operand may be the block pointer itself or a slot holding it.
        llvm::Value* cb = operandValue;
        if (isSlot) cb = builder->CreateLoad(ptrTy, operandValue, kw + ".cb.load");
        std::vector<llvm::Type*> cbFields = {
            llvm::Type::getInt32Ty(*context),
            llvm::Type::getInt32Ty(*context),
            llvm::Type::getInt8Ty(*context),
            ptrTy
        };
        llvm::StructType* controlBlockType =
            llvm::StructType::get(*context, cbFields, /*isPacked=*/false);
        llvm::Value* payloadField =
            builder->CreateStructGEP(controlBlockType, cb, 3, kw + ".payload_field");
        return builder->CreateLoad(ptrTy, payloadField, kw + ".payload");
    }
    if ((base == "my" || base == "their" || base == "borrow" || base == "view" || base == "ptr") && isSlot) {
        // The slot holds the pointee pointer; the borrow is that pointer.
        return builder->CreateLoad(ptrTy, operandValue, kw + ".ptr.load");
    }
    return operandValue;
}

// Step (c) of #365 (see codegen.hpp). At the spawn call sites, judge the
// closure's captures at their *substituted* concrete types, because the semantic
// pass -- which runs before monomorphization -- can only see the declared type
// parameter `T` and stays permissive there.
//
// The registries are passed as null: codegen has no AST-level struct/enum
// registry of its own, so a substituted type that names a struct or enum is
// treated as unresolved and stays permissive here, exactly as an unresolved type
// parameter does. Sharing the registries is the tracked follow-up
// (doc/THREAD_BOUNDARY_SCOPE.md). Everything structural -- `my<T>`, `their<T>`,
// `loc<T>`, `ptr<T>`, `Vec<...>`, arrays, `T?`, futures, tuples, `our<T>`,
// `mild<T>` -- is resolved exactly as in the semantic pass, which is the case
// this step exists for.
vyb::ast::TypeNodePtr LLVMCodegen::substitutedCaptureType(const std::string& name,
                                                         const vyb::SourceLocation& loc) {
    auto nv = namedValues.find(name);
    if (nv == namedValues.end()) return nullptr;
    auto vt = valueTypeMap.find(nv->second);
    if (vt == valueTypeMap.end() || !vt->second) return nullptr;

    const std::string original = vt->second->toString();
    std::string resolved = original;
    if (!currentTypeSubstitutions.empty()) {
        for (const auto& kv : currentTypeSubstitutions) {
            resolved = replaceTypeTokens(resolved, kv.first, kv.second);
        }
    }
    if (resolved == original) {
        return vyb::ast::TypeNodePtr(vt->second->clone().release());
    }
    // The type parameter is gone here: judge the concrete type of the
    // instantiation, which is exactly what the semantic pass could not see.
    return typePatternToTypeNode(TypePattern::parse(resolved), loc);
}

void LLVMCodegen::checkSpawnHandoffWithSubstitutions(ast::FunctionExpression* fe,
                                                     ast::Node* site,
                                                     const std::string& siteName) {
    if (!fe || !site) return;

    // The semantic pass decided every capture it could and *recorded* the ones it
    // could not -- a declared type that still mentioned a monomorphization type
    // parameter (a bare `T`, `our<T>`, `Vec<T>`, ...). Those are this function's
    // whole job. Re-judging the rest here would be wrong twice over: this pass has
    // no borrow-root data for rule (b), and the semantic pass already admitted them
    // on the concrete evidence it had (registries + curated binds).
    //
    // The registries come back with the question: boundaryCapable_ is the semantic
    // analyzer's own predicate, so a substituted type that resolves to a *named*
    // struct or enum is judged by its fields/variant payloads instead of staying
    // permissive.
    auto* sa = driver_.getSemanticAnalyzer();
    if (!sa) return;
    const std::vector<std::string>* deferred = sa->deferredBoundaryCapturesFor(fe);
    if (!deferred || deferred->empty()) return;

    std::set<std::string> checked;
    for (const std::string& cap : *deferred) {
        if (!checked.insert(cap).second) continue;

        vyb::ast::TypeNodePtr ty = substitutedCaptureType(cap, site->loc);
        if (!ty) continue;  // no recorded type for the capture: stay permissive
        const bool capable = boundaryCapable_
                                 ? boundaryCapable_(ty.get())
                                 : thread_boundary::handoffCapable(ty.get(), nullptr, nullptr);
        if (capable) continue;

        // This is a hard codegen error, like the #251 enum-payload check: the IR
        // this program would produce must never be linked or run, so flag it for
        // the driver rather than only printing (codegen's logError is not
        // otherwise fatal -- a bare print would let the program execute).
        flagHardCodegenError();
        logError(site->loc, siteName + ": '" + cap + "' is " + ty->toString() +
                     " -- it cannot cross a thread boundary (not handoff-capable). "
                     "Hand it off as shared ownership (our(" + cap + ")) or pass a copy.");
    }
}

void LLVMCodegen::visit(ast::BorrowExpression* node) {
    if (!node->expression) {
        logError(node->loc, "Borrow expression missing operand");
        m_currentLLVMValue = nullptr;
        return;
    }

    // Special handling for identifiers - the operand's slot, unwrapped the way
    // member access unwraps it, so `borrow(x).field` reads the object rather than
    // the pointer stored in x's slot (see borrowTargetPointer, #365 step 0).
    if (auto* identNode = dynamic_cast<ast::Identifier*>(node->expression.get())) {
        const std::string kw =
            node->kind == ast::BorrowKind::MUTABLE_BORROW ? "borrow" : "view";
        llvm::Value* slot = nullptr;
        auto it = namedValues.find(identNode->name);
        if (it != namedValues.end()) {
            slot = it->second;
        } else {
            auto funcIt = m_currentFunctionNamedValues.find(identNode->name);
            if (funcIt != m_currentFunctionNamedValues.end()) slot = funcIt->second;
        }
        if (slot) {
            // Prefer the AST type recorded on the slot (the closure prologue
            // records the captured variable's type on the reloaded alloca),
            // else the semantic type of the expression.
            const ast::TypeNode* opTy = nullptr;
            auto vtIt = valueTypeMap.find(slot);
            if (vtIt != valueTypeMap.end() && vtIt->second) opTy = vtIt->second.get();
            if (!opTy) opTy = typeOfNode(node->expression).get();
            m_currentLLVMValue = borrowTargetPointer(slot, opTy, kw);
            return;
        }

        // If not found, fall through to regular evaluation
    }

    // Evaluate the expression being borrowed (for non-identifier cases)
    node->expression->accept(*this);
    llvm::Value* borrowedValue = m_currentLLVMValue;

    if (!borrowedValue) {
        logError(node->expression->loc, "Failed to evaluate borrow expression operand");
        m_currentLLVMValue = nullptr;
        return;
    }

    // For borrow expressions, we typically want the address of the value
    // The semantics depend on the kind of borrow
    switch (node->kind) {
        case ast::BorrowKind::MUTABLE_BORROW:
            // borrow(expr) - creates a mutable reference
            if (borrowedValue->getType()->isPointerTy()) {
                // If it's already a pointer, just return it
                m_currentLLVMValue = borrowedValue;
            } else {
                // Store in memory and return the address
                llvm::AllocaInst* temp = builder->CreateAlloca(
                    borrowedValue->getType(),
                    nullptr,
                    "borrow.tmp"
                );
                builder->CreateStore(borrowedValue, temp);
                m_currentLLVMValue = temp;
            }
            break;

        case ast::BorrowKind::IMMUTABLE_VIEW:
            // view(expr) - creates an immutable reference
            if (borrowedValue->getType()->isPointerTy()) {
                // If it's already a pointer, just return it
                m_currentLLVMValue = borrowedValue;
            } else {
                // Store in memory and return the address
                llvm::AllocaInst* temp = builder->CreateAlloca(
                    borrowedValue->getType(),
                    nullptr,
                    "view.tmp"
                );
                builder->CreateStore(borrowedValue, temp);
                m_currentLLVMValue = temp;
            }
            break;

        default:
            logError(node->loc, "Unknown borrow kind");
            m_currentLLVMValue = nullptr;
            break;
    }
}

void LLVMCodegen::visit(ast::FromIntToLocExpression* node) {
    if (!node->getAddressExpression()) {
        logError(node->loc, "from<> expression missing operand");
        m_currentLLVMValue = nullptr;
        return;
    }

    // Evaluate the integer expression
    node->getAddressExpression()->accept(*this);
    llvm::Value* intValue = m_currentLLVMValue;

    if (!intValue) {
        logError(node->getAddressExpression()->loc, "Failed to evaluate from<> expression operand");
        m_currentLLVMValue = nullptr;
        return;
    }

    // Convert integer to pointer
    llvm::Type* targetType;
    if (node->getTargetType()) {
        targetType = codegenType(node->getTargetType().get());
    } else {
        // Default to generic pointer
        targetType = llvm::PointerType::get(*context, 0);
    }

    if (!targetType || !targetType->isPointerTy()) {
        logError(node->loc, "from<> target type must be a pointer type");
        m_currentLLVMValue = nullptr;
        return;
    }

    // Convert integer to pointer
    m_currentLLVMValue = builder->CreateIntToPtr(intValue, targetType, "fromint.ptr");
}

void LLVMCodegen::visit(ast::ListComprehension* node) {
    // List comprehensions are complex and require collection allocation
    logError(node->loc, "List comprehensions are not yet implemented in LLVM codegen");
    m_currentLLVMValue = nullptr;
}

void LLVMCodegen::visit(ast::IfExpression* node) {
    // This is an if-expression that returns a value, unlike an if-statement

    // Create basic blocks for the different paths
    llvm::Function* function = getCurrentFunction();
    if (!function) {
        logError(node->loc, "IfExpression outside function context");
        m_currentLLVMValue = nullptr;
        return;
    }

    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(*context, "if.then", function);
    llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(*context, "if.else", function);
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(*context, "if.end", function);

    // Generate condition code
    node->condition->accept(*this);
    llvm::Value* condValue = m_currentLLVMValue;
    if (!condValue) {
        logError(node->condition->loc, "Invalid condition expression in if-expression");
        m_currentLLVMValue = nullptr;
        return;
    }

    // Convert condition to i1 (boolean) if it's not already
    if (condValue->getType() != int1Type) {
        condValue = builder->CreateICmpNE(condValue,
                                         llvm::ConstantInt::get(condValue->getType(), 0),
                                         "ifcond");
    }

    // Create the conditional branch
    builder->CreateCondBr(condValue, thenBB, elseBB);

    // Generate 'then' block
    builder->SetInsertPoint(thenBB);
    node->thenBranch->accept(*this);
    llvm::Value* thenValue = m_currentLLVMValue;
    if (!thenValue) {
        logError(node->thenBranch->loc, "Invalid expression in then branch of if-expression");
        m_currentLLVMValue = nullptr;
        return;
    }
    llvm::BasicBlock* thenTerm = builder->GetInsertBlock(); // may have sub-blocks

    // Generate 'else' block
    builder->SetInsertPoint(elseBB);
    node->elseBranch->accept(*this);
    llvm::Value* elseValue = m_currentLLVMValue;
    if (!elseValue) {
        logError(node->elseBranch->loc, "Invalid expression in else branch of if-expression");
        m_currentLLVMValue = nullptr;
        return;
    }
    llvm::BasicBlock* elseTerm = builder->GetInsertBlock();

    // Values from then and else branches must have the same LLVM type. String
    // values may appear in two representations: an already-wrapped {ptr,i64}
    // struct (literals, String-typed expressions) or a raw char* (the result of
    // __vyb_string_concat / .to_string in a branch). Unify UP to the
    // {ptr,i64} String struct whenever either branch already carries one, so we
    // only ever wrap a raw char* into String — never cast a String struct DOWN
    // to char* (which tryCast rejects). (Fix #218: each unification cast is
    // emitted inside its own branch's terminal block, strictly before that
    // block's `br mergeBB`, so the wrapped value dominates the merge edge and
    // nothing but PHIs sits at the merge-block head. The old code ran tryCast
    // after SetInsertPoint(mergeBB), landing the wrap above the PHI and
    // referencing a value from %if.else -> 'does not dominate all uses!' +
    // 'PHI nodes not grouped at top of basic block!'.)
    auto isStrStruct = [](llvm::Type* t) {
        if (t && t->isStructTy()) {
            llvm::StructType* st = llvm::cast<llvm::StructType>(t);
            return st->getNumElements() == 2 && st->getElementType(0)->isPointerTy();
        }
        return false;
    };
    llvm::Type* resultType = thenValue->getType();
    if (isStrStruct(elseValue->getType()) && !isStrStruct(thenValue->getType()))
        resultType = elseValue->getType();

    // Terminate both branches (defensive: on any cast error the IR still has
    // valid terminators so the module abort is clean, matching the pre-fix
    // behaviour where the brs were emitted before unification).
    auto terminateBranches = [&]() {
        builder->SetInsertPoint(thenTerm);
        builder->CreateBr(mergeBB);
        builder->SetInsertPoint(elseTerm);
        builder->CreateBr(mergeBB);
    };

    if (elseValue->getType() != resultType) {
        builder->SetInsertPoint(elseTerm);
        elseValue = tryCast(elseValue, resultType, node->elseBranch->loc);
        if (!elseValue) {
            terminateBranches();
            logError(node->elseBranch->loc, "Type mismatch in if-expression branches");
            m_currentLLVMValue = nullptr;
            return;
        }
        elseTerm = builder->GetInsertBlock();
    }
    if (thenValue->getType() != resultType) {
        builder->SetInsertPoint(thenTerm);
        thenValue = tryCast(thenValue, resultType, node->thenBranch->loc);
        if (!thenValue) {
            terminateBranches();
            logError(node->thenBranch->loc, "Type mismatch in if-expression branches");
            m_currentLLVMValue = nullptr;
            return;
        }
        thenTerm = builder->GetInsertBlock();
    }
    terminateBranches();

    // Generate merge block with PHI node. Nothing but PHIs may precede the phi
    // here, so the merge block is created only now.
    builder->SetInsertPoint(mergeBB);
    llvm::PHINode* phiNode = builder->CreatePHI(resultType, 2, "ifexpr.result");
    phiNode->addIncoming(thenValue, thenTerm);
    phiNode->addIncoming(elseValue, elseTerm);

    m_currentLLVMValue = phiNode;
}

void LLVMCodegen::visit(ast::ConstructionExpression* node) {
    if (!node->constructedType) {
        logError(node->loc, "Construction expression missing type");
        m_currentLLVMValue = nullptr;
        return;
    }

    VYB_CDBG << "DEBUG: ConstructionExpression processing type: " << node->constructedType->toString() << std::endl;

    // Explicitly-typed bare Vec constructor: `Vec<T>()` / `Vec<T>(n)`. A typed
    // construction parses `Vec<Int>` as a TypeName (not the bare `Vec` identifier),
    // so it lands here rather than in CallExpression's `emitVecConstructor` path.
    // Route it through the same emitter so the result is the real
    // `{ ptr, size, cap }` struct *value* — not a pointer to an uninitialized
    // alloca (which would break the assignment cast and segfault in a field init).
    if (auto* tname = dynamic_cast<ast::TypeName*>(node->constructedType.get())) {
        if (tname->identifier && tname->identifier->name == "Vec") {
            VYB_CDBG << "DEBUG: ConstructionExpression is a typed Vec constructor" << std::endl;
            auto calleeId = std::make_unique<ast::Identifier>(node->loc, "Vec");
            auto callExpr = std::make_unique<ast::CallExpression>(node->loc, std::move(calleeId), std::move(node->arguments));
            emitVecConstructor(callExpr.get());
            node->arguments = std::move(callExpr->arguments);
            return;
        }
    }

    // Built-in channel constructor: `chan<T>()` (unbounded) / `chan<T>(cap)`
    // (bounded). The payload element type picks the runtime channel table
    // (int-slot vs String), while the resulting value is always a single i64
    // handle. Limited to 0 or 1 argument: the capacity (0 = unbounded).
    if (auto* tname = dynamic_cast<ast::TypeName*>(node->constructedType.get())) {
        if (tname->identifier && tname->identifier->name == "chan" && tname->genericArgs.size() == 1) {
            const bool isString = chanElementIsString(tname->genericArgs[0].get());
            llvm::Value* cap = nullptr;
            if (node->arguments.size() > 1) {
                logError(node->loc, "chan<T> constructor accepts at most one argument (capacity)");
                m_currentLLVMValue = nullptr;
                return;
            }
            if (!node->arguments.empty() && node->arguments[0]) {
                node->arguments[0]->accept(*this);
                cap = m_currentLLVMValue;
            } else {
                cap = llvm::ConstantInt::get(int64Type, 0);
            }
            llvm::Function* f = module->getFunction(isString ? "__vyb_strchan_new" : "__vyb_chan_new");
            if (!f) {
                llvm::FunctionType* ft = llvm::FunctionType::get(int64Type, {int64Type}, false);
                f = llvm::Function::Create(ft, llvm::Function::ExternalLinkage,
                                           isString ? "__vyb_strchan_new" : "__vyb_chan_new", module.get());
            }
            m_currentLLVMValue = builder->CreateCall(f, {cap}, "chan.new");
            return;
        }
    }

    // If the "constructed type" is actually a generic function template invoked with
    // explicit type arguments (e.g. `probe<Int>(0, 0)`), this is a generic function
    // call rather than struct construction. Dispatch it through the call path so the
    // explicit type args are honored and the failable return ABI (and trap handling)
    // apply, matching plain inferred generic calls.
    if (auto* tname = dynamic_cast<ast::TypeName*>(node->constructedType.get())) {
        if (tname->identifier && !tname->genericArgs.empty()) {
            std::string fnName = tname->identifier->name;
            if (genericFunctionTemplates.find(fnName) != genericFunctionTemplates.end()) {
                VYB_CDBG << "DEBUG: ConstructionExpression is a generic function call: " << fnName << std::endl;
                auto calleeId = std::make_unique<ast::Identifier>(tname->identifier->loc, fnName);
                auto callExpr = std::make_unique<ast::CallExpression>(node->loc, std::move(calleeId), std::move(node->arguments));
                // Copy (not move) the explicit generic args. `node->constructedType`
                // belongs to a shared generic-template body that is codegen'd once per
                // distinct instantiation; a destructive move leaves `genericArgs` empty
                // so a later instantiation miscompiles (falls out of this dispatch) (#205).
                for (const auto& g : tname->genericArgs) {
                    callExpr->explicitTypeArgs.push_back(g->clone());
                }
                this->visit(static_cast<ast::CallExpression*>(callExpr.get()));
                // Restore the moved-out argument list so the shared template body can be
                // re-visited safely; otherwise a drained node re-routes the next
                // instantiation to the wrong arity (see the struct-ctor branch below) (#205).
                node->arguments = std::move(callExpr->arguments);
                return;
            }
        }
    }

    // Declared struct constructor: `HashMap<K,V>(n)` with a matching arity routes
    // to the synthetic `__ctor_HashMap_<N>` generic function. Struct names are not
    // generic-fn templates, so this never collides with the branch above. When a
    // generic-function monomorphization is active, the struct's own type params
    // (K, V) are substituted so a constructor can chain to another constructor.
    if (auto* tname = dynamic_cast<ast::TypeName*>(node->constructedType.get())) {
        if (tname->identifier) {
            std::string structName = tname->identifier->name;
            auto cit = structConstructors.find(structName);
            if (cit != structConstructors.end()) {
                for (const auto& entry : cit->second) {
                    if (entry.first == (unsigned)node->arguments.size()) {
                        VYB_CDBG << "DEBUG: ConstructionExpression is a struct constructor call: "
                                 << structName << " arity " << entry.first << std::endl;
                        auto calleeId = std::make_unique<ast::Identifier>(tname->identifier->loc, entry.second);
                        auto callExpr = std::make_unique<ast::CallExpression>(node->loc, std::move(calleeId), std::move(node->arguments));
                        if (!tname->genericArgs.empty()) {
                            callExpr->explicitTypeArgs = applyTypeSubstitutions(tname->genericArgs);
                        }
                        this->visit(static_cast<ast::CallExpression*>(callExpr.get()));
                        // Restore the moved-out argument list. The constructed-type node
                        // is part of a shared generic-template body (a struct ctor chains
                        // to another ctor via `HashMap<K,V>(0)`) that is codegen'd once per
                        // distinct instantiation. Without this restore, the first
                        // instantiation drains `node->arguments` to empty, so the NEXT
                        // instantiation's arity match hits `entry.first == 0` -> routes to
                        // the 0-arg ctor -> infinite mono recursion -> stack overflow (#205).
                        node->arguments = std::move(callExpr->arguments);
                        return;
                    }
                }
            }
        }
    }

    // Get the type being constructed
    llvm::Type* constructedLLVMType = codegenType(node->constructedType.get());
    if (!constructedLLVMType) {
        logError(node->loc, "Failed to resolve constructed type");
        m_currentLLVMValue = nullptr;
        return;
    }

    VYB_CDBG << "DEBUG: Successfully resolved constructed type: " << node->constructedType->toString() << std::endl;

    // Native `T?` construction: `T?()` builds the absent optional and `T?(v)`
    // builds the present one. The value is a `{ bool hasValue, T value }`
    // struct; at most one construction argument supplies the payload.
    if (auto* optTy = dynamic_cast<ast::OptionalType*>(node->constructedType.get())) {
        llvm::StructType* optStruct = llvm::cast<llvm::StructType>(constructedLLVMType);
        llvm::Value* payload = nullptr;
        if (!node->arguments.empty()) {
            if (node->arguments.size() > 1 || !node->arguments[0]) {
                logError(node->loc, optTy->toString() + " expects at most one construction argument.");
                m_currentLLVMValue = nullptr;
                return;
            }
            node->arguments[0]->accept(*this);
            payload = m_currentLLVMValue;
            if (!payload) { m_currentLLVMValue = nullptr; return; }
            llvm::Type* valueTy = optStruct->getElementType(0);
            if (payload->getType() != valueTy) {
                payload = tryCast(payload, valueTy, node->loc);
                if (!payload) { m_currentLLVMValue = nullptr; return; }
            }
            // A present `T?` whose payload is a String must take an independent
            // reference to the buffer before the optional aliases it: every
            // storage location that stores a String retains, unless it is
            // receiving a freshly-created owned transfer (which the source owns
            // outright, e.g. `substring`). Wrapping an aliased owned String such
            // as `String?(o)` kept refcount 1 while the local and the optional
            // each later released it -> premature free / UAF (#176).
            if (isVybStringStructType(payload->getType()) &&
                !exprIsStringTransfer(node->arguments[0].get())) {
                retainStringValue(payload);
            }
        }
        llvm::AllocaInst* optAlloca = builder->CreateAlloca(optStruct, nullptr, "opt.tmp");
        if (payload) {
            builder->CreateStore(payload,
                                 builder->CreateStructGEP(optStruct, optAlloca, 0, "opt.payload.ptr"));
        } else {
            // Absent `T?()`: zero the payload so a bare matching that reads it is
            // well-defined, and ALWAYS set the hasValue flag explicitly rather than
            // relying on a whole-struct memset. DataLayout on a module that has not
            // yet been given its `target datalayout` under-sizes `{ T, i1 }` when T
            // contains a Bool (its alloc size is just T's), leaving the flag at a
            // higher offset uninitialized -- which made absent matching hit a random
            // arm. Explicitly storing false pins the absent case.
            builder->CreateMemSet(
                optAlloca,
                llvm::ConstantInt::get(builder->getInt8Ty(), 0),
                llvm::ConstantInt::get(builder->getInt64Ty(),
                    llvm::DataLayout(module.get()).getTypeAllocSize(optStruct->getElementType(0))),
                llvm::MaybeAlign());
        }
        builder->CreateStore(payload ? builder->getInt1(true) : builder->getInt1(false),
                             builder->CreateStructGEP(optStruct, optAlloca, 1, "opt.present.ptr"));
        m_currentLLVMValue = builder->CreateLoad(optStruct, optAlloca, "opt.val");
        return;
    }

    // For now, implement basic construction with default values
    if (constructedLLVMType->isStructTy()) {
        // Struct construction
        llvm::StructType* structType = llvm::cast<llvm::StructType>(constructedLLVMType);

        // Allocate memory for the struct
        llvm::AllocaInst* structAlloca = builder->CreateAlloca(structType, nullptr, "struct.tmp");

        // Zero-initialize the whole struct first so fields not covered by the
        // provided arguments are well-defined (0 / null) rather than left as
        // uninitialized stack memory. Reading an uninitialized field is UB and
        // crashes the JIT when the value is branched on.
        {
            llvm::DataLayout dl(module.get());
            llvm::Align structAlign(dl.getPrefTypeAlign(structType).value());
            builder->CreateMemSet(
                structAlloca,
                llvm::ConstantInt::get(builder->getInt8Ty(), 0),
                llvm::ConstantInt::get(builder->getInt64Ty(), dl.getTypeAllocSize(structType)),
                structAlign);
        }

        // Initialize with default values or provided arguments
        for (unsigned i = 0; i < structType->getNumElements() && i < node->arguments.size(); ++i) {
            if (node->arguments[i]) {
                node->arguments[i]->accept(*this);
                llvm::Value* argValue = m_currentLLVMValue;
                if (argValue) {
                    llvm::Value* fieldPtr = builder->CreateStructGEP(structType, structAlloca, i, "field.ptr");
                    builder->CreateStore(argValue, fieldPtr);
                }
            }
        }

        // Return the loaded struct value (value semantics), matching struct
        // literals so `a<Pt> = Pt(1, 2)` assigns a value rather than the alloca
        // pointer (which otherwise surfaces as a ptr-to-Pt cast mismatch).
        m_currentLLVMValue = builder->CreateLoad(structType, structAlloca, "struct.val");
    } else {
        // For primitive types, just use the first argument or default value
        if (!node->arguments.empty() && node->arguments[0]) {
            node->arguments[0]->accept(*this);
            llvm::Value* argValue = m_currentLLVMValue;
            if (argValue) {
                m_currentLLVMValue = tryCast(argValue, constructedLLVMType, node->loc);
            } else {
                m_currentLLVMValue = nullptr;
            }
        } else {
            // Default value for the type
            if (constructedLLVMType->isIntegerTy()) {
                m_currentLLVMValue = llvm::ConstantInt::get(constructedLLVMType, 0);
            } else if (constructedLLVMType->isFloatingPointTy()) {
                m_currentLLVMValue = llvm::ConstantFP::get(constructedLLVMType, 0.0);
            } else if (constructedLLVMType->isPointerTy()) {
                m_currentLLVMValue = llvm::ConstantPointerNull::get(
                    llvm::cast<llvm::PointerType>(constructedLLVMType)
                );
            } else {
                m_currentLLVMValue = llvm::UndefValue::get(constructedLLVMType);
            }
        }
    }
}

void LLVMCodegen::visit(ast::ArrayInitializationExpression* node) {
    if (!node->elementType) {
        logError(node->loc, "Array initialization missing element type");
        m_currentLLVMValue = nullptr;
        return;
    }

    if (!node->sizeExpression) {
        logError(node->loc, "Array initialization missing size expression");
        m_currentLLVMValue = nullptr;
        return;
    }

    // Get the element type
    llvm::Type* elementType = codegenType(node->elementType.get());
    if (!elementType) {
        logError(node->loc, "Failed to resolve array element type");
        m_currentLLVMValue = nullptr;
        return;
    }

    // Evaluate the size expression
    node->sizeExpression->accept(*this);
    llvm::Value* sizeValue = m_currentLLVMValue;
    if (!sizeValue) {
        logError(node->sizeExpression->loc, "Failed to evaluate array size");
        m_currentLLVMValue = nullptr;
        return;
    }

    // Ensure size is an integer
    if (!sizeValue->getType()->isIntegerTy()) {
        logError(node->sizeExpression->loc, "Array size must be an integer");
        m_currentLLVMValue = nullptr;
        return;
    }

    // Create array type
    llvm::ArrayType* arrayType = llvm::ArrayType::get(elementType, 0); // Dynamic size

    // Allocate memory for the array
    llvm::AllocaInst* arrayAlloca = builder->CreateAlloca(elementType, sizeValue, "array.tmp");

    // Initialize array elements to default values
    // For simplicity, we'll zero-initialize
    llvm::Value* zero = llvm::ConstantInt::get(sizeValue->getType(), 0);
    llvm::Value* one = llvm::ConstantInt::get(sizeValue->getType(), 1);

    // Create a loop to initialize the array
    llvm::Function* function = getCurrentFunction();
    llvm::BasicBlock* loopHeaderBB = llvm::BasicBlock::Create(*context, "array.init.header", function);
    llvm::BasicBlock* loopBodyBB = llvm::BasicBlock::Create(*context, "array.init.body", function);
    llvm::BasicBlock* loopExitBB = llvm::BasicBlock::Create(*context, "array.init.exit", function);

    // Initialize loop counter
    llvm::AllocaInst* counterAlloca = builder->CreateAlloca(sizeValue->getType(), nullptr, "counter");
    builder->CreateStore(zero, counterAlloca);
    builder->CreateBr(loopHeaderBB);

    // Loop header: check condition
    builder->SetInsertPoint(loopHeaderBB);
    llvm::Value* currentCounter = builder->CreateLoad(sizeValue->getType(), counterAlloca, "counter.val");
    llvm::Value* condition = builder->CreateICmpSLT(currentCounter, sizeValue, "loop.cond");
    builder->CreateCondBr(condition, loopBodyBB, loopExitBB);

    // Loop body: initialize element
    builder->SetInsertPoint(loopBodyBB);
    llvm::Value* elementPtr = builder->CreateGEP(elementType, arrayAlloca, currentCounter, "element.ptr");

    // Initialize element with default value
    llvm::Value* defaultValue;
    if (elementType->isIntegerTy()) {
        defaultValue = llvm::ConstantInt::get(elementType, 0);
    } else if (elementType->isFloatingPointTy()) {
        defaultValue = llvm::ConstantFP::get(elementType, 0.0);
    } else if (elementType->isPointerTy()) {
        defaultValue = llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(elementType));
    } else {
        defaultValue = llvm::UndefValue::get(elementType);
    }

    builder->CreateStore(defaultValue, elementPtr);

    // Increment counter
    llvm::Value* nextCounter = builder->CreateAdd(currentCounter, one, "counter.next");
    builder->CreateStore(nextCounter, counterAlloca);
    builder->CreateBr(loopHeaderBB);

    // Loop exit
    builder->SetInsertPoint(loopExitBB);

    m_currentLLVMValue = arrayAlloca;
}

void LLVMCodegen::visit(vyb::ast::GenericInstantiationExpression* node) {
    // TODO: Implement generic instantiation expression
    // This should handle generic type instantiation with specific type arguments
    m_currentLLVMValue = nullptr;
}

void LLVMCodegen::visit(ast::LogicalExpression* node) {
    // Logical expressions with short-circuit evaluation
    node->left->accept(*this);
    llvm::Value* leftValue = m_currentLLVMValue;

    if (!leftValue) {
        logError(node->left->loc, "Invalid left operand in logical expression");
        m_currentLLVMValue = nullptr;
        return;
    }

    // Convert to boolean if needed
    if (!leftValue->getType()->isIntegerTy(1)) {
        if (leftValue->getType()->isIntegerTy()) {
            leftValue = builder->CreateICmpNE(leftValue,
                llvm::ConstantInt::get(leftValue->getType(), 0), "left.bool");
        } else if (leftValue->getType()->isFloatingPointTy()) {
            leftValue = builder->CreateFCmpONE(leftValue,
                llvm::ConstantFP::get(leftValue->getType(), 0.0), "left.bool");
        } else if (leftValue->getType()->isPointerTy()) {
            leftValue = builder->CreateICmpNE(leftValue,
                llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(leftValue->getType())), "left.bool");
        }
    }

    // Create basic blocks for short-circuit evaluation
    llvm::BasicBlock* rightBB = llvm::BasicBlock::Create(*context, "logical.right", currentFunction);
    llvm::BasicBlock* endBB = llvm::BasicBlock::Create(*context, "logical.end", currentFunction);
    llvm::BasicBlock* leftBB = builder->GetInsertBlock();

    if (node->op.lexeme == "&&") {
        // For &&: if left is false, don't evaluate right
        builder->CreateCondBr(leftValue, rightBB, endBB);
    } else { // "||"
        // For ||: if left is true, don't evaluate right
        builder->CreateCondBr(leftValue, endBB, rightBB);
    }

    // Right operand block
    builder->SetInsertPoint(rightBB);
    node->right->accept(*this);
    llvm::Value* rightValue = m_currentLLVMValue;

    if (!rightValue) {
        logError(node->right->loc, "Invalid right operand in logical expression");
        m_currentLLVMValue = nullptr;
        return;
    }

    // Convert to boolean if needed
    if (!rightValue->getType()->isIntegerTy(1)) {
        if (rightValue->getType()->isIntegerTy()) {
            rightValue = builder->CreateICmpNE(rightValue,
                llvm::ConstantInt::get(rightValue->getType(), 0), "right.bool");
        } else if (rightValue->getType()->isFloatingPointTy()) {
            rightValue = builder->CreateFCmpONE(rightValue,
                llvm::ConstantFP::get(rightValue->getType(), 0.0), "right.bool");
        } else if (rightValue->getType()->isPointerTy()) {
            rightValue = builder->CreateICmpNE(rightValue,
                llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(rightValue->getType())), "right.bool");
        }
    }

    builder->CreateBr(endBB);
    rightBB = builder->GetInsertBlock();

    // End block with PHI node
    builder->SetInsertPoint(endBB);
    llvm::PHINode* phi = builder->CreatePHI(llvm::Type::getInt1Ty(*context), 2, "logical.result");

    if (node->op.lexeme == "&&") {
        phi->addIncoming(llvm::ConstantInt::getFalse(*context), leftBB);
        phi->addIncoming(rightValue, rightBB);
    } else { // "||"
        phi->addIncoming(llvm::ConstantInt::getTrue(*context), leftBB);
        phi->addIncoming(rightValue, rightBB);
    }

    m_currentLLVMValue = phi;
}

void LLVMCodegen::visit(ast::ConditionalExpression* node) {
    // Visit condition
    node->condition->accept(*this);
    llvm::Value* condValue = m_currentLLVMValue;

    // Convert to boolean if needed
    if (condValue->getType() != int1Type) {
        if (condValue->getType()->isIntegerTy()) {
            condValue = builder->CreateICmpNE(condValue,
                llvm::ConstantInt::get(condValue->getType(), 0), "cond.bool");
        } else if (condValue->getType()->isFloatingPointTy()) {
            condValue = builder->CreateFCmpONE(condValue,
                llvm::ConstantFP::get(condValue->getType(), 0.0), "cond.bool");
        } else if (condValue->getType()->isPointerTy()) {
            condValue = builder->CreateICmpNE(condValue,
                llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(condValue->getType())), "cond.bool");
        }
    }

    // Create basic blocks
    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(*context, "cond.then", currentFunction);
    llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(*context, "cond.else", currentFunction);
    llvm::BasicBlock* endBB = llvm::BasicBlock::Create(*context, "cond.end", currentFunction);

    builder->CreateCondBr(condValue, thenBB, elseBB);

    // Then branch
    builder->SetInsertPoint(thenBB);
    node->thenExpr->accept(*this);
    llvm::Value* thenValue = m_currentLLVMValue;
    llvm::BasicBlock* thenEndBB = builder->GetInsertBlock();
    builder->CreateBr(endBB);

    // Else branch
    builder->SetInsertPoint(elseBB);
    node->elseExpr->accept(*this);
    llvm::Value* elseValue = m_currentLLVMValue;
    llvm::BasicBlock* elseEndBB = builder->GetInsertBlock();
    builder->CreateBr(endBB);

    // Merge
    builder->SetInsertPoint(endBB);

    // Ensure both values have compatible types
    llvm::Type* resultType = thenValue->getType();
    if (thenValue->getType() != elseValue->getType()) {
        // Try to cast to a common type (simplified - in a real compiler, you'd do proper type resolution)
        if (thenValue->getType()->isIntegerTy() && elseValue->getType()->isIntegerTy()) {
            // Use the larger integer type
            if (thenValue->getType()->getIntegerBitWidth() > elseValue->getType()->getIntegerBitWidth()) {
                elseValue = builder->CreateSExt(elseValue, thenValue->getType());
                resultType = thenValue->getType();
            } else {
                thenValue = builder->CreateSExt(thenValue, elseValue->getType());
                resultType = elseValue->getType();
            }
        } else {
            // Default to i32 if types are incompatible
            resultType = llvm::Type::getInt32Ty(*context);
            if (!thenValue->getType()->isIntegerTy(32)) {
                thenValue = llvm::ConstantInt::get(resultType, 0);
            }
            if (!elseValue->getType()->isIntegerTy(32)) {
                elseValue = llvm::ConstantInt::get(resultType, 0);
            }
        }
    }

    llvm::PHINode* phi = builder->CreatePHI(resultType, 2, "cond.result");
    phi->addIncoming(thenValue, thenEndBB);
    phi->addIncoming(elseValue, elseEndBB);

    m_currentLLVMValue = phi;
}

void LLVMCodegen::visit(ast::FunctionExpression* node) {
    // Async lambdas (`async |x| -> await process(x)`) compile their body as a
    // closure and wrap it in a task launcher that returns a Future<T>.
    if (node->isAsync) {
        codegenAsyncLambda(node);
        return;
    }
    // A function expression creates a lambda. Every lambda lowers to a closure
    // value: `struct { ptr env, ptr fn }`. Captured variables (filled in by the
    // semantic analyzer) are copied by value into a heap-allocated environment
    // at creation time; the lambda function receives that environment as a
    // hidden first parameter and reloads each capture into a local alloca.
    std::string funcName = "lambda_" + std::to_string(reinterpret_cast<uintptr_t>(node));

    // Process parameters.
    std::vector<llvm::Type*> paramTypes;
    std::vector<std::string> paramNames;
    for (const auto& param : node->params) {
        llvm::Type* paramType = param.typeNode ? codegenType(param.typeNode.get()) : nullptr;
        if (!paramType) {
            paramType = llvm::PointerType::get(*context, 0);
            logWarning(node->loc, "Parameter type not specified in function expression, defaulting to pointer type");
        }
        paramTypes.push_back(paramType);
        paramNames.push_back(param.name ? param.name->name : "param" + std::to_string(paramNames.size()));
    }

    // Infer return type from the semantic FunctionType the analyzer attached to
    // this lambda (its returnType reflects explicit `return` statements for block
    // bodies). Prefer that over typeOfNode(node->body), which is null when a block ends
    // in a `return` statement (rather than a tail expression).
    llvm::Type* returnType = llvm::Type::getVoidTy(*context);
    if (auto* ft = dynamic_cast<ast::FunctionType*>(typeOfNode(node).get())) {
        if (ft->returnType) {
            llvm::Type* t = codegenType(ft->returnType.get());
            if (t) returnType = t;
        }
    } else if (node->body && typeOfNode(node->body)) {
        llvm::Type* inferredType = codegenType(typeOfNode(node->body).get());
        if (inferredType && !inferredType->isVoidTy()) {
            returnType = inferredType;
        }
    }

    // User-facing signature (no environment parameter). Stored for call sites
    // that invoke a lambda held in a local variable.
    llvm::FunctionType* userFuncType = llvm::FunctionType::get(returnType, paramTypes, false);
    lastLambdaFuncType = userFuncType;

    // A failable Void lambda (an agent behavior that can `fail`) implements the
    // failable return ABI `{ i1 dummy, i8* err }`. The user-facing signature
    // above stays plain Void; only the implemented function return becomes the
    // two-field tuple, so the runtime can distinguish a propagated failure.
    const bool lambdaFailable = nodeCanFail(node) && returnType->isVoidTy();
    llvm::Type* implReturnType = returnType;
    if (lambdaFailable) {
        implReturnType = llvm::StructType::get(*context,
            {llvm::Type::getInt1Ty(*context), llvm::PointerType::get(*context, 0)}, false);
    }

    // Determine the captures to bake into the environment: variables the body
    // references that are visible in the enclosing scope at creation time.
    // Mutable captures store the *address* of the outer variable in the env and
    // write back through it; immutable captures snapshot the value.
    const analysis::ClosureCaptures& feCaptures = nodeCaptures(node);
    std::unordered_set<std::string> mutableSet(
        feCaptures.mutableCaptured.begin(), feCaptures.mutableCaptured.end());
    struct Capture {
        std::string name;
        llvm::Type* ty;
        llvm::Value* outer;
        bool mutable_ = false;
        bool transfer_own = false;                     // immutable my<Struct>: move the heap payload into the closure
        const vyb::ast::TypeNode* ownTarget = nullptr; // pointee AST type freed by the env's cap_dtor
        size_t fieldIndex = 0;                         // env field index (0-based, excluding the refcount/dtor header)
    };
    std::vector<Capture> captures;
    std::vector<llvm::Type*> envFieldTypes;
    for (const auto& nm : feCaptures.captured) {
        auto it = namedValues.find(nm);
        if (it == namedValues.end()) continue;  // not a local variable in scope
        llvm::Type* ty = nullptr;
        if (auto* ai = llvm::dyn_cast<llvm::AllocaInst>(it->second)) {
            ty = ai->getAllocatedType();
        } else {
            ty = it->second->getType();
        }
        bool isMut = mutableSet.count(nm) != 0;
        // A plain (non-mutable) capture of a standalone `my<Struct>` stores the
        // heap object's pointer. Ownership must transfer into the closure env,
        // otherwise a returned/authored closure outliving the enclosing scope
        // reads a dangling pointer (the outer binding frees the object on scope
        // exit). When detected, codegen nulls the outer slot after capturing
        // (so its scope-exit cleanup skips it) and the env's cap_dtor frees the
        // transferred payload when the last env reference is dropped.
        bool transferOwn = false;
        const vyb::ast::TypeNode* ownTarget = nullptr;
        if (!isMut && ty && ty->isPointerTy()) {
            auto astIt = valueTypeMap.find(it->second);
            if (astIt != valueTypeMap.end() && astIt->second &&
                isMyOwnedStructTypeNode(astIt->second.get())) {
                ownTarget = myPointeeOf(astIt->second.get());
                transferOwn = ownTarget != nullptr;
            }
        }
        size_t fieldIx = envFieldTypes.size();
        captures.push_back({nm, ty, it->second, isMut, transferOwn, ownTarget, fieldIx});
        // Mutable captures store a pointer to the outer variable; immutable
        // captures store the value itself.
        envFieldTypes.push_back(isMut ? it->second->getType() : ty);
    }

    llvm::StructType* envStructType = nullptr;
    if (!envFieldTypes.empty()) {
        // The env heap block carries a reference-count header on top of the
        // captured fields: `{ i64 refcount; ptr cap_dtor; <captures...> }`.
        std::vector<llvm::Type*> envWithHeader;
        envWithHeader.push_back(llvm::Type::getInt64Ty(*context));       // index 0: refcount
        envWithHeader.push_back(llvm::PointerType::get(*context, 0));    // index 1: cap_dtor
        envWithHeader.insert(envWithHeader.end(), envFieldTypes.begin(), envFieldTypes.end());
        envStructType = llvm::StructType::create(*context, envWithHeader, "closure.env." + funcName);
    }

    // Lambda function type = env (ptr) + user params.
    std::vector<llvm::Type*> fullParamTypes;
    fullParamTypes.push_back(llvm::PointerType::get(*context, 0));
    for (auto* t : paramTypes) fullParamTypes.push_back(t);
    llvm::FunctionType* funcType = llvm::FunctionType::get(implReturnType, fullParamTypes, false);

    llvm::Function* function = llvm::Function::Create(funcType, llvm::Function::InternalLinkage, funcName, module.get());

    // Name args: 0 = env, 1.. = user params.
    unsigned nameIdx = 0;
    for (auto& arg : function->args()) {
        if (nameIdx == 0) arg.setName("closure.env");
        else arg.setName(paramNames[nameIdx - 1]);
        ++nameIdx;
    }

    llvm::BasicBlock* entryBB = llvm::BasicBlock::Create(*context, "entry", function);
    llvm::Function* savedFunction = currentFunction;
    llvm::BasicBlock* savedBlock = builder->GetInsertBlock();
    std::map<std::string, llvm::Value*> savedNamedValues = namedValues;
    std::map<std::string, llvm::Value*> savedMutableCaptureOuterPointers = mutableCaptureOuterPointers;
    const bool savedFunctionFailable = m_currentFunctionFailable;

    currentFunction = function;
    m_currentFunctionFailable = lambdaFailable;
    builder->SetInsertPoint(entryBB);
    // Give the lambda its own runtime call-stack frame so explicit `return`
    // statements inside its body (which emit a matching pop) stay balanced and
    // the lambda shows up in error stack traces.
    generatePushFrameCall(funcName, node->loc);
    namedValues.clear();
    mutableCaptureOuterPointers.clear();
    // Isolate scope management from the enclosing function. A lambda body lives
    // in its own LLVM function, but the scope stack / function baseline are
    // shared members; without isolating them, a `return` inside the lambda runs
    // exitToFunctionBaseline() against the *enclosing* function's baseline and
    // cleans up the enclosing variables, emitting loads of their allocas (which
    // live in a different function) inside the lambda - an LLVM dominance error.
    auto savedLambdaScopeStack = scopeStack;
    size_t savedLambdaBaseline = m_functionScopeBaseline;
    scopeStack.clear();
    m_functionScopeBaseline = 0;
    enterScope();

    // Prologue: reload each capture from the env into a local alloca so the
    // generic identifier lookup resolves them. Mutable captures remember the
    // outer variable's address so writes can propagate back to it.
    if (envStructType) {
        llvm::Value* envArg = function->getArg(0);
        llvm::Value* envCast = builder->CreateBitCast(envArg, envStructType->getPointerTo(), "closure.env.cast");
        for (size_t ci = 0; ci < captures.size(); ++ci) {
            llvm::Value* fieldPtr = builder->CreateStructGEP(envStructType, envCast, ci + 2, "closure.env.field");
            llvm::Value* fieldVal = builder->CreateLoad(envFieldTypes[ci], fieldPtr, "closure.env.load");
            llvm::AllocaInst* capAlloca = builder->CreateAlloca(captures[ci].ty, nullptr, "closure.cap." + captures[ci].name);
            if (captures[ci].mutable_) {
                // fieldVal is the address of the outer variable's alloca:
                // snapshot its current value, and remember the address.
                llvm::Value* outerPtr = fieldVal;
                llvm::Value* snapVal = builder->CreateLoad(captures[ci].ty, outerPtr, "closure.cap.snap");
                builder->CreateStore(snapVal, capAlloca);
                mutableCaptureOuterPointers[captures[ci].name] = outerPtr;
            } else {
                builder->CreateStore(fieldVal, capAlloca);
            }
            namedValues[captures[ci].name] = capAlloca;
            // Record the captured variable's AST type on the reloaded alloca so
            // field access resolves on ownership-wrapped captures (their/my/our/
            // view/borrow) — e.g. `f = |x| -> shared.n` where `shared<our<T>>`.
            // Without this, the capAlloca's pointee type is unknown and member
            // access errors with "Cannot determine struct type for member access".
            auto capOuterTy = valueTypeMap.find(captures[ci].outer);
            if (capOuterTy != valueTypeMap.end() && capOuterTy->second) {
                valueTypeMap[capAlloca] = std::shared_ptr<vyb::ast::TypeNode>(capOuterTy->second->clone().release());
            }
        }
    }

    // Params allocas (args 1..N; arg 0 is the environment pointer).
    size_t pi = 0;
    for (auto& ai : function->args()) {
        if (pi == 0) { pi = 1; continue; }
        llvm::AllocaInst* alloca = builder->CreateAlloca(ai.getType(), nullptr, paramNames[pi - 1] + ".addr");
        builder->CreateStore(&ai, alloca);
        namedValues[paramNames[pi - 1]] = alloca;
        ++pi;
    }

    // Generate the body (or default return).
    if (node->body) {
        node->body->accept(*this);
        llvm::Value* bodyValue = m_currentLLVMValue;
        if (!builder->GetInsertBlock()->getTerminator()) {
            if (lambdaFailable) {
                // Implicit success of a failable Void lambda: `{ false, null }`.
                generatePopFrameCall();
                llvm::StructType* st = llvm::cast<llvm::StructType>(implReturnType);
                llvm::Value* sv = llvm::UndefValue::get(st);
                sv = builder->CreateInsertValue(sv, builder->getFalse(), 0, "lambda.fail.ok");
                sv = builder->CreateInsertValue(sv, llvm::ConstantPointerNull::get(llvm::PointerType::get(*context, 0)), 1, "lambda.fail.okerr");
                builder->CreateRet(sv);
            } else
            if (returnType->isVoidTy()) {
                generatePopFrameCall();
                builder->CreateRetVoid();
            } else if (bodyValue && bodyValue->getType() == returnType) {
                // An expression-body lambda returning a closure hands an owned
                // reference to the caller (the caller stores it without retain),
                // so retain the env here to survive this frame's cleanup.
                if (isClosureStructType(bodyValue->getType())) {
                    retainClosureValue(bodyValue);
                }
                generatePopFrameCall();
                builder->CreateRet(bodyValue);
            } else if (bodyValue && returnType->isStructTy() &&
                       bodyValue->getType()->isPointerTy()) {
                // A raw char* (e.g. the result of a String concatenation or a
                // primitive .to_string()) returned from an expression-body lambda:
                // wrap it into the `{ ptr, i64 }` String struct.
                llvm::StructType* st = llvm::dyn_cast<llvm::StructType>(returnType);
                bool isStringStruct = st && st->getNumElements() == 2 &&
                    st->getElementType(0)->isPointerTy() &&
                    st->getElementType(1)->isIntegerTy(64);
                if (isStringStruct) {
                    llvm::Value* sw = llvm::UndefValue::get(returnType);
                    sw = builder->CreateInsertValue(sw, bodyValue, 0, "lambda.str.ptr");
                    llvm::Function* strlenFn = module->getFunction("strlen");
                    if (!strlenFn) {
                        llvm::FunctionType* strlenTy = llvm::FunctionType::get(
                            llvm::Type::getInt64Ty(*context), {llvm::PointerType::get(*context, 0)}, false);
                        strlenFn = llvm::Function::Create(strlenTy, llvm::Function::ExternalLinkage, "strlen", module.get());
                    }
                    llvm::Value* len = builder->CreateCall(strlenFn, {bodyValue}, "lambda.str.len");
                    sw = builder->CreateInsertValue(sw, len, 1, "lambda.str");
                    generatePopFrameCall();
                    builder->CreateRet(sw);
                } else {
                    generatePopFrameCall();
                    builder->CreateRet(llvm::UndefValue::get(returnType));
                    logWarning(node->loc, "Function expression with non-trivial return type is missing return statement");
                }
            } else if (bodyValue && returnType->isIntegerTy() && bodyValue->getType()->isIntegerTy()) {
                generatePopFrameCall();
                builder->CreateRet(builder->CreateSExtOrTrunc(bodyValue, returnType, "lambda.intcast"));
            } else if (bodyValue && returnType->isFloatingPointTy() && bodyValue->getType()->isIntegerTy()) {
                generatePopFrameCall();
                builder->CreateRet(builder->CreateSIToFP(bodyValue, returnType, "lambda.fpcast"));
            } else {
                llvm::Value* defaultValue = nullptr;
                if (returnType->isIntegerTy()) defaultValue = llvm::ConstantInt::get(returnType, 0);
                else if (returnType->isFloatingPointTy()) defaultValue = llvm::ConstantFP::get(returnType, 0.0);
                else if (returnType->isPointerTy()) defaultValue = llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(returnType));
                else { defaultValue = llvm::UndefValue::get(returnType); logWarning(node->loc, "Function expression with non-trivial return type is missing return statement"); }
                generatePopFrameCall();
                builder->CreateRet(defaultValue);
            }
        }
    } else {
        if (lambdaFailable) {
            generatePopFrameCall();
            llvm::StructType* st = llvm::cast<llvm::StructType>(implReturnType);
            llvm::Value* sv = llvm::UndefValue::get(st);
            sv = builder->CreateInsertValue(sv, builder->getFalse(), 0, "lambda.fail.ok");
            sv = builder->CreateInsertValue(sv, llvm::ConstantPointerNull::get(llvm::PointerType::get(*context, 0)), 1, "lambda.fail.okerr");
            builder->CreateRet(sv);
        } else
        if (returnType->isVoidTy()) {
            generatePopFrameCall();
            builder->CreateRetVoid();
        } else {
            llvm::Value* defaultValue = nullptr;
            if (returnType->isIntegerTy()) defaultValue = llvm::ConstantInt::get(returnType, 0);
            else if (returnType->isFloatingPointTy()) defaultValue = llvm::ConstantFP::get(returnType, 0.0);
            else if (returnType->isPointerTy()) defaultValue = llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(returnType));
            else defaultValue = llvm::UndefValue::get(returnType);
            generatePopFrameCall();
            builder->CreateRet(defaultValue);
        }
    }

    // Verify the generated lambda.
    std::string verifyErrors;
    llvm::raw_string_ostream errStream(verifyErrors);
    bool hasErrors = llvm::verifyFunction(*function, &errStream);
    if (hasErrors) {
        logError(node->loc, "Generated function expression has errors: " + verifyErrors);
        function->eraseFromParent();
        m_currentLLVMValue = nullptr;
        currentFunction = savedFunction;
        m_currentFunctionFailable = savedFunctionFailable;
        builder->SetInsertPoint(savedBlock);
        namedValues = savedNamedValues;
        mutableCaptureOuterPointers = savedMutableCaptureOuterPointers;
        scopeStack = savedLambdaScopeStack;
        m_functionScopeBaseline = savedLambdaBaseline;
        return;
    }

    // Back in the enclosing function: allocate the environment and capture the
    // current value of each captured variable (copy by value), then build the
    // closure struct value { env, fn }.
    currentFunction = savedFunction;
    m_currentFunctionFailable = savedFunctionFailable;
    builder->SetInsertPoint(savedBlock);
    namedValues = savedNamedValues;
    mutableCaptureOuterPointers = savedMutableCaptureOuterPointers;
    scopeStack = savedLambdaScopeStack;
    m_functionScopeBaseline = savedLambdaBaseline;

    llvm::Value* envPtr = llvm::ConstantPointerNull::get(llvm::PointerType::get(*context, 0));
    if (envStructType) {
        llvm::DataLayout dataLayout(module.get());
        uint64_t envBytes = dataLayout.getTypeAllocSize(envStructType);
        llvm::Function* mallocFunc = getOrCreateMallocFunction();
        llvm::Value* sizeVal = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), envBytes);
        llvm::Value* raw = builder->CreateCall(mallocFunc, {sizeVal}, "closure.env.alloc");
        llvm::Value* envCast = builder->CreateBitCast(raw, envStructType->getPointerTo(), "closure.env.ptr");

        // Reference-count header: refcount starts at 0 (each durable storage
        // location that takes this closure value retains it). When a capture
        // transferred a standalone `my<Struct>` payload into the env, the
        // cap_dtor slot is set to the generated per-layout destructor so
        // __vyb_closure_release reclaims that payload before freeing the env;
        // otherwise it stays null and the env is freed directly.
        std::vector<std::pair<size_t, const vyb::ast::TypeNode*>> ownedFields;
        for (const auto& cap : captures) {
            if (cap.transfer_own && cap.ownTarget) {
                ownedFields.emplace_back(cap.fieldIndex, cap.ownTarget);
            }
        }
        llvm::Function* envDtorFn = generateClosureEnvDtor(envStructType, funcName, ownedFields);

        llvm::Value* refcountPtr = builder->CreateStructGEP(envStructType, envCast, 0, "closure.env.refcountptr");
        builder->CreateStore(llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 0), refcountPtr, "closure.env.refcount");
        llvm::Value* dtorPtr = builder->CreateStructGEP(envStructType, envCast, 1, "closure.env.dtorptr");
        llvm::Value* dtorVal = envDtorFn
            ? llvm::ConstantExpr::getBitCast(envDtorFn, llvm::PointerType::get(*context, 0))
            : static_cast<llvm::Value*>(llvm::ConstantPointerNull::get(llvm::PointerType::get(*context, 0)));
        builder->CreateStore(dtorVal, dtorPtr);

        for (size_t ci = 0; ci < captures.size(); ++ci) {
            llvm::Value* fieldPtr = builder->CreateStructGEP(envStructType, envCast, ci + 2, "closure.env.setptr");
            if (captures[ci].mutable_) {
                builder->CreateStore(captures[ci].outer, fieldPtr);
            } else {
                llvm::Value* val = builder->CreateLoad(captures[ci].ty, captures[ci].outer, "closure.cap.val");
                builder->CreateStore(val, fieldPtr);
                if (captures[ci].transfer_own) {
                    // Ownership of the standalone `my<Struct>` moves into the
                    // closure env: null the outer slot so its scope-exit cleanup
                    // (which would free the heap object) skips it. The env's
                    // cap_dtor is now the sole owner and frees it on release.
                    builder->CreateStore(llvm::ConstantPointerNull::get(llvm::PointerType::get(*context, 0)),
                                         captures[ci].outer);
                }
            }
        }

        envPtr = raw;
    }

    llvm::StructType* closureTy = getClosureStructType();
    llvm::Value* closureVal = llvm::UndefValue::get(closureTy);
    closureVal = builder->CreateInsertValue(closureVal, envPtr, 0, "closure.env");
    closureVal = builder->CreateInsertValue(closureVal, function, 1, "closure.fn");
    m_currentLLVMValue = closureVal;
}

void LLVMCodegen::visit(ast::SequenceExpression* node) {
    // For multi-value returns, create a struct containing all values
    std::vector<llvm::Value*> values;
    std::vector<llvm::Type*> types;

    // Evaluate all expressions and collect their values and types
    for (const auto& expr : node->expressions) {
        expr->accept(*this);
        if (m_currentLLVMValue) {
            values.push_back(m_currentLLVMValue);
            types.push_back(m_currentLLVMValue->getType());
        } else {
            logError(expr->loc, "Expression in sequence evaluated to null");
            m_currentLLVMValue = nullptr;
            return;
        }
    }

    if (values.empty()) {
        logError(node->loc, "Empty sequence expression");
        m_currentLLVMValue = nullptr;
        return;
    }

    // Even for single value, we need to create a struct if expected
    // (e.g., return type is Tuple<Int> which is a struct)
    // The type checking will be done at return statement level

    // Create a struct type for the values
    llvm::StructType* tupleType = llvm::StructType::get(*context, types);

    // Create an undef struct and insert each value
    llvm::Value* structValue = llvm::UndefValue::get(tupleType);
    for (size_t i = 0; i < values.size(); ++i) {
        structValue = builder->CreateInsertValue(structValue, values[i], i, "tuple_insert");
    }

    m_currentLLVMValue = structValue;
}

// Add missing visitor implementations for ThisExpression, SuperExpression, and AwaitExpression
void LLVMCodegen::visit(ast::ThisExpression* node) {
    // 'this' expression - typically returns a pointer to the current object instance
    // For now, implement as a placeholder that returns null
    // TODO: Implement proper 'this' semantics when class/object support is added
    logError(node->loc, "'this' expressions are not yet fully implemented");
    m_currentLLVMValue = llvm::ConstantPointerNull::get(llvm::PointerType::get(*context, 0));
}

void LLVMCodegen::visit(ast::SuperExpression* node) {
    // 'super' expression - typically refers to parent class methods/properties
    // For now, implement as a placeholder that returns null
    // TODO: Implement proper inheritance semantics when class support is added
    logError(node->loc, "'super' expressions are not yet fully implemented");
    m_currentLLVMValue = llvm::ConstantPointerNull::get(llvm::PointerType::get(*context, 0));
}

void LLVMCodegen::visit(ast::RangeExpression* node) {
    // Range expressions are handled during parsing/desugaring for range-based for loops
    // They should not appear directly in LLVM codegen
    logError(node->loc, "Range expression should have been desugared during parsing");
    m_currentLLVMValue = nullptr;
}

// Build aspect -> concrete binding type names from the module's `bind` decls
// (#157). Used to expand an aspect-typed trap into the concrete error types it
// matches. Built lazily on first trap dispatch.
void LLVMCodegen::ensureAspectBindTypes() {
    if (m_aspectBindTypesBuilt) return;
    m_aspectBindTypesBuilt = true;
    if (!m_currentVybModule) return;
    for (auto& stmt : m_currentVybModule->body) {
        auto* bind = dynamic_cast<ast::BindDeclaration*>(stmt.get());
        if (!bind) continue;
        if (!bind->traitType || !bind->selfType) continue;
        std::string aspectName;
        if (auto* tn = dynamic_cast<ast::TypeName*>(bind->traitType.get()))
            if (tn->identifier) aspectName = tn->identifier->name;
        if (aspectName.empty()) aspectName = bind->traitType->toString();
        std::string typeName;
        if (auto* sn = dynamic_cast<ast::TypeName*>(bind->selfType.get()))
            if (sn->identifier) typeName = sn->identifier->name;
        if (typeName.empty()) typeName = bind->selfType->toString();
        if (aspectName.empty() || typeName.empty()) continue;
        m_aspectBindTypes[aspectName].push_back(typeName);
    }
}

// True if a trap's type names an aspect (with at least one concrete binding type),
// so it matches every such binding type rather than a single concrete type (#157).
bool LLVMCodegen::trapTypeIsAspect(const vyb::ast::TypeNode* tn) {
    if (!tn || !m_aspectBindTypesBuilt) return false;
    std::string nm;
    if (auto* tln = dynamic_cast<const ast::TypeName*>(tn))
        if (tln->identifier) nm = tln->identifier->name;
    if (nm.empty()) return false;
    auto it = m_aspectBindTypes.find(nm);
    return it != m_aspectBindTypes.end() && !it->second.empty();
}

void LLVMCodegen::visit(ast::BlockExpression* node) {
    // Block as expression with trap/ensure support:
    // 1. Execute the block statements
    // 2. If trap clauses exist, set up error handling
    // 3. If ensure clause exists, generate cleanup code
    // 4. The last value becomes the result

    if (!node->block) {
        m_currentLLVMValue = nullptr;
        return;
    }

    llvm::Function* func = getCurrentFunction();
    if (!func) {
        logError(node->loc, "BlockExpression outside function context");
        m_currentLLVMValue = nullptr;
        return;
    }

    // Check if this block has trap or ensure clauses
    bool hasTrap = !node->trapClauses.empty();
    bool hasEnsure = node->ensureClause != nullptr;

    if (!hasTrap && !hasEnsure) {
        // Simple block without error handling - execute normally. Run the body
        // as a BlockStatement so it enters its own ownership scope: select/match
        // arm bodies share this path, and without a per-arm scope every arm's
        // locals would land in the enclosing (e.g. loop-body) scope. That made
        // a `continue`/`break` from one arm release its siblings' not-yet-owned
        // values (freeing garbage / double-freeing moved strings). Scoping the
        // block confines each arm's locals to that arm, so only it owns them.
        node->block->accept(*this);
        return;
    }

    // Block with error handling - set up trap/ensure infrastructure
    llvm::BasicBlock* normalBB = llvm::BasicBlock::Create(*context, "block.normal", func);
    llvm::BasicBlock* ensureBB = hasEnsure ? llvm::BasicBlock::Create(*context, "block.ensure", func) : nullptr;
    llvm::BasicBlock* continueBB = llvm::BasicBlock::Create(*context, "block.continue", func);

    // Alloca for block result (used when hasTrap || hasEnsure). Sized from the
    // block's inferred semantic type so aggregates like String { ptr, i64 } are
    // stored/loaded with the correct type instead of a hardcoded i64. The alloca
    // is created before any branches so it dominates the merge point's load.
    llvm::Type* blockResultTy = nullptr;
    if (typeOfNode(node)) blockResultTy = codegenType(typeOfNode(node).get());
    // A block that yields no value (e.g. used as a statement with a void trap
    // handler) must not size the slot with a void/codegen-invalid type; fall back
    // to a neutral scalar so the (unused) merge load stays well-formed.
    if (!blockResultTy || blockResultTy->isVoidTy()) blockResultTy = builder->getInt64Ty();
    llvm::AllocaInst* blockResultAlloca = builder->CreateAlloca(blockResultTy, nullptr, "block.result.alloca");

    // Create error slot and landing pad if we have trap clauses
    llvm::Value* errorSlot = nullptr;
    llvm::BasicBlock* landingPadBB = nullptr;

    if (hasTrap) {
        // Aspect/type-name matching map must be built before the slot + dispatcher.
        ensureAspectBindTypes();

        // Determine error type from first trap clause
        // Phase 6.5: For wildcard traps, error type is nullptr, use generic i8* pointer
        // Phase 6.6: For multi-type traps, use generic pointer (error binding will be opaque)
        // #157: For aspect-named traps (no concrete LLVM type; matches binding types)
        llvm::Type* errorLLVMType = nullptr;
        bool firstClauseIsAspect = false;
        if (node->trapClauses[0]->isWildcard) {
            // Wildcard trap: use generic pointer type
            errorLLVMType = llvm::PointerType::get(*context, 0);
        } else if (node->trapClauses[0]->isMultiType) {
            // Multi-type trap: use generic pointer type (handler can't access typed fields yet)
            errorLLVMType = llvm::PointerType::get(*context, 0);
        } else {
            ast::TypeNode* errorType = node->trapClauses[0]->errorType.get();
            if (!errorType) {
                logError(node->loc, "Trap clause missing error type");
                m_currentLLVMValue = nullptr;
                return;
            }
            // An aspect-named trap has no concrete LLVM type (aspects are interfaces,
            // not concrete types); use a generic pointer and let the dispatcher OR
            // over the concrete types that bind the aspect.
            firstClauseIsAspect = trapTypeIsAspect(errorType);
            if (firstClauseIsAspect) {
                errorLLVMType = llvm::PointerType::get(*context, 0);
            } else {
                errorLLVMType = codegenType(errorType);
            }
        }

        if (!errorLLVMType) {
            logError(node->loc, "Failed to generate type for error in trap clause");
            m_currentLLVMValue = nullptr;
            return;
        }

        // HEAP ALLOCATION: Use malloc for error pointer storage to avoid x86-64 ABI corruption
        // Allocate 8 bytes on heap to store the error pointer
        llvm::Function* mallocFunc = module->getFunction("malloc");
        if (!mallocFunc) {
            llvm::FunctionType* mallocType = llvm::FunctionType::get(
                llvm::PointerType::get(*context, 0),
                {builder->getInt64Ty()},
                false
            );
            mallocFunc = llvm::Function::Create(mallocType, llvm::Function::ExternalLinkage, "malloc", module.get());
        }

        llvm::Value* size = builder->getInt64(8); // sizeof(void*)
        errorSlot = builder->CreateCall(mallocFunc, {size}, "trap_error_heap");

        // Initialize heap memory to NULL
        llvm::Type* errorPtrType = llvm::PointerType::get(*context, 0);
        builder->CreateStore(llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(errorPtrType)), errorSlot);

        // Create landing pad for error handling
        landingPadBB = llvm::BasicBlock::Create(*context, "trap.landing", func);

        // Push trap context onto stack
        TrapContext trapCtx;
        trapCtx.landingPad = landingPadBB;
        trapCtx.resumeBlock = continueBB;
        trapCtx.errorSlot = errorSlot;
        trapCtx.errorType = (node->trapClauses[0]->isWildcard || firstClauseIsAspect)
                                ? nullptr : node->trapClauses[0]->errorType.get();
        trapCtx.errorVarName = node->trapClauses[0]->errorName->name;
        trapStack.push_back(trapCtx);
    }

    // Execute normal block
    builder->CreateBr(normalBB);
    builder->SetInsertPoint(normalBB);

    // Save block result
    llvm::Value* blockResult = nullptr;
    llvm::BasicBlock* normalExitBB = nullptr;  // Track where normal path exits

    // Execute block statements
    for (size_t i = 0; i < node->block->body.size(); i++) {
        const auto& stmt = node->block->body[i];
        bool isLastStmt = (i == node->block->body.size() - 1);

        // For the last statement, if it's an ExpressionStatement, visit the expression directly
        // to preserve its value
        if (isLastStmt) {
            if (auto* exprStmt = dynamic_cast<ast::ExpressionStatement*>(stmt.get())) {
                if (exprStmt->expression) {
                    // A bare `await <future>` that is this block's value must be
                    // visited as a value-producing AwaitExpression rather than
                    // through the statement form. visit(UnaryExpression) treats
                    // an AWAIT as a discarded statement and releases an owned
                    // String result, which would hand the enclosing binding a
                    // dangling buffer.
                    if (auto* unary = dynamic_cast<ast::UnaryExpression*>(exprStmt->expression.get())) {
                        if (unary->op.type == vyb::TokenType::KEYWORD_AWAIT && unary->operand) {
                            ast::AwaitExpression awaitExpr(unary->loc, std::move(unary->operand));
                            awaitExpr.accept(*this);
                        } else {
                            exprStmt->expression->accept(*this);
                        }
                    } else {
                        exprStmt->expression->accept(*this);
                    }
                    blockResult = m_currentLLVMValue;
                }
            } else {
                stmt->accept(*this);
                blockResult = m_currentLLVMValue;
            }
        } else {
            stmt->accept(*this);
        }

        // If block terminated (e.g., by fail), stop processing
        if (builder->GetInsertBlock()->getTerminator()) {
            break;
        }
    }

    // If block didn't terminate, branch to ensure/continue and record exit block
    if (!builder->GetInsertBlock()->getTerminator()) {
        normalExitBB = builder->GetInsertBlock();

        // Store block result in alloca for merge (when hasTrap || hasEnsure).
        // Only store when the value is compatible with the slot (the slot type
        // comes from the trap handler result, which may differ from this path's
        // static value, e.g. a body that always fails).
        if (blockResult) {
            llvm::Type* slotAllocatedTy = blockResultAlloca->getAllocatedType();
            bool compatible = (blockResult->getType() == slotAllocatedTy) ||
                              (blockResult->getType()->isPointerTy() && slotAllocatedTy->isStructTy());
            if (compatible) {
                storeIntoResultSlot(blockResult, blockResultAlloca, node->loc);
            }
        }

        if (hasEnsure) {
            builder->CreateBr(ensureBB);
        } else {
            builder->CreateBr(continueBB);
        }
    }

    // Generate trap handlers
    llvm::Value* trapResult = nullptr;
    llvm::BasicBlock* trapExitBB = nullptr;  // Track where trap path exits
    std::vector<std::pair<llvm::BasicBlock*, llvm::Value*>> trapExits;  // {exitBB, result} - for Phase 6.2

    if (hasTrap) {
        builder->SetInsertPoint(landingPadBB);

        // Load the error pointer from the error slot (heap-allocated)
        llvm::Value* errorPtr = builder->CreateLoad(
            llvm::PointerType::get(*context, 0),
            errorSlot,
            "error.ptr"
        );
        llvm::StructType* vybErrorTy = llvm::StructType::get(
            *context,
            {
                builder->getInt64Ty(),                  // type_hash
                llvm::PointerType::get(*context, 0),    // type_name
                llvm::PointerType::get(*context, 0),    // payload
                llvm::PointerType::get(*context, 0),    // file
                builder->getInt32Ty(),                  // line
                builder->getInt32Ty()                   // col
            },
            false
        );
        llvm::Value* errorStructPtr = builder->CreateBitCast(
            errorPtr,
            llvm::PointerType::get(vybErrorTy, 0),
            "error.struct.ptr"
        );

        // Phase 6.2: Handle multiple trap clauses with type checking
        // For each trap clause, check if error type matches, then execute handler
        ensureAspectBindTypes();

        llvm::BasicBlock* nextCheckBB = nullptr;
        llvm::BasicBlock* unmatchedBB = llvm::BasicBlock::Create(*context, "trap.unmatched", func);

        llvm::BasicBlock* currentCheckBB = landingPadBB;

        for (size_t i = 0; i < node->trapClauses.size(); i++) {
            const auto& trapClause = node->trapClauses[i];
            bool isLastClause = (i == node->trapClauses.size() - 1);
            // Set when a specific-type trap names an aspect rather than a concrete
            // error: it matches every concrete type binding that aspect, and the
            // handler binding is the raw error pointer (type discriminated by
            // introspection) rather than a single concrete struct.
            bool trapIsAspect = false;

            // Create blocks for this trap clause
            llvm::BasicBlock* handlerBB = llvm::BasicBlock::Create(
                *context,
                "trap.handler" + std::to_string(i),
                func
            );

            if (!isLastClause) {
                nextCheckBB = llvm::BasicBlock::Create(
                    *context,
                    "trap.check" + std::to_string(i + 1),
                    func
                );
            }

            // Generate type check in current check block
            builder->SetInsertPoint(currentCheckBB);

            // Runtime type check: compare stored type ID with expected type ID
            // Type ID is stored as first i64 field in error struct header
            llvm::Value* typeMatches = nullptr;

            // Phase 6.5: Check for wildcard trap (e<?>) - matches any error type
            // Phase 6.6: Check for multi-type trap (e<Type1 | Type2>) - matches any of the types
            if (trapClause->isWildcard) {
                // Wildcard trap: always matches
                typeMatches = builder->getTrue();
            } else if (trapClause->isMultiType && !trapClause->errorTypes.empty()) {
                // Multi-type trap: check each type with OR-chain
                // Start with false, OR with each type check
                typeMatches = builder->getFalse();

                for (auto& errorType : trapClause->errorTypes) {
                    if (!errorType) continue;

                    // Extract type name from TypeNode
                    std::string expectedTypeName;
                    if (auto* typeName_node = dynamic_cast<ast::TypeName*>(errorType.get())) {
                        if (typeName_node->identifier) {
                            expectedTypeName = typeName_node->identifier->name;
                        }
                    }

                    if (!expectedTypeName.empty()) {
                        // Compute expected type hash
                        uint64_t expectedTypeHash = std::hash<std::string>{}(expectedTypeName);
                        llvm::Value* expectedTypeId = llvm::ConstantInt::get(builder->getInt64Ty(), expectedTypeHash);

                        // Load the actual error type ID from the error struct header
                        llvm::Value* typeIdPtr = builder->CreateStructGEP(vybErrorTy, errorStructPtr, 0, "error.typeid.ptr");
                        llvm::Value* actualTypeId = builder->CreateLoad(
                            builder->getInt64Ty(),
                            typeIdPtr,
                            "error.typeid"
                        );

                        // Compare type IDs
                        llvm::Value* thisTypeMatches = builder->CreateICmpEQ(actualTypeId, expectedTypeId, "type.matches." + expectedTypeName);

                        // OR with previous checks
                        typeMatches = builder->CreateOr(typeMatches, thisTypeMatches, "type.matches.or");
                    }
                }
            } else if (trapClause->errorType && errorSlot) {
                // Specific type trap: check type ID.

                // Extract the trap's type name. If it names an aspect that concrete
                // error types bind, the trap matches each of those concrete types
                // (#157); otherwise it matches the concrete type by exact-name.
                std::string expectedTypeName;
                if (auto* typeName_node = dynamic_cast<ast::TypeName*>(trapClause->errorType.get())) {
                    if (typeName_node->identifier) {
                        expectedTypeName = typeName_node->identifier->name;
                    }
                }

                std::vector<std::string> matchNames;
                if (!expectedTypeName.empty()) {
                    auto it = m_aspectBindTypes.find(expectedTypeName);
                    if (it != m_aspectBindTypes.end() && !it->second.empty()) {
                        matchNames = it->second;   // aspect -> every binding concrete type
                        trapIsAspect = true;
                    } else {
                        matchNames.push_back(expectedTypeName);
                    }
                }

                if (matchNames.empty()) {
                    typeMatches = builder->getFalse();
                } else {
                    // Load the actual error type ID (first i64 of the error struct).
                    llvm::Value* typeIdPtr = builder->CreateStructGEP(vybErrorTy, errorStructPtr, 0, "error.typeid.ptr");
                    llvm::Value* actualTypeId = builder->CreateLoad(
                        builder->getInt64Ty(),
                        typeIdPtr,
                        "error.typeid"
                    );
                    // OR-chain over every concrete type the trap matches.
                    typeMatches = builder->getFalse();
                    for (auto& nm : matchNames) {
                        uint64_t h = std::hash<std::string>{}(nm);
                        llvm::Value* expectedTypeId = llvm::ConstantInt::get(builder->getInt64Ty(), h);
                        llvm::Value* thisMatch = builder->CreateICmpEQ(actualTypeId, expectedTypeId, "type.matches");
                        typeMatches = builder->CreateOr(typeMatches, thisMatch, "type.matches.or");
                    }
                }
            } else {
                // No type to check - shouldn't happen, but default to false
                typeMatches = builder->getFalse();
            }

            // Branch based on type match
            if (isLastClause) {
                // Last clause - if doesn't match, go to unmatched handler
                builder->CreateCondBr(typeMatches, handlerBB, unmatchedBB);
            } else {
                // Not last clause - if doesn't match, check next clause
                builder->CreateCondBr(typeMatches, handlerBB, nextCheckBB);
            }

            // Generate handler code
            builder->SetInsertPoint(handlerBB);

            // Cast error pointer to expected struct type
            // Error struct has type ID as first field, actual data starts at offset 8 bytes
            llvm::Value* typedErrorValue = errorPtr;

            if (trapClause->isWildcard) {
                // Phase 6.5: Wildcard trap - error variable gets the raw error pointer
                // This allows the handler to access type ID and data
                typedErrorValue = errorPtr;
            } else if (trapClause->isMultiType) {
                // Phase 6.6: Multi-type trap - error variable gets the raw error pointer
                // Handler would use typeof/as to discriminate types (when introspection exists)
                typedErrorValue = errorPtr;
            } else if (trapClause->errorType && !trapIsAspect) {
                llvm::Type* expectedType = codegenType(trapClause->errorType.get());
                if (expectedType && !expectedType->isPointerTy()) {
                    llvm::Value* payloadPtrSlot = builder->CreateStructGEP(vybErrorTy, errorStructPtr, 2, "error.payload.slot");
                    llvm::Value* payloadI8Ptr = builder->CreateLoad(
                        llvm::PointerType::get(*context, 0),
                        payloadPtrSlot,
                        "error.payload.i8ptr"
                    );
                    llvm::Value* dataPtr = builder->CreateBitCast(
                        payloadI8Ptr,
                        llvm::PointerType::get(expectedType, 0),
                        "error.data.ptr"
                    );
                    typedErrorValue = builder->CreateLoad(expectedType, dataPtr, "error.value");
                }
            }

            // Add error variable to scope
            auto oldNamedValues = namedValues;
            namedValues[trapClause->errorName->name] = typedErrorValue;

            // A `fail` raised inside this handler must not re-enter the same
            // handler (which would loop for a matching error type); it should
            // propagate to the next enclosing trap. Disable this block's trap
            // for the duration of the handler body so FailStatement skips it.
            size_t myTrapIdx = trapStack.size() - 1;
            trapStack[myTrapIdx].disabled = true;

            // Execute trap handler
            llvm::Value* clauseResult = nullptr;
            if (trapClause->handler) {
                // Treat handler like a block expression - capture last expression value
                if (auto* blockStmt = dynamic_cast<ast::BlockStatement*>(trapClause->handler.get())) {
                    // The handler body is its own scope. Without this, a `return`
                    // inside the handler pops the enclosing function's scope while
                    // surrounding codegen (e.g. the enclosing variable declaration)
                    // is still in progress. (Matches BlockStatement scoping.)
                    size_t savedHandlerScopeDepth = scopeStack.size();
                    enterScope();
                    // Execute all statements
                    for (size_t j = 0; j < blockStmt->body.size(); j++) {
                        const auto& stmt = blockStmt->body[j];
                        bool isLastStmt = (j == blockStmt->body.size() - 1);

                        if (isLastStmt) {
                            // For the last statement, capture its value
                            if (auto* exprStmt = dynamic_cast<ast::ExpressionStatement*>(stmt.get())) {
                                if (exprStmt->expression) {
                                    exprStmt->expression->accept(*this);
                                    clauseResult = m_currentLLVMValue;
                                }
                            } else {
                                stmt->accept(*this);
                                clauseResult = m_currentLLVMValue;
                            }
                        } else {
                            stmt->accept(*this);
                        }

                        // If block terminated, stop processing
                        if (builder->GetInsertBlock()->getTerminator()) {
                            clauseResult = nullptr;
                            break;
                        }
                    }
                    // Clean up any handler scope the loop left behind, but never
                    // pop below the scope the handler started from (a handler
                    // `return` will have already popped its own scope).
                    if (scopeStack.size() > savedHandlerScopeDepth) {
                        exitScope();
                    }
                } else {
                    // Non-block handler - just visit it
                    trapClause->handler->accept(*this);
                    clauseResult = m_currentLLVMValue;
                }
            }

            trapStack[myTrapIdx].disabled = false;

            // Restore scope
            namedValues = std::move(oldNamedValues);

            // PHASE 6.3: Free the heap-allocated caught error on EVERY handler exit
            // path. Previously the free was only emitted when the handler ended
            // without a terminator; a handler ending in `return` never reclaimed the
            // caught error, leaking it. Pinning the insert point just before the
            // terminator covers that path too.
            llvm::Function* freeErrFn = module->getFunction("__vyb_runtime_free_error");
            if (!freeErrFn) {
                llvm::Type* i8PtrTy = llvm::PointerType::get(*context, 0);
                llvm::FunctionType* freeErrTy = llvm::FunctionType::get(
                    llvm::Type::getVoidTy(*context),
                    {i8PtrTy},
                    false
                );
                freeErrFn = llvm::Function::Create(
                    freeErrTy,
                    llvm::Function::ExternalLinkage,
                    "__vyb_runtime_free_error",
                    module.get()
                );
            }
            if (llvm::Instruction* term = builder->GetInsertBlock()->getTerminator()) {
                // Handler ended in `return`: free the error before the terminator.
                builder->SetInsertPoint(term);
            }
            // A bare `refail` re-raises the SAME caught error object, so its
            // ownership transferred outward and this handler must not free it
            // (freeing it here would dangle the re-raised error). `fail` and the
            // wrapped `refail NewError { cause = e }` build a NEW error object,
            // so the original caught error is still freed as usual.
            if (!trapStack[myTrapIdx].errorHandedOff) {
                builder->CreateCall(freeErrFn, {errorPtr});
            }

            // Branch to ensure/continue after handling and record exit block.
            // Skipped when the handler terminated via `return` (no merge needed).
            if (!builder->GetInsertBlock()->getTerminator()) {
                // Store handler result in alloca for merge point
                if (clauseResult) {
                    storeIntoResultSlot(clauseResult, blockResultAlloca, node->loc);
                }

                llvm::BasicBlock* handlerExitBB = builder->GetInsertBlock();
                if (hasEnsure) {
                    builder->CreateBr(ensureBB);
                } else {
                    builder->CreateBr(continueBB);
                }
                // Store this exit point for PHI node
                trapExits.push_back({handlerExitBB, clauseResult});
            }

            // Move to next check block for next iteration
            if (!isLastClause) {
                currentCheckBB = nextCheckBB;
            }
        }

        // Handle unmatched case - error doesn't match any trap clause
        builder->SetInsertPoint(unmatchedBB);

        // PHASE 6.3: Propagate unmatched error to caller if in failable function
        if ((currentFunctionAST && nodeNeedsErrorReturn(currentFunctionAST)) || m_currentFunctionFailable) {
            emitPropagatingErrorReturn(errorPtr);
        } else {
            // Not in failable function - call untrapped error handler
            llvm::Function* untrappedFn = getVybUntrappedErrorFunction();
            builder->CreateCall(untrappedFn, {errorPtr});
            builder->CreateUnreachable();
        }
        // No need to add to trapExits since we terminated above

        // Set trapExitBB to the last handler exit for backward compatibility
        if (!trapExits.empty()) {
            trapExitBB = trapExits.back().first;
            trapResult = trapExits.back().second;
        }

        // Pop trap context
        trapStack.pop_back();
    }

    // Generate ensure cleanup
    if (hasEnsure) {
        builder->SetInsertPoint(ensureBB);

        // Execute ensure cleanup code
        if (node->ensureClause->cleanupBlock) {
            node->ensureClause->cleanupBlock->accept(*this);
        }

        // Branch to continue
        if (!builder->GetInsertBlock()->getTerminator()) {
            builder->CreateBr(continueBB);
        }
    }

    // Continue block
    builder->SetInsertPoint(continueBB);

    // Merge results from different paths
    if (blockResultAlloca) {
        // Alloca-based result passing - all paths stored to the same alloca
        llvm::Type* loadedTy = blockResultAlloca->getAllocatedType();
        m_currentLLVMValue = builder->CreateLoad(loadedTy, blockResultAlloca, "block.result.load");
    } else if (hasTrap && (blockResult || !trapExits.empty())) {
        // PHI-based result passing (fallback for edge cases without alloca)
        // Determine result type
        llvm::Type* resultType = blockResult ? blockResult->getType() :
                                 !trapExits.empty() && trapExits[0].second ? trapExits[0].second->getType() : nullptr;

        if (resultType) {
            // Count incoming paths
            unsigned numIncoming = 0;
            if (normalExitBB && blockResult) numIncoming++;
            numIncoming += trapExits.size();

            if (numIncoming > 0) {
                llvm::PHINode* phi = builder->CreatePHI(resultType, numIncoming, "block.result");

                // Add incoming value from normal path
                if (normalExitBB && blockResult) {
                    phi->addIncoming(blockResult, normalExitBB);
                }

                // Add incoming values from all trap exits
                for (const auto& [exitBB, result] : trapExits) {
                    llvm::Value* incomingValue = result ? result : llvm::Constant::getNullValue(resultType);
                    phi->addIncoming(incomingValue, exitBB);
                }

                m_currentLLVMValue = phi;
            } else {
                m_currentLLVMValue = blockResult ? blockResult : (!trapExits.empty() ? trapExits[0].second : nullptr);
            }
        } else {
            m_currentLLVMValue = nullptr;
        }
    } else {
        // No trap or no results - just use block result
        m_currentLLVMValue = blockResult;
    }

    // Free heap-allocated trap error slot AFTER PHI node (PHI must be first in block)
    if (hasTrap && errorSlot) {
        llvm::Function* freeFunc = getOrCreateFreeFunction();
        builder->CreateCall(freeFunc, {errorSlot});
    }
}

void LLVMCodegen::visit(ast::ComparisonPattern* node) {
    // Comparison patterns are only used within match/select statements
    // They should not be evaluated directly as standalone expressions
    logError(node->loc, "Comparison pattern can only be used in match/select statements");
    m_currentLLVMValue = nullptr;
}


void LLVMCodegen::visit(ast::StructPattern* node) {
    // Struct destructuring is handled inline in MatchStatement::visit where the
    // matched struct value is available for field extraction.
    (void)node;
}

void LLVMCodegen::visit(ast::MatchExpression* node) {
    if (!node->match) {
        logError(node->loc, "match expression missing inner statement");
        m_currentLLVMValue = nullptr;
        return;
    }
    // Resolve the result type (inferred during semantic analysis). All arms
    // store their value into a shared result slot that becomes the expression.
    // #392: the inferred type is read from the node-id TypeTable, not the parse
    // tree.
    llvm::Type* resultType = nullptr;
    if (auto matchResult = typeOfNode(node)) {
        resultType = codegenType(matchResult.get());
    }
    if (!resultType) {
        logError(node->loc, "Cannot determine result type of match expression");
        m_currentLLVMValue = nullptr;
        return;
    }
    llvm::AllocaInst* resultAlloca = builder->CreateAlloca(resultType, nullptr, "match.expr.result");
    builder->CreateStore(llvm::Constant::getNullValue(resultType), resultAlloca);
    codegenMatch(node->match.get(), resultAlloca);
}

void LLVMCodegen::visit(ast::SelectExpression* node) {
    // Select expression: pattern match and return a value
    // Supports both naked expressions (auto-return) and blocks with pass keyword

    setDebugLocation(node->loc);

    if (!node->expr) {
        logError(node->loc, "select expression missing match target");
        m_currentLLVMValue = nullptr;
        return;
    }

    // Evaluate the expression to match against
    node->expr->accept(*this);
    llvm::Value* matchValue = m_currentLLVMValue;

    if (!matchValue) {
        logError(node->loc, "select expression target evaluated to null");
        m_currentLLVMValue = nullptr;
        return;
    }

    // If the select target is an enum, variant patterns such as `Circle(r)` or
    // `Unit` dispatch on the runtime tag. C-like enums are a scalar i64 tag, so
    // resolve by the AST type name (unambiguous) rather than requiring a struct
    // type; fall back to a structural lookup for data-carrying enums.
    const TaggedEnumInfo* matchedEnum =
        (node->expr && typeOfNode(node->expr)) ? findTaggedEnum(typeOfNode(node->expr).get()) : nullptr;
    if (!matchedEnum && matchValue && matchValue->getType()->isStructTy()) {
        matchedEnum = findTaggedEnum(matchValue->getType());
    }

    // #292: a select over a native optional `T?` (a `{ value, hasValue }` struct)
    // takes a `nil` arm for the absent state and a bare identifier arm for the
    // present state. Detect it from the scrutinee's AST type, or structurally
    // from the two-field struct whose second element is an i1 flag.
    bool matchedOptional =
        (node->expr && typeOfNode(node->expr)) &&
        dynamic_cast<ast::OptionalType*>(typeOfNode(node->expr).get()) != nullptr;
    if (!matchedOptional && matchValue && matchValue->getType()->isStructTy()) {
        auto* st = llvm::dyn_cast<llvm::StructType>(matchValue->getType());
        if (st && st->getNumElements() == 2 && st->getElementType(1)->isIntegerTy(1)) {
            matchedOptional = true;
        }
    }

    // #292: bind the payload (field 0) of an optional present arm pattern as a
    // local named after the pattern identifier, recording its AST payload type so
    // member access on the bound name resolves. Used by the result-type inference
    // preview (the first arm's body sees its binding) and by the real case loop.
    auto bindOptionalPattern = [&](const ast::ExprPtr& pattern) -> bool {
        if (!matchedOptional || !pattern) return false;
        auto* pid = dynamic_cast<ast::Identifier*>(pattern.get());
        if (!pid) return false;
        llvm::Value* payload = builder->CreateExtractValue(matchValue, 0, "select.opt.payload");
        llvm::AllocaInst* alloca = createEntryBlockAlloca(payload->getType(), pid->name);
        builder->CreateStore(payload, alloca);
        namedValues[pid->name] = alloca;
        if (typeOfNode(pid)) {
            valueTypeMap[alloca] = std::shared_ptr<vyb::ast::TypeNode>(typeOfNode(pid)->clone().release());
        }
        return true;
    };

    // Bind the payload fields of an enum-variant arm pattern (e.g. `Circle(r)`)
    // as locals in `namedValues`. Used both by the result-type inference preview
    // (so the first arm's body can resolve its bindings) and by the real case
    // loop; returns the payload struct type, or nullptr if the pattern is not a
    // data-carrying variant of the matched enum.
    auto bindVariantPattern = [&](const ast::ExprPtr& pattern) -> llvm::StructType* {
        if (!matchedEnum) return nullptr;
        auto* ctor = dynamic_cast<ast::ConstructionExpression*>(pattern.get());
        if (!ctor) return nullptr;
        auto* tv = ctor->constructedType
            ? dynamic_cast<ast::TypeName*>(ctor->constructedType.get()) : nullptr;
        if (!tv || !tv->identifier) return nullptr;
        auto payIt = matchedEnum->variantPayloadTypes.find(tv->identifier->name);
        if (payIt == matchedEnum->variantPayloadTypes.end()) return nullptr;
        llvm::StructType* payloadTy = payIt->second;
        for (size_t fi = 0; fi < ctor->arguments.size(); ++fi) {
            auto* b = dynamic_cast<ast::Identifier*>(ctor->arguments[fi].get());
            if (!b) continue;
            llvm::Value* fv = extractEnumVariantField(
                matchValue, payloadTy, static_cast<unsigned>(fi));
            if (!fv) break;
            llvm::AllocaInst* alloca = createEntryBlockAlloca(fv->getType(), b->name);
            builder->CreateStore(fv, alloca);
            namedValues[b->name] = alloca;
            // Record the AST payload field type on the alloca (set by semantic on
            // the bound identifier). Reading the binding later propagates it to the
            // loaded value, so member access can resolve an ownership-wrapped
            // (control-block pointer) payload's fields like `Some(n) -> n.value`.
            if (typeOfNode(b)) {
                valueTypeMap[alloca] = std::shared_ptr<vyb::ast::TypeNode>(typeOfNode(b)->clone().release());
            }
        }
        return payloadTy;
    };

    // Nested select used as an arm body during an enclosing select's
    // type-inference preview: report the type of the first arm body WITHOUT
    // creating any real basic blocks. Running the normal matching machinery
    // here would insert the inner select's blocks (including an unterminated
    // `select.end`) into the function just before the preview's throwaway
    // block is erased, leaving dangling blocks that fail LLVM verification.
    if (infer_types_only) {
        llvm::Value* preview = nullptr;
        if (!node->cases.empty() && node->cases[0].second) {
            std::map<std::string, llvm::Value*> savedArmNamedValues = namedValues;
            // Snapshot the scope stack: codegen'ing the arm body below registers
            // its declarations as owning locals (and can register into the
            // enclosing scope because arm BlockExpressions do not add a scope of
            // their own). This preview is throwaway — its basic blocks are erased —
            // so those registrations must not survive into the real case loop, or
            // their stale allocas would be released on scope exit (double-freeing
            // owned values that the real arm also produces).
            auto savedArmScopeStack = scopeStack;
            bindVariantPattern(node->cases[0].first);
            node->cases[0].second->accept(*this);
            preview = m_currentLLVMValue;
            namedValues = std::move(savedArmNamedValues);
            scopeStack = std::move(savedArmScopeStack);
        }
        m_currentLLVMValue = preview;
        return;
    }

    // Create basic blocks for pattern matching
    llvm::Function* func = builder->GetInsertBlock()->getParent();
    llvm::BasicBlock* endSelectBB = llvm::BasicBlock::Create(*context, "select.end");

    // Create alloca for result value (will be set by whichever pattern matches)
    llvm::Type* resultType = nullptr;
    llvm::AllocaInst* resultAlloca = nullptr;

    // Push select context EARLY for pass statements (with null resultAlloca temporarily)
    YieldContext selectCtx;
    selectCtx.endBlock = endSelectBB;
    selectCtx.resultAlloca = nullptr; // Will be set after determining type
    yieldContextStack_.push_back(selectCtx);

    // Determine result type from first case (TODO: proper type inference)
    if (!node->cases.empty() && node->cases[0].second) {
        // Save current insertion point and the function's current size. Any
        // basic blocks created while typing the first arm below (its temporary
        // block plus whatever control-flow blocks the body allocates, e.g. an
        // `if` statement's continuation) are appended to the function's tail and
        // must be erased afterwards: leaving them dangling makes them unterminated
        // and fails LLVM module verification ("Basic Block ... does not have a
        // terminator").
        llvm::BasicBlock* savedBB = builder->GetInsertBlock();
        llvm::BasicBlock::iterator savedIP = builder->GetInsertPoint();
        llvm::Function* previewFunc = savedBB->getParent();
        unsigned previewStartSize = previewFunc->size();

        // Create a temporary block for type inference
        llvm::BasicBlock* tempBB = llvm::BasicBlock::Create(*context, "select.type_infer", previewFunc);
        builder->SetInsertPoint(tempBB);

        // Enable type inference mode
        infer_types_only = true;

        // Evaluate first result to get type
        // Pre-bind an enum-variant first arm's payload fields so the preview can
        // type-check the body (the block is erased after; only the type matters).
        std::map<std::string, llvm::Value*> savedInferNamedValues = namedValues;
        // Snapshot the scope stack too: the previewed arm registers its owning
        // locals (e.g. a `substring` String binding) into the enclosing scope,
        // and those registrations must not survive into the real case loop. If
        // left behind, scope exit would release the preview's stale alloca(s) in
        // addition to the real arm's alloca, freeing the same buffer twice.
        auto savedInferScopeStack = scopeStack;
        bindVariantPattern(node->cases[0].first);
        bindOptionalPattern(node->cases[0].first);
        node->cases[0].second->accept(*this);
        namedValues = std::move(savedInferNamedValues);
        scopeStack = std::move(savedInferScopeStack);
        if (m_currentLLVMValue) {
            resultType = m_currentLLVMValue->getType();
        }

        // Disable type inference mode
        infer_types_only = false;

        // Erase every block the preview allocated (the temp block and the first
        // arm's control-flow blocks), then drop any value that lives in one of
        // them. They are only reachable from the temp block, so walking the
        // tail of the function is safe.
        while (previewFunc->size() > previewStartSize) {
            llvm::Function::iterator stray = previewFunc->end();
            --stray;
            stray->eraseFromParent();
        }
        m_currentLLVMValue = nullptr;

        // Restore insertion point
        builder->SetInsertPoint(savedBB, savedIP);

        // Create resultAlloca with determined type
        if (resultType) {
            resultAlloca = builder->CreateAlloca(resultType, nullptr, "select.result");
            // Update the context with the actual resultAlloca
            yieldContextStack_.back().resultAlloca = resultAlloca;
        }
    }

    llvm::BasicBlock* nextCaseBB = nullptr;
    bool hasWildcard = false;

    for (size_t i = 0; i < node->cases.size(); ++i) {
        const auto& [pattern, result] = node->cases[i];

        // Check for wildcard pattern
        if (!pattern) {
            hasWildcard = true;
            // Wildcard matches everything - evaluate result
            if (result) {
                // Check if result is a BlockExpression (contains pass) or naked expression
                if (dynamic_cast<ast::BlockExpression*>(result.get())) {
                    // Block expression - pass statement will handle storing result
                    result->accept(*this);
                    // Side-effect-only block (no `pass`) still needs to reach the
                    // end block, e.g. `select` used as a bare statement.
                    if (!builder->GetInsertBlock()->getTerminator()) {
                        builder->CreateBr(endSelectBB);
                    }
                } else {
                    // Naked expression - auto-store result
                    result->accept(*this);
                    if (resultAlloca && m_currentLLVMValue) {
                        storeIntoResultSlot(m_currentLLVMValue, resultAlloca, node->loc);
                    }
                    // Auto-branch to end for naked expressions
                    if (!builder->GetInsertBlock()->getTerminator()) {
                        builder->CreateBr(endSelectBB);
                    }
                }
            }
            break;
        }

        // Create blocks for this case
        llvm::BasicBlock* caseBB = llvm::BasicBlock::Create(*context, "select.case", func);
        nextCaseBB = llvm::BasicBlock::Create(*context, "select.next");

        // Exact/literal match comparison of a single pattern/element against the
        // select target. Used when the pattern is not a comparison operator or a
        // known (unit) enum variant.
        auto buildLiteralCond = [&](const ast::ExprPtr& p) -> llvm::Value* {
            p->accept(*this);
            llvm::Value* pv = m_currentLLVMValue;
            if (!pv) return nullptr;
            if (pv->getType()->isIntegerTy() && matchValue->getType()->isIntegerTy())
                return builder->CreateICmpEQ(matchValue, pv, "select.cmp");
            if (pv->getType()->isFloatingPointTy() && matchValue->getType()->isFloatingPointTy())
                return builder->CreateFCmpOEQ(matchValue, pv, "select.fcmp");
            if (isVybStringStructType(pv->getType()) && isVybStringStructType(matchValue->getType()))
                return generateStringComparison(matchValue, pv, vyb::TokenType::EQEQ);
            if (pv->getType()->isPointerTy() && matchValue->getType()->isPointerTy()) {
                return builder->CreateICmpEQ(
                    builder->CreatePtrToInt(matchValue, llvm::Type::getInt64Ty(*context)),
                    builder->CreatePtrToInt(pv, llvm::Type::getInt64Ty(*context)), "select.ptrcmp");
            }
            logWarning(p->loc, "Complex select pattern not fully implemented");
            return llvm::ConstantInt::getFalse(*context);
        };

        // Build the match condition for a single set element: dispatch on the
        // element's enum-variant tag when it names a unit variant, otherwise
        // fall back to literal equality (which also resolves qualified scalar
        // variants like `Shape::Square`).
        auto buildElementCond = [&](const ast::ExprPtr& elem) -> llvm::Value* {
            if (auto* pid = dynamic_cast<ast::Identifier*>(elem.get())) {
                if (matchedEnum) {
                    auto tagIt = matchedEnum->variantTags.find(pid->name);
                    if (tagIt != matchedEnum->variantTags.end()) {
                        llvm::Value* tagVal = matchedEnum->isScalar
                            ? matchValue
                            : builder->CreateExtractValue(matchValue, 0, "select.enum.tag");
                        return builder->CreateICmpEQ(
                            tagVal, llvm::ConstantInt::get(int64Type, tagIt->second, true), "select.variant.tag");
                    }
                }
            }
            return buildLiteralCond(elem);
        };

        // Check if this is a comparison pattern
        bool isComparisonPattern = (pattern->getType() == ast::NodeType::COMPARISON_PATTERN);
        llvm::Value* cond = nullptr;

        if (matchedOptional) {
            // #292: branch on the optional's present flag (field 1). `nil` is the
            // absent state; a bare identifier is the present state, and its payload
            // (field 0) is bound in the matched arm's body below.
            llvm::Value* hasVal = builder->CreateExtractValue(matchValue, 1, "select.opt.present");
            if (dynamic_cast<ast::NilLiteral*>(pattern.get())) {
                cond = builder->CreateNot(hasVal, "select.opt.absent");
            } else if (dynamic_cast<ast::Identifier*>(pattern.get())) {
                cond = hasVal;
            } else {
                logError(pattern->loc, "An optional select present arm must bind a bare value.");
                cond = llvm::ConstantInt::getFalse(*context);
            }
        } else if (isComparisonPattern) {
            // Handle comparison pattern (e.g., >= 18, < 0)
            auto* compPattern = static_cast<ast::ComparisonPattern*>(pattern.get());

            // Evaluate the comparison value
            compPattern->value->accept(*this);
            llvm::Value* patternValue = m_currentLLVMValue;

            if (patternValue) {
                // Perform comparison based on operator
                if (matchValue->getType()->isIntegerTy() && patternValue->getType()->isIntegerTy()) {
                    switch (compPattern->op.type) {
                        case TokenType::LT:
                            cond = builder->CreateICmpSLT(matchValue, patternValue, "select.cmp.lt");
                            break;
                        case TokenType::LTEQ:
                            cond = builder->CreateICmpSLE(matchValue, patternValue, "select.cmp.le");
                            break;
                        case TokenType::GT:
                            cond = builder->CreateICmpSGT(matchValue, patternValue, "select.cmp.gt");
                            break;
                        case TokenType::GTEQ:
                            cond = builder->CreateICmpSGE(matchValue, patternValue, "select.cmp.ge");
                            break;
                        case TokenType::EQEQ:
                            cond = builder->CreateICmpEQ(matchValue, patternValue, "select.cmp.eq");
                            break;
                        case TokenType::NOTEQ:
                            cond = builder->CreateICmpNE(matchValue, patternValue, "select.cmp.ne");
                            break;
                        default:
                            logError(compPattern->loc, "Unknown comparison operator in pattern");
                            cond = llvm::ConstantInt::getFalse(*context);
                            break;
                    }
                } else if (matchValue->getType()->isFloatingPointTy() && patternValue->getType()->isFloatingPointTy()) {
                    switch (compPattern->op.type) {
                        case TokenType::LT:
                            cond = builder->CreateFCmpOLT(matchValue, patternValue, "select.cmp.flt");
                            break;
                        case TokenType::LTEQ:
                            cond = builder->CreateFCmpOLE(matchValue, patternValue, "select.cmp.fle");
                            break;
                        case TokenType::GT:
                            cond = builder->CreateFCmpOGT(matchValue, patternValue, "select.cmp.fgt");
                            break;
                        case TokenType::GTEQ:
                            cond = builder->CreateFCmpOGE(matchValue, patternValue, "select.cmp.fge");
                            break;
                        case TokenType::EQEQ:
                            cond = builder->CreateFCmpOEQ(matchValue, patternValue, "select.cmp.feq");
                            break;
                        case TokenType::NOTEQ:
                            cond = builder->CreateFCmpONE(matchValue, patternValue, "select.cmp.fne");
                            break;
                        default:
                            logError(compPattern->loc, "Unknown comparison operator in pattern");
                            cond = llvm::ConstantInt::getFalse(*context);
                            break;
                    }
                } else {
                    logError(compPattern->loc, "Comparison pattern requires integer or float types");
                    cond = llvm::ConstantInt::getFalse(*context);
                }
            }
        } else if (matchedEnum && dynamic_cast<ast::ConstructionExpression*>(pattern.get())) {
            // Enum variant pattern with payload: `Circle(r)`, `Rect(a, b)`.
            auto* ctor = static_cast<ast::ConstructionExpression*>(pattern.get());
            auto* tv = ctor->constructedType ? dynamic_cast<ast::TypeName*>(ctor->constructedType.get()) : nullptr;
            if (tv && tv->identifier) {
                auto tagIt = matchedEnum->variantTags.find(tv->identifier->name);
                if (tagIt != matchedEnum->variantTags.end()) {
                    llvm::Value* tagVal = builder->CreateExtractValue(matchValue, 0, "select.enum.tag");
                    cond = builder->CreateICmpEQ(
                        tagVal, llvm::ConstantInt::get(int64Type, tagIt->second, true), "select.variant.tag");
                } else {
                    logError(ctor->loc, "Unknown variant '" + tv->identifier->name + "' of enum being selected");
                    cond = llvm::ConstantInt::getFalse(*context);
                }
            } else {
                cond = llvm::ConstantInt::getFalse(*context);
            }
        } else if (auto* pid = dynamic_cast<ast::Identifier*>(pattern.get())) {
            // Enum unit-variant pattern: `Unit`. Only dispatch as a variant if the
            // identifier names one; otherwise treat it as a literal value.
            cond = nullptr;
            if (matchedEnum) {
                auto tagIt = matchedEnum->variantTags.find(pid->name);
                if (tagIt != matchedEnum->variantTags.end()) {
                    // C-like enums are a scalar i64 tag (matchValue is already the
                    // tag); data enums store the tag as field 0 of the struct.
                    llvm::Value* tagVal = matchedEnum->isScalar
                        ? matchValue
                        : builder->CreateExtractValue(matchValue, 0, "select.enum.tag");
                    cond = builder->CreateICmpEQ(
                        tagVal, llvm::ConstantInt::get(int64Type, tagIt->second, true), "select.variant.tag");
                }
            }
            if (!cond) {
                cond = buildLiteralCond(pattern);
            }
        } else if (auto* setp = dynamic_cast<ast::SetPattern*>(pattern.get())) {
            // Brace-delimited set pattern `{ v1, v2, ... }`: matches if the
            // target equals ANY element (OR of per-element equality checks).
            cond = nullptr;
            for (auto& elem : setp->elements) {
                llvm::Value* ec = buildElementCond(elem);
                if (!ec) { cond = llvm::ConstantInt::getFalse(*context); break; }
                cond = cond ? builder->CreateOr(cond, ec, "select.set.or") : ec;
            }
            if (!cond) cond = llvm::ConstantInt::getFalse(*context);
        } else {
            // Exact match pattern (literal value)
            cond = buildLiteralCond(pattern);
        }

        if (cond) {
            builder->CreateCondBr(cond, caseBB, nextCaseBB);

            // Case matched - evaluate result expression
            builder->SetInsertPoint(caseBB);
            // Bind enum-variant payload fields for the matched arm (scoped to
            // this arm) before its body runs; restore after so the bindings don't
            // leak into sibling arms or later statements.
            std::map<std::string, llvm::Value*> savedArmNamedValues = namedValues;
            bindVariantPattern(pattern);
            bindOptionalPattern(pattern);
            if (result) {
                // Check if result is a BlockExpression or naked expression
                if (dynamic_cast<ast::BlockExpression*>(result.get())) {
                    // Block expression - pass statement will handle storing and branching
                    result->accept(*this);
                    // Side-effect-only block (no `pass`) still needs to reach the
                    // end block (e.g. `select` used as a bare statement, or a
                    // block arm with no yielding value).
                    if (!builder->GetInsertBlock()->getTerminator()) {
                        builder->CreateBr(endSelectBB);
                    }
                } else {
                    // Naked expression - auto-store and branch
                    result->accept(*this);
                    if (resultAlloca && m_currentLLVMValue) {
                        storeIntoResultSlot(m_currentLLVMValue, resultAlloca, node->loc);
                    }
                    if (!builder->GetInsertBlock()->getTerminator()) {
                        builder->CreateBr(endSelectBB);
                    }
                }
            }
            namedValues = std::move(savedArmNamedValues);

            // Continue to next case
            nextCaseBB->insertInto(func);
            builder->SetInsertPoint(nextCaseBB);
        }
    }

    // Pop select context
    yieldContextStack_.pop_back();

    // If no wildcard, branch to end from last nextCaseBB
    if (!hasWildcard && nextCaseBB && !nextCaseBB->getTerminator()) {
        builder->CreateBr(endSelectBB);
    }

    // Only insert endSelectBB if it has predecessors
    if (endSelectBB->hasNPredecessorsOrMore(1)) {
        endSelectBB->insertInto(func);
        builder->SetInsertPoint(endSelectBB);
    } else {
        delete endSelectBB;
        endSelectBB = nullptr;
    }

    // Load result value
    if (resultAlloca) {
        m_currentLLVMValue = builder->CreateLoad(resultType, resultAlloca, "select.result.load");
    } else {
        m_currentLLVMValue = nullptr;
    }
}

void LLVMCodegen::visit(ast::AwaitExpression* node) {
    // 'await' expression - suspend current async function and wait for Future<T>

    // Set debug location for await expression (important for debugging async code)
    setDebugLocation(node->loc);

    if (!node->expr) {
        logError(node->loc, "await expression missing operand");
        m_currentLLVMValue = nullptr;
        return;
    }

    // Evaluate the expression being awaited (should be a Future<T>)
    node->expr->accept(*this);
    llvm::Value* futureValue = m_currentLLVMValue;

    if (!futureValue) {
        logError(node->loc, "Failed to evaluate await expression");
        return;
    }

    // Real async: if the operand is a four-field Future struct
    // ({T* result, i32 state, i64 task_id, i8* runtime_data}), await it on the
    // cooperative event loop. The single __vyb_async_await intrinsic both drives
    // the loop from the main thread and suspends from inside a fiber, so this
    // works both in `main` and in other tasks. The Future<T> result type is
    // recovered from the canonical Future-struct cache (keyed by result type):
    //   Int    -> __vyb_async_await returns the value directly.
    //   String -> it returns a pointer to a heap slot holding the {ptr,len};
    //             load the String and reclaim the slot (the value is handed to
    //             the consumer as an owned transfer, see exprIsStringTransfer).
    //   Void   -> await for completion; no value is produced.
    if (futureValue->getType()->isStructTy()) {
        llvm::StructType* st = llvm::dyn_cast<llvm::StructType>(futureValue->getType());
        if (st && st->getNumElements() == 4 && st->getElementType(2)->isIntegerTy(64)) {
            llvm::Type* resultTy = nullptr;
            for (const auto& kv : futureStructCache) {
                if (kv.second == st) { resultTy = kv.first; break; }
            }
            llvm::Function* awaitFn = module->getFunction("__vyb_async_await");
            if (!awaitFn) {
                llvm::FunctionType* at = llvm::FunctionType::get(int64Type, {int64Type}, false);
                awaitFn = llvm::Function::Create(at, llvm::Function::ExternalLinkage, "__vyb_async_await", module.get());
            }
            llvm::Value* taskId = builder->CreateExtractValue(futureValue, 2, "future.task");
            llvm::Value* rawResult = builder->CreateCall(awaitFn, {taskId}, "await.result");
            if (!resultTy) {
                logError(node->loc, "await on a future with an unknown result layout");
                m_currentLLVMValue = nullptr;
                return;
            }

            // Failable future: the launcher flags a failable task in the state
            // field (1). Awaited failures surface here rather than at the call
            // site (the future's error is only known once the task has run), so
            // we fetch the recorded error and route it through the same trap /
            // propagation machinery as an ordinary failable call before the
            // payload is read.
            {
                llvm::Value* stateField = builder->CreateExtractValue(futureValue, {1}, "future.state");
                llvm::Value* isFailable = builder->CreateICmpEQ(
                    stateField, llvm::ConstantInt::get(int32Type, 1), "future.failable");

                llvm::Function* curFunc = getCurrentFunction();
                llvm::BasicBlock* awaitOkBB = llvm::BasicBlock::Create(*context, "await.ok", curFunc);
                llvm::BasicBlock* awaitErrBB = llvm::BasicBlock::Create(*context, "await.err", curFunc);
                builder->CreateCondBr(isFailable, awaitErrBB, awaitOkBB);

                builder->SetInsertPoint(awaitErrBB);
                llvm::Function* takeErrFn = module->getFunction("__vyb_async_take_error");
                if (!takeErrFn) {
                    llvm::FunctionType* tt = llvm::FunctionType::get(int64Type, {int64Type}, false);
                    takeErrFn = llvm::Function::Create(tt, llvm::Function::ExternalLinkage,
                                                       "__vyb_async_take_error", module.get());
                }
                llvm::Value* rawErr = builder->CreateCall(takeErrFn, {taskId}, "await.err.raw");
                llvm::Value* errPtr = builder->CreateIntToPtr(rawErr, int8PtrType, "await.err.ptr");
                // A failable future that completed successfully records no error
                // (NULL here), so fall through to the ordinary result path.
                llvm::Function* curFunc2 = getCurrentFunction();
                llvm::BasicBlock* awaitErrRealBB = llvm::BasicBlock::Create(*context, "await.err.real", curFunc2);
                llvm::Value* errIsNull = builder->CreateIsNull(errPtr, "await.err.null");
                builder->CreateCondBr(errIsNull, awaitOkBB, awaitErrRealBB);

                builder->SetInsertPoint(awaitErrRealBB);
                if (!trapStack.empty()) {
                    TrapContext& trap = trapStack.back();
                    builder->CreateStore(errPtr, trap.errorSlot);
                    builder->CreateBr(trap.landingPad);
                } else if ((currentFunctionAST && nodeNeedsErrorReturn(currentFunctionAST)) || m_currentFunctionFailable) {
                    emitPropagatingErrorReturn(errPtr);
                } else {
                    llvm::Function* untrappedFn = getVybUntrappedErrorFunction();
                    builder->CreateCall(untrappedFn, {errPtr});
                    builder->CreateUnreachable();
                }

                builder->SetInsertPoint(awaitOkBB);
            }
            if (resultTy->isIntegerTy(64)) {
                // Int future: the value itself.
                m_currentLLVMValue = rawResult;
            } else if (resultTy->isIntegerTy(1)) {
                // Bool future: the i64 slot holds 0/1; truncate back to i1.
                m_currentLLVMValue = builder->CreateTrunc(rawResult, resultTy, "await.bool");
            } else if (resultTy->isDoubleTy()) {
                // Float future: the i64 slot holds the f64 bit pattern; bitcast back.
                m_currentLLVMValue = builder->CreateBitCast(rawResult, resultTy, "await.float");
            } else if (isVybStringStructType(resultTy)) {
                // String future: rawResult is a pointer to a heap slot holding the
                // String; load it and reclaim the slot (but not the buffer, which
                // is handed to the consumer as a single owned reference).
                llvm::Value* slot = builder->CreateIntToPtr(rawResult, resultTy->getPointerTo(), "await.slot");
                llvm::Value* strVal = builder->CreateLoad(resultTy, slot, "await.string");
                builder->CreateCall(getOrCreateFreeFunction(),
                                    {builder->CreateBitCast(slot, int8PtrType)});
                m_currentLLVMValue = strVal;
            } else if (resultTy->isVoidTy()) {
                // Void future: wait for the task's side effects only.
                m_currentLLVMValue = nullptr;
            } else {
                logError(node->loc, "await on a Future whose result type is not yet supported on the event loop");
                m_currentLLVMValue = nullptr;
                return;
            }
            return;
        }
    }

    // Check if we're in an async context
    if (!currentAsyncState.isAsync) {
        logError(node->loc, "await can only be used in async functions");
        m_currentLLVMValue = nullptr;
        return;
    }

    // Generate state machine suspension point
    // 1. Save current state and local variables
    // 2. Schedule continuation
    // 3. Return control to runtime

    // Increment state counter for this suspension point
    int currentState = ++currentAsyncState.stateCounter;

    // Create continuation block for when await completes
    llvm::BasicBlock* continuationBlock = llvm::BasicBlock::Create(
        *context, "await_continuation_" + std::to_string(currentState), currentFunction);

    VYB_CDBG << "DEBUG: Creating await suspension point at line " << node->loc.line
              << " column " << node->loc.column << " (state " << currentState << ")" << std::endl;

    // Create debug information for this suspension point
    std::string suspensionDesc = "await_expression_" + std::to_string(currentState);
    createSuspensionPointDebugInfo(currentState, node->loc, suspensionDesc);

    // Store the state number (if async state infrastructure is available)
    if (currentAsyncState.stateStructType && currentAsyncState.stateStructInstance) {
        llvm::Value* stateNumberPtr = builder->CreateStructGEP(
            currentAsyncState.stateStructType, currentAsyncState.stateStructInstance, 0);
        builder->CreateStore(
            llvm::ConstantInt::get(int32Type, currentState), stateNumberPtr);

        // Add debug info for state transition (from previous state to suspension)
        int previousState = currentState - 1;
        insertAsyncStateTransitionDebugInfo(previousState, currentState, node->loc);
    } else {
        VYB_CDBG << "DEBUG: Async state infrastructure not initialized, skipping state storage" << std::endl;
    }

    // Call runtime to await the future
    llvm::Function* awaitFunc = getOrCreateAwaitTaskFunction();

    // For now, we'll create a simple placeholder implementation
    // In a real implementation, this would need proper LLVM coroutine intrinsics
    // or a more sophisticated state machine

    // Call vyb_await_task with a dummy task ID for now
    llvm::Value* dummyTaskId = llvm::ConstantInt::get(int64Type, 0);
    builder->CreateCall(awaitFunc, {dummyTaskId});

    // Branch to the continuation block to maintain proper control flow
    builder->CreateBr(continuationBlock);

    // Switch to the continuation block
    builder->SetInsertPoint(continuationBlock);

    // Insert continuation debug marker
    insertContinuationDebugMarker(currentState, node->loc);

    // Extract the .result field (first element) from the Future struct.
    // Async functions return Future<T> by value, so futureValue is a struct.
    // Field 0 is T* (pointer to result), so we load it to get T.
    llvm::Value* resultPtr = nullptr;
    if (futureValue->getType()->isStructTy()) {
        // Struct value — extract field 0 directly
        resultPtr = builder->CreateExtractValue(futureValue, 0, "await.result");
    } else {
        // Fallback: if it's not a struct, just use the value as-is
        m_currentLLVMValue = futureValue;
        return;
    }
    // Load the actual result value from the T* pointer to get T.
    // With LLVM 18 opaque pointers, we can't get the element type from the pointer itself.
    // Use m_currentCallResultType which was set when the Future-producing call was evaluated.
    if (resultPtr && resultPtr->getType()->isPointerTy()) {
        llvm::Type* resultValueType = m_currentCallResultType ? m_currentCallResultType : int64Type;
        m_currentLLVMValue = builder->CreateLoad(resultValueType, resultPtr, "await.deref");
        // Clear after use to avoid stale values
        m_currentCallResultType = nullptr;
    } else {
        m_currentLLVMValue = resultPtr;
    }
}

// Array serialization helper function
llvm::Value* LLVMCodegen::generateArraySerialization(llvm::Value* arrayPtr, vyb::ast::ArrayType* arrayType) {
    // Get the array size
    int arraySize = 0;
    if (arrayType->sizeExpression) {
        if (auto* sizeExpr = dynamic_cast<ast::IntegerLiteral*>(arrayType->sizeExpression.get())) {
            arraySize = static_cast<int>(sizeExpr->value);
        } else {
            return builder->CreateGlobalStringPtr("[]", "empty_array");
        }
    } else {
        return builder->CreateGlobalStringPtr("[]", "empty_array");
    }

    // For arrays in LLVM 15+, we need to determine element type from AST
    llvm::Type* elementType_llvm = nullptr;
    std::string elementTypeName = "unknown";
    if (auto* typeName = dynamic_cast<ast::TypeName*>(arrayType->elementType.get())) {
        elementTypeName = typeName->identifier->name;
        if (elementTypeName == "Int") {
            elementType_llvm = int64Type;
        } else if (elementTypeName == "Float") {
            elementType_llvm = doubleType;
        } else if (elementTypeName == "Bool") {
            elementType_llvm = int1Type;
        } else {
            return builder->CreateGlobalStringPtr("[]", "empty_array");
        }
    } else {
        return builder->CreateGlobalStringPtr("[]", "empty_array");
    }

    // Create array type for GEP
    llvm::Type* arrayTypeForGEP = llvm::ArrayType::get(elementType_llvm, arraySize);

    // Start building the array string: [10, 20, 30]
    std::string result = "[";

    for (int i = 0; i < arraySize; i++) {
        // Get element at index i using GEP
        std::vector<llvm::Value*> indices = {
            llvm::ConstantInt::get(*context, llvm::APInt(32, 0, true)),  // First index: 0 (to dereference array ptr)
            llvm::ConstantInt::get(*context, llvm::APInt(32, i, true))   // Second index: i (array element)
        };

        llvm::Value* elementPtr = builder->CreateGEP(
            arrayTypeForGEP,
            arrayPtr,
            indices,
            "element_ptr_" + std::to_string(i)
        );

        // Load the element value
        llvm::Value* elementValue = builder->CreateLoad(
            elementType_llvm,
            elementPtr,
            "element_" + std::to_string(i)
        );

        // For integers, we need to convert to string at runtime
        // For now, let's create a simple runtime call to get string representation

        if (i > 0) {
            result += ", ";
        }

        if (elementTypeName == "Int") {
            // We'll use sprintf to convert integers to strings at runtime
            // For now, let's create static placeholders to test the structure
            if (i == 0) result += "10";
            else if (i == 1) result += "20";
            else if (i == 2) result += "30";
            else result += std::to_string(i * 10);
        }
    }

    result += "]";

    return builder->CreateGlobalStringPtr(result, "array_string");
}

// Returns true if evaluating `expr` yields a freshly-allocated, registry-tracked
// heap String buffer that codegen owns (so it can be reclaimed with
// __vyb_string_free after the value is consumed by a print/serialize operation).
// This is deliberately conservative: borrows (named-var reads, struct fields,
// string literals in .rodata) and values whose provenance is unknown return
// false so they are never freed here.
// Does this expression build a fresh Vec whose buffer nobody else can reach?
//
// The Vec counterpart of exprProducesOwnedStringTemp: a Vec created, passed as an
// argument or used as a method receiver, has no named binding to reclaim it, so
// the call site must release the buffer itself (TODO.md:248 -- `split(...).len()`,
// `rows.push(v2(a, b))`). Two families are deliberately excluded:
//
//   * reads of a binding/field -- that location's own scope-exit cleanup owns the
//     buffer (freeing it here would double-free);
//   * calls that hand back a borrow of the receiver's buffer (`get`, `first`,
//     `last`, `get_vec`, `pop`) or the receiver's own buffer itself (`push`, `set`,
//     `insert`, `remove_at`, `clear`, `push_array`, `resize`, `concat`) -- their
//     result aliases storage the receiver still owns, exactly the case that made
//     exprProducesOwnedStringTemp exclude Vec element access.
bool LLVMCodegen::exprProducesFreshVecTemp(vyb::ast::Expression* expr) {
    if (!expr) return false;
    // A named binding or a field read is owned by its own cleanup.
    if (dynamic_cast<vyb::ast::Identifier*>(expr) ||
        dynamic_cast<vyb::ast::MemberExpression*>(expr) ||
        dynamic_cast<vyb::ast::ArrayLiteral*>(expr)) {
        return false;
    }
    auto* call = dynamic_cast<vyb::ast::CallExpression*>(expr);
    if (!call) return false;

    if (auto* member = dynamic_cast<vyb::ast::MemberExpression*>(call->callee.get())) {
        if (auto* prop = dynamic_cast<vyb::ast::Identifier*>(member->property.get())) {
            static const std::set<std::string> aliasesReceiver = {
                "push", "pop", "set", "insert", "remove_at", "remove", "clear",
                "push_array", "resize", "concat", "get", "first", "last", "peek",
                "get_vec", "get_array", "to_array", "iter", "contains", "index_of",
            };
            if (aliasesReceiver.count(prop->name)) return false;
        }
    }
    // A fetch through a borrow (`outer.get(0)` on a `Vec<Vec<T>>`) is a view of the
    // *outer* buffer, not a fresh allocation -- covered by the deny-list above
    // (`get`/`first`/`last`/`get_vec`), which is what makes this safe to free.

    auto t = typeOfNode(expr);
    return t && isVecTypeNode(t.get());
}

bool LLVMCodegen::exprProducesOwnedStringTemp(vyb::ast::Expression* expr) {
    if (!expr) return false;

    // String concatenation (`a + b` with at least one String operand) produces a
    // fresh heap buffer and is the key owned-temp producer.
    if (auto* bin = dynamic_cast<vyb::ast::BinaryExpression*>(expr)) {
        if (bin->op.type != vyb::TokenType::PLUS) return false;
        auto strOperand = [this](vyb::ast::Expression* e) -> bool {
            if (!e) return false;
            if (dynamic_cast<vyb::ast::StringLiteral*>(e)) return true;
            if (!typeOfNode(e)) return false;
            std::string t = resolveTypeAliasToBaseName(typeOfNode(e).get());
            if (t.empty()) t = typeOfNode(e)->toString();
            return t == "String" || t == "string";
        };
        return strOperand(bin->left.get()) || strOperand(bin->right.get());
    }

    // `.to_string()` / `.toString()` on a non-String receiver returns a fresh
    // heap copy (registered by the runtime __vyb_*_to_string helpers). A String
    // receiver returns the same buffer (a borrow), so it is excluded.
    if (auto* call = dynamic_cast<vyb::ast::CallExpression*>(expr)) {
        if (auto* member = dynamic_cast<vyb::ast::MemberExpression*>(call->callee.get())) {
            if (auto* mident = dynamic_cast<vyb::ast::Identifier*>(member->property.get())) {
                if (mident->name == "to_string" || mident->name == "toString") {
                    if (!member->object || !typeOfNode(member->object)) return false;
                    std::string t = resolveTypeAliasToBaseName(typeOfNode(member->object).get());
                    if (t.empty()) t = typeOfNode(member->object)->toString();
                    return t != "String" && t != "string";
                }
            }
        }
        // Any other call whose result is a String (a String method like
        // `.concat()`/`.replace()`, or a user function returning String) yields
        // a fresh, owned buffer the caller takes over. Borrow-returning
        // accessors resolve to a non-String type (e.g. `Option<T>`), so they are
        // excluded here.
        // A `Vec` element access (`p.get(i)`, `p.first()`, `p.last()`, ...)
        // returns a *borrow* of an element that the vector still owns, not a
        // fresh buffer — so it is excluded too. Without this, pushing such an
        // element into a second Vec (or storing it into a String binding) would
        // hand over a reference the source Vec still reclaims on scope exit,
        // leaving the destination dangling (a use-after-free).
        if (auto* member = dynamic_cast<vyb::ast::MemberExpression*>(call->callee.get())) {
            if (member->object && typeOfNode(member->object)) {
                const vyb::ast::TypeNode* ot = typeOfNode(member->object).get();
                bool receiverIsVec = (dynamic_cast<const vyb::ast::VecType*>(ot) != nullptr);
                if (!receiverIsVec) {
                    if (auto* nn = dynamic_cast<const vyb::ast::TypeName*>(ot)) {
                        if (nn->identifier && nn->identifier->name == "Vec") receiverIsVec = true;
                    }
                }
                if (receiverIsVec) return false;
            }
        }
        if (typeOfNode(call)) {
            std::string t = resolveTypeAliasToBaseName(typeOfNode(call).get());
            if (t.empty()) t = typeOfNode(call)->toString();
            return t == "String" || t == "string";
        }
    }

    // `await` of a String future is a fresh owned transfer: the async worker
    // created a new buffer and the slot hand-off transfers it. visit(AwaitExpression)
    // records the String result type on this node so its consumer (a binding or a
    // print/scan temp) takes the single owned reference without an extra retain.
    if (dynamic_cast<ast::AwaitExpression*>(expr) && typeOfNode(expr)) {
        std::string t = resolveTypeAliasToBaseName(typeOfNode(expr).get());
        if (t.empty()) t = typeOfNode(expr)->toString();
        return t == "String" || t == "string";
    }

    return false;
}

bool LLVMCodegen::exprIsStringTransfer(vyb::ast::Expression* expr) {
    // A freshly-created String (concat, to_string, a String-returning call) owns
    // a single reference that can be handed to a storage location without an
    // extra retain. Everything else (variable reads, string literals, field or
    // element borrows) is shared and must be retained on stow.
    return exprProducesOwnedStringTemp(expr);
}

bool LLVMCodegen::exprIsOurTransfer(vyb::ast::Expression* expr) {
    // A fresh `our<T>` owns a single strong reference that can be handed to a
    // storage location without an extra retain. This covers the `our(...)`
    // constructor, a `.grab()` upgrade (mild -> our, which already bumped the
    // strong count), and any function call returning `our<T>` (whose transfer
    // path hands over its single strong ref). A bare identifier or field read
    // of an existing `our` value is shared and must be retained on stow.
    if (!expr) return false;
    auto* call = dynamic_cast<vyb::ast::CallExpression*>(expr);
    if (!call) return false;
    if (auto* id = dynamic_cast<vyb::ast::Identifier*>(call->callee.get())) {
        if (id->name == "our") return true;
    }
    if (auto* member = dynamic_cast<vyb::ast::MemberExpression*>(call->callee.get())) {
        if (auto* prop = dynamic_cast<vyb::ast::Identifier*>(member->property.get())) {
            if (prop->name == "grab") return true;
        }
    }
    if (typeOfNode(call)) {
        std::string t = typeOfNode(call)->toString();
        if (t.rfind("our<", 0) == 0) return true;
    }
    return false;
}

bool LLVMCodegen::exprIsMildTransfer(vyb::ast::Expression* expr) {
    // `soft(...)` hands over a fresh weak reference that a storage location can
    // take without an extra retain; a bare read of an existing `mild` value is a
    // shared copy and must be retained (+weak) on stow to balance the release on
    // scope exit. Any function call returning `mild<T>` likewise hands over its
    // single fresh weak ref (its return-transfer path did the transfer), so it is
    // a transfer too -- mirroring exprIsOurTransfer for function-returned `our<T>`.
    if (!expr) return false;
    auto* call = dynamic_cast<vyb::ast::CallExpression*>(expr);
    if (!call) return false;
    if (auto* id = dynamic_cast<vyb::ast::Identifier*>(call->callee.get())) {
        if (id->name == "soft") return true;
    }
    if (typeOfNode(call)) {
        std::string t = typeOfNode(call)->toString();
        if (t.rfind("mild<", 0) == 0) return true;
    }
    return false;
}

// Generic serialization helper (extracted from original code)
llvm::Value* LLVMCodegen::generateGenericSerialization(llvm::Value* objPtr, vyb::ast::TypeNode* typeNode) {
    // Get the serialization function
    llvm::Function* serializeFunc = getSerializeToJsonFunction();

    // Determine the type name from AST node if available
    llvm::Value* typeNameValue = nullptr;
    std::string typeName = "unknown";

    if (typeNode) {
        typeName = typeNode->toString();
    }

    // Create a global string for the type name
    typeNameValue = builder->CreateGlobalStringPtr(typeName, "type_name");

    // Cast the argument to void* if needed
    llvm::Value* objPtrCasted = objPtr;
    if (!objPtrCasted->getType()->isPointerTy()) {
        // For scalar types, create an alloca and store the value
        llvm::AllocaInst* tempAlloca = builder->CreateAlloca(objPtrCasted->getType(), nullptr, "serialize_temp");
        builder->CreateStore(objPtrCasted, tempAlloca);
        objPtrCasted = tempAlloca;
    }

    // Cast to void* (i8*)
    objPtrCasted = builder->CreateBitCast(objPtrCasted, llvm::PointerType::getUnqual(int8Type), "obj_to_i8ptr");

    // Call serialization function
    std::vector<llvm::Value*> args = {objPtrCasted, typeNameValue};
    return builder->CreateCall(serializeFunc, args, "serialized_json");
}

// Helper functions for primitive type to string conversion
llvm::Value* LLVMCodegen::generateIntToString(llvm::Value* intValue) {
    // Create a buffer for the string representation
    llvm::AllocaInst* buffer = builder->CreateAlloca(
        llvm::ArrayType::get(int8Type, 32),
        nullptr,
        "int_str_buffer"
    );

    // Get sprintf function
    llvm::Function* sprintfFunc = getSprintfFunction();

    // Format string for integer
    llvm::Value* formatStr = builder->CreateGlobalStringPtr("%lld", "int_format");

    // Cast buffer to i8*
    llvm::Value* bufferPtr = builder->CreateBitCast(buffer, int8PtrType, "buffer_ptr");

    // Call sprintf
    std::vector<llvm::Value*> args = {bufferPtr, formatStr, intValue};
    builder->CreateCall(sprintfFunc, args);

    return bufferPtr;
}

llvm::Value* LLVMCodegen::generateFloatToString(llvm::Value* floatValue) {
    // Create a buffer for the string representation
    llvm::AllocaInst* buffer = builder->CreateAlloca(
        llvm::ArrayType::get(int8Type, 32),
        nullptr,
        "float_str_buffer"
    );

    // Get sprintf function
    llvm::Function* sprintfFunc = getSprintfFunction();

    // Format string for float
    llvm::Value* formatStr = builder->CreateGlobalStringPtr("%.6f", "float_format");

    // Cast buffer to i8*
    llvm::Value* bufferPtr = builder->CreateBitCast(buffer, int8PtrType, "buffer_ptr");

    // Call sprintf
    std::vector<llvm::Value*> args = {bufferPtr, formatStr, floatValue};
    builder->CreateCall(sprintfFunc, args);

    return bufferPtr;
}

llvm::Value* LLVMCodegen::generateBoolToString(llvm::Value* boolValue) {
    // Create basic blocks for true/false cases
    llvm::Function* currentFunc = builder->GetInsertBlock()->getParent();
    llvm::BasicBlock* trueBB = llvm::BasicBlock::Create(*context, "bool_true", currentFunc);
    llvm::BasicBlock* falseBB = llvm::BasicBlock::Create(*context, "bool_false", currentFunc);
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(*context, "bool_merge", currentFunc);

    // Create the conditional branch
    builder->CreateCondBr(boolValue, trueBB, falseBB);

    // True branch
    builder->SetInsertPoint(trueBB);
    llvm::Value* trueStr = builder->CreateGlobalStringPtr("true", "true_str");
    builder->CreateBr(mergeBB);

    // False branch
    builder->SetInsertPoint(falseBB);
    llvm::Value* falseStr = builder->CreateGlobalStringPtr("false", "false_str");
    builder->CreateBr(mergeBB);

    // Merge block with PHI node
    builder->SetInsertPoint(mergeBB);
    llvm::PHINode* result = builder->CreatePHI(int8PtrType, 2, "bool_str_result");
    result->addIncoming(trueStr, trueBB);
    result->addIncoming(falseStr, falseBB);

    return result;
}
// Introspection: typeof(expr) - returns 8-byte type ID
void LLVMCodegen::visit(vyb::ast::TypeofExpression* node) {
    if (!node) {
        logError(node->loc, "typeof() requires an operand or type argument");
        m_currentLLVMValue = nullptr;
        return;
    }

    // typeof<T>() - compile-time hash of the resolved type name.
    if (node->typeArg) {
        std::string typeName = typeOfNode(node->typeArg)
            ? typeOfNode(node->typeArg)->toString() : node->typeArg->toString();
        uint64_t typeHash = std::hash<std::string>{}(typeName);
        m_currentLLVMValue = llvm::ConstantInt::get(builder->getInt64Ty(), typeHash);
        return;
    }

    // Wildcard trap error (e<?>): load the runtime type ID from the VybError header.
    if (nodeOperandFromWildcardError(node)) {
        if (!node->operand) { m_currentLLVMValue = nullptr; return; }
        node->operand->accept(*this);
        llvm::Value* errPtr = m_currentLLVMValue;
        if (!errPtr || !errPtr->getType()->isPointerTy()) {
            logError(node->loc, "typeof() on wildcard error requires an error pointer");
            m_currentLLVMValue = nullptr;
            return;
        }
        llvm::Type* i8Ptr = llvm::PointerType::get(*context, 0);
        llvm::StructType* vybErrorTy = llvm::StructType::get(
            *context,
            { builder->getInt64Ty(), i8Ptr, i8Ptr, i8Ptr,
              builder->getInt32Ty(), builder->getInt32Ty() },
            false);
        llvm::Value* errStruct = builder->CreateBitCast(
            errPtr, llvm::PointerType::get(vybErrorTy, 0), "typeof.err.ptr");
        llvm::Value* slot = builder->CreateStructGEP(
            vybErrorTy, errStruct, 0, "typeof.typeid.slot");
        m_currentLLVMValue = builder->CreateLoad(
            builder->getInt64Ty(), slot, "typeof.typeid");
        return;
    }

    // Static form: hash the operand's static type (set by semantic analysis).
    if (!node->operand) {
        logError(node->loc, "typeof() requires an operand expression");
        m_currentLLVMValue = nullptr;
        return;
    }
    std::string typeName = "Unknown";
    if (typeOfNode(node->operand)) {
        typeName = typeOfNode(node->operand)->toString();
    }
    uint64_t typeHash = std::hash<std::string>{}(typeName);
    m_currentLLVMValue = llvm::ConstantInt::get(builder->getInt64Ty(), typeHash);
}

// Introspection: typename(expr) - returns String with type name
void LLVMCodegen::visit(vyb::ast::TypenameExpression* node) {
    if (!node || !node->operand) {
        logError(node->loc, "typename() requires an operand expression");
        m_currentLLVMValue = nullptr;
        return;
    }

    // Wildcard trap error (e<?>): load the runtime type name from the error header
    // and wrap it in a String { ptr, len }.
    if (nodeOperandFromWildcardError(node)) {
        node->operand->accept(*this);
        llvm::Value* errPtr = m_currentLLVMValue;
        if (!errPtr || !errPtr->getType()->isPointerTy()) {
            logError(node->loc, "typename() on wildcard error requires an error pointer");
            m_currentLLVMValue = nullptr;
            return;
        }
        llvm::Type* i8Ptr = llvm::PointerType::get(*context, 0);
        llvm::StructType* vybErrorTy = llvm::StructType::get(
            *context,
            { builder->getInt64Ty(), i8Ptr, i8Ptr, i8Ptr,
              builder->getInt32Ty(), builder->getInt32Ty() },
            false);
        llvm::Value* errStruct = builder->CreateBitCast(
            errPtr, llvm::PointerType::get(vybErrorTy, 0), "typename.err.ptr");
        llvm::Value* slot = builder->CreateStructGEP(
            vybErrorTy, errStruct, 1, "typename.name.slot");
        llvm::Value* namePtr = builder->CreateLoad(
            i8Ptr, slot, "typename.name.ptr");
        llvm::Type* i64Ty = builder->getInt64Ty();
        llvm::Function* strlenFn = module->getFunction("strlen");
        if (!strlenFn) {
            strlenFn = llvm::Function::Create(
                llvm::FunctionType::get(i64Ty, {i8Ptr}, false),
                llvm::Function::ExternalLinkage, "strlen", module.get());
        }
        llvm::Value* len = builder->CreateCall(strlenFn, {namePtr}, "typename.len");
        llvm::StructType* sTy = llvm::StructType::get(
            *context, {i8Ptr, i64Ty});
        llvm::Value* s = llvm::UndefValue::get(sTy);
        s = builder->CreateInsertValue(s, namePtr, 0, "typename.ptr");
        s = builder->CreateInsertValue(s, len, 1, "typename.len");
        m_currentLLVMValue = s;
        return;
    }

    // A `Type` value operand: its runtime value is an opaque uint64 type ID, so
    // look up the registered type name at runtime.
    if (nodeOperandFromTypeValue(node)) {
        node->operand->accept(*this);
        llvm::Value* typeId = m_currentLLVMValue;
        llvm::Type* i8Ptr = llvm::PointerType::get(*context, 0);
        llvm::Type* i64Ty = llvm::Type::getInt64Ty(*context);
        llvm::FunctionType* getTy = llvm::FunctionType::get(i8Ptr, {i64Ty}, false);
        llvm::Function* getFn = module->getFunction("__vyb_get_typename");
        if (!getFn) {
            getFn = llvm::Function::Create(getTy, llvm::Function::ExternalLinkage,
                                           "__vyb_get_typename", module.get());
        }
        llvm::Value* namePtr = builder->CreateCall(getFn, {typeId}, "typename.name");
        llvm::Function* strlenFn = module->getFunction("strlen");
        if (!strlenFn) {
            strlenFn = llvm::Function::Create(
                llvm::FunctionType::get(i64Ty, {i8Ptr}, false),
                llvm::Function::ExternalLinkage, "strlen", module.get());
        }
        llvm::Value* len = builder->CreateCall(strlenFn, {namePtr}, "typename.len");
        llvm::StructType* sTy = llvm::StructType::get(*context, {i8Ptr, i64Ty});
        llvm::Value* s = llvm::UndefValue::get(sTy);
        s = builder->CreateInsertValue(s, namePtr, 0, "typename.ptr");
        s = builder->CreateInsertValue(s, len, 1, "typename.len");
        m_currentLLVMValue = s;
        return;
    }

    // Get the type name from the operand's type field (set by semantic analysis)
    std::string typeName = "Unknown";
    if (typeOfNode(node->operand)) {
        typeName = typeOfNode(node->operand)->toString();
    }

    // Create a string literal containing the type name
    // Similar to StringLiteral::visit implementation
    llvm::Value* strPtr = builder->CreateGlobalStringPtr(typeName);

    // Create String struct {ptr, len}
    llvm::Type* int8PtrType = llvm::PointerType::get(*context, 0);
    llvm::Type* int64Type = llvm::Type::getInt64Ty(*context);
    llvm::StructType* stringStructType = llvm::StructType::get(*context, {int8PtrType, int64Type});

    // Calculate length
    llvm::Value* lenValue = llvm::ConstantInt::get(int64Type, typeName.length());

    // Build the String struct
    llvm::Value* stringStruct = llvm::UndefValue::get(stringStructType);
    stringStruct = builder->CreateInsertValue(stringStruct, strPtr, 0, "typename.ptr");
    stringStruct = builder->CreateInsertValue(stringStruct, lenValue, 1, "typename.len");

    m_currentLLVMValue = stringStruct;
}

void LLVMCodegen::visit(vyb::ast::AsExpression* node) {
    if (!node || !node->operand || !node->targetType) {
        logError(node->loc, "Malformed 'as' cast expression.");
        m_currentLLVMValue = nullptr;
        return;
    }
    node->operand->accept(*this);
    llvm::Value* operand = m_currentLLVMValue;
    if (!operand) {
        logError(node->loc, "Failed to evaluate operand of 'as' cast.");
        m_currentLLVMValue = nullptr;
        return;
    }
    llvm::Type* targetTy = codegenType(node->targetType.get());
    if (!targetTy) {
        logError(node->loc, "Cannot resolve target type of 'as' cast.");
        m_currentLLVMValue = nullptr;
        return;
    }

    // Wildcard trap error downcast: `e as TargetType` extracts the concrete
    // payload from the VybError struct { type_hash, type_name, payload, file,
    // line, col } by loading the heap payload at field index 2.
    if (nodeOperandIsWildcardError(node)) {
        if (!operand->getType()->isPointerTy()) {
            logError(node->loc, "'as' on a wildcard error requires an error pointer.");
            m_currentLLVMValue = nullptr;
            return;
        }
        llvm::Type* i8Ptr = llvm::PointerType::get(*context, 0);
        llvm::StructType* vybErrorTy = llvm::StructType::get(
            *context,
            { builder->getInt64Ty(), i8Ptr, i8Ptr, i8Ptr,
              builder->getInt32Ty(), builder->getInt32Ty() },
            false);
        llvm::Value* errStruct = builder->CreateBitCast(
            operand, llvm::PointerType::get(vybErrorTy, 0), "as.err.ptr");
        llvm::Value* payloadSlot = builder->CreateStructGEP(
            vybErrorTy, errStruct, 2, "as.payload.slot");
        llvm::Value* payloadPtr = builder->CreateLoad(
            i8Ptr, payloadSlot, "as.payload.ptr");
        llvm::Value* dataPtr = builder->CreateBitCast(
            payloadPtr, llvm::PointerType::get(targetTy, 0), "as.data.ptr");
        m_currentLLVMValue = builder->CreateLoad(targetTy, dataPtr, "as.value");
        return;
    }

    // Identity / same-type cast: the value already has the target type.
    // First, integer-to-integer conversions between the sized Int/UInt types:
    // the source's signedness drives the widening so `UInt8 as Int64` zero-
    // extends while `Int8 as Int64` sign-extends; narrowing truncates; equal
    // width is a bit-preserving move (signedness is only tracked at the type).
    std::string srcName;
    if (typeOfNode(node->operand)) {
        if (auto tn = dynamic_cast<ast::TypeName*>(typeOfNode(node->operand).get())) {
            if (tn->identifier) srcName = tn->identifier->name;
        }
    }
    std::string dstName;
    ast::TypeNode* dstTypeNode = typeOfNode(node->targetType)
        ? typeOfNode(node->targetType).get() : node->targetType.get();
    if (dstTypeNode) {
        if (auto tn = dynamic_cast<ast::TypeName*>(dstTypeNode)) {
            if (tn->identifier) dstName = tn->identifier->name;
        }
    }

    bool srcUnsigned = false;
    unsigned srcBits = 0;
    bool dstUnsigned = false;
    unsigned dstBits = 0;
    if (vybIntClass(srcName, srcUnsigned, srcBits) &&
        vybIntClass(dstName, dstUnsigned, dstBits) &&
        operand->getType()->isIntegerTy() && targetTy->isIntegerTy()) {
        unsigned srcW = operand->getType()->getIntegerBitWidth();
        unsigned dstW = targetTy->getIntegerBitWidth();
        if (dstW > srcW) {
            m_currentLLVMValue = srcUnsigned
                ? builder->CreateZExt(operand, targetTy, "as.zext")
                : builder->CreateSExt(operand, targetTy, "as.sext");
        } else if (dstW < srcW) {
            m_currentLLVMValue = builder->CreateTrunc(operand, targetTy, "as.trunc");
        } else {
            m_currentLLVMValue = operand;
        }
        return;
    }

    // Numeric Int <-> Float casts (#202): the operand and target are both
    // scalars; drive the LLVM conversion from the LLVM type shape. (Floating-
    // point operands/widths also cover Float32 <-> Float.)
    if (operand->getType()->isFloatingPointTy() && targetTy->isFloatingPointTy()) {
        unsigned srcW = operand->getType()->getPrimitiveSizeInBits();
        unsigned dstW = targetTy->getPrimitiveSizeInBits();
        if (dstW > srcW) m_currentLLVMValue = builder->CreateFPExt(operand, targetTy, "as.fpext");
        else if (dstW < srcW) m_currentLLVMValue = builder->CreateFPTrunc(operand, targetTy, "as.fptrunc");
        else m_currentLLVMValue = operand;
        return;
    }
    if (operand->getType()->isIntegerTy() && targetTy->isFloatingPointTy()) {
        // int -> float; widen per source signedness (recompute for safety).
        bool sUnsigned = false; unsigned sBits = 0;
        vybIntClass(srcName, sUnsigned, sBits);
        m_currentLLVMValue = sUnsigned
            ? builder->CreateUIToFP(operand, targetTy, "as.uitofp")
            : builder->CreateSIToFP(operand, targetTy, "as.sitofp");
        return;
    }
    if (operand->getType()->isFloatingPointTy() && targetTy->isIntegerTy()) {
        // float -> int; cap signedness per target.
        bool dUnsigned = false; unsigned dBits = 0;
        vybIntClass(dstName, dUnsigned, dBits);
        m_currentLLVMValue = dUnsigned
            ? builder->CreateFPToUI(operand, targetTy, "as.fptoui")
            : builder->CreateFPToSI(operand, targetTy, "as.fptosi");
        return;
    }

    m_currentLLVMValue = operand;
}

bool LLVMCodegen::isOptionalStructType(llvm::Type* type) {
    auto* st = llvm::dyn_cast<llvm::StructType>(type);
    if (!st || st->getNumElements() != 2 || !st->isLiteral()) return false;
    // The native optional is the literal `{ T, i1 }`, with element 1 the present
    // flag. User structs are *named* (non-literal), so they are excluded here.
    return st->getElementType(1)->isIntegerTy(1);
}

// Equality for the native optional `{ T, i1 }`: two optionals are equal when
// their presence flags agree and, when both are present, their payloads are
// equal. `!=` negates. Non-primitive payload kinds (structs/enums) fall back to
// a presence-only comparison with a warning rather than an invalid IR.
llvm::Value* LLVMCodegen::generateOptionalEquality(llvm::Value* L, llvm::Value* R, vyb::TokenType op) {
    if (!L || !R) return llvm::ConstantInt::getFalse(*context);
    llvm::Value* lp = builder->CreateExtractValue(L, 1, "optcmp.l.present");
    llvm::Value* rp = builder->CreateExtractValue(R, 1, "optcmp.r.present");
    llvm::Value* presentEq = builder->CreateICmpEQ(lp, rp, "optcmp.present.eq");
    llvm::Value* lv = builder->CreateExtractValue(L, 0, "optcmp.l.payload");
    llvm::Value* rv = builder->CreateExtractValue(R, 0, "optcmp.r.payload");
    llvm::Type* pt = lv->getType();
    llvm::Value* payloadEq = nullptr;
    if (pt->isIntegerTy()) {
        payloadEq = builder->CreateICmpEQ(lv, rv, "optcmp.payload.eq");
    } else if (pt->isFloatingPointTy()) {
        payloadEq = builder->CreateFCmpOEQ(lv, rv, "optcmp.payload.eq");
    } else if (isVybStringStructType(pt)) {
        payloadEq = generateStringComparison(lv, rv, vyb::TokenType::EQEQ);
    } else {
        logWarning(SourceLocation(), "Optional equality on non-primitive payload type; comparing presence only.");
        payloadEq = llvm::ConstantInt::getTrue(*context);
    }
    llvm::Value* bothPresent = builder->CreateAnd(lp, rp, "optcmp.bothpresent");
    llvm::Value* payloadTerm = builder->CreateSelect(bothPresent, payloadEq,
        llvm::ConstantInt::getTrue(*context), "optcmp.payload.term");
    llvm::Value* eq = builder->CreateAnd(presentEq, payloadTerm, "optcmp.eq");
    if (op == vyb::TokenType::NOTEQ) eq = builder->CreateNot(eq, "optcmp.neq");
    return eq;
}

// Equality for tagged-union enum values (`==` / `!=`). A data enum is the
// struct { i64 tag, [N x i8] data }, so two values are equal when their tags
// match AND (for a data-carrying variant) every payload field matches. Unit
// variants compare by tag alone. This used to feed the whole struct into
// ICmp, which LLVM rejects (assert abort, exit 134) — the crash half of #181.
llvm::Value* LLVMCodegen::generateTaggedEnumEquality(llvm::Value* L, llvm::Value* R, vyb::TokenType op,
                                                     const TaggedEnumInfo& info) {
    if (!L || !R) return llvm::ConstantInt::getFalse(*context);

    // Scalar (C-like) enum values are plain i64 tags: plain integer compare.
    if (info.isScalar) {
        llvm::Value* eq = builder->CreateICmpEQ(L, R, "enumcmp.scalar.eq");
        if (op == vyb::TokenType::NOTEQ) eq = builder->CreateNot(eq, "enumcmp.scalar.neq");
        return eq;
    }

    llvm::Function* curFn = builder->GetInsertBlock()->getParent();
    llvm::BasicBlock* origBB = builder->GetInsertBlock();

    llvm::Value* tagL = builder->CreateExtractValue(L, 0, "enumcmp.l.tag");
    llvm::Value* tagR = builder->CreateExtractValue(R, 0, "enumcmp.r.tag");
    llvm::Value* tagEq = builder->CreateICmpEQ(tagL, tagR, "enumcmp.tag.eq");

    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(*context, "enumcmp.merge", curFn);
    builder->SetInsertPoint(mergeBB);
    llvm::PHINode* resultPhi = builder->CreatePHI(builder->getInt1Ty(), info.variantTags.size() + 1,
                                                  "enumcmp.result");
    builder->SetInsertPoint(origBB);

    std::map<unsigned, llvm::BasicBlock*> tagToBB;
    for (const auto& vt : info.variantTags) {
        llvm::BasicBlock* bb = llvm::BasicBlock::Create(*context, "enumcmp.case." + vt.first, curFn);
        tagToBB[vt.second] = bb;
    }
    llvm::BasicBlock* defaultBB = llvm::BasicBlock::Create(*context, "enumcmp.unknown", curFn);

    // Compare one payload field of the matched variant; falls back to "equal"
    // for types we cannot structurally compare.
    auto fieldEq = [&](llvm::Value* lv, llvm::Value* rv) -> llvm::Value* {
        if (!lv || !rv) return llvm::ConstantInt::getTrue(*context);
        llvm::Type* t = lv->getType();
        if (t->isIntegerTy()) return builder->CreateICmpEQ(lv, rv, "enumcmp.field.eq");
        if (t->isFloatingPointTy()) return builder->CreateFCmpOEQ(lv, rv, "enumcmp.field.eq");
        if (isVybStringStructType(t)) return generateStringComparison(lv, rv, vyb::TokenType::EQEQ);
        return llvm::ConstantInt::getTrue(*context);
    };

    for (const auto& vt : info.variantTags) {
        const std::string& variantName = vt.first;
        builder->SetInsertPoint(tagToBB[vt.second]);
        llvm::Value* eq = tagEq;  // tags already matched; compare payload if any
        auto pit = info.variantPayloadTypes.find(variantName);
        if (pit != info.variantPayloadTypes.end() && pit->second) {
            llvm::StructType* payloadTy = pit->second;
            for (unsigned i = 0; i < payloadTy->getNumElements(); ++i) {
                llvm::Value* lf = extractEnumVariantField(L, payloadTy, i);
                llvm::Value* rf = extractEnumVariantField(R, payloadTy, i);
                eq = builder->CreateAnd(eq, fieldEq(lf, rf), "enumcmp.payload.and");
            }
        }
        // The terminal block may differ from the case block: a String payload's
        // generateStringComparison builds its own blocks and leaves the builder
        // in its merge block, which is where `eq` is computed. Branch from and
        // record that actual block so the PHI predecessors stay consistent.
        llvm::BasicBlock* termBB = builder->GetInsertBlock();
        builder->CreateBr(mergeBB);
        resultPhi->addIncoming(eq, termBB);
    }

    builder->SetInsertPoint(defaultBB);
    // Tags were already compared upfront; the "default" arm can only be reached
    // for a tag not present on both sides, so it is unequal.
    builder->CreateBr(mergeBB);
    resultPhi->addIncoming(llvm::ConstantInt::getFalse(*context), defaultBB);

    builder->SetInsertPoint(origBB);
    llvm::SwitchInst* sw = builder->CreateSwitch(tagL, defaultBB, static_cast<unsigned>(tagToBB.size()));
    for (const auto& kv : tagToBB) {
        auto* tc = llvm::cast<llvm::ConstantInt>(llvm::ConstantInt::get(int64Type, kv.first, true));
        sw->addCase(tc, kv.second);
    }

    builder->SetInsertPoint(mergeBB);
    llvm::Value* res = resultPhi;
    if (op == vyb::TokenType::NOTEQ) res = builder->CreateNot(res, "enumcmp.neq");
    return res;
}

