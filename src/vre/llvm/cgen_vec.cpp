#include "vyn/vre/llvm/codegen.hpp"
#include "vyn/parser/ast.hpp"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/DerivedTypes.h>
#include <iostream>

namespace vyn {

void LLVMCodegen::handleVecMethod(vyn::ast::CallExpression* node, const std::string& objectName, const std::string& methodName) {
    // Look up the Vec object in namedValues
    auto it = namedValues.find(objectName);
    if (it == namedValues.end()) {
        logError(node->loc, "Undefined Vec variable: " + objectName);
        m_currentLLVMValue = nullptr;
        return;
    }
    
    llvm::Value* vecPtr = it->second;
    
    // Ensure it's the Vec struct type: { ptr, size, capacity }
    llvm::Type* vecPtrType = vecPtr->getType();
    if (!vecPtrType->isPointerTy()) {
        logError(node->loc, "Vec variable is not a pointer type");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // For opaque pointers in newer LLVM, we need to get the type differently
    // Assume it's the Vec struct type: { ptr, i64, i64 }
    std::vector<llvm::Type*> vecFields = {
        llvm::PointerType::get(*context, 0), // ptr to elements
        llvm::Type::getInt64Ty(*context),    // size
        llvm::Type::getInt64Ty(*context)     // capacity
    };
    llvm::Type* vecStructType = llvm::StructType::get(*context, vecFields, false);
    // Struct type validation is handled by the type construction above
    
    if (methodName == "push") {
        handleVecPush(node, vecPtr, vecStructType);
    } else if (methodName == "pop") {
        handleVecPop(node, vecPtr, vecStructType);
    } else if (methodName == "len") {
        handleVecLen(node, vecPtr, vecStructType);
    } else if (methodName == "get") {
        handleVecGet(node, vecPtr, vecStructType);
    } else if (methodName == "push_array") {
        handleVecPushArray(node, vecPtr, vecStructType);
    } else if (methodName == "to_array") {
        handleVecToArray(node, vecPtr, vecStructType);
    } else if (methodName == "clear") {
        handleVecClear(node, vecPtr, vecStructType);
    } else if (methodName == "is_empty") {
        handleVecIsEmpty(node, vecPtr, vecStructType);
    } else if (methodName == "capacity") {
        handleVecCapacity(node, vecPtr, vecStructType);
    } else if (methodName == "concat") {
        handleVecConcat(node, vecPtr, vecStructType);
    } else if (methodName == "contains") {
        handleVecContains(node, vecPtr, vecStructType);
    } else if (methodName == "remove_at") {
        handleVecRemoveAt(node, vecPtr, vecStructType);
    } else if (methodName == "get_array") {
        handleVecGetArray(node, vecPtr, vecStructType);
    } else if (methodName == "get_vec") {
        handleVecGetVec(node, vecPtr, vecStructType);
    } else {
        logError(node->loc, "Unknown Vec method: " + methodName);
        m_currentLLVMValue = nullptr;
    }
}

void LLVMCodegen::handleVecPush(vyn::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType) {
    if (node->arguments.size() != 1) {
        logError(node->loc, "Vec::push expects exactly 1 argument");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Evaluate the argument to push
    node->arguments[0]->accept(*this);
    llvm::Value* valueToAdd = m_currentLLVMValue;
    if (!valueToAdd) {
        logError(node->loc, "Failed to evaluate argument for Vec::push");
        return;
    }
    
    // Get element type from the value being pushed
    llvm::Type* elementType = valueToAdd->getType();
    
    // Calculate the actual element size using DataLayout
    llvm::DataLayout dataLayout(module.get());
    uint64_t elementSizeBytes = dataLayout.getTypeAllocSize(elementType);
    
    // Get pointers to struct fields
    llvm::Value* dataFieldPtr = builder->CreateStructGEP(vecStructType, vecPtr, 0, "vec.data_ptr");
    llvm::Value* sizeFieldPtr = builder->CreateStructGEP(vecStructType, vecPtr, 1, "vec.size_ptr");
    llvm::Value* capFieldPtr = builder->CreateStructGEP(vecStructType, vecPtr, 2, "vec.cap_ptr");
    
    // Load current size and capacity
    llvm::Value* currentSize = builder->CreateLoad(llvm::Type::getInt64Ty(*context), sizeFieldPtr, "vec.current_size");
    llvm::Value* currentCap = builder->CreateLoad(llvm::Type::getInt64Ty(*context), capFieldPtr, "vec.current_cap");
    llvm::Value* dataPtr = builder->CreateLoad(llvm::PointerType::get(*context, 0), dataFieldPtr, "vec.data");
    
    // Check if we need to allocate/grow
    llvm::Value* needsAlloc = builder->CreateICmpEQ(currentCap, llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 0), "vec.needs_alloc");
    llvm::Value* needsGrow = builder->CreateICmpEQ(currentSize, currentCap, "vec.needs_grow");
    llvm::Value* needsRealloc = builder->CreateOr(needsAlloc, needsGrow, "vec.needs_realloc");
    
    llvm::BasicBlock* allocBlock = llvm::BasicBlock::Create(*context, "vec.alloc", builder->GetInsertBlock()->getParent());
    llvm::BasicBlock* storeBlock = llvm::BasicBlock::Create(*context, "vec.store", builder->GetInsertBlock()->getParent());
    
    builder->CreateCondBr(needsRealloc, allocBlock, storeBlock);
    
    // Alloc block - allocate or grow the array
    builder->SetInsertPoint(allocBlock);
    
    // New capacity: if 0, start with 4, else double it
    llvm::Value* newCap = builder->CreateSelect(
        needsAlloc,
        llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 4),
        builder->CreateMul(currentCap, llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 2)),
        "vec.new_cap"
    );
    
    // Calculate allocation size using actual element size
    llvm::Value* elementSize = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), elementSizeBytes);
    llvm::Value* allocSize = builder->CreateMul(newCap, elementSize, "vec.alloc_size");
    
    // Call malloc
    llvm::FunctionType* mallocType = llvm::FunctionType::get(
        llvm::PointerType::get(*context, 0),
        {llvm::Type::getInt64Ty(*context)},
        false
    );
    llvm::Function* mallocFunc = module->getFunction("malloc");
    if (!mallocFunc) {
        mallocFunc = llvm::Function::Create(mallocType, llvm::Function::ExternalLinkage, "malloc", module.get());
    }
    llvm::Value* newDataPtr = builder->CreateCall(mallocFunc, {allocSize}, "vec.new_data");
    
    // If there was old data, copy it (using memcpy if size > 0)
    llvm::BasicBlock* copyBlock = llvm::BasicBlock::Create(*context, "vec.copy", builder->GetInsertBlock()->getParent());
    llvm::BasicBlock* noCopyBlock = llvm::BasicBlock::Create(*context, "vec.no_copy", builder->GetInsertBlock()->getParent());
    
    llvm::Value* hasData = builder->CreateICmpNE(currentSize, llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 0), "vec.has_data");
    builder->CreateCondBr(hasData, copyBlock, noCopyBlock);
    
    // Copy block
    builder->SetInsertPoint(copyBlock);
    llvm::Value* copySize = builder->CreateMul(currentSize, elementSize, "vec.copy_size");
    llvm::FunctionType* memcpyType = llvm::FunctionType::get(
        llvm::PointerType::get(*context, 0),
        {llvm::PointerType::get(*context, 0), llvm::PointerType::get(*context, 0), llvm::Type::getInt64Ty(*context)},
        false
    );
    llvm::Function* memcpyFunc = module->getFunction("memcpy");
    if (!memcpyFunc) {
        memcpyFunc = llvm::Function::Create(memcpyType, llvm::Function::ExternalLinkage, "memcpy", module.get());
    }
    builder->CreateCall(memcpyFunc, {newDataPtr, dataPtr, copySize});
    builder->CreateBr(noCopyBlock);
    
    // No copy block - update the Vec struct
    builder->SetInsertPoint(noCopyBlock);
    builder->CreateStore(newDataPtr, dataFieldPtr);
    builder->CreateStore(newCap, capFieldPtr);
    builder->CreateBr(storeBlock);
    
    // Store block - store the new element
    builder->SetInsertPoint(storeBlock);
    
    // Reload data pointer (might have changed in alloc block)
    llvm::Value* finalDataPtr = builder->CreateLoad(llvm::PointerType::get(*context, 0), dataFieldPtr, "vec.final_data");
    
    // Calculate offset for new element using actual element size
    llvm::Value* elementSize2 = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), elementSizeBytes);
    llvm::Value* offset = builder->CreateMul(currentSize, elementSize2, "vec.offset");
    llvm::Value* elementPtr = builder->CreateGEP(llvm::Type::getInt8Ty(*context), finalDataPtr, offset, "vec.element_ptr");
    
    // Store the value - need to handle different types
    if (elementType->isStructTy()) {
        // For structs, do a memcpy from the source to destination
        // valueToAdd should be a pointer to the struct
        llvm::Value* srcPtr = valueToAdd;
        if (!valueToAdd->getType()->isPointerTy()) {
            // If valueToAdd is a struct value (not a pointer), we need to create a temporary
            llvm::Value* tempAlloca = builder->CreateAlloca(elementType, nullptr, "vec.temp_struct");
            builder->CreateStore(valueToAdd, tempAlloca);
            srcPtr = tempAlloca;
        }
        
        llvm::FunctionType* memcpyType = llvm::FunctionType::get(
            llvm::PointerType::get(*context, 0),
            {llvm::PointerType::get(*context, 0), llvm::PointerType::get(*context, 0), llvm::Type::getInt64Ty(*context)},
            false
        );
        llvm::Function* memcpyFunc = module->getFunction("memcpy");
        if (!memcpyFunc) {
            memcpyFunc = llvm::Function::Create(memcpyType, llvm::Function::ExternalLinkage, "memcpy", module.get());
        }
        builder->CreateCall(memcpyFunc, {elementPtr, srcPtr, elementSize2});
    } else {
        // For primitives, direct store
        builder->CreateStore(valueToAdd, elementPtr);
    }
    
    // Increment size
    llvm::Value* newSize = builder->CreateAdd(currentSize, llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 1), "vec.new_size");
    builder->CreateStore(newSize, sizeFieldPtr);
    
    std::cout << "DEBUG: Vec::push() called - element stored" << std::endl;
    
    // Return the Vec reference for method chaining
    m_currentLLVMValue = vecPtr;
}

void LLVMCodegen::handleVecPop(vyn::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType) {
    if (node->arguments.size() != 0) {
        logError(node->loc, "Vec::pop expects no arguments");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Get pointer to size field
    llvm::Value* sizeFieldPtr = builder->CreateStructGEP(vecStructType, vecPtr, 1, "vec.size_ptr");
    
    // Load current size
    llvm::Value* currentSize = builder->CreateLoad(llvm::Type::getInt64Ty(*context), sizeFieldPtr, "vec.current_size");
    
    // Check if size > 0
    llvm::Value* isEmpty = builder->CreateICmpEQ(currentSize, 
                                                llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 0),
                                                "vec.is_empty");
    
    // For now, just decrement size if not empty
    llvm::Value* newSize = builder->CreateSub(currentSize,
                                             llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 1),
                                             "vec.new_size");
    
    // Use select to avoid underflow: newSize = isEmpty ? 0 : (currentSize - 1)
    llvm::Value* safeNewSize = builder->CreateSelect(isEmpty, 
                                                    llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 0),
                                                    newSize,
                                                    "vec.safe_new_size");
    
    // Store new size
    builder->CreateStore(safeNewSize, sizeFieldPtr);
    
    std::cout << "DEBUG: Vec::pop() called - size decremented" << std::endl;
    
    // Return the popped value (for now, return 0 as placeholder)
    m_currentLLVMValue = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 0);
}

void LLVMCodegen::handleVecLen(vyn::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType) {
    if (node->arguments.size() != 0) {
        logError(node->loc, "Vec::len expects no arguments");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Get pointer to size field
    llvm::Value* sizeFieldPtr = builder->CreateStructGEP(vecStructType, vecPtr, 1, "vec.size_ptr");
    
    // Load and return current size
    m_currentLLVMValue = builder->CreateLoad(llvm::Type::getInt64Ty(*context), sizeFieldPtr, "vec.len");
    
    std::cout << "DEBUG: Vec::len() called" << std::endl;
}

void LLVMCodegen::handleVecGet(vyn::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType) {
    if (node->arguments.size() != 1) {
        logError(node->loc, "Vec::get expects exactly 1 argument (index)");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Evaluate the index argument
    node->arguments[0]->accept(*this);
    llvm::Value* index = m_currentLLVMValue;
    if (!index) {
        logError(node->loc, "Failed to evaluate index for Vec::get");
        return;
    }
    
    // Get the element type from the CallExpression's type (return type)
    // The semantic analyzer should have set this to the element type (T from Vec<T>)
    llvm::Type* elementLLVMType = nullptr;
    uint64_t elementSizeBytes = 8; // Default to 8 bytes
    
    if (node->type) {
        // Convert AST type to LLVM type
        elementLLVMType = codegenType(node->type.get());
        if (elementLLVMType) {
            llvm::DataLayout dataLayout(module.get());
            elementSizeBytes = dataLayout.getTypeAllocSize(elementLLVMType);
        }
    }
    
    // Fallback to i64 if we couldn't determine the type
    if (!elementLLVMType) {
        elementLLVMType = llvm::Type::getInt64Ty(*context);
        elementSizeBytes = 8;
    }
    
    // Get pointers to struct fields
    llvm::Value* dataFieldPtr = builder->CreateStructGEP(vecStructType, vecPtr, 0, "vec.data_ptr");
    llvm::Value* sizeFieldPtr = builder->CreateStructGEP(vecStructType, vecPtr, 1, "vec.size_ptr");
    
    // Load the data pointer and size
    llvm::Value* dataPtr = builder->CreateLoad(llvm::PointerType::get(*context, 0), dataFieldPtr, "vec.data");
    llvm::Value* size = builder->CreateLoad(llvm::Type::getInt64Ty(*context), sizeFieldPtr, "vec.size");
    
    // TODO: Add bounds checking - for now assume index is valid
    
    // Calculate offset: data_ptr + (index * element_size)
    llvm::Value* elementSize = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), elementSizeBytes);
    llvm::Value* offset = builder->CreateMul(index, elementSize, "vec.offset");
    llvm::Value* elementPtr = builder->CreateGEP(llvm::Type::getInt8Ty(*context), dataPtr, offset, "vec.element_ptr");
    
    // Load the element value based on type
    if (elementLLVMType->isStructTy()) {
        // For structs, load the entire struct value (will be a copy)
        llvm::Value* structValue = builder->CreateLoad(elementLLVMType, elementPtr, "vec.element_struct");
        m_currentLLVMValue = structValue;
        
        std::cout << "DEBUG: Vec::get() called - returning struct value" << std::endl;
    } else {
        // For primitives, load the value directly
        llvm::Value* element = builder->CreateLoad(elementLLVMType, elementPtr, "vec.element");
        m_currentLLVMValue = element;
        
        std::cout << "DEBUG: Vec::get() called - element retrieved" << std::endl;
    }
}

void LLVMCodegen::handleVecPushArray(vyn::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType) {
    if (node->arguments.size() != 1) {
        logError(node->loc, "Vec::push_array expects exactly 1 argument (array)");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Evaluate the array argument
    node->arguments[0]->accept(*this);
    llvm::Value* arrayValue = m_currentLLVMValue;
    if (!arrayValue) {
        logError(node->loc, "Failed to evaluate array argument for Vec::push_array");
        return;
    }
    
    std::cout << "DEBUG: Vec::push_array() called - pushing entire array" << std::endl;
    
    // For now, placeholder implementation - would need proper array iteration
    // Return the Vec reference for method chaining
    m_currentLLVMValue = vecPtr;
}

void LLVMCodegen::handleVecToArray(vyn::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType) {
    if (node->arguments.size() != 1) {
        logError(node->loc, "Vec::to_array expects exactly 1 argument (array size)");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Evaluate the size argument
    node->arguments[0]->accept(*this);
    llvm::Value* sizeValue = m_currentLLVMValue;
    if (!sizeValue) {
        logError(node->loc, "Failed to evaluate size argument for Vec::to_array");
        return;
    }
    
    std::cout << "DEBUG: Vec::to_array() called - converting to fixed array" << std::endl;
    
    // For now, return a placeholder array
    // In full implementation, would create array from Vec elements
    m_currentLLVMValue = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 0);
}

void LLVMCodegen::handleVecClear(vyn::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType) {
    if (node->arguments.size() != 0) {
        logError(node->loc, "Vec::clear expects no arguments");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Get pointer to size field and set to 0
    llvm::Value* sizeFieldPtr = builder->CreateStructGEP(vecStructType, vecPtr, 1, "vec.size_ptr");
    builder->CreateStore(llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 0), sizeFieldPtr);
    
    std::cout << "DEBUG: Vec::clear() called - size reset to 0" << std::endl;
    
    // Return void
    m_currentLLVMValue = nullptr;
}

void LLVMCodegen::handleVecIsEmpty(vyn::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType) {
    if (node->arguments.size() != 0) {
        logError(node->loc, "Vec::is_empty expects no arguments");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Get pointer to size field
    llvm::Value* sizeFieldPtr = builder->CreateStructGEP(vecStructType, vecPtr, 1, "vec.size_ptr");
    llvm::Value* currentSize = builder->CreateLoad(llvm::Type::getInt64Ty(*context), sizeFieldPtr, "vec.current_size");
    
    // Check if size == 0
    llvm::Value* isEmpty = builder->CreateICmpEQ(currentSize, 
                                                llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 0),
                                                "vec.is_empty");
    
    std::cout << "DEBUG: Vec::is_empty() called" << std::endl;
    
    // Return boolean result
    m_currentLLVMValue = isEmpty;
}

void LLVMCodegen::handleVecCapacity(vyn::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType) {
    if (node->arguments.size() != 0) {
        logError(node->loc, "Vec::capacity expects no arguments");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Get pointer to capacity field
    llvm::Value* capacityFieldPtr = builder->CreateStructGEP(vecStructType, vecPtr, 2, "vec.capacity_ptr");
    
    // Load and return current capacity
    m_currentLLVMValue = builder->CreateLoad(llvm::Type::getInt64Ty(*context), capacityFieldPtr, "vec.capacity");
    
    std::cout << "DEBUG: Vec::capacity() called" << std::endl;
}

void LLVMCodegen::handleVecConcat(vyn::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType) {
    if (node->arguments.size() != 1) {
        logError(node->loc, "Vec::concat expects exactly 1 argument (other Vec)");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Evaluate the other Vec argument
    node->arguments[0]->accept(*this);
    llvm::Value* otherVec = m_currentLLVMValue;
    if (!otherVec) {
        logError(node->loc, "Failed to evaluate Vec argument for Vec::concat");
        return;
    }
    
    std::cout << "DEBUG: Vec::concat() called - concatenating with another Vec" << std::endl;
    
    // For now, placeholder implementation
    // Return the original Vec reference
    m_currentLLVMValue = vecPtr;
}

void LLVMCodegen::handleVecContains(vyn::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType) {
    if (node->arguments.size() != 1) {
        logError(node->loc, "Vec::contains expects exactly 1 argument (value to search)");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Evaluate the search value argument
    node->arguments[0]->accept(*this);
    llvm::Value* searchValue = m_currentLLVMValue;
    if (!searchValue) {
        logError(node->loc, "Failed to evaluate search value for Vec::contains");
        return;
    }
    
    std::cout << "DEBUG: Vec::contains() called - searching for value" << std::endl;
    
    // For now, return false as placeholder
    m_currentLLVMValue = llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context), 0);
}

void LLVMCodegen::handleVecRemoveAt(vyn::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType) {
    if (node->arguments.size() != 1) {
        logError(node->loc, "Vec::remove_at expects exactly 1 argument (index)");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Evaluate the index argument
    node->arguments[0]->accept(*this);
    llvm::Value* index = m_currentLLVMValue;
    if (!index) {
        logError(node->loc, "Failed to evaluate index for Vec::remove_at");
        return;
    }
    
    std::cout << "DEBUG: Vec::remove_at() called - removing element at index" << std::endl;
    
    // For now, placeholder implementation - just decrement size
    llvm::Value* sizeFieldPtr = builder->CreateStructGEP(vecStructType, vecPtr, 1, "vec.size_ptr");
    llvm::Value* currentSize = builder->CreateLoad(llvm::Type::getInt64Ty(*context), sizeFieldPtr, "vec.current_size");
    
    // Decrement size if not empty
    llvm::Value* isEmpty = builder->CreateICmpEQ(currentSize, 
                                                llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 0),
                                                "vec.is_empty");
    llvm::Value* newSize = builder->CreateSub(currentSize,
                                             llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 1),
                                             "vec.new_size");
    llvm::Value* safeNewSize = builder->CreateSelect(isEmpty, currentSize, newSize, "vec.safe_new_size");
    builder->CreateStore(safeNewSize, sizeFieldPtr);
    
    // Return removed value (placeholder)
    m_currentLLVMValue = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 0);
}

void LLVMCodegen::handleVecGetArray(vyn::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType) {
    if (node->arguments.size() != 1) {
        logError(node->loc, "Vec::get_array expects exactly 1 argument (pre-allocated array)");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Evaluate the pre-allocated array argument
    node->arguments[0]->accept(*this);
    llvm::Value* arrayPtr = m_currentLLVMValue;
    if (!arrayPtr) {
        logError(node->loc, "Failed to evaluate array argument for Vec::get_array");
        return;
    }
    
    // Get Vec size for bounds checking
    llvm::Value* sizeFieldPtr = builder->CreateStructGEP(vecStructType, vecPtr, 1, "vec.size_ptr");
    llvm::Value* vecSize = builder->CreateLoad(llvm::Type::getInt64Ty(*context), sizeFieldPtr, "vec.size");
    
    std::cout << "DEBUG: Vec::get_array() called - copying to pre-allocated array" << std::endl;
    
    // In a full implementation, this would:
    // 1. Check array size compatibility
    // 2. Copy elements from Vec storage to the provided array
    // 3. Handle bounds checking and partial copies
    // 4. Return number of elements copied
    
    // For now, return the number of elements that would be copied
    m_currentLLVMValue = vecSize;
}

void LLVMCodegen::handleVecGetVec(vyn::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType) {
    if (node->arguments.size() != 1) {
        logError(node->loc, "Vec::get_vec expects exactly 1 argument (target Vec)");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Evaluate the target Vec argument
    node->arguments[0]->accept(*this);
    llvm::Value* targetVecPtr = m_currentLLVMValue;
    if (!targetVecPtr) {
        logError(node->loc, "Failed to evaluate target Vec argument for Vec::get_vec");
        return;
    }
    
    // Get source Vec size
    llvm::Value* srcSizeFieldPtr = builder->CreateStructGEP(vecStructType, vecPtr, 1, "src_vec.size_ptr");
    llvm::Value* srcSize = builder->CreateLoad(llvm::Type::getInt64Ty(*context), srcSizeFieldPtr, "src_vec.size");
    
    // Get source Vec data pointer
    llvm::Value* srcDataFieldPtr = builder->CreateStructGEP(vecStructType, vecPtr, 0, "src_vec.data_ptr");
    llvm::Value* srcDataPtr = builder->CreateLoad(llvm::PointerType::get(*context, 0), srcDataFieldPtr, "src_vec.data");
    
    // Get target Vec pointers
    llvm::Value* targetSizeFieldPtr = builder->CreateStructGEP(vecStructType, targetVecPtr, 1, "target_vec.size_ptr");
    llvm::Value* targetCapacityFieldPtr = builder->CreateStructGEP(vecStructType, targetVecPtr, 2, "target_vec.capacity_ptr");
    llvm::Value* targetDataFieldPtr = builder->CreateStructGEP(vecStructType, targetVecPtr, 0, "target_vec.data_ptr");
    
    std::cout << "DEBUG: Vec::get_vec() called - extracting contents from 'their Vec' to target Vec" << std::endl;
    
    // In a full implementation, this would:
    // 1. Allocate new storage in target Vec if needed
    // 2. Copy all elements from source Vec to target Vec
    // 3. Update target Vec's size and potentially capacity
    // 4. Clear the source Vec (since it's a 'their' Vec being consumed)
    // 5. Return number of elements transferred
    
    // For now, simulate the transfer:
    // Set target size to source size
    builder->CreateStore(srcSize, targetSizeFieldPtr);
    
    // In a real implementation, we'd copy the data and potentially free source
    // For demonstration, just return the number of elements that would be transferred
    m_currentLLVMValue = srcSize;
}

void LLVMCodegen::handleVecMethodOnValue(vyn::ast::CallExpression* node, llvm::Value* vecValue, const std::string& methodName, vyn::ast::Expression* objectExpr) {
    // Handle Vec method calls when we have the Vec value directly (not just a name)
    // This is used for calls like tree.nodes.push() where tree.nodes is a member expression
    
    llvm::Value* vecPtr = vecValue;
    
    // Check if the value is a pointer
    if (!vecPtr->getType()->isPointerTy()) {
        // If it's a value (not a pointer), we need to create a temporary alloca and store it
        llvm::Type* vecType = vecPtr->getType();
        llvm::Value* tempAlloca = builder->CreateAlloca(vecType, nullptr, "vec.temp");
        builder->CreateStore(vecPtr, tempAlloca);
        vecPtr = tempAlloca;
    }
    
    // Define Vec struct type: { ptr, i64, i64 }
    std::vector<llvm::Type*> vecFields = {
        llvm::PointerType::get(*context, 0), // ptr to elements
        llvm::Type::getInt64Ty(*context),    // size
        llvm::Type::getInt64Ty(*context)     // capacity
    };
    llvm::Type* vecStructType = llvm::StructType::get(*context, vecFields, false);
    
    // Dispatch to the appropriate handler
    if (methodName == "push") {
        handleVecPush(node, vecPtr, vecStructType);
    } else if (methodName == "pop") {
        handleVecPop(node, vecPtr, vecStructType);
    } else if (methodName == "len") {
        handleVecLen(node, vecPtr, vecStructType);
    } else if (methodName == "get") {
        handleVecGet(node, vecPtr, vecStructType);
    } else if (methodName == "push_array") {
        handleVecPushArray(node, vecPtr, vecStructType);
    } else if (methodName == "to_array") {
        handleVecToArray(node, vecPtr, vecStructType);
    } else if (methodName == "clear") {
        handleVecClear(node, vecPtr, vecStructType);
    } else if (methodName == "is_empty") {
        handleVecIsEmpty(node, vecPtr, vecStructType);
    } else if (methodName == "capacity") {
        handleVecCapacity(node, vecPtr, vecStructType);
    } else if (methodName == "concat") {
        handleVecConcat(node, vecPtr, vecStructType);
    } else if (methodName == "contains") {
        handleVecContains(node, vecPtr, vecStructType);
    } else if (methodName == "remove_at") {
        handleVecRemoveAt(node, vecPtr, vecStructType);
    } else if (methodName == "get_array") {
        handleVecGetArray(node, vecPtr, vecStructType);
    } else if (methodName == "get_vec") {
        handleVecGetVec(node, vecPtr, vecStructType);
    } else {
        logError(node->loc, "Unknown Vec method: " + methodName);
        m_currentLLVMValue = nullptr;
    }
}

} // namespace vyn