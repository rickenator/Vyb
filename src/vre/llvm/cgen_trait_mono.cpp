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
                    
                    // TODO: Clone AST, substitute types, generate LLVM function
                    // For now, just log that we found it
                    logWarning(SourceLocation(), 
                              "Trait method monomorphization found match but AST cloning not yet implemented");
                    
                    return nullptr;
                }
            }
        }
    }
    
    std::cout << "DEBUG: No matching generic impl found for " << concreteType << "::" << methodName << std::endl;
    return nullptr;
}

} // namespace vyn
