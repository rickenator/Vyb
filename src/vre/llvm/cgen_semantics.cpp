#include "vyn/vre/llvm/codegen.hpp"
#include "vyn/semantic.hpp"
#include "vyn/parser/ast.hpp"

using namespace vyn;

// Helper method to extract original type name using semantic analysis data
std::string LLVMCodegen::extractOriginalTypeNameFromSemantics(vyn::ast::Expression* expr) {
    if (!expr) {
        return "unknown";
    }
    
    // Get the semantic analyzer from the driver
    SemanticAnalyzer* semanticAnalyzer = driver_.getSemanticAnalyzer();
    if (!semanticAnalyzer) {
        // Fallback to the existing AST-based method
        return extractOriginalTypeName(expr);
    }
    
    // Look up the expression type in the semantic analyzer's expressionTypes map
    auto& expressionTypes = semanticAnalyzer->getExpressionTypes();
    auto it = expressionTypes.find(expr);
    
    if (it != expressionTypes.end() && it->second) {
        ast::TypeNode* typeNode = it->second;
        
        // Check if this is a type alias by examining the type node
        if (auto typeName = dynamic_cast<ast::TypeName*>(typeNode)) {
            if (typeName->identifier) {
                // This is an identifier type - could be a type alias
                std::string typeNameStr = typeName->identifier->name;
                
                // Check if this is a type alias by looking in our typeAliasMap
                auto aliasIt = typeAliasMap.find(typeNameStr);
                if (aliasIt != typeAliasMap.end()) {
                    // This is a type alias, return the alias name
                    return typeNameStr;
                }
                
                // Not a type alias, but still return the identifier name
                return typeNameStr;
            }
        }
        
        // For other types, return their string representation
        return typeNode->toString();
    }
    
    // Fallback to AST-based analysis if semantic information is not available
    return extractOriginalTypeNameFromAST(expr);
}

// Helper method to extract original type name from AST directly (fallback method)
std::string LLVMCodegen::extractOriginalTypeNameFromAST(vyn::ast::Expression* expr) {
    if (!expr) {
        return "unknown";
    }
    
    // Handle different expression types
    if (auto intLiteral = dynamic_cast<ast::IntegerLiteral*>(expr)) {
        return "Int";
    } else if (auto floatLiteral = dynamic_cast<ast::FloatLiteral*>(expr)) {
        return "Float";
    } else if (auto stringLiteral = dynamic_cast<ast::StringLiteral*>(expr)) {
        return "String";
    } else if (auto boolLiteral = dynamic_cast<ast::BooleanLiteral*>(expr)) {
        return "Bool";
    } else if (auto nilLiteral = dynamic_cast<ast::NilLiteral*>(expr)) {
        return "Nil";
    } else if (auto identifier = dynamic_cast<ast::Identifier*>(expr)) {
        // For identifiers, we'd need to look up their declared type
        // For now, return a placeholder
        return "Identifier(" + identifier->name + ")";
    } else if (auto callExpr = dynamic_cast<ast::CallExpression*>(expr)) {
        // Handle intrinsic calls like notype()
        if (auto calleeId = dynamic_cast<ast::Identifier*>(callExpr->callee.get())) {
            if (calleeId->name == "notype") {
                // For notype(), return empty string to suppress type metadata
                return "";
            } else if (calleeId->name == "lit") {
                // For lit(), extract the type from the argument
                if (!callExpr->arguments.empty()) {
                    return extractOriginalTypeNameFromAST(callExpr->arguments[0].get());
                }
            }
        }
        // For other call expressions, try to determine from context
        return "CallResult";
    } else if (auto memberExpr = dynamic_cast<ast::MemberExpression*>(expr)) {
        return "MemberAccess";
    } else if (auto arrayExpr = dynamic_cast<ast::ArrayElementExpression*>(expr)) {
        return "ArrayElement";
    }
    
    // Default fallback
    return "Unknown";
}
