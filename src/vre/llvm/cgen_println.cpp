#include "vyn/vre/llvm/codegen.hpp"
#include "vyn/parser/ast.hpp"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/DerivedTypes.h>

namespace vyn {

// Implementation for println function in the Vyn language
// For use in the CallExpression visitor
llvm::Function* LLVMCodegen::getPrintlnFunction() {
    // Check if the println function has already been declared in the module
    llvm::Function* printlnFunc = module->getFunction("println");
    
    if (!printlnFunc) {
        // Create function signature for println
        std::vector<llvm::Type*> paramTypes = {int8PtrType};
        llvm::FunctionType* printlnType = llvm::FunctionType::get(
            llvm::Type::getVoidTy(*context),  // Return type: void
            paramTypes,                       // Parameters: (char*)
            false                             // Not vararg
        );
        
        // Declare the function in the module
        printlnFunc = llvm::Function::Create(
            printlnType,
            llvm::Function::ExternalLinkage,
            "println",
            module.get()
        );
        
        // Set parameter names for readability
        auto args = printlnFunc->arg_begin();
        args->setName("str");
    }
    
    return printlnFunc;
}

// Implementation for auto-serialization to convert objects to JSON strings
llvm::Function* LLVMCodegen::getSerializeToJsonFunction() {
    // Check if the serialization function has already been declared in the module
    llvm::Function* serializeFunc = module->getFunction("__vyn_serialize_to_json");
    
    if (!serializeFunc) {
        // Create function signature for serialization
        std::vector<llvm::Type*> paramTypes = {
            llvm::PointerType::getUnqual(int8Type),  // void* obj
            int8PtrType                               // const char* type_name
        };
        
        llvm::FunctionType* serializeType = llvm::FunctionType::get(
            int8PtrType,                            // Return type: char*
            paramTypes,                             // Parameters: (void*, const char*)
            false                                   // Not vararg
        );
        
        // Declare the function in the module
        serializeFunc = llvm::Function::Create(
            serializeType,
            llvm::Function::ExternalLinkage,
            "__vyn_serialize_to_json",
            module.get()
        );
        
        // Set parameter names for readability
        auto args = serializeFunc->arg_begin();
        args->setName("obj");
        (++args)->setName("type_name");
    }
    
    return serializeFunc;
}

// Get the function for actual println implementation
llvm::Function* LLVMCodegen::getVynPrintlnFunction() {
    // Check if the __vyn_println function has already been declared in the module
    llvm::Function* vynPrintlnFunc = module->getFunction("__vyn_println");
    
    if (!vynPrintlnFunc) {
        // Create function signature for __vyn_println
        std::vector<llvm::Type*> paramTypes = {int8PtrType};
        llvm::FunctionType* vynPrintlnType = llvm::FunctionType::get(
            llvm::Type::getVoidTy(*context),  // Return type: void
            paramTypes,                       // Parameters: (const char*)
            false                             // Not vararg
        );
        
        // Declare the function in the module
        vynPrintlnFunc = llvm::Function::Create(
            vynPrintlnType,
            llvm::Function::ExternalLinkage,
            "__vyn_println",
            module.get()
        );
        
        // Set parameter names for readability
        auto args = vynPrintlnFunc->arg_begin();
        args->setName("str");
    }
    
    return vynPrintlnFunc;
}

llvm::Function* LLVMCodegen::getSprintfFunction() {
    // Check if sprintf function already exists
    llvm::Function* sprintfFunc = module->getFunction("sprintf");
    if (sprintfFunc) {
        return sprintfFunc;
    }
    
    // Create sprintf function type: int sprintf(char*, const char*, ...)
    std::vector<llvm::Type*> sprintfParamTypes = {
        int8PtrType,  // char* buffer
        int8PtrType   // const char* format
    };
    
    llvm::FunctionType* sprintfType = llvm::FunctionType::get(
        int32Type,               // return type: int
        sprintfParamTypes,       // parameter types
        true                     // is variadic
    );
    
    // Create the sprintf function declaration
    sprintfFunc = llvm::Function::Create(
        sprintfType,
        llvm::Function::ExternalLinkage,
        "sprintf",
        module.get()
    );
    
    return sprintfFunc;
}

// Runtime function for panic - terminates program with message
llvm::Function* LLVMCodegen::getVynPanicFunction() {
    // Check if the panic function has already been declared
    llvm::Function* panicFunc = module->getFunction("__vyn_runtime_panic");
    
    if (!panicFunc) {
        // Create function signature for panic: void __vyn_runtime_panic(const char* message)
        std::vector<llvm::Type*> paramTypes = {int8PtrType};
        llvm::FunctionType* panicType = llvm::FunctionType::get(
            llvm::Type::getVoidTy(*context),  // Return type: void (noreturn)
            paramTypes,                       // Parameters: (const char*)
            false                             // Not vararg
        );
        
        // Declare the function in the module
        panicFunc = llvm::Function::Create(
            panicType,
            llvm::Function::ExternalLinkage,
            "__vyn_runtime_panic",
            module.get()
        );
        
        // Mark as noreturn
        panicFunc->setDoesNotReturn();
        
        // Set parameter name
        auto args = panicFunc->arg_begin();
        args->setName("message");
    }
    
    return panicFunc;
}

// Runtime function for untrapped errors - terminates program with error info
llvm::Function* LLVMCodegen::getVynUntrappedErrorFunction() {
    // Check if the untrapped error function has already been declared
    llvm::Function* untrappedFunc = module->getFunction("__vyn_runtime_untrapped_error");
    
    if (!untrappedFunc) {
        // Create function signature: void __vyn_runtime_untrapped_error(void* error)
        std::vector<llvm::Type*> paramTypes = {int8PtrType};
        llvm::FunctionType* untrappedType = llvm::FunctionType::get(
            llvm::Type::getVoidTy(*context),  // Return type: void (noreturn)
            paramTypes,                       // Parameters: (void*)
            false                             // Not vararg
        );
        
        // Declare the function in the module
        untrappedFunc = llvm::Function::Create(
            untrappedType,
            llvm::Function::ExternalLinkage,
            "__vyn_runtime_untrapped_error",
            module.get()
        );
        
        // Mark as noreturn
        untrappedFunc->setDoesNotReturn();
        
        // Set parameter name
        auto args = untrappedFunc->arg_begin();
        args->setName("error");
    }
    
    return untrappedFunc;
}

} // namespace vyn
