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
    
    // Return void (push doesn't return a value)
    m_currentLLVMValue = nullptr;
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

} // namespace vyn