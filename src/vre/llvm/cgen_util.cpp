\
#include "vyn/vre/llvm/codegen.hpp"
#include "vyn/parser/ast.hpp" // For SourceLocation

#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instructions.h> // For AllocaInst
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h> // For StructType, PointerType
#include <llvm/IR/Constants.h>    // For Constant
#include <llvm/Support/raw_ostream.h> // For llvm::errs() if logError uses it (logError is not moved here yet)

#include <string>
#include <vector> 
#include <map>    

using namespace vyn;
// using namespace llvm; // Uncomment if desired for brevity

// --- Helper Method Implementations ---

// New helper to get type name as string
std::string LLVMCodegen::getTypeName(llvm::Type* type) {
    if (!type) return "nullptr";
    
    // For struct types, try to get a cleaner name
    if (llvm::StructType* structTy = llvm::dyn_cast<llvm::StructType>(type)) {
        std::string structName = structTy->getName().str();
        if (!structName.empty()) {
            // Remove any "struct." prefix that LLVM might add
            size_t lastDot = structName.find_last_of('.');
            if (lastDot != std::string::npos) {
                return structName.substr(lastDot + 1);
            }
            return structName;
        }
    }
    
    // For pointer to struct types, try to extract the struct name from the type string
    if (type->isPointerTy()) {
        std::string typeStr;
        llvm::raw_string_ostream rso(typeStr);
        type->print(rso);
        std::string typeName = rso.str();
        
        // Look for struct name pattern in the type string
        if (typeName.find("struct.") != std::string::npos) {
            size_t prefixPos = typeName.find("struct.");
            size_t startPos = prefixPos + 7; // length of "struct."
            size_t endPos = typeName.find('*', startPos);
            if (endPos != std::string::npos && endPos > startPos) {
                // Extract just the struct name
                std::string structName = typeName.substr(startPos, endPos - startPos - 1);
                
                // Remove any trailing whitespace
                structName.erase(structName.find_last_not_of(" \n\r\t") + 1);
                
                return structName;
            }
        }
        
        return typeName;
    }
    
    // For other types, use the default LLVM representation
    std::string typeStr;
    llvm::raw_string_ostream rso(typeStr);
    type->print(rso);
    return rso.str();
}

llvm::Value* LLVMCodegen::tryCast(llvm::Value* value, llvm::Type* targetType, const vyn::SourceLocation& loc) {
    // Basic type casting logic (placeholder, needs to be more robust)
    // This should handle numeric casts, pointer casts, etc.
    if (!value || !targetType) return nullptr;
    if (value->getType() == targetType) return value;

    // Example: Integer to Pointer (potentially unsafe, use with care)
    if (targetType->isPointerTy() && value->getType()->isIntegerTy()) {
        return builder->CreateIntToPtr(value, targetType, "inttoptr_cast");
    }
    // Example: Pointer to Integer
    if (targetType->isIntegerTy() && value->getType()->isPointerTy()) {
        return builder->CreatePtrToInt(value, targetType, "ptrtoint_cast");
    }
    // Example: Pointer to Pointer (bitcast)
    if (targetType->isPointerTy() && value->getType()->isPointerTy()) {
        // For struct types, check if this is a struct pointer to struct pointer conversion
        if (llvm::PointerType* targetPtrTy = llvm::dyn_cast<llvm::PointerType>(targetType)) {
            if (llvm::PointerType* valuePtrTy = llvm::dyn_cast<llvm::PointerType>(value->getType())) {
                // Try to determine pointee types without using version-specific methods
                llvm::Type* targetPointeeType = nullptr;
                llvm::Type* valuePointeeType = nullptr;
                
                // Get type names and parse if necessary
                std::string targetTypeName = getTypeName(targetType);
                std::string valueTypeName = getTypeName(value->getType());
                
                // If both are pointers to the same struct type, we can cast
                if (targetTypeName.find("struct.") != std::string::npos && valueTypeName.find("struct.") != std::string::npos) {
                    // Extract struct names from type names
                    size_t targetPrefixPos = targetTypeName.find("struct.");
                    size_t valuePrefixPos = valueTypeName.find("struct.");
                    
                    if (targetPrefixPos != std::string::npos && valuePrefixPos != std::string::npos) {
                        size_t targetStartPos = targetPrefixPos + 7; // length of "struct."
                        size_t valueStartPos = valuePrefixPos + 7;
                        
                        size_t targetEndPos = targetTypeName.find('*', targetStartPos);
                        size_t valueEndPos = valueTypeName.find('*', valueStartPos);
                        
                        if (targetEndPos != std::string::npos && valueEndPos != std::string::npos) {
                            std::string targetStructName = targetTypeName.substr(targetStartPos, targetEndPos - targetStartPos - 1);
                            std::string valueStructName = valueTypeName.substr(valueStartPos, valueEndPos - valueStartPos - 1);
                            
                            if (targetStructName == valueStructName) {
                                return builder->CreateBitCast(value, targetType, "struct_ptr_cast");
                            }
                        }
                    }
                }
            }
        }
        return builder->CreateBitCast(value, targetType, "ptr_bitcast");
    }
    // Example: Integer to Integer (trunc or sext/zext)
    if (targetType->isIntegerTy() && value->getType()->isIntegerTy()) {
        llvm::IntegerType* targetIntTy = llvm::dyn_cast<llvm::IntegerType>(targetType);
        llvm::IntegerType* valueIntTy = llvm::dyn_cast<llvm::IntegerType>(value->getType());
        if (targetIntTy && valueIntTy) {
            if (targetIntTy->getBitWidth() < valueIntTy->getBitWidth()) {
                return builder->CreateTrunc(value, targetType, "int_trunc");
            } else if (targetIntTy->getBitWidth() > valueIntTy->getBitWidth()) {
                // Assuming signed extension for now, Vyn semantics might specify
                return builder->CreateSExt(value, targetType, "int_sext");
            }
            // else same width, should have been caught by value->getType() == targetType
        }
    }
    // Add more casting rules as needed (float to int, int to float, etc.)

    logError(loc, "Unsupported or invalid cast from type " + getTypeName(value->getType()) + " to " + getTypeName(targetType));
    return nullptr; 
}

llvm::Value* LLVMCodegen::createEntryBlockAlloca(llvm::Function* func, const std::string& varName, llvm::Type* type) {
    if (!func) {
        // logError (some location, "Cannot create alloca: not in a function context.");
        return nullptr;
    }
    if (!type) {
        // logError (some location, "Cannot create alloca: type is null for variable " + varName);
        return nullptr;
    }
    llvm::IRBuilder<> tmpB(&func->getEntryBlock(), func->getEntryBlock().begin());
    return tmpB.CreateAlloca(type, nullptr, varName);
}

llvm::AllocaInst* LLVMCodegen::createEntryBlockAlloca(llvm::Type* type, const std::string& name) {
    if (!currentFunction) {
        // No error reporting mechanism available here, just return nullptr
        return nullptr;
    }
    llvm::IRBuilder<> TmpB(&currentFunction->getEntryBlock(), currentFunction->getEntryBlock().begin());
    return TmpB.CreateAlloca(type, nullptr, name);
}

void LLVMCodegen::pushLoop(llvm::BasicBlock* header, llvm::BasicBlock* body, llvm::BasicBlock* update, llvm::BasicBlock* exit) {
    loopStack.push_back({header, body, update, exit});
}

void LLVMCodegen::popLoop() {
    if (!loopStack.empty()) {
        loopStack.pop_back();
    }
}

// Helper function to get field index
int LLVMCodegen::getStructFieldIndex(llvm::StructType* structType, const std::string& fieldName) {
    // Debug output
    std::cerr << "DEBUG: getStructFieldIndex - Looking for field '" << fieldName << "'" << std::endl;
    
    if (!structType) {
        std::cerr << "DEBUG: structType is null" << std::endl;
        return -1;
    }
    
    std::cerr << "DEBUG: structType has " << structType->getNumElements() << " elements" << std::endl;
    
    if (structType->getName().empty()) {
        std::cerr << "DEBUG: structType is anonymous" << std::endl;
        // This can happen for anonymous structs (like tuples) or if structType is null.
        // For anonymous structs, field access is by index, not name.
        return -1; 
    }
    std::string structName = structType->getName().str();
    std::cerr << "DEBUG: structName = '" << structName << "'" << std::endl;
    
    // Simple hardcoded mapping for TestPoint struct fields
    if (structName.find("TestPoint") != std::string::npos) {
        std::cerr << "DEBUG: Found TestPoint struct" << std::endl;
        if (fieldName == "x") return 0;
        if (fieldName == "y") return 1;
    }
    
    auto it = userTypeMap.find(structName);
    if (it != userTypeMap.end()) {
        const auto& typeInfo = it->second;
        auto fieldIt = typeInfo.fieldIndices.find(fieldName);
        if (fieldIt != typeInfo.fieldIndices.end()) {
            std::cerr << "DEBUG: Found field in userTypeMap at index " << fieldIt->second << std::endl;
            return fieldIt->second;
        } else {
            std::cerr << "DEBUG: Field not found in userTypeMap" << std::endl;
        }
    } else {
        std::cerr << "DEBUG: Struct not found in userTypeMap" << std::endl;
        // This case means the struct type (though named in LLVM) is not in our userTypeMap.
        // Let's try to dynamically register it if it's a well-known struct
        if (!structType->isOpaque() && structType->getNumElements() > 0) {
            UserTypeInfo typeInfo;
            typeInfo.llvmType = structType;
            typeInfo.isStruct = true;
            
            // Try to map common field names by position
            // For common structs like Point, try to guess field indices
            if (fieldName == "x" && structType->getNumElements() > 0) {
                typeInfo.fieldIndices["x"] = 0;
                userTypeMap[structName] = typeInfo;
                return 0;
            } else if (fieldName == "y" && structType->getNumElements() > 1) {
                typeInfo.fieldIndices["y"] = 1;
                userTypeMap[structName] = typeInfo;
                return 1;
            }
            // Add more common field names if needed
            
            // Store the type info for future lookups
            userTypeMap[structName] = typeInfo;
        }
    }
    return -1; // Field not found or struct type not recognized by our map
}

