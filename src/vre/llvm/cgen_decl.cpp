\
#include "vyn/vre/llvm/codegen.hpp"
#include "vyn/parser/ast.hpp"

#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instructions.h> // For AllocaInst, ReturnInst, BranchInst etc.
#include <llvm/IR/Constants.h>    // For Constant, UndefValue
#include <llvm/IR/Verifier.h>     // For verifyFunction
#include <llvm/Support/raw_ostream.h> // For llvm::errs for verifyFunction output

#include <string>
#include <vector>
#include <map>

using namespace vyn;
// using namespace llvm; // Uncomment if desired for brevity

// --- Declarations ---

void LLVMCodegen::visit(ast::AspectDeclaration* node) {
    // Traits are interfaces/type constraints that don't generate runtime code by themselves
    // They're primarily used for compile-time type checking and polymorphism
    
    // Verify trait name
    if (!node->name) {
        logError(node->loc, "Trait declaration missing name");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    std::string traitName = node->name->name;
    if (traitName.empty()) {
        logError(node->loc, "Trait declaration has empty name");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Process generic parameters if any
    std::vector<std::string> genericParamNames;
    for (const auto& param : node->genericParams) {
        if (param && param->name) {
            genericParamNames.push_back(param->name->name);
        }
    }
    
    // For now, we just store information about the trait methods without generating actual code
    // In a full implementation, this information would be used for vtable generation when
    // types implement this trait
    
    // Skip visiting trait method signatures - they contain placeholder types like 'Self'
    // which have no concrete LLVM representation. The actual method implementations
    // will be generated when processing impl blocks.
    
    std::cout << "DEBUG: Trait '" << traitName << "' declaration skipped in codegen (interface only, no runtime code)" << std::endl;
    
    // Traits don't produce runtime values
    m_currentLLVMValue = nullptr;
}

void LLVMCodegen::visit(ast::NamespaceDeclaration* node) {
    // Namespaces provide scoping for declarations but don't generate runtime code themselves
    
    // Check if namespace name is valid
    if (!node->name) {
        logError(node->loc, "Namespace declaration missing name");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    std::string namespaceName = node->name->name;
    if (namespaceName.empty()) {
        logError(node->loc, "Namespace declaration has empty name");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Store the current namespace context to restore it later
    std::string currentNamespacePrefix = ""; // In a full implementation, we would track the current namespace
    std::string newNamespacePrefix = namespaceName + "::";
    
    // Process all declarations in the namespace
    // For each declaration, we'll mangle its name to include the namespace prefix
    for (const auto& member : node->members) {
        if (member) {
            // Process the declaration
            member->accept(*this);
        }
    }
    
    // Restore the previous namespace context
    // (In a full implementation)
    
    // Namespaces don't produce runtime values
    m_currentLLVMValue = nullptr;
}

std::unique_ptr<llvm::Module> LLVMCodegen::releaseModule() {
    return std::move(module);
}

std::unique_ptr<llvm::LLVMContext> LLVMCodegen::releaseContext() {
    return std::move(context);
}

void LLVMCodegen::visit(vyn::ast::VariableDeclaration* node) {
    llvm::Value* initialVal = nullptr;
    llvm::Type* varType = nullptr;

    // Variable declaration processing
    if (node->typeNode) {
        varType = codegenType(node->typeNode.get());
        if (!varType) {
            logError(node->loc, "Could not determine LLVM type for variable '" + node->id->name + "'.");
            m_currentLLVMValue = nullptr;
            return;
        }
    }

    if (node->init) {
        // Make sure the initializer knows its intended type if available
        if (node->typeNode && !node->init->type) {
            node->init->type = node->typeNode->clone();
        }
        
        node->init->accept(*this);
        initialVal = m_currentLLVMValue;
        if (!initialVal) {
            logError(node->init->loc, "Initialization expression for variable '" + node->id->name + "' evaluated to null.");
            m_currentLLVMValue = nullptr;
            return;
        }
        
        // If the initializer was an ObjectLiteral, ensure we properly set up type information
        if (auto* objLiteral = dynamic_cast<vyn::ast::ObjectLiteral*>(node->init.get())) {
            if (objLiteral->typePath && !node->typeNode) {
                // Create a TypeNode for the variable based on the ObjectLiteral's type
                node->typeNode = objLiteral->typePath->clone();
            }
        }
        
        if (!varType) {
            varType = initialVal->getType();
        } else {
            // Type check: initialVal type must be assignable to varType
            if (initialVal->getType() != varType) {
                llvm::Value* castedVal = tryCast(initialVal, varType, node->init->loc);
                if (castedVal) {
                    initialVal = castedVal;
                } else {
                    logError(node->init->loc, "Type mismatch for variable '" + node->id->name + 
                                             "'. Initializer type " + getTypeName(initialVal->getType()) + 
                                             " is not assignable to declared type " + getTypeName(varType));
                    m_currentLLVMValue = nullptr;
                    return;
                }
            }
        }
    } else { // No initializer
        if (!varType) {
            logError(node->loc, "Variable '" + node->id->name + "' has no initializer and no explicit type. Type inference from no initializer is not possible.");
            m_currentLLVMValue = nullptr;
            return;
        }
        // Default initialization (e.g., 0 for numbers, nullptr for pointers)
        // Ensure it's not a void type, which cannot be default initialized this way.
        if (varType->isVoidTy()){
            logError(node->loc, "Cannot declare a variable of type void: " + node->id->name);
            m_currentLLVMValue = nullptr;
            return;
        }
        initialVal = llvm::Constant::getNullValue(varType);
    }

    if (!currentFunction) { // Global variable
        // Global variables must have constant initializers.
        if (!llvm::isa<llvm::Constant>(initialVal)) {
            logError(node->init ? node->init->loc : node->loc, 
                     "Global variable '" + node->id->name + "' initializer must be a constant.");
            m_currentLLVMValue = nullptr;
            return;
        }
        auto* globalVar = new llvm::GlobalVariable(
            *module,
            varType,
            node->isConst, // LLVM's const for global means its value is constant, not necessarily its memory
            llvm::GlobalValue::PrivateLinkage, // Or ExternalLinkage if exported
            static_cast<llvm::Constant*>(initialVal),
            node->id->name
        );
        namedValues[node->id->name] = globalVar;
        m_currentLLVMValue = globalVar;
        // Propagate type info for struct/class variables
        if (node->typeNode) {
            valueTypeMap[globalVar] = std::shared_ptr<vyn::ast::TypeNode>(node->typeNode->clone());
        }
    } else { // Local variable
        llvm::AllocaInst* alloca = llvm::dyn_cast_or_null<llvm::AllocaInst>(
            createEntryBlockAlloca(currentFunction, node->id->name, varType)
        );
        if (!alloca) {
             logError(node->loc, "Failed to create alloca instruction for local variable '" + node->id->name + "'.");
             m_currentLLVMValue = nullptr;
             return;
        }
        builder->CreateStore(initialVal, alloca);
        // Register the variable in namedValues
        namedValues[node->id->name] = alloca;
        // Store the type info for this variable
        if (node->id->type) {
            valueTypeMap[alloca] = node->id->type;
        }
        
        // Determine ownership kind from variable's type annotation
        ast::OwnershipKind ownership = ast::OwnershipKind::MY; // Default to MY ownership
        bool needsCleanup = false;
        
        // Check if this is a Vec type that needs memory management
        bool isVecType = false;
        
        // Check AST type information first
        if (node->typeNode) {
            std::string typeString = node->typeNode->toString();
            std::cout << "DEBUG: Variable '" << node->id->name << "' has AST type: '" << typeString << "'" << std::endl;
            if (typeString.find("Vec") != std::string::npos) {
                isVecType = true;
                needsCleanup = true;
                std::cout << "DEBUG: Variable '" << node->id->name << "' is a Vec type (from AST) requiring cleanup" << std::endl;
            }
        }
        
        // Fall back to LLVM struct type name check
        if (!isVecType) {
            if (auto structType = llvm::dyn_cast<llvm::StructType>(varType)) {
                std::string typeName = structType->getName().str();
                std::cout << "DEBUG: Variable '" << node->id->name << "' has struct type: '" << typeName << "'" << std::endl;
                if (typeName.find("Vec") != std::string::npos) {
                    needsCleanup = true;
                    std::cout << "DEBUG: Variable '" << node->id->name << "' is a Vec type (from LLVM) requiring cleanup" << std::endl;
                }
            }
        }
        
        // Register variable for scope-based cleanup
        registerVariable(node->id->name, alloca, initialVal, ownership, varType, needsCleanup);
        
        // Create debug information for the variable
        if (debugBuilder && !debugScopeStack.empty()) {
            std::string typeName = getTypeName(varType);
            llvm::DIType* debugType = getDebugType(varType, typeName);
            if (debugType) {
                llvm::DILocalVariable* debugVar = createDebugVariableInfo(
                    node->id->name, debugType, node->loc);
                if (debugVar) {
                    insertDebugVariableDeclaration(debugVar, alloca, node->loc);
                }
            }
        }
        
        m_currentLLVMValue = alloca;
        // Propagate type info for struct/class variables
        if (node->typeNode) {
            valueTypeMap[alloca] = std::shared_ptr<vyn::ast::TypeNode>(node->typeNode->clone());
        }
    }
}

void LLVMCodegen::visit(vyn::ast::FunctionDeclaration* node) {
    std::vector<llvm::Type*> paramTypes;
    std::vector<std::string> paramNames;
    for (const auto& paramNode : node->params) {
        if (!paramNode.typeNode) {
            logError(paramNode.name->loc, "Parameter '" + paramNode.name->name + "' in function '" + node->id->name + "' is missing a type annotation.");
            m_currentLLVMValue = nullptr; return;
        }
        llvm::Type* llvmType = codegenType(paramNode.typeNode.get());
        if (!llvmType) {
            logError(paramNode.name->loc, "Could not determine LLVM type for parameter '" + paramNode.name->name + "' in function '" + node->id->name + "'.");
            m_currentLLVMValue = nullptr; return;
        }
        paramTypes.push_back(llvmType);
        paramNames.push_back(paramNode.name->name);
    }

    llvm::Type* returnType = nullptr;
    if (node->returnTypeNode) {
        if (currentAsyncState.isAsync) {
            // For async functions, the actual return type is wrapped in Future<T>
            llvm::Type* originalReturnType = codegenType(node->returnTypeNode.get());
            if (!originalReturnType) {
                logError(node->loc, "Could not determine LLVM return type for async function '" + node->id->name + "'.");
                m_currentLLVMValue = nullptr; return;
            }
            returnType = createFutureStructType(originalReturnType);
        } else {
            returnType = codegenType(node->returnTypeNode.get());
            std::cerr << "DEBUG: Function " << node->id->name << " return type resolved to: " 
                      << getTypeName(returnType) << " with pointer: " << returnType << std::endl;
            if (!returnType) {
                logError(node->loc, "Could not determine LLVM return type for function '" + node->id->name + "'.");
                m_currentLLVMValue = nullptr; return;
            }
        }
        
        // TODO: Auto-serialization for main function is disabled for now to fix type verification
        // When enabled, main functions with complex types will serialize to JSON and return i32
        // if (node->id->name == "main" && 
        //     !returnType->isIntegerTy() && 
        //     !returnType->isVoidTy() && 
        //     !(returnType->isPointerTy() && returnType == int8PtrType) &&
        //     !functionBodyReturnsLitIntrinsic(node->body.get())) {
        //     returnType = int32Type; // main returns exit code when auto-serializing
        // }
    } else {
        if (currentAsyncState.isAsync) {
            // Async void function returns Future<void>
            returnType = createFutureStructType(llvm::Type::getVoidTy(*context));
        } else {
            returnType = llvm::Type::getVoidTy(*context);
        }
    }
    
    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes, false /*isVarArg*/);
    
    // Check for existing function (could be forward declaration or redefinition)
    llvm::Function* func = module->getFunction(node->id->name);
    if (func) {
        if (func->getFunctionType() != funcType) {
            logError(node->loc, "Redefinition of function '" + node->id->name + "' with different signature.");
            m_currentLLVMValue = nullptr; return;
        }
        if (!func->empty() && node->body) { // Already has a body, and we are trying to define another
            logError(node->loc, "Redefinition of function '" + node->id->name + "'.");
            m_currentLLVMValue = nullptr; return;
        }
        // If it was a forward declaration and types match, we are now providing the body.
    } else {
        func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, node->id->name, module.get());
    }

    // Set current function for subsequent codegen (body, variable declarations)
    llvm::Function* oldFunction = currentFunction;
    currentFunction = func;

    // Create debug information for the function
    llvm::DISubprogram* debugFunction = nullptr;
    if (node->body) { // Only create debug info for functions with bodies
        debugFunction = createDebugFunctionInfo(func, node->id->name, node->loc, node->isAsync);
    }

    // Handle async functions
    AsyncState oldAsyncState = currentAsyncState;
    if (node->isAsync) {
        currentAsyncState.isAsync = true;
        currentAsyncState.asyncFunction = func;
        currentAsyncState.stateCounter = 0;
        
        // Initialize debug information for async state machine
        initializeAsyncStateDebugInfo(node->id->name, node->loc);
        
        // For async functions, modify return type to Future<T> if not already
        if (node->returnTypeNode) {
            // Check if return type is already Future<T>
            auto futureType = dynamic_cast<ast::FutureType*>(node->returnTypeNode.get());
            if (!futureType) {
                // Wrap the return type in Future<T>
                // The actual LLVM function will return a Future struct
                llvm::Type* originalReturnType = returnType;
                llvm::StructType* futureStructType = createFutureStructType(originalReturnType);
                // Note: We keep the original function signature for now
                // The async transformation will happen during codegen
            }
        }
    } else {
        currentAsyncState.isAsync = false;
    }

    // Store old namedValues and create a new scope for the function arguments and locals
    std::map<std::string, llvm::Value*> oldNamedValues;
    oldNamedValues.swap(namedValues); 

    if (node->body) { 
        llvm::BasicBlock* entryBB = llvm::BasicBlock::Create(*context, "entry", func);
        builder->SetInsertPoint(entryBB);

        // Set debug location for function entry
        setDebugLocation(node->loc);

        // Initialize scope management for function body
        enterScope();

        // Create allocas for parameters and store initial argument values
        auto argIt = func->arg_begin();
        for (size_t i = 0; i < paramTypes.size(); ++i, ++argIt) {
            llvm::Argument* argVal = &*argIt;
            argVal->setName(paramNames[i]);

            llvm::AllocaInst* alloca = llvm::dyn_cast_or_null<llvm::AllocaInst>(createEntryBlockAlloca(func, paramNames[i], paramTypes[i]));
            if (!alloca) {
                logError(node->params[i].name->loc, "Failed to create alloca for parameter '" + paramNames[i] + "'.");
                // Cleanup might be needed here
                exitScope(); // Clean up function scope before exiting
                currentFunction = oldFunction;
                namedValues.swap(oldNamedValues);
                m_currentLLVMValue = nullptr; return;
            }
            builder->CreateStore(argVal, alloca);
            namedValues[paramNames[i]] = alloca;
            
            // Store type information for function parameters
            if (node->params[i].typeNode) {
                valueTypeMap[alloca] = std::shared_ptr<vyn::ast::TypeNode>(node->params[i].typeNode->clone());
                std::cout << "DEBUG: Stored type mapping for parameter '" << paramNames[i] << "'" << std::endl;
            }
            
            // Register parameter for scope-based cleanup (parameters have MY ownership by default)
            registerVariable(paramNames[i], alloca, argVal, ast::OwnershipKind::MY, paramTypes[i], false);
            
            // Create debug information for the parameter
            if (debugBuilder && !debugScopeStack.empty()) {
                std::string typeName = getTypeName(paramTypes[i]);
                llvm::DIType* debugType = getDebugType(paramTypes[i], typeName);
                if (debugType) {
                    llvm::DILocalVariable* debugVar = createDebugVariableInfo(
                        paramNames[i], debugType, node->params[i].name->loc);
                    if (debugVar) {
                        insertDebugVariableDeclaration(debugVar, alloca, node->params[i].name->loc);
                    }
                }
            }
        }
        
        std::cout << "DEBUG: FunctionDeclaration - about to process function body" << std::endl;
        node->body->accept(*this); // Generate code for the function body
        std::cout << "DEBUG: FunctionDeclaration - finished processing function body" << std::endl;

        // Clean up function scope before return
        exitScope();

        // Verify function return: ensure all paths return if non-void, or add implicit void return
        if (returnType->isVoidTy()) {
            // Check if the last block has a terminator. If not, add ret void.
            if (!func->empty() && !func->back().getTerminator()) {
                // Make sure we're inserting at the end of the last block
                builder->SetInsertPoint(&func->back());
                builder->CreateRetVoid();
            }
        } else {
            // For non-void functions, ensure all paths return. LLVM's verifier will catch most issues.
            // A simple check: if the last block in a non-empty function doesn't have a terminator, it's an error.
            if (!func->empty() && !func->back().getTerminator()) {
                logError(node->loc, "Function '" + node->id->name + "' has a non-void return type but may not return on all paths (missing return at end of body).");
                // Optionally, builder->CreateUnreachable(); if this state should be impossible.
            }
        }
        
        // Verify the generated function
        if (llvm::verifyFunction(*func, &llvm::errs())) {
            logError(node->loc, "LLVM function verification failed for '" + node->id->name + "'. Errors printed to stderr.");
            // func->print(llvm::errs()); // Print the malformed function
            // Consider erasing the function: func->eraseFromParent();
            // For now, let it be, so errors are visible.
        }

        // Pop debug scope for function
        if (debugFunction) {
            popDebugScope();
        }

    } // else it's a forward declaration or extern, no body to generate now.

    // Restore outer scope and async state
    currentFunction = oldFunction;
    currentAsyncState = oldAsyncState;
    namedValues.swap(oldNamedValues); 

    m_currentLLVMValue = func; // The "value" of a function declaration is the function itself
}

void LLVMCodegen::visit(vyn::ast::StructDeclaration* node) {
    std::string nameStr = node->name->name;
    
    // Check if this is a generic struct (has type parameters like Box<T>)
    if (!node->genericParams.empty()) {
        std::cout << "DEBUG: Storing generic struct template: " << nameStr << " with " 
                  << node->genericParams.size() << " type parameters" << std::endl;
        // Store the AST node for later monomorphization when instantiated (e.g., Box<Int>)
        genericStructTemplates[nameStr] = node;
        m_currentLLVMValue = nullptr;
        return; // Don't generate LLVM type yet
    }
    
    // Non-generic struct: generate LLVM type immediately
    llvm::StructType* structType = llvm::StructType::create(*context, nameStr);

    UserTypeInfo typeInfo;
    typeInfo.llvmType = structType;
    typeInfo.isStruct = true;
    
    // Add opaque struct to map BEFORE processing field types (for circular references)
    userTypeMap[nameStr] = typeInfo;
    std::cerr << "DEBUG: Stored " << nameStr << " in userTypeMap with LLVM type pointer: " << structType << std::endl;

    std::vector<llvm::Type*> fieldTypes;
    for (size_t i = 0; i < node->fields.size(); ++i) {
        const auto& fieldDecl = node->fields[i];
        if (!fieldDecl->typeNode) { // Changed .type to ->typeNode based on ast.hpp for FieldDeclaration
            logError(fieldDecl->name->loc, "Field \'" + fieldDecl->name->name + "\' in struct \'" + nameStr + "\' is missing a type.");
            m_currentLLVMValue = nullptr; return;
        }
        std::cout << "DEBUG: Processing field '" << fieldDecl->name->name << "' with type: " << fieldDecl->typeNode->toString() << std::endl;
        llvm::Type* fieldType = codegenType(fieldDecl->typeNode.get()); // Changed .type to ->typeNode
        if (!fieldType) {
            logError(fieldDecl->name->loc, "Could not determine LLVM type for field \'" + fieldDecl->name->name + "\' in struct \'" + nameStr + "\'.");
            m_currentLLVMValue = nullptr; return;
        }
        std::cout << "DEBUG: Successfully generated LLVM type for field '" << fieldDecl->name->name << "'" << std::endl;
        fieldTypes.push_back(fieldType);
        typeInfo.fieldIndices[fieldDecl->name->name] = i; // Changed .name to ->name
    }

    structType->setBody(fieldTypes, /*isPacked=*/false);
    std::cout << "DEBUG: Set struct body for " << nameStr << " with " << fieldTypes.size() << " fields, struct is opaque: " << structType->isOpaque() << std::endl;
    // Update the map entry with complete field information
    userTypeMap[nameStr] = typeInfo;
    m_currentLLVMValue = nullptr; // structType is an llvm::Type*, not llvm::Value*
}

void LLVMCodegen::visit(vyn::ast::ClassDeclaration* node) {
    // Similar to StructDeclaration, but might involve vtables, inheritance, etc. in the future.
    // For now, treat classes like structs.
    std::string nameStr = node->name->name;
    llvm::StructType* classType = llvm::StructType::create(*context, nameStr);
    currentClassType = classType; // Set for member functions

    UserTypeInfo typeInfo;
    typeInfo.llvmType = classType;
    typeInfo.isStruct = false; // It's a class

    std::vector<llvm::Type*> fieldTypes;
    // TODO: Add vtable pointer as the first field if Vyn classes have virtual methods.
    // llvm::Type* vtablePtrType = llvm::PointerType::getUnqual(llvm::FunctionType::get(voidType, true)->getPointerTo());
    // fieldTypes.push_back(vtablePtrType);
    // typeInfo.fieldIndices["_vptr"] = 0; // Example vptr

    unsigned fieldIdxOffset = fieldTypes.size(); // Start field indices after any implicit members like vptr

    for (size_t i = 0; i < node->members.size(); ++i) {
        // Assuming members are fields for now. Methods are handled separately or as part of ImplDeclaration.
        // This part needs to distinguish between FieldDeclaration and MethodDeclaration within ClassDeclaration.
        // For now, let's assume `node->members` contains `FieldDeclaration` like nodes.
        // If `node->members` is a generic `Declaration*`, we need to dyn_cast.
        // Let's assume `node->fields` for explicit field declarations as in StructDecl for now.
        // This needs clarification from ast.hpp for ClassDeclaration members.
        // For now, skipping direct field processing here, assuming Impl blocks or similar will define them,
        // or that ClassDeclaration has a `fields` member like StructDeclaration.
        // Let's assume `node->fields` for now, similar to StructDeclaration for simplicity.
        /*
        if (auto* fieldDeclNode = dynamic_cast<ast::FieldDeclaration*>(node->members[i].get())) {
             if (!fieldDeclNode->typeNode) { logError(...); return; }
             llvm::Type* fieldType = codegenType(fieldDeclNode->typeNode.get());
             if (!fieldType) { logError(...); return; }
             fieldTypes.push_back(fieldType);
             typeInfo.fieldIndices[fieldDeclNode->id->name] = i + fieldIdxOffset;
        } else if (auto* methodDeclNode = dynamic_cast<ast::FunctionDeclaration*>(node->members[i].get())) {
            // Method declarations might contribute to vtable or be standalone functions.
            // Their codegen is typically handled when visiting the FunctionDeclaration itself,
            // with `currentClassType` set to associate them.
        }
        */
    }
    // Placeholder: If ClassDeclaration has a `fields` vector like StructDeclaration:
    // Iterating over node->members which are Declaration*, need to cast to FieldDeclaration*
    for (const auto& memberDecl : node->members) { // Changed from node->fields to node->members
        if (auto* fieldDecl = dynamic_cast<ast::FieldDeclaration*>(memberDecl.get())) {
            if (!fieldDecl->typeNode) { // was fieldDecl.type
                logError(fieldDecl->name->loc, "Field \'" + fieldDecl->name->name + "\' in class \'" + nameStr + "\' is missing a type.");
                m_currentLLVMValue = nullptr; return;
            }
            llvm::Type* fieldType = codegenType(fieldDecl->typeNode.get()); // was fieldDecl.type
            if (!fieldType) {
                logError(fieldDecl->name->loc, "Could not determine LLVM type for field \'" + fieldDecl->name->name + "\' in class \'" + nameStr + "\'.");
                m_currentLLVMValue = nullptr; return;
            }
            fieldTypes.push_back(fieldType);
            typeInfo.fieldIndices[fieldDecl->name->name] = fieldTypes.size() -1 + fieldIdxOffset; // was fieldDecl.name
        }
        // else if (auto* methodDecl = dynamic_cast<ast::FunctionDeclaration*>(memberDecl.get())) {
        // Methods are handled when `impl` block is visited or if defined inline.
        // Here we are only collecting explicit field types for the class structure.
        // }
    }


    classType->setBody(fieldTypes, /*isPacked=*/false);
    userTypeMap[nameStr] = typeInfo;
    m_currentLLVMValue = nullptr; // classType is an llvm::Type*, not llvm::Value*
    currentClassType = nullptr; // Reset after processing class
}

void LLVMCodegen::visit(vyn::ast::TypeAliasDeclaration* node) {
    // type<UnderlyingType> AliasName;
    // This registers the type alias so that when codegenType encounters the alias name,
    // it can resolve to the underlying LLVM type.
    
    if (!node->name || node->name->name.empty()) {
        logError(node->loc, "Type alias declaration missing or has empty name");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    if (!node->typeNode) {
        logError(node->loc, "Type alias declaration missing underlying type");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Resolve the underlying type to LLVM type
    llvm::Type* underlyingLlvmType = codegenType(node->typeNode.get());
    if (!underlyingLlvmType) {
        logError(node->loc, "Could not resolve underlying type for type alias '" + node->name->name + "'");
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // Register the type alias mapping
    std::string aliasName = node->name->name;
    typeAliasMap[aliasName] = underlyingLlvmType;
    
    // Debug output to verify registration
    if (verbose) {
        logWarning(node->loc, "Registered type alias '" + aliasName + "' -> underlying LLVM type");
    }
    m_currentLLVMValue = nullptr; // No direct LLVM value for an alias declaration itself.
}

void LLVMCodegen::visit(vyn::ast::ImportDeclaration* node) {
    // Imports are typically handled by a module loader or linker phase before/during codegen.
    // The codegen itself might not do much other than note that symbols from `node->modulePath` are available.
    // If Vyn compiles modules separately and links them, this is a linker concern.
    // If it\'s more like C++ #include, then parsing of the imported module would happen earlier.
    // ast.hpp: std::unique_ptr<StringLiteral> source;
    // std::vector<ImportSpecifier> specifiers;
    // std::unique_ptr<Identifier> defaultImport;
    // std::unique_ptr<Identifier> namespaceImport;

    std::string importSource = node->source ? node->source->value : "unknown"; // Using 'source' from ast.hpp

    logError(node->loc, "ImportDeclaration handling is typically a pre-codegen or linking step. Module source: " + importSource);
    m_currentLLVMValue = nullptr;
}

void LLVMCodegen::visit(vyn::ast::FieldDeclaration* node) {
    // FieldDeclarations are part of StructDeclaration or ClassDeclaration.
    // They are processed within those visitors, not typically visited standalone by the main codegen loop.
    // If visited standalone, it implies context is missing (e.g. which struct/class it belongs to).
    logError(node->loc, "FieldDeclaration '" + node->name->name + "' visited standalone. Should be part of struct/class.");
    m_currentLLVMValue = nullptr;
}

void LLVMCodegen::visit(vyn::ast::BindDeclaration* node) {
    // impl blocks associate methods with types.
    // Generic impl blocks (e.g., impl<T> Display for Box<T>) are templates that don't
    // generate code until instantiated with concrete types.
    
    // Check if this is a generic impl block
    if (!node->genericParams.empty()) {
        std::cout << "DEBUG: Skipping generic impl block for " << node->selfType->toString() 
                  << " - codegen happens on instantiation" << std::endl;
        m_currentLLVMValue = nullptr;
        return;
    }
    
    // For non-generic impl blocks, generate the methods for the concrete type
    // ast.hpp: TypeNodePtr selfType; // The type being implemented
    // ast.hpp: std::vector<std::unique_ptr<FunctionDeclaration>> methods;

    llvm::Type* targetType = codegenType(node->selfType.get());
    if (!targetType || !targetType->isStructTy()) {
        logError(node->loc, "Target type for impl block is not a known struct/class type: " + node->selfType->toString());
        m_currentLLVMValue = nullptr; return;
    }
    llvm::StructType* oldCurrentClassType = currentClassType;
    currentClassType = llvm::dyn_cast<llvm::StructType>(targetType);
    m_currentImplTypeNode = node->selfType.get();

    for (const auto& member : node->methods) {
        // Members are typically FunctionDeclarations (methods)
        member->accept(*this); // This will call visit(FunctionDeclaration*)
    }

    currentClassType = oldCurrentClassType;
    m_currentImplTypeNode = nullptr;
    m_currentLLVMValue = nullptr; // Impl block itself doesn't produce a value
}

void LLVMCodegen::visit(vyn::ast::EnumDeclaration* node) {
    // Enums in Vyn could be C-like (integer-based) or more complex (tagged unions like Rust/Swift).
    // This implementation sketch assumes C-like enums for simplicity, mapping to integers.
    // For tagged unions, each variant would be a struct, and the enum type itself a wrapper struct or i8* + tag.
    logError(node->loc, "EnumDeclaration codegen is not fully implemented (assuming C-like integer enums for now).");
    
    // For C-like enums, we might just define constants for each variant.
    // llvm::Type* enumBaseType = int32Type; // Or infer from values
    // int64_t currentValue = 0;
    // for (const auto& variantNode : node->variants) {
    //     std::string variantName = node->name->name + "::" + variantNode->name->name; // Or however Vyn names enum variants
    //     if (variantNode->value) { // If explicit value
    //         variantNode->value->accept(*this);
    //         if (auto* constInt = llvm::dyn_cast_or_null<llvm::ConstantInt>(m_currentLLVMValue)) {
    //             currentValue = constInt->getSExtValue();
    //         } else {
    //             logError(variantNode->loc, "Enum variant value for '" + variantName + "' is not a constant integer.");
    //         }
    //     }
    //     llvm::Constant* enumConst = llvm::ConstantInt::get(enumBaseType, currentValue);
    //     // Store this constant somewhere accessible, e.g. in namedValues or a specific enum map.
    //     // namedValues[variantName] = enumConst; 
    //     // This makes `MyEnum::Variant` resolve to the integer constant.
    //     currentValue++;
    // }
    m_currentLLVMValue = nullptr; // Enum declaration itself doesn't have a direct LLVM value in this model
}

void LLVMCodegen::visit(vyn::ast::EnumVariant* node) {
    // Visited as part of EnumDeclaration. Not typically standalone.
    logError(node->loc, "EnumVariant visited standalone.");
    m_currentLLVMValue = nullptr;
}

void LLVMCodegen::visit(vyn::ast::GenericParameter* node) {
    // Generic parameters are resolved during template instantiation (monomorphization).
    // Standalone codegen for a GenericParameter is not typical.
    logError(node->loc, "GenericParameter visited standalone. Should be resolved during template instantiation.");
    m_currentLLVMValue = nullptr;
}

void LLVMCodegen::visit(vyn::ast::TemplateDeclaration* node) {
    // Template declarations are blueprints. Code is generated when they are instantiated.
    // No LLVM IR is generated for template declarations themselves - only for instantiations.
    // Silently skip template declarations during codegen.
    std::cout << "DEBUG: Skipping TemplateDeclaration '" 
              << (node && node->name ? node->name->name : "<unnamed>") 
              << "' - codegen happens on instantiation" << std::endl;
    m_currentLLVMValue = nullptr;
}

void LLVMCodegen::createFunctionForwardDeclaration(vyn::ast::FunctionDeclaration* node) {
    std::cout << "DEBUG: Creating forward declaration for function: " << node->id->name << std::endl;
    
    // Check if function already exists
    llvm::Function* existingFunc = module->getFunction(node->id->name);
    if (existingFunc) {
        std::cout << "DEBUG: Function " << node->id->name << " already exists, skipping forward declaration" << std::endl;
        return;
    }
    
    // Extract parameter types
    std::vector<llvm::Type*> paramTypes;
    for (const auto& paramNode : node->params) {
        if (!paramNode.typeNode) {
            logError(paramNode.name->loc, "Parameter '" + paramNode.name->name + "' in function '" + node->id->name + "' is missing a type annotation.");
            return;
        }
        llvm::Type* llvmType = codegenType(paramNode.typeNode.get());
        if (!llvmType) {
            logError(paramNode.name->loc, "Could not determine LLVM type for parameter '" + paramNode.name->name + "' in function '" + node->id->name + "'.");
            return;
        }
        paramTypes.push_back(llvmType);
    }

    // Extract return type
    llvm::Type* returnType = nullptr;
    if (node->returnTypeNode) {
        if (currentAsyncState.isAsync) {
            // For async functions, the actual return type is wrapped in Future<T>
            llvm::Type* originalReturnType = codegenType(node->returnTypeNode.get());
            if (!originalReturnType) {
                logError(node->loc, "Could not determine LLVM return type for async function '" + node->id->name + "'.");
                return;
            }
            returnType = createFutureStructType(originalReturnType);
        } else {
            returnType = codegenType(node->returnTypeNode.get());
            if (!returnType) {
                logError(node->loc, "Could not determine LLVM return type for function '" + node->id->name + "'.");
                return;
            }
        }
    } else {
        if (currentAsyncState.isAsync) {
            // Async void function returns Future<void>
            returnType = createFutureStructType(llvm::Type::getVoidTy(*context));
        } else {
            returnType = llvm::Type::getVoidTy(*context);
        }
    }
    
    // Create function type and forward declaration
    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes, false /*isVarArg*/);
    llvm::Function* func = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, node->id->name, module.get());
    
    std::cout << "DEBUG: Successfully created forward declaration for function: " << node->id->name << std::endl;
}


