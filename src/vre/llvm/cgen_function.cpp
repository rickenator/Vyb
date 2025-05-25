// Include necessary headers
#include "vyn/vre/llvm/codegen.hpp"
#include <llvm/IR/Function.h>

namespace vyn {

// Implementation of getCurrentFunction method
llvm::Function* LLVMCodegen::getCurrentFunction() {
    return currentFunction;
}

} // namespace vyn
