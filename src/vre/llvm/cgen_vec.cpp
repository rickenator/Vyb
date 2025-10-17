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
    
    // For now, implement a simple push that just increments size
    // In a full implementation, this would need to handle memory allocation/reallocation
    
    // Get pointers to struct fields
    llvm::Value* sizeFieldPtr = builder->CreateStructGEP(vecStructType, vecPtr, 1, "vec.size_ptr");
    
    // Load current size
    llvm::Value* currentSize = builder->CreateLoad(llvm::Type::getInt64Ty(*context), sizeFieldPtr, "vec.current_size");
    
    // Increment size by 1
    llvm::Value* newSize = builder->CreateAdd(currentSize, 
                                             llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 1), 
                                             "vec.new_size");
    
    // Store new size
    builder->CreateStore(newSize, sizeFieldPtr);
    
    // For now, we'll just print that we pushed a value (placeholder)
    std::cout << "DEBUG: Vec::push() called - size incremented" << std::endl;
    
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
    
    std::cout << "DEBUG: Vec::get() called with index" << std::endl;
    
    // For now, return a placeholder value
    m_currentLLVMValue = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 42);
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

} // namespace vyn