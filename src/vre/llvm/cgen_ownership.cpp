#include "vyn/vre/llvm/codegen.hpp"
#include "vyn/parser/ast.hpp"
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Constants.h>
#include <iostream>

using namespace vyn;

// ============================================================================
// CONTROL BLOCK STRUCTURE FOR our<T> AND mild<T>
// ============================================================================
// 
// Control blocks enable mild<T> references to detect when our<T> objects
// are freed while allowing the control block to survive for weak reference
// tracking.
//
// Memory Layout:
//   struct ControlBlock {
//     i32 strong_count;    // Number of our<T> references (shared ownership)
//     i32 weak_count;      // Number of mild<T> references (weak references)
//     i1  object_freed;    // True when object destroyed (for .released() check)
//     T*  object_ptr;      // Pointer to actual object data
//   }
//
// Lifecycle:
//   1. our() allocates control block + object
//   2. soft() increments weak_count
//   3. our<T> destructor: decrements strong_count, if 0 frees object & sets object_freed
//   4. mild<T> destructor: decrements weak_count
//   5. Control block freed when BOTH strong_count==0 AND weak_count==0
//
// Thread Safety:
//   - All count increments/decrements use atomic operations
//   - object_freed reads use atomic load with acquire semantics
//
// ============================================================================

// Helper function to get or create control block struct type
llvm::StructType* LLVMCodegen::getControlBlockType(llvm::Type* objectPtrType) {
    // Control block: { i32 strong_count, i32 weak_count, i1 object_freed, T* object_ptr }
    std::vector<llvm::Type*> fields = {
        llvm::Type::getInt32Ty(*context),  // strong_count
        llvm::Type::getInt32Ty(*context),  // weak_count
        llvm::Type::getInt1Ty(*context),   // object_freed
        objectPtrType                       // object_ptr (pointer to actual object)
    };
    
    return llvm::StructType::get(*context, fields, /*isPacked=*/false);
}

// --- Scope Management ---
void LLVMCodegen::enterScope() {
    std::cout << "DEBUG: Entering new scope (depth: " << scopeStack.size() + 1 << ")" << std::endl;
    scopeStack.emplace_back(); // Create new scope level
}

void LLVMCodegen::exitScope() {
    if (scopeStack.empty()) {
        std::cout << "WARNING: Attempted to exit scope but no scopes active" << std::endl;
        return;
    }
    
    std::cout << "DEBUG: Exiting scope (depth: " << scopeStack.size() << "), cleaning up " 
              << scopeStack.back().size() << " variables" << std::endl;
    
    // Clean up all variables in current scope in reverse order (LIFO)
    auto& currentScope = scopeStack.back();
    for (auto it = currentScope.rbegin(); it != currentScope.rend(); ++it) {
        cleanupVariable(*it);
    }
    
    scopeStack.pop_back(); // Remove current scope
}

void LLVMCodegen::registerVariable(const std::string& name, llvm::Value* allocaInst, llvm::Value* value, 
                                  ast::OwnershipKind ownership, llvm::Type* type, bool needsCleanup) {
    if (scopeStack.empty()) {
        std::cout << "ERROR: No active scope to register variable: " << name << std::endl;
        return;
    }
    
    std::cout << "DEBUG: Registering variable '" << name << "' with ownership: " 
              << static_cast<int>(ownership) << ", needsCleanup: " << needsCleanup << std::endl;
    
    ScopeVariable var;
    var.name = name;
    var.allocaInst = allocaInst;
    var.value = value;
    var.ownership = ownership;
    var.needsCleanup = needsCleanup;
    var.type = type;
    var.isVecWithMallocData = needsCleanup; // Vec types that need cleanup have malloc'd data
    
    if (var.isVecWithMallocData) {
        std::cout << "DEBUG: Variable '" << name << "' identified as Vec with malloc'd data" << std::endl;
    }
    
    scopeStack.back().push_back(var);
    
    // Handle reference counting for our<T>
    if (ownership == ast::OwnershipKind::OUR) {
        incrementRefCount(name);
    }
}

void LLVMCodegen::cleanupVariable(const ScopeVariable& var) {
    std::cout << "DEBUG: Cleaning up variable '" << var.name << "', needsCleanup: " 
              << var.needsCleanup << ", isVecWithMallocData: " << var.isVecWithMallocData << std::endl;
    
    // Skip cleanup if not needed
    if (!var.needsCleanup) {
        std::cout << "DEBUG: Skipping cleanup for '" << var.name << "' (needsCleanup=false)" << std::endl;
        return;
    }
    
    switch (var.ownership) {
        case ast::OwnershipKind::MY: {
            // Unique ownership - immediate cleanup
            if (var.isVecWithMallocData) {
                std::cout << "DEBUG: Performing MY cleanup for Vec with malloc'd data: " << var.name << std::endl;
                
                // Safety check: ensure we have valid alloca and builder
                if (!var.allocaInst || !builder || !currentFunction) {
                    std::cout << "ERROR: Invalid state for cleanup of " << var.name << std::endl;
                    return;
                }
                
                // Load the Vec struct to access the data pointer
                llvm::Value* vecPtr = builder->CreateLoad(var.type, var.allocaInst, var.name + "_cleanup_load");
                
                // Extract the data pointer (field 0) from the Vec struct
                llvm::Value* dataPtr = builder->CreateExtractValue(vecPtr, 0, var.name + "_data_ptr");
                
                // Create null check before freeing
                llvm::Value* isNotNull = builder->CreateICmpNE(dataPtr, 
                    llvm::ConstantPointerNull::get(llvm::PointerType::get(*context, 0)), 
                    var.name + "_null_check");
                
                llvm::BasicBlock* freeBlock = llvm::BasicBlock::Create(*context, var.name + "_free_block", currentFunction);
                llvm::BasicBlock* continueBlock = llvm::BasicBlock::Create(*context, var.name + "_continue", currentFunction);
                
                builder->CreateCondBr(isNotNull, freeBlock, continueBlock);
                
                // Free block
                builder->SetInsertPoint(freeBlock);
                llvm::Function* freeFunc = getOrCreateFreeFunction();
                builder->CreateCall(freeFunc, {dataPtr});
                std::cout << "DEBUG: Generated free() call for " << var.name << std::endl;
                builder->CreateBr(continueBlock);
                
                // Continue block
                builder->SetInsertPoint(continueBlock);
                
                // If this cleanup is happening right before a return, we need to
                // ensure the continue block has proper termination
                // This will be handled by the subsequent return statement
            } else {
                std::cout << "DEBUG: Skipping MY cleanup for non-Vec variable: " << var.name << std::endl;
            }
            break;
        }
        
        case ast::OwnershipKind::OUR: {
            // Reference counted - decrement and cleanup if zero
            decrementRefCount(var.name);
            break;
        }
        
        case ast::OwnershipKind::THEIR: {
            // Borrowed reference - no cleanup needed
            std::cout << "DEBUG: No cleanup needed for borrowed reference: " << var.name << std::endl;
            break;
        }
    }
}

void LLVMCodegen::incrementRefCount(const std::string& name) {
    std::cout << "DEBUG: Incrementing refcount for: " << name << std::endl;
    
    // Check if refcount storage already exists
    auto it = refCountStorage.find(name);
    if (it == refCountStorage.end()) {
        // Create new refcount storage
        llvm::Type* int32Type = llvm::Type::getInt32Ty(*context);
        llvm::Value* refCountAlloca = createEntryBlockAlloca(int32Type, name + "_refcount");
        builder->CreateStore(llvm::ConstantInt::get(int32Type, 1), refCountAlloca);
        refCountStorage[name] = refCountAlloca;
        refCounts[name] = 1;
    } else {
        // Increment existing refcount
        llvm::Value* currentCount = builder->CreateLoad(llvm::Type::getInt32Ty(*context), it->second, name + "_refcount_load");
        llvm::Value* newCount = builder->CreateAdd(currentCount, llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 1));
        builder->CreateStore(newCount, it->second);
        refCounts[name]++;
    }
}

void LLVMCodegen::decrementRefCount(const std::string& name) {
    std::cout << "DEBUG: Decrementing refcount for: " << name << std::endl;
    
    auto it = refCountStorage.find(name);
    if (it == refCountStorage.end()) {
        std::cout << "ERROR: Attempted to decrement refcount for unknown variable: " << name << std::endl;
        return;
    }
    
    // Load current refcount
    llvm::Value* currentCount = builder->CreateLoad(llvm::Type::getInt32Ty(*context), it->second, name + "_refcount_load");
    llvm::Value* newCount = builder->CreateSub(currentCount, llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 1));
    builder->CreateStore(newCount, it->second);
    
    // Check if refcount reached zero
    llvm::Value* isZero = builder->CreateICmpEQ(newCount, llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0));
    
    llvm::BasicBlock* cleanupBlock = llvm::BasicBlock::Create(*context, name + "_refcount_cleanup", currentFunction);
    llvm::BasicBlock* continueBlock = llvm::BasicBlock::Create(*context, name + "_refcount_continue", currentFunction);
    
    builder->CreateCondBr(isZero, cleanupBlock, continueBlock);
    
    // Cleanup block - perform actual cleanup when refcount hits zero
    builder->SetInsertPoint(cleanupBlock);
    
    // Find the variable in scope stack and perform cleanup
    for (auto& scope : scopeStack) {
        for (auto& var : scope) {
            if (var.name == name && var.isVecWithMallocData) {
                std::cout << "DEBUG: Performing OUR cleanup for Vec with malloc'd data: " << name << std::endl;
                
                // Same cleanup logic as MY ownership but triggered by refcount
                llvm::Value* vecPtr = builder->CreateLoad(var.type, var.allocaInst, name + "_refcount_cleanup_load");
                llvm::Value* dataPtr = builder->CreateExtractValue(vecPtr, 0, name + "_refcount_data_ptr");
                
                llvm::Value* isNotNull = builder->CreateICmpNE(dataPtr, 
                    llvm::ConstantPointerNull::get(llvm::PointerType::get(*context, 0)), 
                    name + "_refcount_null_check");
                
                llvm::BasicBlock* freeBlock = llvm::BasicBlock::Create(*context, name + "_refcount_free", currentFunction);
                llvm::BasicBlock* cleanupContinue = llvm::BasicBlock::Create(*context, name + "_refcount_cleanup_continue", currentFunction);
                
                builder->CreateCondBr(isNotNull, freeBlock, cleanupContinue);
                
                builder->SetInsertPoint(freeBlock);
                llvm::Function* freeFunc = getOrCreateFreeFunction();
                builder->CreateCall(freeFunc, {dataPtr});
                builder->CreateBr(cleanupContinue);
                
                builder->SetInsertPoint(cleanupContinue);
                break;
            }
        }
    }
    
    builder->CreateBr(continueBlock);
    
    // Continue block
    builder->SetInsertPoint(continueBlock);
    
    // Update local counter
    if (refCounts[name] > 0) {
        refCounts[name]--;
    }
}

// --- Helper Functions for Memory Management ---
llvm::Function* LLVMCodegen::getOrCreateFreeFunction() {
    llvm::Function* freeFunc = module->getFunction("free");
    if (!freeFunc) {
        std::cout << "DEBUG: Creating free() function declaration" << std::endl;
        llvm::FunctionType* freeFuncType = llvm::FunctionType::get(
            llvm::Type::getVoidTy(*context),                    // return type
            {llvm::PointerType::get(*context, 0)},              // parameter: void*
            false                                                // not variadic
        );
        freeFunc = llvm::Function::Create(freeFuncType, llvm::Function::ExternalLinkage, "free", module.get());
    }
    return freeFunc;
}

llvm::Function* LLVMCodegen::getOrCreateMallocFunction() {
    llvm::Function* mallocFunc = module->getFunction("malloc");
    if (!mallocFunc) {
        std::cout << "DEBUG: Creating malloc() function declaration" << std::endl;
        llvm::FunctionType* mallocFuncType = llvm::FunctionType::get(
            llvm::PointerType::get(*context, 0),                // return type: void*
            {llvm::Type::getInt64Ty(*context)},                 // parameter: size_t
            false                                                // not variadic
        );
        mallocFunc = llvm::Function::Create(mallocFuncType, llvm::Function::ExternalLinkage, "malloc", module.get());
    }
    return mallocFunc;
}

llvm::Function* LLVMCodegen::getOrCreateMemsetFunction() {
    llvm::Function* memsetFunc = module->getFunction("memset");
    if (!memsetFunc) {
        std::cout << "DEBUG: Creating memset() function declaration" << std::endl;
        llvm::FunctionType* memsetFuncType = llvm::FunctionType::get(
            llvm::PointerType::get(*context, 0),                // return type: void*
            {llvm::PointerType::get(*context, 0),               // void* ptr
             llvm::Type::getInt32Ty(*context),                  // int value
             llvm::Type::getInt64Ty(*context)},                 // size_t size
            false                                                // not variadic
        );
        memsetFunc = llvm::Function::Create(memsetFuncType, llvm::Function::ExternalLinkage, "memset", module.get());
    }
    return memsetFunc;
}