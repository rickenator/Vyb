#include "vyn/vre/llvm/codegen.hpp"
#include "vyn/parser/ast.hpp"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/DerivedTypes.h>
#include <iostream>

namespace vyn {

// String struct: { ptr: *i8, len: i64 }
// This represents a Vyn String type as a fat pointer with length

void LLVMCodegen::handleStringMethod(vyn::ast::CallExpression* node, const std::string& objectName, const std::string& methodName) {
    // Look up the String object in namedValues
    auto it = namedValues.find(objectName);
    if (it == namedValues.end()) {
        logError(node->loc, "Undefined String variable: " + objectName);
        m_currentLLVMValue = nullptr;
        return;
    }
    
    llvm::Value* strPtr = it->second;
    
    // Ensure it's a pointer type
    if (!strPtr->getType()->isPointerTy()) {
        logError(node->loc, "String variable is not a pointer type");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Define String struct type: { ptr: *i8, len: i64 }
    std::vector<llvm::Type*> strFields = {
        llvm::PointerType::get(*context, 0), // ptr to bytes
        llvm::Type::getInt64Ty(*context)     // length
    };
    llvm::Type* strStructType = llvm::StructType::get(*context, strFields, false);
    
    if (methodName == "len" || methodName == "length") {
        handleStringLen(node, strPtr, strStructType);
    } else if (methodName == "concat") {
        handleStringConcat(node, strPtr, strStructType);
    } else if (methodName == "substring" || methodName == "substr") {
        handleStringSubstring(node, strPtr, strStructType);
    } else if (methodName == "char_at") {
        handleStringCharAt(node, strPtr, strStructType);
    } else if (methodName == "to_bytes") {
        handleStringToBytes(node, strPtr, strStructType);
    } else if (methodName == "from_bytes") {
        handleStringFromBytes(node, strPtr, strStructType);
    } else if (methodName == "starts_with") {
        handleStringStartsWith(node, strPtr, strStructType);
    } else if (methodName == "ends_with") {
        handleStringEndsWith(node, strPtr, strStructType);
    } else if (methodName == "contains") {
        handleStringContains(node, strPtr, strStructType);
    } else if (methodName == "to_upper") {
        handleStringToUpper(node, strPtr, strStructType);
    } else if (methodName == "to_lower") {
        handleStringToLower(node, strPtr, strStructType);
    } else {
        logError(node->loc, "Unknown String method: " + methodName);
        m_currentLLVMValue = nullptr;
    }
}

void LLVMCodegen::handleStringLen(vyn::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType) {
    if (node->arguments.size() != 0) {
        logError(node->loc, "String::len expects no arguments");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Get pointer to length field (index 1)
    llvm::Value* lenFieldPtr = builder->CreateStructGEP(strStructType, strPtr, 1, "str.len_ptr");
    
    // Load and return length
    m_currentLLVMValue = builder->CreateLoad(llvm::Type::getInt64Ty(*context), lenFieldPtr, "str.len");
    
    std::cout << "DEBUG: String::len() called" << std::endl;
}

void LLVMCodegen::handleStringConcat(vyn::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType) {
    if (node->arguments.size() != 1) {
        logError(node->loc, "String::concat expects exactly 1 argument (other String)");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Evaluate the other string argument
    node->arguments[0]->accept(*this);
    llvm::Value* otherStr = m_currentLLVMValue;
    if (!otherStr) {
        logError(node->loc, "Failed to evaluate argument for String::concat");
        return;
    }
    
    // Get fields from first string
    llvm::Value* str1DataPtr = builder->CreateStructGEP(strStructType, strPtr, 0, "str1.data_ptr");
    llvm::Value* str1LenPtr = builder->CreateStructGEP(strStructType, strPtr, 1, "str1.len_ptr");
    llvm::Value* str1Data = builder->CreateLoad(llvm::PointerType::get(*context, 0), str1DataPtr, "str1.data");
    llvm::Value* str1Len = builder->CreateLoad(llvm::Type::getInt64Ty(*context), str1LenPtr, "str1.len");
    
    // Get fields from second string (otherStr should be a String struct value or pointer)
    llvm::Value* str2DataPtr;
    llvm::Value* str2LenPtr;
    llvm::Value* str2Data;
    llvm::Value* str2Len;
    
    if (otherStr->getType()->isPointerTy()) {
        str2DataPtr = builder->CreateStructGEP(strStructType, otherStr, 0, "str2.data_ptr");
        str2LenPtr = builder->CreateStructGEP(strStructType, otherStr, 1, "str2.len_ptr");
        str2Data = builder->CreateLoad(llvm::PointerType::get(*context, 0), str2DataPtr, "str2.data");
        str2Len = builder->CreateLoad(llvm::Type::getInt64Ty(*context), str2LenPtr, "str2.len");
    } else if (otherStr->getType()->isStructTy()) {
        // Extract values from struct
        str2Data = builder->CreateExtractValue(otherStr, 0, "str2.data");
        str2Len = builder->CreateExtractValue(otherStr, 1, "str2.len");
    } else {
        logError(node->loc, "Invalid type for String::concat argument");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Calculate new length
    llvm::Value* newLen = builder->CreateAdd(str1Len, str2Len, "str.new_len");
    
    // Allocate new buffer (+1 for null terminator)
    llvm::Value* allocSize = builder->CreateAdd(newLen, llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 1), "str.alloc_size");
    
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
    
    // Create new String struct
    llvm::Value* resultStr = llvm::UndefValue::get(strStructType);
    resultStr = builder->CreateInsertValue(resultStr, newData, 0, "str.result_data");
    resultStr = builder->CreateInsertValue(resultStr, newLen, 1, "str.result_len");
    
    std::cout << "DEBUG: String::concat() called" << std::endl;
    
    m_currentLLVMValue = resultStr;
}

void LLVMCodegen::handleStringSubstring(vyn::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType) {
    if (node->arguments.size() < 1 || node->arguments.size() > 2) {
        logError(node->loc, "String::substring expects 1 or 2 arguments (start, [end])");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // TODO: Implement substring
    std::cout << "DEBUG: String::substring() called - not yet implemented" << std::endl;
    m_currentLLVMValue = nullptr;
}

void LLVMCodegen::handleStringCharAt(vyn::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType) {
    if (node->arguments.size() != 1) {
        logError(node->loc, "String::char_at expects exactly 1 argument (index)");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // TODO: Implement char_at
    std::cout << "DEBUG: String::char_at() called - not yet implemented" << std::endl;
    m_currentLLVMValue = nullptr;
}

void LLVMCodegen::handleStringToBytes(vyn::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType) {
    if (node->arguments.size() != 0) {
        logError(node->loc, "String::to_bytes expects no arguments");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Get data pointer from string
    llvm::Value* dataPtr = builder->CreateStructGEP(strStructType, strPtr, 0, "str.data_ptr");
    llvm::Value* data = builder->CreateLoad(llvm::PointerType::get(*context, 0), dataPtr, "str.data");
    
    std::cout << "DEBUG: String::to_bytes() called" << std::endl;
    
    // Return the byte array pointer
    m_currentLLVMValue = data;
}

void LLVMCodegen::handleStringFromBytes(vyn::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType) {
    if (node->arguments.size() != 2) {
        logError(node->loc, "String::from_bytes expects exactly 2 arguments (byte_ptr, length)");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Evaluate arguments
    node->arguments[0]->accept(*this);
    llvm::Value* bytePtr = m_currentLLVMValue;
    
    node->arguments[1]->accept(*this);
    llvm::Value* length = m_currentLLVMValue;
    
    if (!bytePtr || !length) {
        logError(node->loc, "Failed to evaluate arguments for String::from_bytes");
        return;
    }
    
    // Create String struct from bytes
    llvm::Value* resultStr = llvm::UndefValue::get(strStructType);
    resultStr = builder->CreateInsertValue(resultStr, bytePtr, 0, "str.from_bytes_data");
    resultStr = builder->CreateInsertValue(resultStr, length, 1, "str.from_bytes_len");
    
    std::cout << "DEBUG: String::from_bytes() called" << std::endl;
    
    m_currentLLVMValue = resultStr;
}

void LLVMCodegen::handleStringStartsWith(vyn::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType) {
    std::cout << "DEBUG: String::starts_with() called - not yet implemented" << std::endl;
    m_currentLLVMValue = llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context), 0);
}

void LLVMCodegen::handleStringEndsWith(vyn::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType) {
    std::cout << "DEBUG: String::ends_with() called - not yet implemented" << std::endl;
    m_currentLLVMValue = llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context), 0);
}

void LLVMCodegen::handleStringContains(vyn::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType) {
    std::cout << "DEBUG: String::contains() called - not yet implemented" << std::endl;
    m_currentLLVMValue = llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context), 0);
}

void LLVMCodegen::handleStringToUpper(vyn::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType) {
    std::cout << "DEBUG: String::to_upper() called - not yet implemented" << std::endl;
    m_currentLLVMValue = nullptr;
}

void LLVMCodegen::handleStringToLower(vyn::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType) {
    std::cout << "DEBUG: String::to_lower() called - not yet implemented" << std::endl;
    m_currentLLVMValue = nullptr;
}

} // namespace vyn
