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

} // namespace vyn
