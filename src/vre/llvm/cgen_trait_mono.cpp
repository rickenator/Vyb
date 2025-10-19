#include "vyn/vre/llvm/codegen.hpp"
#include "vyn/semantic.hpp"
#include <sstream>
#include <algorithm>

namespace vyn {

// Parse a type string like "Box<Int>" into {base: "Box", args: ["Int"]}
LLVMCodegen::TypePattern LLVMCodegen::TypePattern::parse(const std::string& typeStr) {
    TypePattern pattern;
    
    size_t anglePos = typeStr.find('<');
    if (anglePos == std::string::npos) {
        // Simple type without generic args
        pattern.base = typeStr;
        return pattern;
    }
    
    // Extract base type
    pattern.base = typeStr.substr(0, anglePos);
    
    // Extract generic arguments
    size_t start = anglePos + 1;
    size_t end = typeStr.find_last_of('>');
    if (end == std::string::npos || end <= start) {
        // Malformed, return what we have
        return pattern;
    }
    
    std::string argsStr = typeStr.substr(start, end - start);
    
    // Split by comma (simple split, doesn't handle nested generics yet)
    std::stringstream ss(argsStr);
    std::string arg;
    while (std::getline(ss, arg, ',')) {
        // Trim whitespace
        arg.erase(0, arg.find_first_not_of(" \t"));
        arg.erase(arg.find_last_not_of(" \t") + 1);
        if (!arg.empty()) {
            pattern.args.push_back(arg);
        }
    }
    
    return pattern;
}

// Check if a concrete type matches this pattern, extracting type substitutions
// Example: pattern="Box<T>", concrete="Box<Int>" -> true, subst={"T":"Int"}
bool LLVMCodegen::TypePattern::matchesPattern(const TypePattern& concrete, 
                                              std::map<std::string, std::string>& substitutions) const {
    // Base types must match exactly
    if (this->base != concrete.base) {
        return false;
    }
    
    // Number of type arguments must match
    if (this->args.size() != concrete.args.size()) {
        return false;
    }
    
    // Match each argument
    for (size_t i = 0; i < this->args.size(); ++i) {
        const std::string& patternArg = this->args[i];
        const std::string& concreteArg = concrete.args[i];
        
        // Check if pattern arg is a type parameter (single uppercase letter or capitalized identifier)
        // Simple heuristic: if it's a single char or starts with capital and no '<', it's a type param
        bool isTypeParam = (patternArg.length() == 1 && std::isupper(patternArg[0])) ||
                          (patternArg.find('<') == std::string::npos && std::isupper(patternArg[0]));
        
        if (isTypeParam) {
            // This is a type parameter - record the substitution
            auto it = substitutions.find(patternArg);
            if (it != substitutions.end()) {
                // Already have a substitution for this param - must be consistent
                if (it->second != concreteArg) {
                    return false;
                }
            } else {
                substitutions[patternArg] = concreteArg;
            }
        } else {
            // Concrete type - must match exactly
            if (patternArg != concreteArg) {
                return false;
            }
        }
    }
    
    return true;
}

// Convert TypePattern to mangled name: Box<Int> -> Box_Int
std::string LLVMCodegen::TypePattern::toMangled() const {
    if (args.empty()) {
        return base;
    }
    
    std::string result = base;
    for (const auto& arg : args) {
        result += "_" + arg;
    }
    return result;
}

// Extract base pattern from concrete type: "Box<Int>" -> "Box"
std::string LLVMCodegen::extractBasePattern(const std::string& concreteType) {
    TypePattern parsed = TypePattern::parse(concreteType);
    return parsed.base;
}

// Get full type name from an expression (e.g., variable reference)
std::string LLVMCodegen::getFullTypeName(vyn::ast::Expression* expr) {
    if (!expr) return "";
    
    // Try to get type from the expression itself
    if (expr->type) {
        return expr->type->toString();
    }
    
    // For identifiers, check if we have type info
    if (auto ident = dynamic_cast<ast::Identifier*>(expr)) {
        if (ident->type) {
            return ident->type->toString();
        }
    }
    
    return "";
}

// Monomorphize a trait method for a concrete type
llvm::Function* LLVMCodegen::monomorphizeTraitMethod(const std::string& concreteType,
                                                     const std::string& traitName,
                                                     const std::string& methodName) {
    std::cout << "DEBUG: Monomorphizing trait method: " << traitName << "::" << methodName 
              << " for " << concreteType << std::endl;
    
    // Check cache first
    std::string cacheKey = concreteType + "::" + methodName;
    auto cacheIt = monomorphizedMethods.find(cacheKey);
    if (cacheIt != monomorphizedMethods.end()) {
        std::cout << "DEBUG: Found cached monomorphized method: " << cacheKey << std::endl;
        return cacheIt->second;
    }
    
    // Get semantic analyzer from driver
    if (!driver_.hasSemanticAnalyzer()) {
        logError(SourceLocation(), "SemanticAnalyzer not available for trait monomorphization");
        return nullptr;
    }
    
    SemanticAnalyzer* semantic = driver_.getSemanticAnalyzer();
    
    // Parse the concrete type to extract pattern matching info
    TypePattern concretePattern = TypePattern::parse(concreteType);
    
    std::cout << "DEBUG: Parsed concrete type - base: " << concretePattern.base 
              << ", args: " << concretePattern.args.size() << std::endl;
    
    // Search through generic trait impls to find a matching pattern
    const auto& genericImpls = semantic->getGenericTraitImpls();
    for (const auto& typeEntry : genericImpls) {
        const std::string& pattern = typeEntry.first;
        TypePattern templatePattern = TypePattern::parse(pattern);
        
        std::cout << "DEBUG: Checking pattern: " << pattern << " (base: " << templatePattern.base << ")" << std::endl;
        
        // Try to match the pattern
        std::map<std::string, std::string> typeSubstitutions;
        if (templatePattern.matchesPattern(concretePattern, typeSubstitutions)) {
            std::cout << "DEBUG: Pattern matched! Type substitutions:" << std::endl;
            for (const auto& sub : typeSubstitutions) {
                std::cout << "  " << sub.first << " -> " << sub.second << std::endl;
            }
            
            // Check if this pattern has an impl for the requested trait
            const auto& traitMap = typeEntry.second;
            auto traitIt = traitMap.find(traitName);
            if (traitIt != traitMap.end()) {
                const GenericImplInfo* implInfo = traitIt->second.get();
                
                // Check if the method exists in this impl
                auto methodIt = implInfo->methods.find(methodName);
                if (methodIt != implInfo->methods.end()) {
                    ast::FunctionDeclaration* methodAST = methodIt->second;
                    
                    std::cout << "DEBUG: Found method in generic impl! Will monomorphize." << std::endl;
                    
                    // Clone the method AST for modification
                    // Since we don't have a deep clone method for FunctionDeclaration,
                    // we'll work with the original and generate specialized code directly
                    
                    // Build specialized function name: TypeName_MethodName (e.g., "Box_Int_show")
                    std::string specializedName = concretePattern.toMangled() + "_" + methodName;
                    std::cout << "DEBUG: Generating specialized function: " << specializedName << std::endl;
                    
                    // Get the method's signature
                    std::vector<llvm::Type*> paramTypes;
                    
                    // First parameter is always Self (the object type)
                    llvm::Type* selfType = resolveTypeForMonomorphization(concretePattern, typeSubstitutions);
                    if (!selfType) {
                        logError(SourceLocation(), "Failed to resolve Self type for " + concreteType);
                        return nullptr;
                    }
                    paramTypes.push_back(selfType);
                    
                    // Add remaining parameters
                    for (size_t i = 0; i < methodAST->params.size(); ++i) {
                        const auto& param = methodAST->params[i];
                        
                        // Skip first parameter if it's Self (already handled)
                        if (i == 0 && param.typeNode) {
                            if (auto typeName = dynamic_cast<ast::TypeName*>(param.typeNode.get())) {
                                if (typeName->identifier && typeName->identifier->name == "Self") {
                                    continue;  // Already added
                                }
                            }
                        }
                        
                        // Substitute type parameters in this parameter's type
                        llvm::Type* paramType = resolveParameterTypeWithSubstitution(
                            param.typeNode.get(), typeSubstitutions);
                        if (!paramType) {
                            logError(SourceLocation(), "Failed to resolve parameter type for " + param.name->name);
                            return nullptr;
                        }
                        paramTypes.push_back(paramType);
                    }
                    
                    // Determine return type with substitution
                    llvm::Type* returnType = llvm::Type::getVoidTy(*context);
                    if (methodAST->returnTypeNode) {
                        returnType = resolveReturnTypeWithSubstitution(
                            methodAST->returnTypeNode.get(), typeSubstitutions);
                        if (!returnType) {
                            logError(SourceLocation(), "Failed to resolve return type");
                            return nullptr;
                        }
                    }
                    
                    // Create the specialized function
                    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes, false);
                    llvm::Function* specializedFunc = llvm::Function::Create(
                        funcType,
                        llvm::Function::ExternalLinkage,
                        specializedName,
                        module.get()
                    );
                    
                                        // Cache it before generating body
                    monomorphizedMethods[cacheKey] = specializedFunc;
                    
                    // Generate the function body with type substitution active
                    std::cout << "DEBUG: Generating specialized function body..." << std::endl;
                    
                    // Save current state - INCLUDING builder insert point!
                    auto savedTypeSubstitutions = currentTypeSubstitutions;
                    auto savedNamedValues = namedValues;
                    llvm::BasicBlock* savedInsertBlock = builder->GetInsertBlock();
                    llvm::BasicBlock::iterator savedInsertPoint = builder->GetInsertPoint();
                    
                    currentTypeSubstitutions = typeSubstitutions;
                    
                    // Generate function body by visiting the method AST
                    // The visitor will use currentTypeSubstitutions for type resolution
                    llvm::BasicBlock* entry = llvm::BasicBlock::Create(*context, "entry", specializedFunc);
                    builder->SetInsertPoint(entry);
                    
                    // Set up named values for parameters
                    namedValues.clear();
                    
                    size_t argIdx = 0;
                    for (auto& arg : specializedFunc->args()) {
                        if (argIdx == 0) {
                            // First argument is Self
                            arg.setName("self");
                            llvm::AllocaInst* alloca = builder->CreateAlloca(arg.getType(), nullptr, "self.addr");
                            builder->CreateStore(&arg, alloca);
                            namedValues["self"] = alloca;
                        } else {
                            // Regular parameters
                            size_t paramIdx = argIdx - 1;
                            if (paramIdx < methodAST->params.size()) {
                                const auto& param = methodAST->params[paramIdx];
                                arg.setName(param.name->name);
                                llvm::AllocaInst* alloca = builder->CreateAlloca(arg.getType(), nullptr, param.name->name + ".addr");
                                builder->CreateStore(&arg, alloca);
                                namedValues[param.name->name] = alloca;
                            }
                        }
                        argIdx++;
                    }
                    
                    // Visit the function body
                    if (methodAST->body) {
                        methodAST->body->accept(*this);
                    }
                    
                    // Ensure function has a return
                    if (!builder->GetInsertBlock()->getTerminator()) {
                        if (returnType->isVoidTy()) {
                            builder->CreateRetVoid();
                        } else {
                            // Return default value
                            builder->CreateRet(llvm::Constant::getNullValue(returnType));
                        }
                    }
                    
                    // Restore state - INCLUDING builder insert point!
                    namedValues = savedNamedValues;
                    currentTypeSubstitutions = savedTypeSubstitutions;
                    if (savedInsertBlock) {
                        if (savedInsertPoint != savedInsertBlock->end()) {
                            builder->SetInsertPoint(savedInsertBlock, savedInsertPoint);
                        } else {
                            builder->SetInsertPoint(savedInsertBlock);
                        }
                    }
                    
                    std::cout << "DEBUG: Successfully generated specialized function: " << specializedName << std::endl;
                    return specializedFunc;
                }
            }
        }
    }
    
    std::cout << "DEBUG: No matching generic impl found for " << concreteType << "::" << methodName << std::endl;
    return nullptr;
}

// Helper: Resolve the concrete type for monomorphization (e.g., Box<Int> -> %struct.Box_Int)
llvm::Type* LLVMCodegen::resolveTypeForMonomorphization(const TypePattern& pattern, 
                                                        const std::map<std::string, std::string>& substitutions) {
    std::string mangledName = pattern.toMangled();
    
    std::cout << "DEBUG: Resolving type for pattern: " << mangledName << std::endl;
    
    // Check if struct type already exists (without "struct." prefix - the name used in monomorphizeStruct)
    if (llvm::StructType* structType = llvm::StructType::getTypeByName(*context, mangledName)) {
        std::cout << "DEBUG: Found existing struct type: " << mangledName << std::endl;
        return structType;
    }
    
    // Also try with "struct." prefix for compatibility
    std::string structName = "struct." + mangledName;
    if (llvm::StructType* structType = llvm::StructType::getTypeByName(*context, structName)) {
        std::cout << "DEBUG: Found existing struct type: " << structName << std::endl;
        return structType;
    }
    
    // For now, return generic pointer if we can't find the struct
    // In production, this should trigger struct monomorphization
    std::cout << "DEBUG: WARNING: Struct type not found, using i8* as fallback" << std::endl;
    return llvm::PointerType::get(llvm::Type::getInt8Ty(*context), 0);
}

// Helper: Resolve parameter type with substitution (e.g., T -> Int)
llvm::Type* LLVMCodegen::resolveParameterTypeWithSubstitution(vyn::ast::TypeNode* typeNode, 
                                                              const std::map<std::string, std::string>& substitutions) {
    if (!typeNode) {
        return nullptr;
    }
    
    // Check if this is a type parameter that needs substitution
    if (auto typeName = dynamic_cast<ast::TypeName*>(typeNode)) {
        if (typeName->identifier) {
            const std::string& name = typeName->identifier->name;
            
            // Check if it's a type parameter
            auto it = substitutions.find(name);
            if (it != substitutions.end()) {
                std::cout << "DEBUG: Substituting type parameter " << name << " -> " << it->second << std::endl;
                
                // Resolve the concrete type by creating a TypeName node
                auto concreteTypeNode = std::make_unique<ast::TypeName>(
                    SourceLocation(),
                    std::make_unique<ast::Identifier>(SourceLocation(), it->second)
                );
                return codegenType(concreteTypeNode.get());
            }
        }
    }
    
    // Otherwise, use normal type resolution
    return codegenType(typeNode);
}

// Helper: Resolve return type with substitution
llvm::Type* LLVMCodegen::resolveReturnTypeWithSubstitution(vyn::ast::TypeNode* typeNode, 
                                                           const std::map<std::string, std::string>& substitutions) {
    // Same logic as parameter resolution
    return resolveParameterTypeWithSubstitution(typeNode, substitutions);
}

} // namespace vyn
