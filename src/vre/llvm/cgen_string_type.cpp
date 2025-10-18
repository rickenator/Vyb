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
    
    // Evaluate start index
    node->arguments[0]->accept(*this);
    llvm::Value* startIdx = m_currentLLVMValue;
    if (!startIdx) {
        logError(node->arguments[0]->loc, "Failed to evaluate start index for substring");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Load string fields
    llvm::Value* dataPtr = builder->CreateStructGEP(strStructType, strPtr, 0, "str.data_ptr");
    llvm::Value* lenPtr = builder->CreateStructGEP(strStructType, strPtr, 1, "str.len_ptr");
    llvm::Value* data = builder->CreateLoad(llvm::PointerType::get(*context, 0), dataPtr, "str.data");
    llvm::Value* len = builder->CreateLoad(llvm::Type::getInt64Ty(*context), lenPtr, "str.len");
    
    llvm::Value* endIdx;
    if (node->arguments.size() == 2) {
        // Use provided end index
        node->arguments[1]->accept(*this);
        endIdx = m_currentLLVMValue;
        if (!endIdx) {
            logError(node->arguments[1]->loc, "Failed to evaluate end index for substring");
            m_currentLLVMValue = nullptr;
            return;
        }
    } else {
        // End is the string length
        endIdx = len;
    }
    
    // Bounds checking
    llvm::BasicBlock* boundsOkBlock = llvm::BasicBlock::Create(*context, "bounds_ok", currentFunction);
    llvm::BasicBlock* boundsFailBlock = llvm::BasicBlock::Create(*context, "bounds_fail", currentFunction);
    llvm::BasicBlock* mergeBlock = llvm::BasicBlock::Create(*context, "substr_merge", currentFunction);
    
    // Check: start >= 0 && start <= len && end >= start && end <= len
    llvm::Value* startGEZero = builder->CreateICmpSGE(startIdx, llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 0));
    llvm::Value* startLELen = builder->CreateICmpSLE(startIdx, len);
    llvm::Value* endGEStart = builder->CreateICmpSGE(endIdx, startIdx);
    llvm::Value* endLELen = builder->CreateICmpSLE(endIdx, len);
    
    llvm::Value* boundsOk = builder->CreateAnd(startGEZero, startLELen);
    boundsOk = builder->CreateAnd(boundsOk, endGEStart);
    boundsOk = builder->CreateAnd(boundsOk, endLELen);
    
    builder->CreateCondBr(boundsOk, boundsOkBlock, boundsFailBlock);
    
    // Bounds fail: return empty string
    builder->SetInsertPoint(boundsFailBlock);
    llvm::Value* emptyStr = llvm::UndefValue::get(strStructType);
    emptyStr = builder->CreateInsertValue(emptyStr, llvm::ConstantPointerNull::get(llvm::PointerType::get(*context, 0)), 0);
    emptyStr = builder->CreateInsertValue(emptyStr, llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 0), 1);
    builder->CreateBr(mergeBlock);
    
    // Bounds OK: create substring
    builder->SetInsertPoint(boundsOkBlock);
    llvm::Value* subLen = builder->CreateSub(endIdx, startIdx, "substr.len");
    
    // Allocate new buffer (+1 for null terminator)
    llvm::Value* allocSize = builder->CreateAdd(subLen, llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 1));
    llvm::FunctionType* mallocType = llvm::FunctionType::get(
        llvm::PointerType::get(*context, 0),
        {llvm::Type::getInt64Ty(*context)},
        false
    );
    llvm::Function* mallocFunc = module->getFunction("malloc");
    if (!mallocFunc) {
        mallocFunc = llvm::Function::Create(mallocType, llvm::Function::ExternalLinkage, "malloc", module.get());
    }
    llvm::Value* newData = builder->CreateCall(mallocFunc, {allocSize}, "substr.data");
    
    // Copy substring
    llvm::Value* srcOffset = builder->CreateGEP(llvm::Type::getInt8Ty(*context), data, startIdx, "src.offset");
    llvm::FunctionType* memcpyType = llvm::FunctionType::get(
        llvm::PointerType::get(*context, 0),
        {llvm::PointerType::get(*context, 0), llvm::PointerType::get(*context, 0), llvm::Type::getInt64Ty(*context)},
        false
    );
    llvm::Function* memcpyFunc = module->getFunction("memcpy");
    if (!memcpyFunc) {
        memcpyFunc = llvm::Function::Create(memcpyType, llvm::Function::ExternalLinkage, "memcpy", module.get());
    }
    builder->CreateCall(memcpyFunc, {newData, srcOffset, subLen});
    
    // Add null terminator
    llvm::Value* nullPos = builder->CreateGEP(llvm::Type::getInt8Ty(*context), newData, subLen);
    builder->CreateStore(llvm::ConstantInt::get(llvm::Type::getInt8Ty(*context), 0), nullPos);
    
    // Create result string
    llvm::Value* resultStr = llvm::UndefValue::get(strStructType);
    resultStr = builder->CreateInsertValue(resultStr, newData, 0);
    resultStr = builder->CreateInsertValue(resultStr, subLen, 1);
    builder->CreateBr(mergeBlock);
    
    // Merge
    builder->SetInsertPoint(mergeBlock);
    llvm::PHINode* phi = builder->CreatePHI(strStructType, 2, "substr.result");
    phi->addIncoming(emptyStr, boundsFailBlock);
    phi->addIncoming(resultStr, boundsOkBlock);
    
    std::cout << "DEBUG: String::substring() called" << std::endl;
    m_currentLLVMValue = phi;
}

void LLVMCodegen::handleStringCharAt(vyn::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType) {
    if (node->arguments.size() != 1) {
        logError(node->loc, "String::char_at expects exactly 1 argument (index)");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Evaluate index
    node->arguments[0]->accept(*this);
    llvm::Value* idx = m_currentLLVMValue;
    if (!idx) {
        logError(node->arguments[0]->loc, "Failed to evaluate index for char_at");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Load string fields
    llvm::Value* dataPtr = builder->CreateStructGEP(strStructType, strPtr, 0, "str.data_ptr");
    llvm::Value* lenPtr = builder->CreateStructGEP(strStructType, strPtr, 1, "str.len_ptr");
    llvm::Value* data = builder->CreateLoad(llvm::PointerType::get(*context, 0), dataPtr, "str.data");
    llvm::Value* len = builder->CreateLoad(llvm::Type::getInt64Ty(*context), lenPtr, "str.len");
    
    // Bounds checking
    llvm::BasicBlock* inBoundsBlock = llvm::BasicBlock::Create(*context, "in_bounds", currentFunction);
    llvm::BasicBlock* outOfBoundsBlock = llvm::BasicBlock::Create(*context, "out_of_bounds", currentFunction);
    llvm::BasicBlock* mergeBlock = llvm::BasicBlock::Create(*context, "char_at_merge", currentFunction);
    
    llvm::Value* idxGEZero = builder->CreateICmpSGE(idx, llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 0));
    llvm::Value* idxLTLen = builder->CreateICmpSLT(idx, len);
    llvm::Value* inBounds = builder->CreateAnd(idxGEZero, idxLTLen);
    
    builder->CreateCondBr(inBounds, inBoundsBlock, outOfBoundsBlock);
    
    // Out of bounds: return 0
    builder->SetInsertPoint(outOfBoundsBlock);
    llvm::Value* defaultChar = llvm::ConstantInt::get(llvm::Type::getInt8Ty(*context), 0);
    builder->CreateBr(mergeBlock);
    
    // In bounds: load character
    builder->SetInsertPoint(inBoundsBlock);
    llvm::Value* charPtr = builder->CreateGEP(llvm::Type::getInt8Ty(*context), data, idx, "char.ptr");
    llvm::Value* charVal = builder->CreateLoad(llvm::Type::getInt8Ty(*context), charPtr, "char.val");
    builder->CreateBr(mergeBlock);
    
    // Merge
    builder->SetInsertPoint(mergeBlock);
    llvm::PHINode* phi = builder->CreatePHI(llvm::Type::getInt8Ty(*context), 2, "char.result");
    phi->addIncoming(defaultChar, outOfBoundsBlock);
    phi->addIncoming(charVal, inBoundsBlock);
    
    std::cout << "DEBUG: String::char_at() called" << std::endl;
    m_currentLLVMValue = phi;
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
    if (node->arguments.size() != 1) {
        logError(node->loc, "String::starts_with expects exactly 1 argument (prefix string)");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Evaluate prefix argument
    node->arguments[0]->accept(*this);
    llvm::Value* prefixStr = m_currentLLVMValue;
    if (!prefixStr) {
        logError(node->arguments[0]->loc, "Failed to evaluate prefix for starts_with");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Load main string fields
    llvm::Value* dataPtr = builder->CreateStructGEP(strStructType, strPtr, 0, "str.data_ptr");
    llvm::Value* lenPtr = builder->CreateStructGEP(strStructType, strPtr, 1, "str.len_ptr");
    llvm::Value* data = builder->CreateLoad(llvm::PointerType::get(*context, 0), dataPtr, "str.data");
    llvm::Value* len = builder->CreateLoad(llvm::Type::getInt64Ty(*context), lenPtr, "str.len");
    
    // Load prefix string fields
    llvm::Value* prefixData = builder->CreateExtractValue(prefixStr, 0, "prefix.data");
    llvm::Value* prefixLen = builder->CreateExtractValue(prefixStr, 1, "prefix.len");
    
    // Check if prefix length > string length
    llvm::BasicBlock* lenOkBlock = llvm::BasicBlock::Create(*context, "len_ok", currentFunction);
    llvm::BasicBlock* returnFalseBlock = llvm::BasicBlock::Create(*context, "return_false", currentFunction);
    llvm::BasicBlock* compareBlock = llvm::BasicBlock::Create(*context, "compare", currentFunction);
    llvm::BasicBlock* mergeBlock = llvm::BasicBlock::Create(*context, "starts_with_merge", currentFunction);
    
    llvm::Value* lenOk = builder->CreateICmpSLE(prefixLen, len);
    builder->CreateCondBr(lenOk, lenOkBlock, returnFalseBlock);
    
    // If prefix is empty, return true
    builder->SetInsertPoint(lenOkBlock);
    llvm::Value* prefixIsEmpty = builder->CreateICmpEQ(prefixLen, llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 0));
    builder->CreateCondBr(prefixIsEmpty, mergeBlock, compareBlock);
    
    // Compare prefix with start of string using memcmp
    builder->SetInsertPoint(compareBlock);
    llvm::FunctionType* memcmpType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(*context),
        {llvm::PointerType::get(*context, 0), llvm::PointerType::get(*context, 0), llvm::Type::getInt64Ty(*context)},
        false
    );
    llvm::Function* memcmpFunc = module->getFunction("memcmp");
    if (!memcmpFunc) {
        memcmpFunc = llvm::Function::Create(memcmpType, llvm::Function::ExternalLinkage, "memcmp", module.get());
    }
    llvm::Value* cmpResult = builder->CreateCall(memcmpFunc, {data, prefixData, prefixLen}, "cmp.result");
    llvm::Value* isMatch = builder->CreateICmpEQ(cmpResult, llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0));
    builder->CreateBr(mergeBlock);
    
    // Return false
    builder->SetInsertPoint(returnFalseBlock);
    builder->CreateBr(mergeBlock);
    
    // Merge
    builder->SetInsertPoint(mergeBlock);
    llvm::PHINode* phi = builder->CreatePHI(llvm::Type::getInt1Ty(*context), 3, "starts_with.result");
    phi->addIncoming(llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context), 0), returnFalseBlock);
    phi->addIncoming(llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context), 1), lenOkBlock);
    phi->addIncoming(isMatch, compareBlock);
    
    std::cout << "DEBUG: String::starts_with() called" << std::endl;
    m_currentLLVMValue = phi;
}

void LLVMCodegen::handleStringEndsWith(vyn::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType) {
    if (node->arguments.size() != 1) {
        logError(node->loc, "String::ends_with expects exactly 1 argument (suffix string)");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Evaluate suffix argument
    node->arguments[0]->accept(*this);
    llvm::Value* suffixStr = m_currentLLVMValue;
    if (!suffixStr) {
        logError(node->arguments[0]->loc, "Failed to evaluate suffix for ends_with");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Load main string fields
    llvm::Value* dataPtr = builder->CreateStructGEP(strStructType, strPtr, 0, "str.data_ptr");
    llvm::Value* lenPtr = builder->CreateStructGEP(strStructType, strPtr, 1, "str.len_ptr");
    llvm::Value* data = builder->CreateLoad(llvm::PointerType::get(*context, 0), dataPtr, "str.data");
    llvm::Value* len = builder->CreateLoad(llvm::Type::getInt64Ty(*context), lenPtr, "str.len");
    
    // Load suffix string fields
    llvm::Value* suffixData = builder->CreateExtractValue(suffixStr, 0, "suffix.data");
    llvm::Value* suffixLen = builder->CreateExtractValue(suffixStr, 1, "suffix.len");
    
    // Check if suffix length > string length
    llvm::BasicBlock* lenOkBlock = llvm::BasicBlock::Create(*context, "len_ok", currentFunction);
    llvm::BasicBlock* returnFalseBlock = llvm::BasicBlock::Create(*context, "return_false", currentFunction);
    llvm::BasicBlock* compareBlock = llvm::BasicBlock::Create(*context, "compare", currentFunction);
    llvm::BasicBlock* mergeBlock = llvm::BasicBlock::Create(*context, "ends_with_merge", currentFunction);
    
    llvm::Value* lenOk = builder->CreateICmpSLE(suffixLen, len);
    builder->CreateCondBr(lenOk, lenOkBlock, returnFalseBlock);
    
    // If suffix is empty, return true
    builder->SetInsertPoint(lenOkBlock);
    llvm::Value* suffixIsEmpty = builder->CreateICmpEQ(suffixLen, llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 0));
    builder->CreateCondBr(suffixIsEmpty, mergeBlock, compareBlock);
    
    // Compare suffix with end of string using memcmp
    builder->SetInsertPoint(compareBlock);
    llvm::Value* offset = builder->CreateSub(len, suffixLen, "end.offset");
    llvm::Value* endPtr = builder->CreateGEP(llvm::Type::getInt8Ty(*context), data, offset, "end.ptr");
    
    llvm::FunctionType* memcmpType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(*context),
        {llvm::PointerType::get(*context, 0), llvm::PointerType::get(*context, 0), llvm::Type::getInt64Ty(*context)},
        false
    );
    llvm::Function* memcmpFunc = module->getFunction("memcmp");
    if (!memcmpFunc) {
        memcmpFunc = llvm::Function::Create(memcmpType, llvm::Function::ExternalLinkage, "memcmp", module.get());
    }
    llvm::Value* cmpResult = builder->CreateCall(memcmpFunc, {endPtr, suffixData, suffixLen}, "cmp.result");
    llvm::Value* isMatch = builder->CreateICmpEQ(cmpResult, llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0));
    builder->CreateBr(mergeBlock);
    
    // Return false
    builder->SetInsertPoint(returnFalseBlock);
    builder->CreateBr(mergeBlock);
    
    // Merge
    builder->SetInsertPoint(mergeBlock);
    llvm::PHINode* phi = builder->CreatePHI(llvm::Type::getInt1Ty(*context), 3, "ends_with.result");
    phi->addIncoming(llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context), 0), returnFalseBlock);
    phi->addIncoming(llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context), 1), lenOkBlock);
    phi->addIncoming(isMatch, compareBlock);
    
    std::cout << "DEBUG: String::ends_with() called" << std::endl;
    m_currentLLVMValue = phi;
}

void LLVMCodegen::handleStringContains(vyn::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType) {
    if (node->arguments.size() != 1) {
        logError(node->loc, "String::contains expects exactly 1 argument (substring)");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // For now, use a simple implementation that calls strstr
    // A more efficient implementation would use a substring search algorithm
    
    // Load main string fields
    llvm::Value* dataPtr = builder->CreateStructGEP(strStructType, strPtr, 0, "str.data_ptr");
    llvm::Value* data = builder->CreateLoad(llvm::PointerType::get(*context, 0), dataPtr, "str.data");
    
    // Evaluate substring argument
    node->arguments[0]->accept(*this);
    llvm::Value* substringStr = m_currentLLVMValue;
    if (!substringStr) {
        logError(node->arguments[0]->loc, "Failed to evaluate substring for contains");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Load substring fields
    llvm::Value* substringData = builder->CreateExtractValue(substringStr, 0, "substring.data");
    
    // Call strstr
    llvm::FunctionType* strstrType = llvm::FunctionType::get(
        llvm::PointerType::get(*context, 0),
        {llvm::PointerType::get(*context, 0), llvm::PointerType::get(*context, 0)},
        false
    );
    llvm::Function* strstrFunc = module->getFunction("strstr");
    if (!strstrFunc) {
        strstrFunc = llvm::Function::Create(strstrType, llvm::Function::ExternalLinkage, "strstr", module.get());
    }
    llvm::Value* result = builder->CreateCall(strstrFunc, {data, substringData}, "strstr.result");
    
    // Check if result is not NULL
    llvm::Value* isFound = builder->CreateICmpNE(result, llvm::ConstantPointerNull::get(llvm::PointerType::get(*context, 0)), "contains.result");
    
    std::cout << "DEBUG: String::contains() called" << std::endl;
    m_currentLLVMValue = isFound;
}

void LLVMCodegen::handleStringToUpper(vyn::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType) {
    if (!node->arguments.empty()) {
        logError(node->loc, "String::to_upper expects no arguments");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Load string fields
    llvm::Value* dataPtr = builder->CreateStructGEP(strStructType, strPtr, 0, "str.data_ptr");
    llvm::Value* lenPtr = builder->CreateStructGEP(strStructType, strPtr, 1, "str.len_ptr");
    llvm::Value* data = builder->CreateLoad(llvm::PointerType::get(*context, 0), dataPtr, "str.data");
    llvm::Value* len = builder->CreateLoad(llvm::Type::getInt64Ty(*context), lenPtr, "str.len");
    
    // Allocate new buffer (len + 1 for null terminator)
    llvm::Value* bufferSize = builder->CreateAdd(len, llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 1), "buffer.size");
    llvm::FunctionType* mallocType = llvm::FunctionType::get(
        llvm::PointerType::get(*context, 0),
        {llvm::Type::getInt64Ty(*context)},
        false
    );
    llvm::Function* mallocFunc = module->getFunction("malloc");
    if (!mallocFunc) {
        mallocFunc = llvm::Function::Create(mallocType, llvm::Function::ExternalLinkage, "malloc", module.get());
    }
    llvm::Value* newData = builder->CreateCall(mallocFunc, {bufferSize}, "new.data");
    
    // Create loop to convert each character
    llvm::BasicBlock* loopCondBlock = llvm::BasicBlock::Create(*context, "loop.cond", currentFunction);
    llvm::BasicBlock* loopBodyBlock = llvm::BasicBlock::Create(*context, "loop.body", currentFunction);
    llvm::BasicBlock* loopEndBlock = llvm::BasicBlock::Create(*context, "loop.end", currentFunction);
    
    // Initialize index
    llvm::Value* indexPtr = builder->CreateAlloca(llvm::Type::getInt64Ty(*context), nullptr, "index.ptr");
    builder->CreateStore(llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 0), indexPtr);
    builder->CreateBr(loopCondBlock);
    
    // Loop condition: index < len
    builder->SetInsertPoint(loopCondBlock);
    llvm::Value* index = builder->CreateLoad(llvm::Type::getInt64Ty(*context), indexPtr, "index");
    llvm::Value* cond = builder->CreateICmpSLT(index, len, "loop.cond");
    builder->CreateCondBr(cond, loopBodyBlock, loopEndBlock);
    
    // Loop body: convert character and store
    builder->SetInsertPoint(loopBodyBlock);
    llvm::Value* srcPtr = builder->CreateGEP(llvm::Type::getInt8Ty(*context), data, index, "src.ptr");
    llvm::Value* ch = builder->CreateLoad(llvm::Type::getInt8Ty(*context), srcPtr, "ch");
    
    // toupper: if (ch >= 'a' && ch <= 'z') ch = ch - 32
    llvm::Value* isLower = builder->CreateAnd(
        builder->CreateICmpSGE(ch, llvm::ConstantInt::get(llvm::Type::getInt8Ty(*context), 97)),  // 'a'
        builder->CreateICmpSLE(ch, llvm::ConstantInt::get(llvm::Type::getInt8Ty(*context), 122)), // 'z'
        "is.lower"
    );
    llvm::Value* upperCh = builder->CreateSub(ch, llvm::ConstantInt::get(llvm::Type::getInt8Ty(*context), 32), "upper.ch");
    llvm::Value* convertedCh = builder->CreateSelect(isLower, upperCh, ch, "converted.ch");
    
    llvm::Value* dstPtr = builder->CreateGEP(llvm::Type::getInt8Ty(*context), newData, index, "dst.ptr");
    builder->CreateStore(convertedCh, dstPtr);
    
    // Increment index
    llvm::Value* nextIndex = builder->CreateAdd(index, llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 1), "next.index");
    builder->CreateStore(nextIndex, indexPtr);
    builder->CreateBr(loopCondBlock);
    
    // After loop: add null terminator
    builder->SetInsertPoint(loopEndBlock);
    llvm::Value* nullTermPtr = builder->CreateGEP(llvm::Type::getInt8Ty(*context), newData, len, "null.term.ptr");
    builder->CreateStore(llvm::ConstantInt::get(llvm::Type::getInt8Ty(*context), 0), nullTermPtr);
    
    // Create result String struct
    llvm::Value* resultStr = llvm::UndefValue::get(strStructType);
    resultStr = builder->CreateInsertValue(resultStr, newData, 0, "result.data");
    resultStr = builder->CreateInsertValue(resultStr, len, 1, "result.len");
    
    std::cout << "DEBUG: String::to_upper() called" << std::endl;
    m_currentLLVMValue = resultStr;
}

void LLVMCodegen::handleStringToLower(vyn::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType) {
    if (!node->arguments.empty()) {
        logError(node->loc, "String::to_lower expects no arguments");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Load string fields
    llvm::Value* dataPtr = builder->CreateStructGEP(strStructType, strPtr, 0, "str.data_ptr");
    llvm::Value* lenPtr = builder->CreateStructGEP(strStructType, strPtr, 1, "str.len_ptr");
    llvm::Value* data = builder->CreateLoad(llvm::PointerType::get(*context, 0), dataPtr, "str.data");
    llvm::Value* len = builder->CreateLoad(llvm::Type::getInt64Ty(*context), lenPtr, "str.len");
    
    // Allocate new buffer (len + 1 for null terminator)
    llvm::Value* bufferSize = builder->CreateAdd(len, llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 1), "buffer.size");
    llvm::FunctionType* mallocType = llvm::FunctionType::get(
        llvm::PointerType::get(*context, 0),
        {llvm::Type::getInt64Ty(*context)},
        false
    );
    llvm::Function* mallocFunc = module->getFunction("malloc");
    if (!mallocFunc) {
        mallocFunc = llvm::Function::Create(mallocType, llvm::Function::ExternalLinkage, "malloc", module.get());
    }
    llvm::Value* newData = builder->CreateCall(mallocFunc, {bufferSize}, "new.data");
    
    // Create loop to convert each character
    llvm::BasicBlock* loopCondBlock = llvm::BasicBlock::Create(*context, "loop.cond", currentFunction);
    llvm::BasicBlock* loopBodyBlock = llvm::BasicBlock::Create(*context, "loop.body", currentFunction);
    llvm::BasicBlock* loopEndBlock = llvm::BasicBlock::Create(*context, "loop.end", currentFunction);
    
    // Initialize index
    llvm::Value* indexPtr = builder->CreateAlloca(llvm::Type::getInt64Ty(*context), nullptr, "index.ptr");
    builder->CreateStore(llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 0), indexPtr);
    builder->CreateBr(loopCondBlock);
    
    // Loop condition: index < len
    builder->SetInsertPoint(loopCondBlock);
    llvm::Value* index = builder->CreateLoad(llvm::Type::getInt64Ty(*context), indexPtr, "index");
    llvm::Value* cond = builder->CreateICmpSLT(index, len, "loop.cond");
    builder->CreateCondBr(cond, loopBodyBlock, loopEndBlock);
    
    // Loop body: convert character and store
    builder->SetInsertPoint(loopBodyBlock);
    llvm::Value* srcPtr = builder->CreateGEP(llvm::Type::getInt8Ty(*context), data, index, "src.ptr");
    llvm::Value* ch = builder->CreateLoad(llvm::Type::getInt8Ty(*context), srcPtr, "ch");
    
    // tolower: if (ch >= 'A' && ch <= 'Z') ch = ch + 32
    llvm::Value* isUpper = builder->CreateAnd(
        builder->CreateICmpSGE(ch, llvm::ConstantInt::get(llvm::Type::getInt8Ty(*context), 65)),  // 'A'
        builder->CreateICmpSLE(ch, llvm::ConstantInt::get(llvm::Type::getInt8Ty(*context), 90)),  // 'Z'
        "is.upper"
    );
    llvm::Value* lowerCh = builder->CreateAdd(ch, llvm::ConstantInt::get(llvm::Type::getInt8Ty(*context), 32), "lower.ch");
    llvm::Value* convertedCh = builder->CreateSelect(isUpper, lowerCh, ch, "converted.ch");
    
    llvm::Value* dstPtr = builder->CreateGEP(llvm::Type::getInt8Ty(*context), newData, index, "dst.ptr");
    builder->CreateStore(convertedCh, dstPtr);
    
    // Increment index
    llvm::Value* nextIndex = builder->CreateAdd(index, llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), 1), "next.index");
    builder->CreateStore(nextIndex, indexPtr);
    builder->CreateBr(loopCondBlock);
    
    // After loop: add null terminator
    builder->SetInsertPoint(loopEndBlock);
    llvm::Value* nullTermPtr = builder->CreateGEP(llvm::Type::getInt8Ty(*context), newData, len, "null.term.ptr");
    builder->CreateStore(llvm::ConstantInt::get(llvm::Type::getInt8Ty(*context), 0), nullTermPtr);
    
    // Create result String struct
    llvm::Value* resultStr = llvm::UndefValue::get(strStructType);
    resultStr = builder->CreateInsertValue(resultStr, newData, 0, "result.data");
    resultStr = builder->CreateInsertValue(resultStr, len, 1, "result.len");
    
    std::cout << "DEBUG: String::to_lower() called" << std::endl;
    m_currentLLVMValue = resultStr;
}

} // namespace vyn
