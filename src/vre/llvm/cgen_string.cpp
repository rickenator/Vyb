#include "vyn/vre/llvm/codegen.hpp"
#include "vyn/parser/ast.hpp"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/DerivedTypes.h>

namespace vyn {

// Implementation for string concatenation
llvm::Value* LLVMCodegen::generateStringConcatenation(llvm::Value* leftStr, llvm::Value* rightStr, SourceLocation loc) {
    if (!leftStr || !rightStr) {
        logError(loc, "Invalid string operands for concatenation");
        return nullptr;
    }
    
    // If either operand is a String struct {ptr, len}, extract the pointer
    if (leftStr->getType()->isStructTy()) {
        llvm::StructType* st = llvm::cast<llvm::StructType>(leftStr->getType());
        if (st->getNumElements() == 2) {
            leftStr = builder->CreateExtractValue(leftStr, 0, "left.str.data");
        }
    }
    if (rightStr->getType()->isStructTy()) {
        llvm::StructType* st = llvm::cast<llvm::StructType>(rightStr->getType());
        if (st->getNumElements() == 2) {
            rightStr = builder->CreateExtractValue(rightStr, 0, "right.str.data");
        }
    }
    
    // Create function signature for string concatenation helper
    llvm::FunctionType* concatFuncType = llvm::FunctionType::get(
        int8PtrType,                                 // Return type: char* (string)
        {int8PtrType, int8PtrType},                  // Parameters: (char*, char*)
        false                                        // Not vararg
    );
    
    // Get or declare the runtime string concatenation function
    llvm::Function* concatFunc = module->getFunction("__vyn_string_concat");
    
    if (!concatFunc) {
        concatFunc = llvm::Function::Create(
            concatFuncType,
            llvm::Function::ExternalLinkage,
            "__vyn_string_concat",
            module.get()
        );
        
        // Set parameter names for better readability
        auto args = concatFunc->arg_begin();
        args->setName("left");
        (++args)->setName("right");
    }
    
    // Create call to string concatenation function with our two operands
    std::vector<llvm::Value*> args = {leftStr, rightStr};
    return builder->CreateCall(concatFunc, args, "strcattmp");
}

// Helper method to resolve type aliases to their base type names
std::string LLVMCodegen::resolveTypeAliasToBaseName(vyn::ast::TypeNode* typeNode) {
    if (!typeNode) return "";
    
    if (auto typeName = dynamic_cast<vyn::ast::TypeName*>(typeNode)) {
        if (typeName->identifier) {
            std::string name = typeName->identifier->name;
            
            // Check if this is a type alias and resolve it
            auto aliasIt = typeAliasMap.find(name);
            if (aliasIt != typeAliasMap.end()) {
                llvm::Type* resolvedType = aliasIt->second;
                
                // Convert LLVM type back to string name  
                if (resolvedType == int64Type) return "Int";
                if (resolvedType == int8Type) return "Int8";
                if (resolvedType == int32Type) return "Int32";
                if (resolvedType == int1Type) return "Bool";
                if (resolvedType == floatType) return "Float";
                if (resolvedType == doubleType) return "Double";
                if (resolvedType == int8PtrType) return "String";
                
                return name; // Fallback to original name
            }
            
            return name; // Not an alias, return original name
        }
    }
    
    return ""; // Could not resolve
}

// Helper method to generate toString call for a given value and type
llvm::Value* LLVMCodegen::generateToStringCall(llvm::Value* value, llvm::Type* valueType, vyn::ast::TypeNode* astType, SourceLocation loc) {
    if (!value || !valueType) {
        logError(loc, "Invalid value or type for toString conversion");
        return nullptr;
    }
    
    // Check if value is a String struct type
    if (valueType->isStructTy()) {
        llvm::StructType* structType = llvm::cast<llvm::StructType>(valueType);
        if (structType->getNumElements() == 2) {
            // This is a String struct {ptr, len}, extract the data pointer
            llvm::Value* dataPtr = builder->CreateExtractValue(value, 0, "str.data_for_concat");
            return dataPtr;
        }
    }
    
    // Get the base type name, resolving type aliases
    std::string typeName = resolveTypeAliasToBaseName(astType);
    if (typeName.empty() && astType) {
        // Fallback to AST type string representation
        typeName = astType->toString();
    }
    
    // Determine appropriate toString function based on LLVM type and AST type
    std::string toStringFuncName;
    llvm::FunctionType* toStringFuncType = nullptr;
    
    if (valueType == int64Type || typeName == "Int") {
        toStringFuncName = "__vyn_toString_int";
        toStringFuncType = llvm::FunctionType::get(int8PtrType, {int64Type}, false);
    } else if (valueType == int8Type || typeName == "Int8") {
        toStringFuncName = "__vyn_toString_int8";
        toStringFuncType = llvm::FunctionType::get(int8PtrType, {int8Type}, false);
    } else if (valueType == int32Type || typeName == "Int32") {
        toStringFuncName = "__vyn_toString_int32";
        toStringFuncType = llvm::FunctionType::get(int8PtrType, {int32Type}, false);
    } else if (valueType == int1Type || typeName == "Bool") {
        toStringFuncName = "__vyn_toString_bool";
        toStringFuncType = llvm::FunctionType::get(int8PtrType, {int1Type}, false);
    } else if (valueType->isFloatingPointTy() || typeName == "Float" || typeName == "Double") {
        toStringFuncName = "__vyn_toString_float";
        toStringFuncType = llvm::FunctionType::get(int8PtrType, {doubleType}, false);
        // Cast to double if needed
        if (valueType != doubleType) {
            value = builder->CreateFPExt(value, doubleType, "todouble");
        }
    } else if (valueType == int8PtrType || typeName == "String") {
        toStringFuncName = "__vyn_toString_string";
        toStringFuncType = llvm::FunctionType::get(int8PtrType, {int8PtrType}, false);
    } else {
        logError(loc, "Unsupported type for toString conversion: " + (astType ? astType->toString() : "unknown"));
        return nullptr;
    }
    
    // Get or declare the toString function
    llvm::Function* toStringFunc = module->getFunction(toStringFuncName);
    if (!toStringFunc) {
        toStringFunc = llvm::Function::Create(
            toStringFuncType,
            llvm::Function::ExternalLinkage,
            toStringFuncName,
            module.get()
        );
    }
    
    // Cast value to correct type if needed
    if (value->getType() != toStringFuncType->getParamType(0)) {
        if (value->getType()->isIntegerTy() && toStringFuncType->getParamType(0)->isIntegerTy()) {
            // Integer types - extend or truncate as needed
            if (value->getType()->getIntegerBitWidth() < toStringFuncType->getParamType(0)->getIntegerBitWidth()) {
                value = builder->CreateSExt(value, toStringFuncType->getParamType(0), "sext");
            } else if (value->getType()->getIntegerBitWidth() > toStringFuncType->getParamType(0)->getIntegerBitWidth()) {
                value = builder->CreateTrunc(value, toStringFuncType->getParamType(0), "trunc");
            }
        }
    }
    
    // Call the toString function
    return builder->CreateCall(toStringFunc, {value}, "tostring");
}

// Enhanced string concatenation that handles mixed types by converting non-strings to strings
llvm::Value* LLVMCodegen::generateMixedStringConcatenation(llvm::Value* leftValue, llvm::Value* rightValue, 
                                                         vyn::ast::TypeNode* leftTypeNode, vyn::ast::TypeNode* rightTypeNode, SourceLocation loc) {
    if (!leftValue || !rightValue) {
        logError(loc, "Invalid operands for mixed string concatenation");
        return nullptr;
    }
    
    llvm::Value* leftString = nullptr;
    llvm::Value* rightString = nullptr;
    
    // Determine if left operand is already a string
    std::string leftTypeName = resolveTypeAliasToBaseName(leftTypeNode);
    bool leftIsString = (leftValue->getType() == int8PtrType) || (leftTypeName == "String");
    // Also check for String struct type
    if (leftValue->getType()->isStructTy()) {
        llvm::StructType* st = llvm::cast<llvm::StructType>(leftValue->getType());
        if (st->getNumElements() == 2) leftIsString = true;
    }
    
    // Determine if right operand is already a string  
    std::string rightTypeName = resolveTypeAliasToBaseName(rightTypeNode);
    bool rightIsString = (rightValue->getType() == int8PtrType) || (rightTypeName == "String");
    // Also check for String struct type
    if (rightValue->getType()->isStructTy()) {
        llvm::StructType* st = llvm::cast<llvm::StructType>(rightValue->getType());
        if (st->getNumElements() == 2) rightIsString = true;
    }
    
    // Convert left operand to string if needed
    if (leftIsString) {
        leftString = leftValue;
    } else {
        leftString = generateToStringCall(leftValue, leftValue->getType(), leftTypeNode, loc);
        if (!leftString) {
            logError(loc, "Failed to convert left operand to string");
            return nullptr;
        }
    }
    
    // Convert right operand to string if needed
    if (rightIsString) {
        rightString = rightValue;
    } else {
        rightString = generateToStringCall(rightValue, rightValue->getType(), rightTypeNode, loc);
        if (!rightString) {
            logError(loc, "Failed to convert right operand to string");
            return nullptr;
        }
    }
    
    // Now concatenate the two strings
    return generateStringConcatenation(leftString, rightString, loc);
}

} // namespace vyn
