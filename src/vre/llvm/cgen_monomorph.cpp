// Monomorphization: Generate specialized versions of generic types
// This file implements the monomorphization system for generic structs, traits, and functions.
// When generic types like Box<T> are instantiated with concrete types like Box<Int>,
// this system generates specialized LLVM struct types with type parameters substituted.

#include "vyn/vre/llvm/codegen.hpp"
#include "vyn/parser/ast.hpp"
#include <sstream>

namespace vyn {

// Generate a mangled name for a generic type instantiation
// Example: Box with [Int] -> "Box_Int"
//          Vec with [Box<Int>] -> "Vec_Box_Int"
std::string LLVMCodegen::mangleGenericTypeName(const std::string& baseName, const std::vector<ast::TypeNodePtr>& typeArgs) {
    std::stringstream ss;
    ss << baseName;
    
    for (const auto& typeArg : typeArgs) {
        ss << "_";
        if (!typeArg) {
            ss << "Unknown";
            continue;
        }
        
        // Handle different type node categories
        if (auto* typeName = dynamic_cast<ast::TypeName*>(typeArg.get())) {
            if (typeName->identifier) {
                ss << typeName->identifier->name;
                
                // Handle nested generics like Box<Int>
                if (!typeName->genericArgs.empty()) {
                    ss << "_";
                    for (size_t i = 0; i < typeName->genericArgs.size(); ++i) {
                        if (i > 0) ss << "_";
                        if (typeName->genericArgs[i]) {
                            auto* innerType = dynamic_cast<ast::TypeName*>(typeName->genericArgs[i].get());
                            if (innerType && innerType->identifier) {
                                ss << innerType->identifier->name;
                            } else {
                                ss << "Unknown";
                            }
                        }
                    }
                }
            } else {
                ss << "Unknown";
            }
        } else {
            // For other type categories, use toString() and sanitize
            std::string typeStr = typeArg->toString();
            // Replace invalid characters for LLVM struct names
            for (char& c : typeStr) {
                if (c == '<' || c == '>' || c == ',' || c == ' ') {
                    c = '_';
                }
            }
            ss << typeStr;
        }
    }
    
    return ss.str();
}

// Substitute type parameters in a type node
// Example: T -> Int when instantiating Box<T> with Box<Int>
ast::TypeNodePtr substituteTypeParameter(ast::TypeNode* typeNode, 
                                         const std::map<std::string, ast::TypeNode*>& typeParamMap) {
    if (!typeNode) return nullptr;
    
    // If it's a TypeName, check if it's a type parameter
    if (auto* typeName = dynamic_cast<ast::TypeName*>(typeNode)) {
        if (typeName->identifier) {
            const std::string& name = typeName->identifier->name;
            
            // Check if this is a type parameter that needs substitution
            auto it = typeParamMap.find(name);
            if (it != typeParamMap.end()) {
                // Substitute with concrete type
                return it->second->clone();
            }
            
            // Not a type parameter, but might have generic args that need substitution
            if (!typeName->genericArgs.empty()) {
                std::vector<ast::TypeNodePtr> substitutedArgs;
                for (const auto& arg : typeName->genericArgs) {
                    substitutedArgs.push_back(substituteTypeParameter(arg.get(), typeParamMap));
                }
                return std::make_unique<ast::TypeName>(
                    typeName->loc, 
                    std::make_unique<ast::Identifier>(typeName->identifier->loc, typeName->identifier->name),
                    std::move(substitutedArgs)
                );
            }
        }
    }
    
    // For other types, just clone
    return typeNode->clone();
}

// Monomorphize a generic struct: create specialized LLVM type with type parameters substituted
// Example: Box<T> + [Int] -> Box_Int struct with field type T replaced by Int
llvm::StructType* LLVMCodegen::monomorphizeStruct(const std::string& baseName, 
                                                   const std::vector<ast::TypeNodePtr>& typeArgs) {
    // Generate mangled name for this instantiation
    std::string mangledName = mangleGenericTypeName(baseName, typeArgs);
    
    std::cout << "DEBUG: Monomorphizing " << baseName << " with " << typeArgs.size() 
              << " type arguments -> " << mangledName << std::endl;
    
    // Check cache first
    auto cacheIt = monomorphizedStructs.find(mangledName);
    if (cacheIt != monomorphizedStructs.end()) {
        std::cout << "DEBUG: Found cached monomorphized struct: " << mangledName << std::endl;
        return cacheIt->second;
    }
    
    // Look up the generic struct template
    auto templateIt = genericStructTemplates.find(baseName);
    if (templateIt == genericStructTemplates.end()) {
        std::cerr << "ERROR: No generic struct template found for: " << baseName << std::endl;
        return nullptr;
    }
    
    ast::StructDeclaration* templateNode = templateIt->second;
    
    // Verify type argument count matches
    if (typeArgs.size() != templateNode->genericParams.size()) {
        std::cerr << "ERROR: Type argument count mismatch for " << baseName 
                  << ": expected " << templateNode->genericParams.size() 
                  << ", got " << typeArgs.size() << std::endl;
        return nullptr;
    }
    
    // Create type parameter substitution map (T -> Int, etc.)
    std::map<std::string, ast::TypeNode*> typeParamMap;
    for (size_t i = 0; i < templateNode->genericParams.size(); ++i) {
        const auto& param = templateNode->genericParams[i];
        if (param && param->name) {
            std::string paramName = param->name->name;
            typeParamMap[paramName] = typeArgs[i].get();
            std::cout << "DEBUG: Type parameter mapping: " << paramName << " -> " 
                      << typeArgs[i]->toString() << std::endl;
        }
    }
    
    // Create the specialized LLVM struct type
    llvm::StructType* specializedType = llvm::StructType::create(*context, mangledName);
    
    // Create UserTypeInfo for the specialized type
    UserTypeInfo typeInfo;
    typeInfo.llvmType = specializedType;
    typeInfo.isStruct = true;
    
    // Add to userTypeMap early (for circular references)
    userTypeMap[mangledName] = typeInfo;
    
    // Process fields with type substitution
    std::vector<llvm::Type*> fieldTypes;
    for (size_t i = 0; i < templateNode->fields.size(); ++i) {
        const auto& fieldDecl = templateNode->fields[i];
        if (!fieldDecl || !fieldDecl->typeNode) {
            std::cerr << "ERROR: Field missing type in template " << baseName << std::endl;
            return nullptr;
        }
        
        // Substitute type parameters in field type
        ast::TypeNodePtr substitutedType = substituteTypeParameter(fieldDecl->typeNode.get(), typeParamMap);
        
        std::cout << "DEBUG: Field '" << fieldDecl->name->name << "' original type: " 
                  << fieldDecl->typeNode->toString() 
                  << " -> substituted: " << substitutedType->toString() << std::endl;
        
        // Generate LLVM type for substituted field type
        llvm::Type* fieldLLVMType = codegenType(substitutedType.get());
        if (!fieldLLVMType) {
            std::cerr << "ERROR: Could not generate LLVM type for field '" 
                      << fieldDecl->name->name << "' in " << mangledName << std::endl;
            return nullptr;
        }
        
        fieldTypes.push_back(fieldLLVMType);
        typeInfo.fieldIndices[fieldDecl->name->name] = i;
    }
    
    // Set the struct body
    specializedType->setBody(fieldTypes, /*isPacked=*/false);
    
    std::cout << "DEBUG: Created specialized struct " << mangledName 
              << " with " << fieldTypes.size() << " fields" << std::endl;
    
    // Update userTypeMap with complete field information
    userTypeMap[mangledName] = typeInfo;
    
    // Cache the monomorphized struct
    monomorphizedStructs[mangledName] = specializedType;
    
    return specializedType;
}

} // namespace vyn
