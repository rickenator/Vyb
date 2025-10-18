\
#include "vyn/vre/llvm/codegen.hpp"
#include "vyn/parser/ast.hpp"
#include "vyn/parser/source_location.hpp" // For vyn::SourceLocation
#include "vyn/runtime/async_runtime.hpp" // For async runtime integration

// LLVM Headers
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Path.h"

// Standard C++ Headers
#include <iostream> // For logError, std::cerr
#include <map>
#include <memory>
#include <optional>
#include <stack>
#include <string>
#include <system_error> // For std::error_code
#include <vector>

using namespace vyn;

// Error Logging Utility
void LLVMCodegen::logError(const SourceLocation& loc, const std::string& message) {
    std::cerr << "Error at " << (!loc.filePath.empty() ? loc.filePath.c_str() : "unknown_file") << ":" << loc.line << ":" << loc.column << ": " << message << std::endl;
}

// Warning Logging Utility
void LLVMCodegen::logWarning(const SourceLocation& loc, const std::string& message) {
    std::cerr << "Warning at " << (!loc.filePath.empty() ? loc.filePath.c_str() : "unknown_file") << ":" << loc.line << ":" << loc.column << ": " << message << std::endl;
}

// --- RTTI Helper Implementations ---
llvm::StructType* LLVMCodegen::getOrCreateRTTIStructType() {
    if (rttiStructType) return rttiStructType;

    std::vector<llvm::Type*> rttiFields = {
        int32Type,
        int8PtrType
    };
    rttiStructType = llvm::StructType::create(*context, rttiFields, "vyn.RTTI");
    return rttiStructType;
}

llvm::Value* LLVMCodegen::generateRTTIObject(const std::string& typeName, int typeId) {
    llvm::StructType* rttiTy = getOrCreateRTTIStructType();
    llvm::Constant* typeNameGlobal = builder->CreateGlobalStringPtr(typeName);
    llvm::Constant* rttiVals[] = {
        llvm::ConstantInt::get(int32Type, typeId),
        typeNameGlobal
    };
    return llvm::ConstantStruct::get(rttiTy, rttiVals);
}

LLVMCodegen::LLVMCodegen(Driver& driver)
    : driver_(driver),
      context(std::make_unique<llvm::LLVMContext>()),
      module(std::make_unique<llvm::Module>("VynModule", *this->context)), 
      builder(std::make_unique<llvm::IRBuilder<>>(*this->context)),
      debugBuilder(std::make_unique<llvm::DIBuilder>(*module)),
      debugCompileUnit(nullptr),
      debugFile(nullptr),
      m_isLHSOfAssignment(false),
      verbose(false),
      currentFunction(nullptr),
      currentClassType(nullptr),
      m_currentLLVMValue(nullptr),
      m_currentVynModule(nullptr)
{
    voidType = llvm::Type::getVoidTy(*this->context);
    int1Type = llvm::Type::getInt1Ty(*this->context);
    int8Type = llvm::Type::getInt8Ty(*this->context);
    int32Type = llvm::Type::getInt32Ty(*this->context);
    int64Type = llvm::Type::getInt64Ty(*this->context);
    floatType = llvm::Type::getFloatTy(*this->context);
    doubleType = llvm::Type::getDoubleTy(*this->context);
    int8PtrType = llvm::PointerType::getUnqual(int8Type);

    rttiStructType = getOrCreateRTTIStructType(); 

    currentLoopContext = {nullptr, nullptr, nullptr, nullptr};
}

LLVMCodegen::~LLVMCodegen() = default;

void LLVMCodegen::generate(vyn::ast::Module* astModule, const std::string& outputFilename) {
    if (!astModule) {
        logError(SourceLocation(), "Cannot generate code: AST module is null.");
        return;
    }

    // Initialize debug information
    initializeDebugInfo(outputFilename);

    astModule->accept(*this);
    
    // Ensure all core intrinsic functions are declared in the module
    // This prevents JIT runtime errors when functions are not found
    ensureCoreIntrinsicFunctions();

    // Finalize debug information before verification
    finalizeDebugInfo();

    if (llvm::verifyModule(*module, &llvm::errs())) {
        logError(SourceLocation(), "LLVM module verification failed.");
        return;
    }

    std::error_code EC;
    llvm::raw_fd_ostream dest(outputFilename, EC, llvm::sys::fs::OpenFlags{});

    if (EC) {
        logError(SourceLocation(), "Could not open file: " + EC.message());
        return;
    }

    module->print(dest, nullptr);
    dest.flush();
}

void LLVMCodegen::dumpIR() const {
    if (module) {
        module->print(llvm::errs(), nullptr);
    } else {
        std::cerr << "No LLVM module available to dump." << std::endl;
    }
}

void LLVMCodegen::visit(vyn::ast::Module* node) {
    if (!node) return;

    std::cout << "DEBUG: Module visitor called with " << node->body.size() << " statements" << std::endl;

    vyn::ast::Module* previousModule = m_currentVynModule;
    m_currentVynModule = node;

    for (size_t i = 0; i < node->body.size(); ++i) {
        const auto& stmt = node->body[i];
        if (stmt) {
            std::cout << "DEBUG: Processing module statement " << i << " (type: " << static_cast<int>(stmt->getType()) << ")" << std::endl;
            stmt->accept(*this);
        }
    }

    m_currentVynModule = previousModule;
    m_currentLLVMValue = nullptr;
}

// --- Debug Information Implementation ---

void LLVMCodegen::initializeDebugInfo(const std::string& filename) {
    if (!debugBuilder) {
        debugBuilder = std::make_unique<llvm::DIBuilder>(*module);
    }
    
    // Create debug file
    llvm::SmallString<256> currentDir;
    llvm::sys::fs::current_path(currentDir);
    llvm::SmallString<256> absolutePath(filename);
    llvm::sys::fs::make_absolute(currentDir, absolutePath);
    
    std::string directory = llvm::sys::path::parent_path(absolutePath).str();
    std::string fileName = llvm::sys::path::filename(absolutePath).str();
    
    debugFile = debugBuilder->createFile(fileName, directory);
    
    // Create compile unit
    debugCompileUnit = debugBuilder->createCompileUnit(
        llvm::dwarf::DW_LANG_C_plus_plus,  // Use C++ as closest language
        debugFile,
        "Vyn Compiler",      // Producer
        false,               // isOptimized
        "",                  // Flags
        0                    // Runtime version
    );
    
    // Set debug info version
    module->addModuleFlag(llvm::Module::Warning, "Debug Info Version", llvm::DEBUG_METADATA_VERSION);
    module->addModuleFlag(llvm::Module::Warning, "Dwarf Version", 4);
    
    // Push compile unit as initial scope
    debugScopeStack.push(debugCompileUnit);
    
    std::cout << "DEBUG: Initialized debug info for file: " << fileName << " in directory: " << directory << std::endl;
}

void LLVMCodegen::finalizeDebugInfo() {
    if (debugBuilder) {
        debugBuilder->finalize();
        std::cout << "DEBUG: Finalized debug information" << std::endl;
    }
}

llvm::DISubprogram* LLVMCodegen::createDebugFunctionInfo(llvm::Function* function, const std::string& name, 
                                                        const SourceLocation& loc, bool isAsync) {
    if (!debugBuilder || !debugFile) {
        return nullptr;
    }
    
    // Create function type for debug info
    llvm::SmallVector<llvm::Metadata*, 8> paramTypes;
    
    // Add return type (first element)
    llvm::DIType* returnType = getDebugType(function->getReturnType(), "return");
    paramTypes.push_back(returnType);
    
    // Add parameter types
    for (auto& arg : function->args()) {
        llvm::DIType* paramType = getDebugType(arg.getType(), arg.getName().str());
        paramTypes.push_back(paramType);
    }
    
    llvm::DISubroutineType* functionType = debugBuilder->createSubroutineType(
        debugBuilder->getOrCreateTypeArray(paramTypes)
    );
    
    // Create subprogram
    std::string displayName = isAsync ? ("async " + name) : name;
    llvm::DISubprogram* subprogram = debugBuilder->createFunction(
        debugFile,                    // Scope
        displayName,                  // Name
        name,                         // Linkage name
        debugFile,                    // File
        loc.line,                     // Line number
        functionType,                 // Type
        loc.line,                     // Scope line
        llvm::DINode::FlagPrototyped, // Flags
        llvm::DISubprogram::SPFlagDefinition
    );
    
    // Set subprogram for the function
    function->setSubprogram(subprogram);
    
    // Push function scope
    debugScopeStack.push(subprogram);
    
    std::cout << "DEBUG: Created debug info for " << (isAsync ? "async " : "") << "function: " << name 
              << " at line " << loc.line << std::endl;
    
    return subprogram;
}

void LLVMCodegen::setDebugLocation(const SourceLocation& loc) {
    if (!debugBuilder || debugScopeStack.empty()) {
        return;
    }
    
    llvm::DIScope* scope = debugScopeStack.top();
    llvm::DILocation* debugLoc = llvm::DILocation::get(*context, loc.line, loc.column, scope);
    builder->SetCurrentDebugLocation(debugLoc);
}

void LLVMCodegen::pushDebugScope(llvm::DIScope* scope) {
    if (scope) {
        debugScopeStack.push(scope);
    }
}

void LLVMCodegen::popDebugScope() {
    if (!debugScopeStack.empty()) {
        debugScopeStack.pop();
    }
}

llvm::DIType* LLVMCodegen::getDebugType(llvm::Type* llvmType, const std::string& typeName) {
    if (!debugBuilder) {
        return nullptr;
    }
    
    if (llvmType->isVoidTy()) {
        return nullptr; // Void type
    } else if (llvmType->isIntegerTy(1)) {
        return debugBuilder->createBasicType("bool", 1, llvm::dwarf::DW_ATE_boolean);
    } else if (llvmType->isIntegerTy(8)) {
        return debugBuilder->createBasicType("i8", 8, llvm::dwarf::DW_ATE_signed);
    } else if (llvmType->isIntegerTy(32)) {
        return debugBuilder->createBasicType("i32", 32, llvm::dwarf::DW_ATE_signed);
    } else if (llvmType->isIntegerTy(64)) {
        return debugBuilder->createBasicType("i64", 64, llvm::dwarf::DW_ATE_signed);
    } else if (llvmType->isFloatTy()) {
        return debugBuilder->createBasicType("f32", 32, llvm::dwarf::DW_ATE_float);
    } else if (llvmType->isDoubleTy()) {
        return debugBuilder->createBasicType("f64", 64, llvm::dwarf::DW_ATE_float);
    } else if (llvmType->isPointerTy()) {
        // In newer LLVM versions, pointers are opaque
        // Create a generic pointer type
        return debugBuilder->createPointerType(nullptr, 64); // 64-bit opaque pointer
    } else if (llvmType->isStructTy()) {
        llvm::StructType* structType = llvm::cast<llvm::StructType>(llvmType);
        std::string structName = structType->hasName() ? structType->getName().str() : ("struct_" + typeName);
        
        // Create a basic struct type for now
        return debugBuilder->createStructType(
            debugFile,                          // Scope
            structName,                         // Name
            debugFile,                          // File
            0,                                  // Line number
            64 * structType->getNumElements(),  // Size in bits (rough estimate)
            8,                                  // Alignment
            llvm::DINode::FlagZero,             // Flags
            nullptr,                            // Derived from
            llvm::DINodeArray()                 // Elements (empty for now)
        );
    }
    
    // Default case: create an unspecified type
    return debugBuilder->createUnspecifiedType(typeName.empty() ? "unknown" : typeName);
}
