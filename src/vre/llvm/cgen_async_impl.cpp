#include "vyn/vre/llvm/codegen.hpp"
#include "vyn/runtime/async_runtime.hpp"

// LLVM Headers
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"

namespace vyn {

// Implementation of async runtime integration functions for LLVMCodegen

llvm::Function* LLVMCodegen::getOrCreateScheduleTaskFunction() {
    const std::string funcName = "vyn_schedule_async_task";
    
    if (auto existingFunc = module->getFunction(funcName)) {
        return existingFunc;
    }
    
    // Function signature: i64 vyn_schedule_async_task(i8* function_ptr, i8* state_ptr)
    std::vector<llvm::Type*> paramTypes = { int8PtrType, int8PtrType };
    llvm::FunctionType* funcType = llvm::FunctionType::get(int64Type, paramTypes, false);
    
    return llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, funcName, module.get());
}

llvm::Function* LLVMCodegen::getOrCreateAwaitTaskFunction() {
    const std::string funcName = "vyn_await_task";
    
    if (auto existingFunc = module->getFunction(funcName)) {
        return existingFunc;
    }
    
    // Function signature: void vyn_await_task(i64 task_id)
    std::vector<llvm::Type*> paramTypes = { int64Type };
    llvm::FunctionType* funcType = llvm::FunctionType::get(voidType, paramTypes, false);
    
    return llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, funcName, module.get());
}

llvm::Function* LLVMCodegen::getOrCreateCreateFutureFunction() {
    const std::string funcName = "vyn_create_future";
    
    if (auto existingFunc = module->getFunction(funcName)) {
        return existingFunc;
    }
    
    // Function signature: i8* vyn_create_future(i64 task_id)
    std::vector<llvm::Type*> paramTypes = { int64Type };
    llvm::FunctionType* funcType = llvm::FunctionType::get(int8PtrType, paramTypes, false);
    
    return llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, funcName, module.get());
}

llvm::StructType* LLVMCodegen::createFutureStructType(llvm::Type* resultType) {
    // Future<T> struct: { T* result, i32 state, i64 task_id, i8* runtime_data }
    std::vector<llvm::Type*> futureFields = {
        llvm::PointerType::get(resultType, 0),           // T* result
        int32Type,                                       // i32 state
        int64Type,                                       // i64 task_id
        int8PtrType                                      // i8* runtime_data
    };
    
    return llvm::StructType::create(*context, futureFields, "Future");
}

} // namespace vyn