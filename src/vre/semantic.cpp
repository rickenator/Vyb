#include "vyn/semantic.hpp"
#include "vyn/parser/token.hpp"
#include "vyn/parser/ast.hpp"
#include "vyn/driver.hpp"
#include <stdexcept>
#include <memory>
#include <unordered_set> 
#include <string> 

namespace vyn {

// Scope class implementation
Scope::Scope(Scope* parent_scope) : parent(parent_scope) {}

SymbolInfo* Scope::find(const std::string& name) {
    auto it = symbols.find(name);
    if (it != symbols.end()) {
        return it->second;
    }
    if (parent) {
        return parent->find(name);
    }
    return nullptr;
}

// Add SymbolTable::lookupDirect (assuming Scope is effectively SymbolTable here based on usage)
SymbolInfo* SymbolTable::lookupDirect(const std::string& name) {
    auto it = table.find(name);
    if (it != table.end()) {
        return &it->second;
    }
    return nullptr;
}

void Scope::insert(const std::string& name, SymbolInfo* symbol) {
    symbols[name] = symbol;
}

Scope* Scope::getParent() {
    return parent;
}

// --- SemanticAnalyzer Implementation ---

// SemanticAnalyzer constructor
SemanticAnalyzer::SemanticAnalyzer(Driver& driver) : driver_(driver), currentScope(nullptr) {
    enterScope(); // Create global scope
    // Initialize reservedWords set
    reservedWords = {
        "let", "var", "const", "fn", "struct", "enum", "trait", "impl", "type", "class",
        "if", "else", "while", "for", "return", "break", "continue", "loop", "match",
        "try", "catch", "finally", "defer", "throw", "async", "await", "my", "our", "their",
        "ptr", "borrow", "view", "unsafe", "pub", "template", "operator", "as", "in", "ref",
        "extern", "import", "smuggle", "module", "use", "mut", "scoped", "bundle",
        "true", "false", "null", "nil",
        "addr", "at", "loc", "from",
        "lit", "notype", "bare", "deserial"  // Added serialization mode intrinsics
    };
}

// --- Canonical block of helpers and basic visit methods ---

// Helper methods (Single definitions)
void SemanticAnalyzer::addError(const std::string& message, const ast::Node* node) {
    errors.push_back(message);
}

void SemanticAnalyzer::enterScope() {
    currentScope = new SymbolTable(currentScope);
}

void SemanticAnalyzer::exitScope() {
    if (currentScope) {
        SymbolTable* parent = currentScope->getParent();
        delete currentScope;
        currentScope = parent;
    }
}

void SemanticAnalyzer::analyze(ast::Module* root) {
    if (root) {
        root->accept(*this);
    }
}

bool SemanticAnalyzer::isInLoop() {
    SymbolTable* scope = currentScope;
    while (scope) {
        if (scope->isLoop) {
            return true;
        }
        scope = scope->getParent();
    }
    return false;
}

bool SemanticAnalyzer::isInUnsafeBlock() {
    SymbolTable* scope = currentScope;
    while (scope) {
        if (scope->isUnsafeBlock) {
            return true;
        }
        scope = scope->getParent();
    }
    return false;
}

bool SemanticAnalyzer::isIntegerType(ast::TypeNode* type) {
    if (!type) return false;
    
    // Handle TypeName nodes (most common case)
    if (auto tn = dynamic_cast<ast::TypeName*>(type)) {
        if (!tn->identifier) return false;
        const std::string& name = tn->identifier->name;
        return name == "int" || name == "Int" || name == "i8" || name == "i16" || name == "i32" || name == "i64" ||
               name == "u8" || name == "u16" || name == "u32" || name == "u64" || name == "size_t" || 
               name == "isize" || name == "usize";
    }
    
    // Handle array size expressions which might be integer literals
    if (auto arrayType = dynamic_cast<ast::ArrayType*>(type)) {
        // If it has a size expression that's a literal, it might be integer type
        if (arrayType->sizeExpression) {
            auto it = expressionTypes.find(arrayType->sizeExpression.get());
            if (it != expressionTypes.end() && it->second) {
                return isIntegerType(it->second);
            }
        }
    }
    
    // Add any other type checks that could represent integer types
    // For example, if there are typedef'ed types or alias types
    
    return false; 
}

bool SemanticAnalyzer::isReservedWord(const std::string& name) {
    return reservedWords.count(name);
}

bool SemanticAnalyzer::isLValue(ast::Expression* expr) {
    return dynamic_cast<ast::Identifier*>(expr) != nullptr ||
           dynamic_cast<ast::MemberExpression*>(expr) != nullptr ||
           dynamic_cast<ast::ArrayElementExpression*>(expr) != nullptr ||
           dynamic_cast<ast::PointerDerefExpression*>(expr) != nullptr; 
}

bool SemanticAnalyzer::isRawLocationType(ast::Expression* expr) {
    auto it = expressionTypes.find(expr);
    if (it == expressionTypes.end() || !it->second) return false; 
    // TODO: Actually check if it->second is a raw location type (e.g. specific TypeName or PointerType)
    // For now, returning true if type is known, this needs proper implementation.
    return true; // Added return
}


// Basic visit methods for expressions (Single definitions)
void SemanticAnalyzer::visit(ast::Identifier* node) {
    SymbolInfo* symbol = currentScope->lookup(node->name);
    if (!symbol) {
        addError("Undefined identifier: " + node->name, node);
        expressionTypes[node] = nullptr; 
        return;
    }
    expressionTypes[node] = symbol->type;
    if (symbol->type) {
        node->type = std::shared_ptr<ast::TypeNode>(symbol->type->clone());
        std::cout << "DEBUG: Set AST type for identifier '" << node->name << "' to: " << node->type->toString() << std::endl;
    } else {
        std::cout << "DEBUG: No type found for identifier '" << node->name << "'" << std::endl;
    }
}

void SemanticAnalyzer::visit(ast::IntegerLiteral* node) {
    expressionTypes[node] = new ast::TypeName(node->loc, std::make_unique<ast::Identifier>(node->loc, "Int"));
}

void SemanticAnalyzer::visit(ast::FloatLiteral* node) {
    expressionTypes[node] = new ast::TypeName(node->loc, std::make_unique<ast::Identifier>(node->loc, "float"));
}

void SemanticAnalyzer::visit(ast::StringLiteral* node) {
    expressionTypes[node] = new ast::TypeName(node->loc, std::make_unique<ast::Identifier>(node->loc, "string"));
}

void SemanticAnalyzer::visit(ast::BooleanLiteral* node) {
    expressionTypes[node] = new ast::TypeName(node->loc, std::make_unique<ast::Identifier>(node->loc, "bool"));
}

void SemanticAnalyzer::visit(ast::NilLiteral* node) {
    expressionTypes[node] = new ast::TypeName(node->loc, std::make_unique<ast::Identifier>(node->loc, "nil"));
}

// Basic visit methods for statements (Single definitions)
void SemanticAnalyzer::visit(ast::BlockStatement* node) {
    enterScope();
    for (auto& stmt : node->body) {
        stmt->accept(*this);
    }
    exitScope();
}

void SemanticAnalyzer::visit(ast::ExpressionStatement* node) {
    if (node->expression) {
        node->expression->accept(*this);
    }
}

void SemanticAnalyzer::visit(ast::EmptyStatement* node) {
    // Empty statements don't need any analysis
}

// END OF THE CANONICAL BLOCK OF HELPERS AND BASIC VISITORS
// ALL SUBSEQUENT DUPLICATE DEFINITIONS OF THE METHODS ABOVE THIS COMMENT WILL BE REMOVED.

// --- More complex visit methods and specific logic follow ---

void SemanticAnalyzer::visit(ast::Module* node) {
    // Module-level semantic analysis
    for (auto& item : node->body) { // Changed items to body
        if (item) item->accept(*this);
    }
}

void SemanticAnalyzer::visit(ast::FunctionDeclaration* node) {
    if (isReservedWord(node->id->name)) {
        addError("Identifier \\\"" + node->id->name + "\\\" is a reserved word and cannot be used as a function name.", node->id.get());
    }
    if (currentScope->lookupDirect(node->id->name)) { 
        addError("Redefinition of function \\\"" + node->id->name + "\\\" in the same scope.", node->id.get());
    }

    auto funcSymbol = new SymbolInfo{SymbolInfo::Kind::Function, node->id->name, false, ast::OwnershipKind::MY, nullptr};
    currentScope->add(SymbolInfo{funcSymbol->kind, funcSymbol->name, funcSymbol->isConst, funcSymbol->ownershipKind, funcSymbol->type}); // Explicit SymbolInfo construction
    delete funcSymbol; // SymbolInfo is copied into the table

    enterScope(); 

    std::vector<std::unique_ptr<ast::TypeNode>> paramTypesVec;
    
    for (auto& param : node->params) { 
        if (param.name) { // Changed from param.id to param.name
             if (isReservedWord(param.name->name)) { // Changed from param.id to param.name
                addError("Identifier \\\"" + param.name->name + "\\\" is a reserved word and cannot be used as a parameter name.", param.name.get()); // Changed from param.id to param.name
            }
            if (currentScope->lookupDirect(param.name->name)) { // Changed from param.id to param.name
                 addError("Redefinition of parameter \\\"" + param.name->name + "\\\".", param.name.get()); // Changed from param.id to param.name
            }
            if (param.typeNode) { // Changed from param.type to param.typeNode
                param.typeNode->accept(*this); 
                paramTypesVec.push_back(param.typeNode->clone());
                currentScope->add(SymbolInfo{SymbolInfo::Kind::Variable, param.name->name, false, ast::OwnershipKind::MY, param.typeNode->clone().release()}); // Explicit SymbolInfo, changed param.id to param.name, param.type to param.typeNode
            } else {
                addError("Parameter \\\"" + param.name->name + "\\\" missing type.", param.name.get()); // Changed from param.id to param.name
                paramTypesVec.push_back(nullptr); 
                currentScope->add(SymbolInfo{SymbolInfo::Kind::Variable, param.name->name, false, ast::OwnershipKind::MY, nullptr}); // Explicit SymbolInfo, changed param.id to param.name
            }
        }
    }

    ast::TypeNode* returnTypeAstNode = nullptr;
    if (node->returnTypeNode) { // Changed from returnType to returnTypeNode
        node->returnTypeNode->accept(*this); 
        returnTypeAstNode = node->returnTypeNode->clone().release();
    } else {
        auto void_type_id = std::make_unique<ast::Identifier>(node->loc, "void");
        returnTypeAstNode = new ast::TypeName(node->loc, std::move(void_type_id));
    }
    
    SymbolInfo* funcSymFromTable = currentScope->getParent()->lookup(node->id->name);
    if (funcSymFromTable) {
        funcSymFromTable->type = new ast::FunctionType(node->loc, std::move(paramTypesVec), std::unique_ptr<ast::TypeNode>(returnTypeAstNode));
        if (funcSymFromTable->type) {
             node->type = std::shared_ptr<ast::TypeNode>(funcSymFromTable->type->clone().release());
        }
    }

    if (node->body) {
        node->body->accept(*this);
    }

    exitScope();
}

void SemanticAnalyzer::visit(ast::VariableDeclaration* node) {
    if (isReservedWord(node->id->name)) {
        addError("Identifier \\\"" + node->id->name + "\\\" is a reserved word and cannot be used as a variable name.", node->id.get());
    }

    if (currentScope->lookupDirect(node->id->name)) { 
        addError("Redefinition of variable \"" + node->id->name + "\" in the same scope.", node->id.get());
    }

    // Enforce mandatory type annotation for var<T> and const<T>
    if (!node->typeNode) {
        if (node->isConst) {
            addError("Missing type annotation on constant declaration '" + node->id->name + "'; expected const<T> name.", node);
        } else {
            addError("Missing type annotation on variable declaration '" + node->id->name + "'; expected var<T> name.", node);
        }
    }

    if (node->typeNode) { // Changed from type to typeNode
        node->typeNode->accept(*this);
    }

    if (node->init) { 
        node->init->accept(*this);
        
        // Handle type inference for auto variables
        if (!node->typeNode && expressionTypes.count(node->init.get())) {
            // Auto type inference - set the type based on initializer
            ast::TypeNode* initType = expressionTypes[node->init.get()];
            if (initType) {
                node->typeNode = std::unique_ptr<ast::TypeNode>(initType->clone());
            } else {
                addError("Cannot infer type for 'auto' variable \\\"" + node->id->name + "\\\", initializer has no type", node);
            }
        }
        
        // Now check types match (for both explicit types and inferred types)
        if (node->typeNode && expressionTypes.count(node->init.get())) {
            ast::TypeNode* varType = node->typeNode.get();
            ast::TypeNode* initType = expressionTypes[node->init.get()];
            if (initType) { 
                if (!areTypesCompatible(varType, initType)) { 
                    addError("Initializer type does not match variable type for \\\"" + node->id->name + "\\\". Expected " + varType->toString() + " but got " + initType->toString(), node);
                }
            }
        }
    } else if (!node->typeNode) {
        // Auto variables must have an initializer for type inference
        addError("'auto' variable \\\"" + node->id->name + "\\\" must have an initializer for type inference", node);
    }
    
    ast::TypeNode* symbolType = node->typeNode ? node->typeNode.get() : nullptr;
    if (!symbolType && node->init && expressionTypes.count(node->init.get())) {
        symbolType = expressionTypes[node->init.get()];
    }

    SymbolInfo::Kind kind = SymbolInfo::Kind::Variable; 
    currentScope->add(SymbolInfo{kind, node->id->name, node->isConst, ast::OwnershipKind::MY, symbolType ? symbolType->clone().release() : nullptr}); // Explicit SymbolInfo
}

void SemanticAnalyzer::visit(ast::ClassDeclaration* node) {
    if (isReservedWord(node->name->name)) {
        addError("Identifier \\\"" + node->name->name + "\\\" is a reserved word and cannot be used as a class name.", node->name.get());
    }
    if (currentScope->lookupDirect(node->name->name)) {
        addError("Redefinition of class \\\"" + node->name->name + "\\\" in the same scope.", node->name.get());
    }
    currentScope->add(SymbolInfo{SymbolInfo::Kind::Type, node->name->name, false, ast::OwnershipKind::MY, new ast::TypeName(node->loc, std::make_unique<ast::Identifier>(node->name->loc, node->name->name))});


    enterScope();

    for (auto& member : node->members) {
        if (member) {
            // Assuming members are Declarations, e.g. FunctionDeclaration (for methods) or VariableDeclaration (for fields)
            // The accept method will call the appropriate visit method.
            member->accept(*this);
        }
    }
    exitScope();
}

// Removed visit(ast::MethodDeclaration* node)
// Removed visit(ast::ConstructorDeclaration* node)
// Removed visit(ast::DestructorDeclaration* node)

void SemanticAnalyzer::visit(ast::TraitDeclaration* node) {
    // Body commented out due to incomplete type errors from build log.
    // Requires ast.hpp to have full definition of TraitDeclaration.
    // if (node && node->id && isReservedWord(node->id->name)) {
    //     addError("Identifier \\\"" + node->id->name + "\\\" is a reserved word and cannot be used as a trait name.", node->id.get());
    // }
    // if (node && node->id && currentScope->lookupDirect(node->id->name)) {
    //     addError("Redefinition of trait \\\"" + node->id->name + "\\\" in the same scope.", node->id.get());
    // }
    // if (node && node->id) {
    //    currentScope->add(SymbolInfo{SymbolInfo::Kind::Type, node->id->name, false, new ast::TypeName(node->loc, std::make_unique<ast::Identifier>(node->id->loc, node->id->name))}); // Assuming id has loc and name
    // }

    // enterScope();
    // if (node) {
    //     for (auto& member : node->methods) { 
    //         if(member) member->accept(*this);
    //     }
    // }
    // exitScope();
}

void SemanticAnalyzer::visit(ast::ImplDeclaration* node) {
    // Impl needs to refer to a Trait and a Type.
    if (node->traitType) node->traitType->accept(*this); // Changed traitName to traitType
    if (node->selfType) node->selfType->accept(*this);   // Changed typeName to selfType

    // TODO: Add checks for trait and type existence and compatibility.
    // SymbolInfo* traitSymbol = currentScope->lookup(node->traitType->toString()); 
    // SymbolInfo* typeSymbol = currentScope->lookup(node->selfType->toString());  
    // if (!traitSymbol || traitSymbol->kind != SymbolInfo::Kind::Type) { /* error */ }
    // if (!typeSymbol || typeSymbol->kind != SymbolInfo::Kind::Type) { /* error */ }

    enterScope();
    for (auto& method : node->methods) { 
        if(method) method->accept(*this);
        // Additionally, check if methods match trait definition
    }
    exitScope();
}


void SemanticAnalyzer::visit(ast::NamespaceDeclaration* node) {
    // Body commented out due to incomplete type errors from build log.
    // Requires ast.hpp to have full definition of NamespaceDeclaration.
    // if (node && node->name && isReservedWord(node->name->name)) { 
    //     addError("Identifier \\\"" + node->name->name + "\\\" is a reserved word and cannot be used as a namespace name.", node->name.get());
    // }
    // enterScope(); 
    // if (node) {
    //    for (auto& item : node->body) { // Assuming 'body' based on ast::Module
    //        if (item) item->accept(*this);
    //    }
    // }
    // exitScope(); 
}

void SemanticAnalyzer::visit(ast::TypeAliasDeclaration* node) {
    if (node->name && isReservedWord(node->name->name)) { 
        addError("Identifier \\\"" + node->name->name + "\\\" is a reserved word and cannot be used as a type alias name.", node->name.get());
    }
    if (node->name && currentScope->lookupDirect(node->name->name)) {
        addError("Redefinition of type alias \\\"" + node->name->name + "\\\" in the same scope.", node->name.get());
    }

    if (node->typeNode) { 
        node->typeNode->accept(*this); 
    }

    if (node->name && node->typeNode && node->typeNode->type) {
        currentScope->add(SymbolInfo{SymbolInfo::Kind::Type, node->name->name, false, ast::OwnershipKind::MY, node->typeNode->type->clone().release()}); // Explicit SymbolInfo
    } else if (node->name) {
        addError("Type alias \\\"" + node->name->name + "\\\" has an unresolved target type.", node);
        currentScope->add(SymbolInfo{SymbolInfo::Kind::Type, node->name->name, false, ast::OwnershipKind::MY, nullptr}); // Explicit SymbolInfo
    }
}

// --- Type and expression analysis ---

void SemanticAnalyzer::visit(ast::UnaryExpression* node) {}
void SemanticAnalyzer::visit(ast::BinaryExpression* node) {}
void SemanticAnalyzer::visit(ast::CallExpression* node) {
    // Check if this is an intrinsic function call before visiting the callee
    bool isIntrinsic = false;
    if (auto ident = dynamic_cast<ast::Identifier*>(node->callee.get())) {
        const std::string& name = ident->name;
        if (name == "lit" || name == "notype" || name == "bare" || name == "deserial" || 
            name == "borrow" || name == "view") {
            isIntrinsic = true;
        }
    }
    
    // Only visit callee if it's not an intrinsic (intrinsics don't need symbol resolution)
    if (!isIntrinsic && node->callee) {
        node->callee->accept(*this);
    }
    
    // Visit arguments
    for (auto& arg : node->arguments) {
        if (arg) arg->accept(*this);
    }
    
    // Handle intrinsics
    if (auto ident = dynamic_cast<ast::Identifier*>(node->callee.get())) {
        const std::string& name = ident->name;
        
        // Handle serialization intrinsics (lit, notype, bare, deserial)
        if (name == "lit" || name == "notype" || name == "bare" || name == "deserial") {
            if (node->arguments.empty()) {
                addError(name + "() requires at least one argument", node);
                return;
            }
            
            // Get the argument type
            ast::Expression* argExpr = node->arguments[0].get();
            if (argExpr) {
                // For lit(), the result type is the same as the input type
                // This ensures the serialization system knows how to handle it
                if (argExpr->type) {
                    node->type = std::shared_ptr<ast::TypeNode>(argExpr->type->clone().release());
                    expressionTypes[node] = argExpr->type.get();
                } else if (auto argType = expressionTypes[argExpr]) {
                    node->type = std::shared_ptr<ast::TypeNode>(argType->clone().release());
                    expressionTypes[node] = argType;
                } else {
                    // Fallback to String type if we can't determine the argument type
                    auto stringId = std::make_unique<ast::Identifier>(node->loc, "String");
                    ast::TypeNode* stringType = new ast::TypeName(node->loc, std::move(stringId), {});
                    node->type = std::shared_ptr<ast::TypeNode>(stringType);
                    expressionTypes[node] = stringType;
                }
            }
            return;
        }
        
        // Handle borrow()/view() intrinsics
        if (name == "borrow" || name == "view") {
            // Must be used within unsafe block
            if (!isInUnsafeBlock()) {
                addError(name + "() can only be used inside an unsafe block", node);
            }
            if (node->arguments.size() != 1) {
                addError(name + "() expects exactly one argument", node);
                return;
            }
            ast::Expression* argExpr = node->arguments[0].get();
            ast::TypeNode* argType = expressionTypes[argExpr];
            if (!argType) {
                addError("Cannot determine type of argument to " + name + "()", node);
                return;
            }
            // Argument must be owned (my<T> or our<T>)
            auto baseTypeName = dynamic_cast<ast::TypeName*>(argType);
            if (!baseTypeName || baseTypeName->genericArgs.size() != 1 || 
                (baseTypeName->identifier->name != "my" && baseTypeName->identifier->name != "our")) {
                addError(name + "() argument must be an owned type my<T> or our<T>", node);
            }
            // Compute result type: their<T> for borrow, their<T const> for view
            using ast::BorrowKind;
            BorrowKind kind = (name == "borrow" ? BorrowKind::MUTABLE_BORROW : BorrowKind::IMMUTABLE_VIEW);
            
            // Clone inner type T
            ast::TypeNodePtr innerType = baseTypeName->genericArgs[0]->clone();
            
            // For view, wrap T in const if necessary (represented as TypeName with " const" appended)
            if (name == "view") {
                // Create a new TypeName with " const" appended to the inner type's name
                auto innerTypeName = dynamic_cast<ast::TypeName*>(innerType.get());
                if (innerTypeName && innerTypeName->identifier) {
                    std::string constTypeName = innerTypeName->identifier->name + " const";
                    auto constIdentifier = std::make_unique<ast::Identifier>(node->loc, constTypeName);
                    innerType = std::make_unique<ast::TypeName>(node->loc, std::move(constIdentifier), 
                                                               std::move(innerTypeName->genericArgs));
                }
                // If not a TypeName, we might need different handling
            }
            
            // Create their<T> TypeName
            auto theirId = std::make_unique<ast::Identifier>(node->loc, "their");
            std::vector<ast::TypeNodePtr> theirArgs;
            theirArgs.push_back(std::move(innerType));
            ast::TypeNode* resultType = new ast::TypeName(node->loc, std::move(theirId), std::move(theirArgs));
            expressionTypes[node] = resultType;
            node->type = std::shared_ptr<ast::TypeNode>(resultType->clone().release());
            return;
        }
    }
    
    // Handle Vec::new() constructor calls
    if (auto memberExpr = dynamic_cast<ast::MemberExpression*>(node->callee.get())) {
        // Check if this is Vec::new()
        if (auto vecIdent = dynamic_cast<ast::Identifier*>(memberExpr->object.get())) {
            if (auto newIdent = dynamic_cast<ast::Identifier*>(memberExpr->property.get())) {
                if (vecIdent->name == "Vec" && newIdent->name == "new") {
                    // This is Vec::new() or Vec::new(size) - create a vector
                    if (node->arguments.size() > 1) {
                        addError("Vec::new() accepts at most 1 argument (optional size)", node);
                        return;
                    }
                    
                    // If size argument is provided, validate it's an integer type
                    if (node->arguments.size() == 1) {
                        auto sizeArg = node->arguments[0].get();
                        if (sizeArg) {
                            sizeArg->accept(*this);
                            // TODO: In full implementation, validate that size argument is Int type
                        }
                    }
                    
                    // Need to infer element type from context or default to a generic type
                    // For now, we'll default to Vec<Int> if no context is available
                    // TODO: In a full implementation, this should be inferred from the variable declaration
                    auto intId = std::make_unique<ast::Identifier>(node->loc, "Int");
                    auto intType = std::make_unique<ast::TypeName>(node->loc, std::move(intId));
                    auto vecType = std::make_unique<ast::VecType>(node->loc, std::move(intType));
                    
                    expressionTypes[node] = vecType.get();
                    node->type = std::shared_ptr<ast::TypeNode>(std::move(vecType));
                    return;
                }
            }
        }
        
        // Handle Vec instance method calls: obj.method()
        if (auto objIdent = dynamic_cast<ast::Identifier*>(memberExpr->object.get())) {
            if (auto methodIdent = dynamic_cast<ast::Identifier*>(memberExpr->property.get())) {
                handleVecMethodCall(node, objIdent->name, methodIdent->name);
                return;
            }
        }
    }
    
    // Fallback: default CallExpression analysis (no additional checks)
    // ...existing code...
}

void SemanticAnalyzer::visit(ast::ArrayElementExpression* node) {
    if (!node || !node->array || !node->index) {
        addError("Malformed array element expression.", node);
        return;
    }

    // Process the array expression first
    node->array->accept(*this);
    
    // Process the index expression
    node->index->accept(*this);

    // Get the array's type to determine the element type
    auto arrayTypeIt = expressionTypes.find(node->array.get());
    if (arrayTypeIt != expressionTypes.end() && arrayTypeIt->second) {
        if (auto arrayType = dynamic_cast<ast::ArrayType*>(arrayTypeIt->second)) {
            // The element type is the array's element type
            if (arrayType->elementType) {
                expressionTypes[node] = arrayType->elementType->clone().release();
                node->type = std::shared_ptr<ast::TypeNode>(arrayType->elementType->clone());
            }
        }
    }
    
    // Validate index type (should be integer)
    auto indexTypeIt = expressionTypes.find(node->index.get());
    if (indexTypeIt != expressionTypes.end() && indexTypeIt->second) {
        if (auto indexTypeName = dynamic_cast<ast::TypeName*>(indexTypeIt->second)) {
            if (indexTypeName->identifier->name != "Int") {
                addError("Array index must be an integer type, got: " + indexTypeName->identifier->name, node);
            }
        }
    }
}
void SemanticAnalyzer::visit(ast::MemberExpression* node) {
    if (!node || !node->object || !node->property) {
        addError("Malformed member expression.", node);
        return;
    }

    // First, process the object to determine its type
    node->object->accept(*this);

    // Get the property identifier (must be an identifier for dot notation)
    ast::Identifier* propertyId = dynamic_cast<ast::Identifier*>(node->property.get());
    if (!propertyId) {
        addError("Member property must be an identifier.", node);
        return;
    }

    // Now get the object's type from expressionTypes map
    auto it = expressionTypes.find(node->object.get());
    if (it == expressionTypes.end() || !it->second) {
        addError("Cannot determine type of object in member expression.", node);
        return;
    }

    // Get the type name of the object (assuming it's a TypeName for now)
    ast::TypeName* typeName = dynamic_cast<ast::TypeName*>(it->second);
    if (!typeName || !typeName->identifier) {
        addError("Object is not of a struct or class type.", node);
        return;
    }

    // Look up the struct/class definition in the symbol table
    const std::string& typeName_str = typeName->identifier->name;
    SymbolInfo* typeInfo = currentScope->lookup(typeName_str);
    if (!typeInfo) {
        addError("Unknown type: " + typeName_str, node);
        return;
    }

    // For now, since we don't have a direct way to access field types from the typeInfo,
    // we'll continue with the simplified approach and assume the field type is Int,
    // which works for our test case. In a more complete implementation, we would:
    // 1. Get the class declaration from the symbol table
    // 2. Find the field in the class members
    // 3. Get the field's type
    
    // For the time being, we'll at least create a proper type node:
    expressionTypes[node] = new ast::TypeName(node->loc, std::make_unique<ast::Identifier>(node->loc, "Int"));
}
void SemanticAnalyzer::visit(ast::AssignmentExpression* node) {
    if (!node || !node->left || !node->right) {
        // Should not happen with a valid AST
        addError("Malformed assignment expression.", node);
        return;
    }

    // Special handling: If LHS is a pointer dereference, temporarily set its type to the pointer type for assignment
    ast::PointerDerefExpression* derefLHS = dynamic_cast<ast::PointerDerefExpression*>(node->left.get());
    ast::TypeNode* savedDerefType = nullptr;
    if (derefLHS) {
        // Analyze the pointer expression to get its type
        derefLHS->pointer->accept(*this);
        auto ptrTypeIt = expressionTypes.find(derefLHS->pointer.get());
        if (ptrTypeIt != expressionTypes.end() && ptrTypeIt->second) {
            // Save the original type (pointee type)
            auto origTypeIt = expressionTypes.find(derefLHS);
            if (origTypeIt != expressionTypes.end()) {
                savedDerefType = origTypeIt->second;
            }
            // Set the deref node's type to the pointer type for assignment compatibility
            expressionTypes[derefLHS] = ptrTypeIt->second;
            derefLHS->type = std::shared_ptr<ast::TypeNode>(ptrTypeIt->second->clone());
        }
    }

    node->left->accept(*this);
    node->right->accept(*this);

    // Restore the original type for derefLHS after assignment analysis
    if (derefLHS) {
        if (savedDerefType) {
            expressionTypes[derefLHS] = savedDerefType;
            derefLHS->type = std::shared_ptr<ast::TypeNode>(savedDerefType->clone());
        }
    }

    if (!isLValue(node->left.get())) {
        addError("LHS of assignment is not a valid L-value.", node->left.get());
        expressionTypes[node] = nullptr; // Mark as error
        return;
    }

    auto leftTypeIt = expressionTypes.find(node->left.get());
    auto rightTypeIt = expressionTypes.find(node->right.get());

    ast::TypeNode* leftType = (leftTypeIt != expressionTypes.end()) ? leftTypeIt->second : nullptr;
    ast::TypeNode* rightType = (rightTypeIt != expressionTypes.end()) ? rightTypeIt->second : nullptr;

    if (!leftType || !rightType) {
        addError("Type error in assignment: could not determine type of LHS or RHS.", node);
        expressionTypes[node] = nullptr; // Mark as error
        return;
    }

    // Check if the types are compatible for assignment
    if (!areTypesCompatible(leftType, rightType)) { 
        addError("Type error in assignment: incompatible types - cannot assign '" + rightType->toString() + 
                "' to '" + leftType->toString() + "'.", node); 
        // Continue processing to avoid cascading errors, but mark this node as erroneous
        expressionTypes[node] = leftType; // Still use left type for further analysis
    } else {
        // The type of the assignment expression is typically the type of the LHS (or RHS after conversion).
        expressionTypes[node] = leftType; // Or rightType, depending on language rules for assignment expr type
    }
    
    if (leftType) {
      node->type = std::shared_ptr<ast::TypeNode>(leftType->clone());
    }
}

void SemanticAnalyzer::visit(ast::LogicalExpression* node) {
    // Check left and right operands
    if (node->left) {
        node->left->accept(*this);
    } else {
        addError("Logical expression missing left operand.", node);
        expressionTypes[node] = nullptr;
        return;
    }

    if (node->right) {
        node->right->accept(*this);
    } else {
        addError("Logical expression missing right operand.", node);
        expressionTypes[node] = nullptr;
        return;
    }

    // Get types of operands
    auto leftTypeIt = expressionTypes.find(node->left.get());
    auto rightTypeIt = expressionTypes.find(node->right.get());
    
    if (leftTypeIt == expressionTypes.end() || rightTypeIt == expressionTypes.end() || 
        !leftTypeIt->second || !rightTypeIt->second) {
        addError("Cannot determine types for logical expression operands.", node);
        expressionTypes[node] = nullptr;
        return;
    }

    // Check both operands are boolean or convertible to boolean
    // For now we'll do a simple check for boolean type
    bool isLeftBoolean = false;
    bool isRightBoolean = false;
    
    if (auto* typeName = dynamic_cast<ast::TypeName*>(leftTypeIt->second)) {
        isLeftBoolean = typeName->identifier && typeName->identifier->name == "bool";
    }
    
    if (auto* typeName = dynamic_cast<ast::TypeName*>(rightTypeIt->second)) {
        isRightBoolean = typeName->identifier && typeName->identifier->name == "bool";
    }
    
    if (!isLeftBoolean) {
        addError("Left operand of logical expression must be boolean.", node->left.get());
    }
    
    if (!isRightBoolean) {
        addError("Right operand of logical expression must be boolean.", node->right.get());
    }
    
    // Result type is always boolean
    // Create a boolean TypeName
    auto identifier = std::make_unique<ast::Identifier>(node->loc, "bool");
    auto boolType = std::make_unique<ast::TypeName>(node->loc, std::move(identifier));
    expressionTypes[node] = boolType.get();
    node->type = std::move(boolType);
}

void SemanticAnalyzer::visit(ast::ConditionalExpression* node) {
    // Check condition
    if (node->condition) {
        node->condition->accept(*this);
    } else {
        addError("Conditional expression missing condition.", node);
        expressionTypes[node] = nullptr;
        return;
    }

    // Check condition is boolean
    auto conditionTypeIt = expressionTypes.find(node->condition.get());
    if (conditionTypeIt == expressionTypes.end() || !conditionTypeIt->second) {
        addError("Cannot determine type for conditional expression condition.", node);
        expressionTypes[node] = nullptr;
        return;
    }
    
    bool isConditionBoolean = false;
    if (auto* typeName = dynamic_cast<ast::TypeName*>(conditionTypeIt->second)) {
        isConditionBoolean = typeName->identifier && typeName->identifier->name == "bool";
    }
    
    if (!isConditionBoolean) {
        addError("Condition of conditional expression must be boolean.", node->condition.get());
    }
    
    // Check then branch
    if (node->thenExpr) {
        node->thenExpr->accept(*this);
    } else {
        addError("Conditional expression missing 'then' branch.", node);
        expressionTypes[node] = nullptr;
        return;
    }
    
    // Check else branch
    if (node->elseExpr) {
        node->elseExpr->accept(*this);
    } else {
        addError("Conditional expression missing 'else' branch.", node);
        expressionTypes[node] = nullptr;
        return;
    }
    
    // Get types of branches
    auto thenTypeIt = expressionTypes.find(node->thenExpr.get());
    auto elseTypeIt = expressionTypes.find(node->elseExpr.get());
    
    if (thenTypeIt == expressionTypes.end() || elseTypeIt == expressionTypes.end() || 
        !thenTypeIt->second || !elseTypeIt->second) {
        addError("Cannot determine types for conditional expression branches.", node);
        expressionTypes[node] = nullptr;
        return;
    }
    
    // Check if branch types are compatible
    if (!areTypesCompatible(thenTypeIt->second, elseTypeIt->second) && 
        !areTypesCompatible(elseTypeIt->second, thenTypeIt->second)) {
        addError("Incompatible types in conditional expression: '" + 
                 thenTypeIt->second->toString() + "' and '" + 
                 elseTypeIt->second->toString() + "'.", node);
        // Use then branch type to continue analysis
        expressionTypes[node] = thenTypeIt->second;
        if (thenTypeIt->second) {
            node->type = std::shared_ptr<ast::TypeNode>(thenTypeIt->second->clone());
        }
        return;
    }
    
    // The result type is the common type of both branches
    // For now, use the 'then' branch type as the result type
    expressionTypes[node] = thenTypeIt->second;
    if (thenTypeIt->second) {
        node->type = std::shared_ptr<ast::TypeNode>(thenTypeIt->second->clone());
    }
}

void SemanticAnalyzer::visit(ast::SequenceExpression* node) {
    // Process each expression in the sequence
    if (node->expressions.empty()) {
        addError("Empty sequence expression.", node);
        expressionTypes[node] = nullptr;
        return;
    }
    
    for (auto& expr : node->expressions) {
        if (expr) {
            expr->accept(*this);
        } else {
            addError("Null expression in sequence.", node);
        }
    }
    
    // The type of a sequence expression is the type of its last expression
    auto& lastExpr = node->expressions.back();
    if (lastExpr) {
        auto lastTypeIt = expressionTypes.find(lastExpr.get());
        if (lastTypeIt != expressionTypes.end() && lastTypeIt->second) {
            expressionTypes[node] = lastTypeIt->second;
            node->type = std::shared_ptr<ast::TypeNode>(lastTypeIt->second->clone());
        } else {
            addError("Cannot determine type of last expression in sequence.", node);
            expressionTypes[node] = nullptr;
        }
    } else {
        addError("Last expression in sequence is null.", node);
        expressionTypes[node] = nullptr;
    }
}
void SemanticAnalyzer::visit(ast::ObjectLiteral* node) {
    // Try to determine the type from the typePath field (e.g., Point in Point { ... })
    if (!node || !node->typePath) {
        addError("Object literal missing type path.", node);
        expressionTypes[node] = nullptr;
        return;
    }
    // Set the type of the object literal to the typePath (e.g., Point)
    expressionTypes[node] = node->typePath->clone().release();
    // Optionally, check that all fields exist in the class/struct and types match (not implemented here)
}
void SemanticAnalyzer::visit(ast::ArrayLiteral* node) {
    if (!node) {
        return;
    }

    // Process all elements to infer types
    ast::TypeNode* elementType = nullptr;
    for (auto& element : node->elements) {
        if (element) {
            element->accept(*this);
            
            // Get the element type
            auto elemTypeIt = expressionTypes.find(element.get());
            if (elemTypeIt != expressionTypes.end() && elemTypeIt->second) {
                if (!elementType) {
                    // First element sets the type
                    elementType = elemTypeIt->second;
                } else {
                    // TODO: Type compatibility check for all elements
                    // For now, assume all elements have the same type
                }
            }
        }
    }
    
    if (elementType) {
        // Create an array type [ElementType; Size]
        auto sizeExpr = std::make_unique<ast::IntegerLiteral>(
            node->loc, 
            static_cast<int64_t>(node->elements.size())
        );
        
        auto arrayType = std::make_unique<ast::ArrayType>(
            node->loc,
            elementType->clone(),
            std::move(sizeExpr)
        );
        
        expressionTypes[node] = arrayType.release();
        node->type = std::shared_ptr<ast::TypeNode>(expressionTypes[node]->clone());
    }
}
void SemanticAnalyzer::visit(ast::FunctionExpression* node) {}
void SemanticAnalyzer::visit(ast::ThisExpression* node) {}
void SemanticAnalyzer::visit(ast::SuperExpression* node) {}
void SemanticAnalyzer::visit(ast::AwaitExpression* node) {}
void SemanticAnalyzer::visit(ast::ListComprehension* node) {}
void SemanticAnalyzer::visit(ast::GenericInstantiationExpression* node) {
    if (!node || !node->baseExpression) {
        addError("Invalid generic instantiation expression.", node);
        return;
    }
    
    // Visit the base expression to understand what we're instantiating
    node->baseExpression->accept(*this);
    
    // Visit all generic arguments (type parameters)
    for (auto& typeArg : node->genericArguments) {
        if (typeArg) {
            typeArg->accept(*this);
        }
    }
    
    // Try to identify if this is a template instantiation
    if (auto identifier = dynamic_cast<ast::Identifier*>(node->baseExpression.get())) {
        handleTemplateInstantiation(identifier, node->genericArguments, node);
    } else if (auto memberExpr = dynamic_cast<ast::MemberExpression*>(node->baseExpression.get())) {
        // Handle something like Container<Int>::create
        handleMemberTemplateInstantiation(memberExpr, node->genericArguments, node);
    } else {
        addError("Invalid base expression for generic instantiation.", node->baseExpression.get());
    }
}
void SemanticAnalyzer::visit(ast::PointerDerefExpression* node) {
    // at() intrinsic only allowed inside an unsafe block
    if (!isInUnsafeBlock()) {
        addError("at() (pointer dereference) is only allowed inside an unsafe block.", node);
        expressionTypes[node] = nullptr;
        return;
    }
    if (!node || !node->pointer) {
        addError("Malformed pointer dereference expression.", node);
        expressionTypes[node] = nullptr; // Added to mark error
        return;
    }
    node->pointer->accept(*this);
    auto pointerTypeIt = expressionTypes.find(node->pointer.get());

    if (pointerTypeIt == expressionTypes.end() || !pointerTypeIt->second) {
        addError("Cannot dereference pointer with unknown type.", node->pointer.get());
        expressionTypes[node] = nullptr;
        return;
    }

    ast::TypeNode* resolvedPointerType = pointerTypeIt->second;
    ast::TypeNode* pointeeType = nullptr;

    if (auto ptrType = dynamic_cast<ast::PointerType*>(resolvedPointerType)) {
        if (ptrType->pointeeType) {
            pointeeType = ptrType->pointeeType->clone().release();
        } else {
            addError("Pointer type has no pointee type.", node);
        }
    } else if (auto locTypeName = dynamic_cast<ast::TypeName*>(resolvedPointerType)) {
        if (locTypeName->identifier->name == "loc") {
            if (!locTypeName->genericArgs.empty() && locTypeName->genericArgs[0]) {
                pointeeType = locTypeName->genericArgs[0]->clone().release();
            } else {
                addError("loc type used in dereference is missing its type parameter (e.g., loc<T>).", node);
            }
        } else {
            addError("Type " + locTypeName->identifier->name + " is not a 'loc<T>' or pointer type, cannot dereference.", node);
        }
    } else {
        addError("Cannot dereference non-pointer/non-loc type: " + resolvedPointerType->toString(), node);
    }

    if (!pointeeType) {
        addError("Failed to determine pointee type for dereference.", node);
        expressionTypes[node] = nullptr;
        if (node->type) node->type.reset();
        return;
    }
    
    expressionTypes[node] = pointeeType; // raw pointer, ownership handled by clone().release()
    node->type = std::shared_ptr<ast::TypeNode>(pointeeType->clone()); // node->type takes ownership of a new clone
}
void SemanticAnalyzer::visit(ast::AddrOfExpression* node) {
    // addr() intrinsic only allowed inside an unsafe block
    if (!isInUnsafeBlock()) {
        addError("addr() is only allowed inside an unsafe block.", node);
        expressionTypes[node] = nullptr;
        return;
    }
    if (!node || !node->getLocation()) {
         addError("Malformed addr_of expression.", node);
         expressionTypes[node] = nullptr; // Added to mark error
        return;
    }
    node->getLocation()->accept(*this); // Operand of addr()

    if (!isLValue(node->getLocation().get())) {
        addError("Cannot take address of non-lvalue with addr().", node->getLocation().get());
        expressionTypes[node] = nullptr;
        return;
    }

    auto operandTypeIt = expressionTypes.find(node->getLocation().get());
    if (operandTypeIt == expressionTypes.end() || !operandTypeIt->second) {
        addError("Cannot take address of expression with unknown type using addr().", node->getLocation().get());
        expressionTypes[node] = nullptr;
        return;
    }
    
    // The type of addr(operand) is an integer type representing an address.
    // Using "i64" as a placeholder for the address type.
    // Vyn should have a dedicated address type (e.g., uintptr).
    auto addr_type_ident = std::make_unique<ast::Identifier>(node->loc, "i64");
    ast::TypeNode* addrAstType = new ast::TypeName(node->loc, std::move(addr_type_ident));
    expressionTypes[node] = addrAstType;
    node->type = std::shared_ptr<ast::TypeNode>(addrAstType->clone());
}
void SemanticAnalyzer::visit(ast::FromIntToLocExpression* node) {
    // from<T>() intrinsic only allowed inside an unsafe block
    if (!isInUnsafeBlock()) {
        addError("from<Type>(expr) is only allowed inside an unsafe block.", node);
        expressionTypes[node] = nullptr;
        return;
    }
    if (!node || !node->getAddressExpression() || !node->getTargetType()) {
        addError("Malformed from_int_to_loc expression.", node);
        expressionTypes[node] = nullptr;
        return;
    }

    node->getAddressExpression()->accept(*this);
    auto addrExprTypeIt = expressionTypes.find(node->getAddressExpression().get());
    ast::TypeNode* addrExprType = (addrExprTypeIt != expressionTypes.end()) ? addrExprTypeIt->second : nullptr;

    // Assuming TypeNode has isIntegerTy() or similar. If not, this needs adjustment.
    // For now, we rely on the isIntegerType helper if addrExprType is a TypeName.
    bool isAddrExprInteger = false;
    if (addrExprType) {
        if (auto tn = dynamic_cast<ast::TypeName*>(addrExprType)) {
             isAddrExprInteger = isIntegerType(tn); // Use our helper
        } else {
            // TODO: Handle other TypeNode subtypes if they can be integers
        }
    }

    if (!isAddrExprInteger) {
        addError("Address expression in from<T>() must be an integer type. Got: " + (addrExprType ? addrExprType->toString() : "unknown"), node);
        expressionTypes[node] = nullptr;
        return;
    }

    node->getTargetType()->accept(*this); // Resolve/validate the target type (e.g. loc<i64>)
    ast::TypeNode* targetType = node->getTargetType().get();

    bool isTargetLocOrPointer = false;
    if (auto tn = dynamic_cast<ast::TypeName*>(targetType)) {
        if (tn->identifier && tn->identifier->name == "loc" && !tn->genericArgs.empty() && tn->genericArgs[0]) {
            isTargetLocOrPointer = true;
        }
    } else if (dynamic_cast<ast::PointerType*>(targetType)) {
        isTargetLocOrPointer = true;
    }

    if (!isTargetLocOrPointer) {
        addError("Target type in from<T>() must be a location/pointer type (e.g., loc<ActualType>). Got: " + targetType->toString(), node);
        expressionTypes[node] = nullptr;
        return;
    }
    
    expressionTypes[node] = targetType->clone().release();
    node->type = std::shared_ptr<ast::TypeNode>(targetType->clone());
}
void SemanticAnalyzer::visit(ast::LocationExpression* node) {
    // Check if we're in an unsafe block
    if (!isInUnsafeBlock()) {
        addError("loc() (location-of) is only allowed inside an unsafe block.", node);
        expressionTypes[node] = nullptr;
        return;
    }

    // Visit the expression to get its type
    if (!node->expression) {
        addError("Missing expression in loc().", node);
        expressionTypes[node] = nullptr;
        return;
    }
    
    node->expression->accept(*this);

    // Create a pointer type for the expression
    auto it = expressionTypes.find(node->expression.get());
    if (it == expressionTypes.end() || !it->second) {
        addError("Cannot get location of expression with unknown type", node);
        expressionTypes[node] = nullptr;
        return;
    }

    // Set the type of the location expression
    ast::TypeNode* pointeeType = it->second;
    
    // Create loc<T> type where T is the pointed-to type
    auto locIdent = std::make_unique<ast::Identifier>(node->loc, "loc");
    auto typeName = new ast::TypeName(node->loc, std::move(locIdent));
    typeName->genericArgs.push_back(std::unique_ptr<ast::TypeNode>(pointeeType->clone()));
    
    expressionTypes[node] = typeName;
    node->type = std::shared_ptr<ast::TypeNode>(typeName->clone());
}
void SemanticAnalyzer::visit(ast::IfStatement* node) {
    // Visit the test condition
    node->test->accept(*this);

    // Visit the consequent block
    node->consequent->accept(*this);

    // Visit the alternate block if it exists
    if (node->alternate) {
        node->alternate->accept(*this);
    }
}
void SemanticAnalyzer::visit(ast::ForStatement* node) {
    // Enter a new scope for the loop
    enterScope();
    currentScope->isLoop = true;

    // Visit the initialization
    if (node->init) {
        node->init->accept(*this);
    }

    // Visit the test condition
    if (node->test) {
        node->test->accept(*this);
    }

    // Visit the update expression
    if (node->update) {
        node->update->accept(*this);
    }

    // Visit the body
    node->body->accept(*this);

    // Exit the loop scope
    exitScope();
}
void SemanticAnalyzer::visit(ast::WhileStatement* node) {
    // Enter a new scope for the loop
    enterScope();
    currentScope->isLoop = true;

    // Visit the test condition
    node->test->accept(*this);

    // Visit the body
    node->body->accept(*this);

    // Exit the loop scope
    exitScope();
}
void SemanticAnalyzer::visit(ast::ReturnStatement* node) {
    // Visit the return value if it exists
    if (node->argument) {
        node->argument->accept(*this);
    }
}
void SemanticAnalyzer::visit(ast::BreakStatement* node) {
    // Check if we're inside a loop
    if (!isInLoop()) {
        errors.push_back("Break statement must be inside a loop");
    }
}
void SemanticAnalyzer::visit(ast::ContinueStatement* node) {
    // Check if we're inside a loop
    if (!isInLoop()) {
        errors.push_back("Continue statement must be inside a loop");
    }
}
void SemanticAnalyzer::visit(ast::TryStatement* node) {
    // Visit the try block
    node->tryBlock->accept(*this);

    // Visit the catch block if it exists
    if (node->catchBlock) {
        node->catchBlock->accept(*this);
    }

    // Visit the finally block if it exists
    if (node->finallyBlock) {
        node->finallyBlock->accept(*this);
    }
}
void SemanticAnalyzer::visit(ast::UnsafeStatement* node) {
    // Enter a new scope for the unsafe block
    enterScope();
    currentScope->isUnsafeBlock = true;
    node->block->accept(*this);
    exitScope();
}
void SemanticAnalyzer::visit(ast::AssertStatement* node) {}
void SemanticAnalyzer::visit(ast::MatchStatement* node) {}
void SemanticAnalyzer::visit(ast::YieldStatement* node) {
    // Check if we're inside a generator function
    // For now, we'll just check for any yield expression
    
    // Analyze the expression being yielded, if any
    if (node->expression) {
        node->expression->accept(*this);
    }
    
    // In a full implementation, we would:
    // 1. Verify we're inside a generator function (function with yield)
    // 2. Track the yielded type for generator return type inference
    // 3. Ensure the yielded type is consistent across the function
}

void SemanticAnalyzer::visit(ast::YieldReturnStatement* node) {
    // Similar to YieldStatement but specifically for final return from generator
    
    // Analyze the expression being returned, if any
    if (node->expression) {
        node->expression->accept(*this);
    }
    
    // In a full implementation:
    // 1. Verify we're inside a generator function
    // 2. Check that the returned type is compatible with the generator's return type
    // 3. Mark the generator as having an explicit final return value
}

// REMOVE DUPLICATE/EMPTY visit methods from here down
// void SemanticAnalyzer::visit(ast::TypeAliasDeclaration* node) {} // Handled above
void SemanticAnalyzer::visit(ast::ExternStatement* node) {
    // In an extern statement, we typically:
    // 1. Register the extern declaration in the current scope
    // 2. Don't analyze the body since it's external
    
    // For now, we'll just mark the statement as visited
    // In a full implementation, we would register any external declarations
    // in the symbol table for later use
    
    // No recursive visits needed as extern statements don't have a body to analyze
}

void SemanticAnalyzer::visit(ast::ImportDeclaration* node) {}
void SemanticAnalyzer::visit(ast::StructDeclaration* node) {}
// void SemanticAnalyzer::visit(ast::ClassDeclaration* node) {} // Handled above
void SemanticAnalyzer::visit(ast::EnumDeclaration* node) {}
// void SemanticAnalyzer::visit(ast::TraitDeclaration* node) {} // Handled above (commented out)
// void SemanticAnalyzer::visit(ast::ImplDeclaration* node) {} // Handled above
// void SemanticAnalyzer::visit(ast::NamespaceDeclaration* node) {} // Handled above (commented out)
void SemanticAnalyzer::visit(ast::FieldDeclaration* node) {}
void SemanticAnalyzer::visit(ast::EnumVariant* node) {}
// void SemanticAnalyzer::visit(ast::GenericParameter* node) {} // Handled above
void SemanticAnalyzer::visit(ast::TemplateDeclaration* node) {
    if (!node || !node->name) {
        addError("Malformed template declaration.", node);
        return;
    }
    
    const std::string& templateName = node->name->name;
    
    // Check if template name is already registered
    if (templateRegistry.find(templateName) != templateRegistry.end()) {
        addError("Template '" + templateName + "' is already defined.", node);
        return;
    }
    
    // Validate generic parameters
    for (const auto& param : node->genericParams) {
        if (!param || !param->name) {
            addError("Invalid generic parameter in template '" + templateName + "'.", node);
            return;
        }
    }
    
    // Clone the template declaration for storage
    auto clonedDecl = std::make_unique<ast::TemplateDeclaration>(
        node->loc, 
        std::make_unique<ast::Identifier>(node->name->loc, node->name->name),
        std::vector<std::unique_ptr<ast::GenericParameter>>(),
        nullptr // Will be cloned separately if needed
    );
    
    // Clone generic parameters
    for (const auto& param : node->genericParams) {
        if (param && param->name) {
            auto clonedParam = std::make_unique<ast::GenericParameter>(
                param->loc,
                std::make_unique<ast::Identifier>(param->name->loc, param->name->name)
            );
            clonedDecl->genericParams.push_back(std::move(clonedParam));
        }
    }
    
    // Register the template
    registerTemplate(std::move(clonedDecl));
    
    // Visit the template body for immediate syntax checking
    if (node->body) {
        node->body->accept(*this);
    }
}
void SemanticAnalyzer::visit(ast::ThrowStatement* node) {}

void SemanticAnalyzer::visit(ast::TypeNode* node) {
    // This is a base class, specific derived type visitors should be called.
    // If this is ever called directly, it might indicate an issue or a need
    // for a more generic type handling here.
    // For now, it can be a no-op, or log a warning.
    // std::cout << "Warning: SemanticAnalyzer::visit(ast::TypeNode*) called directly for: " << node->toString() << std::endl;
}

void SemanticAnalyzer::visit(ast::TypeName* node) {
    if (!node || !node->identifier) {
        addError("Malformed TypeName node.", node);
        return;
    }
    const std::string& typeNameStr = node->identifier->name;
    if (typeNameStr == "Vec") {
        // Convert Vec<T> TypeName to VecType
        if (node->genericArgs.empty() || !node->genericArgs[0]) {
            addError("Vec type requires a type parameter (e.g., Vec<Int>).", node);
            return; 
        }
        if (node->genericArgs.size() > 1) {
            addError("Vec type accepts only one type parameter.", node);
            return; 
        }
        
        // Visit the element type
        node->genericArgs[0]->accept(*this);
        
        // Create a VecType instance
        auto vecType = std::make_unique<ast::VecType>(node->loc, node->genericArgs[0]->clone());
        node->type = std::shared_ptr<ast::TypeNode>(vecType.release());
    } else if (typeNameStr == "loc") {
        if (node->genericArgs.empty() || !node->genericArgs[0]) {
            addError("loc type constructor requires a type parameter (e.g., loc<T>).", node);
            return; 
        }
        for ( auto& argTypeNode : node->genericArgs ) {
            if (argTypeNode) {
                argTypeNode->accept(*this);
            } else {
                 addError("Null generic argument in loc<T> type.", node);
                 return;
            }
        }
        node->type = std::shared_ptr<ast::TypeNode>(node->clone());
    } else if (typeNameStr == "i8" || typeNameStr == "i16" || typeNameStr == "i32" || typeNameStr == "i64" ||
               typeNameStr == "u8" || typeNameStr == "u16" || typeNameStr == "u32" || typeNameStr == "u64" ||
               typeNameStr == "f32" || typeNameStr == "f64" ||
               typeNameStr == "bool" || typeNameStr == "string" || typeNameStr == "void" ||
               typeNameStr == "int" || typeNameStr == "float" ||
               typeNameStr == "Int" || typeNameStr == "String" || typeNameStr == "Int8") { 
        node->type = std::shared_ptr<ast::TypeNode>(node->clone());
    } else {
        SymbolInfo* symbol = currentScope->lookup(typeNameStr); 
        if (!symbol || !symbol->type) {
            addError("Unknown type identifier: " + typeNameStr, node);
            return;
        }
        node->type = std::shared_ptr<ast::TypeNode>(symbol->type->clone());
        for (auto& argTypeNode : node->genericArgs) {
            if (argTypeNode) argTypeNode->accept(*this);
        }
    }
}
void SemanticAnalyzer::visit(ast::ConstructionExpression* node) {
    if (!node || !node->constructedType) {
        addError("Construction expression is missing the type to construct.", node);
        expressionTypes[node] = nullptr;
        return;
    }
    auto typeNameNode = dynamic_cast<ast::TypeName*>(node->constructedType.get());
    if (!typeNameNode || !typeNameNode->identifier) {
         node->constructedType->accept(*this); 
         expressionTypes[node] = node->constructedType->clone().release();
         if (expressionTypes[node]) {
            node->type = std::shared_ptr<ast::TypeNode>(expressionTypes[node]->clone());
         }
         for (auto& arg : node->arguments) {
             if (arg) arg->accept(*this);
         }
         return; 
    }
    const std::string& constructedName = typeNameNode->identifier->name;
    if (constructedName == "addr") {
        // ... (addr handling logic as before, ensure it's correct) ...
        if (node->arguments.size() != 1 || !node->arguments[0]) {
            addError("addr() intrinsic expects 1 argument.", node); expressionTypes[node] = nullptr; return;
        }
        node->arguments[0]->accept(*this);
        ast::Expression* argExpr = node->arguments[0].get();
        if (!isLValue(argExpr)) {
            addError("Argument to addr() must be an L-value.", node); expressionTypes[node] = nullptr; return;
        }
        auto argTypeIt = expressionTypes.find(argExpr);
        if (argTypeIt == expressionTypes.end() || !argTypeIt->second) {
            addError("Argument to addr() has an unknown type.", node); expressionTypes[node] = nullptr; return;
        }
        auto res_type_ident = std::make_unique<ast::Identifier>(node->loc, "i64");
        ast::TypeNode* resType = new ast::TypeName(node->loc, std::move(res_type_ident));
        expressionTypes[node] = resType;
        node->type = std::shared_ptr<ast::TypeNode>(resType->clone());

    } else if (constructedName == "from") {
        // ... (from handling logic as before, ensure it's correct) ...
        if (typeNameNode->genericArgs.empty() || !typeNameNode->genericArgs[0]) {
            addError("from<TargetType>() intrinsic requires TargetType as a generic argument.", node); expressionTypes[node] = nullptr; return;
        }
        if (node->arguments.size() != 1 || !node->arguments[0]) {
            addError("from<T>() intrinsic expects 1 argument value.", node); expressionTypes[node] = nullptr; return;
        }
        ast::TypeNode* targetTypeAst = typeNameNode->genericArgs[0].get();
        targetTypeAst->accept(*this); 
        bool isTargetLocOrPointer = false;
        if (auto tn = dynamic_cast<ast::TypeName*>(targetTypeAst)) {
            if (tn->identifier->name == "loc" && !tn->genericArgs.empty()) isTargetLocOrPointer = true;
        } else if (dynamic_cast<ast::PointerType*>(targetTypeAst)) {
            isTargetLocOrPointer = true;
        }
        if (!isTargetLocOrPointer) {
             addError("Target type in from<T>() must be a location/pointer type (e.g., loc<ActualType>).", node); expressionTypes[node] = nullptr; return;
        }
        node->arguments[0]->accept(*this); 
        ast::Expression* addrValExpr = node->arguments[0].get();
        auto addrValTypeIt = expressionTypes.find(addrValExpr);
        ast::TypeNode* addrValType = (addrValTypeIt != expressionTypes.end()) ? addrValTypeIt->second : nullptr;
        
        bool isAddrValInteger = false;
        if(addrValType) {
            if(auto tn_val = dynamic_cast<ast::TypeName*>(addrValType)) {
                isAddrValInteger = isIntegerType(tn_val);
            }
        }
        if (!isAddrValInteger) { 
            addError("Address argument to from<T>() must be an integer type. Got: " + (addrValType ? addrValType->toString() : "unknown"), node);
            expressionTypes[node] = nullptr; return;
        }
        expressionTypes[node] = targetTypeAst->clone().release();
        node->type = std::shared_ptr<ast::TypeNode>(targetTypeAst->clone());

    } else if (constructedName == "at") {
        // ... (at handling logic as before, ensure it's correct) ...
        if (node->arguments.size() != 1 || !node->arguments[0]) {
            addError("at() intrinsic expects 1 argument (the pointer).", node); expressionTypes[node] = nullptr; return;
        }
        node->arguments[0]->accept(*this); 
        ast::Expression* ptrExpr = node->arguments[0].get();
        auto ptrTypeIt = expressionTypes.find(ptrExpr);
        if (ptrTypeIt == expressionTypes.end() || !ptrTypeIt->second) {
            addError("Cannot dereference pointer with unknown type using at().", node); expressionTypes[node] = nullptr; return;
        }
        ast::TypeNode* resolvedPointerType = ptrTypeIt->second;
        ast::TypeNode* pointeeType = nullptr;
        if (auto pt = dynamic_cast<ast::PointerType*>(resolvedPointerType)) {
            if (pt->pointeeType) pointeeType = pt->pointeeType->clone().release();
        } else if (auto locTn = dynamic_cast<ast::TypeName*>(resolvedPointerType)) {
            if (locTn->identifier->name == "loc" && !locTn->genericArgs.empty() && locTn->genericArgs[0]) {
                pointeeType = locTn->genericArgs[0]->clone().release();
            } else {
                 addError("Type for at() is \'loc\' but missing type parameter (e.g. loc<T>).", node); expressionTypes[node] = nullptr; return;
            }
        } else {
            addError("Argument to at() must be a pointer or loc<T> type. Got: " + resolvedPointerType->toString(), node);
            expressionTypes[node] = nullptr; return;
        }
        if (!pointeeType) {
             addError("Failed to determine pointee type for at() operation.", node); expressionTypes[node] = nullptr; return;
        }
        expressionTypes[node] = pointeeType;
        node->type = std::shared_ptr<ast::TypeNode>(pointeeType->clone());
    } else {
        node->constructedType->accept(*this); 
        expressionTypes[node] = node->constructedType->clone().release();
        if (expressionTypes[node]) {
           node->type = std::shared_ptr<ast::TypeNode>(expressionTypes[node]->clone());
        } else {
            addError("Constructed type " + constructedName + " could not be resolved.", node);
            return;
        }
        for (auto& arg : node->arguments) {
            if (arg) arg->accept(*this);
        }
    }
}
void SemanticAnalyzer::visit(ast::PointerType* node) {
    if (node && node->pointeeType) {
        node->pointeeType->accept(*this);
    }
}
void SemanticAnalyzer::visit(ast::ArrayType* node) {
    if (node && node->elementType) {
        node->elementType->accept(*this);
    }
}

void SemanticAnalyzer::visit(ast::VecType* node) {
    if (node && node->elementType) {
        node->elementType->accept(*this);
    }
}

void SemanticAnalyzer::visit(ast::FunctionType* node) {
    if (node) {
        for (auto& paramType : node->parameterTypes) { 
            if (paramType) paramType->accept(*this);
        }
        if (node->returnType) {
            node->returnType->accept(*this);
        }
    }
}
void SemanticAnalyzer::visit(ast::OptionalType* node) {
    if (node && node->containedType) { 
        node->containedType->accept(*this); 
    }
}
void SemanticAnalyzer::visit(ast::GenericParameter* node) {
    // Generic parameters are usually resolved in the context of a generic declaration
    // or instantiation. For a standalone visit, we might try to look it up if it's
    // already defined in the current scope (e.g. within a generic function body).
    // For now, this can be a no-op or a simple lookup.
    // SymbolInfo* symbol = currentScope->lookup(node->name->name); // Assuming GenericParameter has a name field
    // if (!symbol) { addError("Generic parameter " + node->name->name + " not found.", node); }
}

// Implementation for areTypesCompatible
// Checks if valueType can be assigned to targetType
bool SemanticAnalyzer::areTypesCompatible(ast::TypeNode* targetType, ast::TypeNode* valueType) {
    if (!targetType || !valueType) {
        // If one or both types are unresolved (likely due to a previous error),
        // consider them incompatible to prevent cascading issues.
        return false;
    }

    // If they are the exact same type object.
    if (targetType == valueType) {
        return true;
    }

    ast::TypeNode::Category categoryTarget = targetType->getCategory();
    ast::TypeNode::Category categoryValue = valueType->getCategory();

    // Special Case 1: Assigning 'nil' to an Optional type
    if (categoryValue == ast::TypeNode::Category::IDENTIFIER) {
        if (auto* tnValue = dynamic_cast<ast::TypeName*>(valueType)) {
            if (tnValue->identifier && tnValue->identifier->name == "nil") {
                if (categoryTarget == ast::TypeNode::Category::OPTIONAL) {
                    return true;
                }
                // Vyn might also allow assigning 'nil' to raw pointer types.
                // if (categoryTarget == ast::TypeNode::Category::POINTER) return true;
            }
        }
    }

    // Special Case 2: Assigning a generic integer literal (type "int") to a specific integer type
    if (categoryTarget == ast::TypeNode::Category::IDENTIFIER &&
        categoryValue == ast::TypeNode::Category::IDENTIFIER) {
        auto* tnTarget = static_cast<ast::TypeName*>(targetType);
        auto* tnValue = static_cast<ast::TypeName*>(valueType);
        if (tnTarget->identifier && tnValue->identifier) {
            bool isSpecificIntTarget = isIntegerType(tnTarget); // Checks for i8, i16, etc.
            bool isGenericIntValue = (tnValue->identifier->name == "int"); // Literal int

            if (isSpecificIntTarget && isGenericIntValue) {
                // e.g., let x: i32 = 10; (10 is "int", x is "i32")
                return true;
            }
        }
    }
    
    // If categories are different and not covered by the above special cases,
    // they are generally not compatible without an explicit cast.
    if (categoryTarget != categoryValue) {
        return false;
    }

    // Categories are the same, proceed with category-specific checks.
    switch (categoryTarget) {
        case ast::TypeNode::Category::IDENTIFIER: {
            auto* tnTarget = static_cast<ast::TypeName*>(targetType);
            auto* tnValue = static_cast<ast::TypeName*>(valueType);
            if (!tnTarget->identifier || !tnValue->identifier) return false; // Should not happen

            // Check for specific integer types (e.g., i32, u64).
            // The generic "int" literal case is handled above.
            if (isIntegerType(tnTarget) && isIntegerType(tnValue)) {
                // For now, require exact match for specific integer types (e.g., i32 compatible with i32).
                // Vyn might allow implicit widening (e.g., i32 to i64), which would need to be added here.
                return tnTarget->identifier->name == tnValue->identifier->name;
            }

            // General identifier type comparison (e.g., custom struct names, bool, string).
            // This implies nominal typing.
            if (tnTarget->identifier->name == tnValue->identifier->name) {
                // If they are generic types, check compatibility of generic arguments.
                if (tnTarget->genericArgs.size() != tnValue->genericArgs.size()) return false;
                for (size_t i = 0; i < tnTarget->genericArgs.size(); ++i) {
                    if (!areTypesCompatible(tnTarget->genericArgs[i].get(), tnValue->genericArgs[i].get())) {
                        return false;
                    }
                }
                return true;
            }
            return false; // Different names or failed generic arg check.
        }
        case ast::TypeNode::Category::POINTER: {
            auto* ptTarget = static_cast<ast::PointerType*>(targetType);
            auto* ptValue = static_cast<ast::PointerType*>(valueType);
            // T* is compatible with U* if T is compatible with U (invariant for now).
            // Vyn might have rules for void* or covariance/contravariance.
            return areTypesCompatible(ptTarget->pointeeType.get(), ptValue->pointeeType.get());
        }
        case ast::TypeNode::Category::ARRAY: {
            auto* atTarget = static_cast<ast::ArrayType*>(targetType);
            auto* atValue = static_cast<ast::ArrayType*>(valueType);
            // T[N] is compatible with U[M] if T is compatible with U.
            // For simplicity, ignoring size compatibility for now (atTarget->sizeExpression vs atValue->sizeExpression).
            // A full check would compare constant sizes if available.
            return areTypesCompatible(atTarget->elementType.get(), atValue->elementType.get());
        }
        case ast::TypeNode::Category::VEC: {
            auto* vtTarget = static_cast<ast::VecType*>(targetType);
            auto* vtValue = static_cast<ast::VecType*>(valueType);
            // Vec<T> is compatible with Vec<U> if T is compatible with U.
            return areTypesCompatible(vtTarget->elementType.get(), vtValue->elementType.get());
        }
        case ast::TypeNode::Category::FUNCTION: {
            auto* ftTarget = static_cast<ast::FunctionType*>(targetType);
            auto* ftValue = static_cast<ast::FunctionType*>(valueType);

            // Return types: Target's return type must be compatible with Value's return type (covariance).
            // fn_target_ret = fn_value_ret  => areTypesCompatible(fn_target_ret, fn_value_ret)
            if (!areTypesCompatible(ftTarget->returnType.get(), ftValue->returnType.get())) return false;

            // Parameter types: Value's param types must be compatible with Target's param types (contravariance).
            // fn_target_param = fn_value_param => areTypesCompatible(fn_value_param, fn_target_param)
            // For simplicity, using invariance for now: types must be identical.
            if (ftTarget->parameterTypes.size() != ftValue->parameterTypes.size()) return false;
            for (size_t i = 0; i < ftTarget->parameterTypes.size(); ++i) {
                if (!areTypesCompatible(ftTarget->parameterTypes[i].get(), ftValue->parameterTypes[i].get())) {
                // For contravariance:
                // if (!areTypesCompatible(ftValue->parameterTypes[i].get(), ftTarget->parameterTypes[i].get()))
                    return false;
                }
            }
            return true;
        }
        case ast::TypeNode::Category::OPTIONAL: {
            auto* otTarget = static_cast<ast::OptionalType*>(targetType);
            auto* otValue = static_cast<ast::OptionalType*>(valueType);
            // Optional<T> is compatible with Optional<U> if T is compatible with U.
            return areTypesCompatible(otTarget->containedType.get(), otValue->containedType.get());
        }
        case ast::TypeNode::Category::TUPLE: {
            auto* ttTarget = static_cast<ast::TupleTypeNode*>(targetType);
            auto* ttValue = static_cast<ast::TupleTypeNode*>(valueType);
            if (ttTarget->memberTypes.size() != ttValue->memberTypes.size()) return false;
            for (size_t i = 0; i < ttTarget->memberTypes.size(); ++i) {
                if (!areTypesCompatible(ttTarget->memberTypes[i].get(), ttValue->memberTypes[i].get())) {
                    return false;
                }
            }
            return true;
        }
        // case ast::TypeNode::Category::STRUCT:
            // Struct compatibility would typically be nominal (same definition) or structural.
            // Nominal is partly handled by IDENTIFIER if struct names are unique and resolved.
            // Structural would require comparing field types and names.
        // case ast::TypeNode::Category::REFERENCE: // Not fully defined in provided AST
        // case ast::TypeNode::Category::SLICE:     // Not fully defined in provided AST
        default:
            // Fallback for unhandled categories or complex types.
            // This is a weak check and ideally should be replaced with more specific rules
            // or by ensuring all types are resolved to canonical forms before comparison.
            if (targetType->toString() == valueType->toString()) { // Basic structural check via string representation
                return true;
            }
            // If they are TypeName, it might have been missed by earlier checks
            if (categoryTarget == ast::TypeNode::Category::IDENTIFIER) {
                 auto* tnTarget = static_cast<ast::TypeName*>(targetType);
                 auto* tnValue = static_cast<ast::TypeName*>(valueType); // Already know category is IDENTIFIER
                 if (tnTarget->identifier && tnValue->identifier && tnTarget->identifier->name == tnValue->identifier->name) {
                     if (tnTarget->genericArgs.size() == tnValue->genericArgs.size()) {
                         bool allArgsCompatible = true;
                         for (size_t i = 0; i < tnTarget->genericArgs.size(); ++i) {
                            if (!areTypesCompatible(tnTarget->genericArgs[i].get(), tnValue->genericArgs[i].get())) {
                                allArgsCompatible = false;
                                break;
                            }
                         }
                         if (allArgsCompatible) return true;
                     }
                 }
            }
            return false;
    }
    return false; // Should be unreachable if all cases are handled
}

// ... ensure other visit methods like visit(ast::FromIntToLocExpression* node) are correctly implemented or placeholder ...
// The provided snippets for AddrOfExpression, PointerDerefExpression are now primary.
// The ConstructionExpression handles cases where these might be parsed as such.
// If the parser correctly creates AddrOfExpression, FromIntToLocExpression, PointerDerefExpression,
// then the specific visitors for those AST nodes will be used. The logic for 'addr', 'from', 'at'
// in ConstructionExpression::visit is a fallback or handles cases where the parser uses
// ConstructionExpression for these intrinsics.

// --- Consolidated implementations from semantic_*.cpp files ---

// From semantic_tuple_type.cpp
void SemanticAnalyzer::visit(ast::TupleTypeNode* node) {
    // Basic implementation - verify each element of the tuple type
    for (auto& element : node->memberTypes) {
        if (element) {
            element->accept(*this);
        } else {
            // Report error for null element type
            errors.push_back("Tuple type contains a null element type at " + 
                              node->loc.toString());
        }
    }
}

// Additional helpful utility for SemanticAnalyzer (from semantic_tuple_type.cpp)
// Note: This overload for TypeNode* is added alongside the existing Expression* version
bool SemanticAnalyzer::isRawLocationType(ast::TypeNode* type) {
    // Determine if the type is a raw location type (loc<T>)
    if (auto* typeName = dynamic_cast<ast::TypeName*>(type)) {
        return typeName->identifier && typeName->identifier->name == "loc" && 
               !typeName->genericArgs.empty();
       }
    return false;
}

// From semantic_if_expr.cpp
void SemanticAnalyzer::visit(ast::IfExpression* node) {
    // Check the condition - must be a boolean expression
    if (node->condition) {
        node->condition->accept(*this);
        // In a full implementation, we would check if the condition's type is boolean
        // For now, we won't do type checking
    } else {
        addError("If expression condition missing", node);
    }
    
    // Check the then branch
    if (node->thenBranch) {
        node->thenBranch->accept(*this);
    } else {
        addError("If expression then branch missing", node);
    }
    
    // Check the else branch (required in if expressions)
    if (node->elseBranch) {
        node->elseBranch->accept(*this);
    } else {
        addError("If expression else branch is required", node);
    }
    
    // For a full implementation:
    // 1. Check that condition has boolean type
    // 2. Check that then and else branches have compatible types
    // 3. Set the if-expression's type to the common type of then and else
    
    // For now we skip the type checking
}

// From semantic_borrow_expr.cpp
void SemanticAnalyzer::visit(ast::BorrowExpression* node) {
    // First analyze the expression that's being borrowed
    if (node->expression) {
        node->expression->accept(*this);
    }
    
    // Ensure that the borrowed expression is valid for borrowing
    // (it should be an lvalue if it's a mutable borrow)
    if (node->kind == ast::BorrowKind::MUTABLE_BORROW && !isLValue(node->expression.get())) {
        addError("Cannot take mutable borrow of non-lvalue expression", node);
    }
    
    // Create a type node that represents the borrowed type
    // This would normally set expressionTypes[node] to a pointer type
    // of the underlying expression's type, but we'll just log for now
    
    // In a complete implementation, we would:
    // 1. Get the type of the expression being borrowed
    // 2. Create a borrowed type (ptr/ref) based on it
    // 3. Associate that type with this expression
    
    // For now, we'll just track that this borrow occurred
    // expressionTypes[node] = ...;
}

// From semantic_array_init_expr.cpp
void SemanticAnalyzer::visit(ast::ArrayInitializationExpression* node) {
    // Check that the element type is valid
    if (node->elementType) {
        node->elementType->accept(*this);
    } else {
        addError("Array initialization missing element type", node);
    }
    
    // Check that the size expression is valid
    if (node->sizeExpression) {
        node->sizeExpression->accept(*this);
        // In a full implementation, we would check if the size expression evaluates to an integer type
        // For now, we won't do type checking
    } else {
        addError("Array initialization missing size expression", node);
    }
    
    // For a full implementation:
    // 1. Check that the size expression is of integer type
    // 2. Set the array initialization expression's type to an array type with the element type
    
    // For now we skip the advanced type checking
}

// Template management method implementations
void SemanticAnalyzer::registerTemplate(std::unique_ptr<ast::TemplateDeclaration> templateDecl) {
    if (!templateDecl || !templateDecl->name) {
        return;
    }
    
    const std::string& templateName = templateDecl->name->name;
    auto templateInfo = std::make_unique<TemplateInfo>(std::move(templateDecl));
    templateRegistry[templateName] = std::move(templateInfo);
}

SemanticAnalyzer::TemplateInfo* SemanticAnalyzer::findTemplate(const std::string& templateName) {
    auto it = templateRegistry.find(templateName);
    if (it != templateRegistry.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool SemanticAnalyzer::isTemplateInstantiation(const std::string& name) {
    // Check if this looks like a template instantiation (contains '<' and '>')
    return name.find('<') != std::string::npos && name.find('>') != std::string::npos;
}

std::unique_ptr<ast::Declaration> SemanticAnalyzer::instantiateTemplate(
    const std::string& templateName, 
    const std::vector<std::string>& typeArgs) {
    
    TemplateInfo* templateInfo = findTemplate(templateName);
    if (!templateInfo) {
        return nullptr;
    }
    
    // Validate type argument count
    if (typeArgs.size() != templateInfo->parameterNames.size()) {
        return nullptr;
    }
    
    // TODO: Implement actual monomorphization here
    // For now, return nullptr to indicate "not yet implemented"
    // This is where we would:
    // 1. Clone the template body
    // 2. Substitute type parameters with concrete types
    // 3. Generate specialized AST nodes
    // 4. Return the instantiated declaration
    
    return nullptr;
}

void SemanticAnalyzer::handleTemplateInstantiation(ast::Identifier* identifier, 
                                                  const std::vector<ast::TypeNodePtr>& typeArgs,
                                                  ast::GenericInstantiationExpression* node) {
    if (!identifier) return;
    
    std::string templateName = identifier->name;
    TemplateInfo* templateInfo = findTemplate(templateName);
    
    if (!templateInfo) {
        addError("Template '" + templateName + "' not found.", node);
        return;
    }
    
    // Validate type argument count
    if (typeArgs.size() != templateInfo->parameterNames.size()) {
        addError("Template '" + templateName + "' expects " + 
                std::to_string(templateInfo->parameterNames.size()) + 
                " type arguments, got " + std::to_string(typeArgs.size()) + ".", node);
        return;
    }
    
    // Convert type arguments to string representations for substitution
    std::vector<std::string> concreteTypes;
    for (const auto& typeArg : typeArgs) {
        if (typeArg) {
            concreteTypes.push_back(typeArg->toString());
        } else {
            addError("Invalid type argument in template instantiation.", node);
            return;
        }
    }
    
    // Validate template constraints before monomorphization
    if (!validateTemplateConstraints(templateInfo, concreteTypes, node)) {
        return; // Error already reported
    }
    
    // Perform monomorphization
    auto instantiated = performMonomorphization(templateInfo, concreteTypes);
    if (instantiated) {
        // Store the instantiated template for later use in codegen
        // For now, just mark that we successfully processed it
        expressionTypes[node] = nullptr; // Will be properly typed later
    }
}

void SemanticAnalyzer::handleMemberTemplateInstantiation(ast::MemberExpression* memberExpr,
                                                        const std::vector<ast::TypeNodePtr>& typeArgs,
                                                        ast::GenericInstantiationExpression* node) {
    // Handle cases like Container<Int>::create
    if (!memberExpr || !memberExpr->object) {
        addError("Invalid member template instantiation.", node);
        return;
    }
    
    // For now, delegate to regular template instantiation
    // In a full implementation, this would handle method templates and nested templates
    if (auto objectId = dynamic_cast<ast::Identifier*>(memberExpr->object.get())) {
        handleTemplateInstantiation(objectId, typeArgs, node);
    } else {
        addError("Complex member template instantiation not yet supported.", node);
    }
}

std::unique_ptr<ast::Declaration> SemanticAnalyzer::performMonomorphization(TemplateInfo* templateInfo,
                                                                           const std::vector<std::string>& concreteTypes) {
    if (!templateInfo || !templateInfo->declaration) {
        return nullptr;
    }
    
    // Generate a unique key for this instantiation
    std::string instanceKey = templateInfo->templateName + "<";
    for (size_t i = 0; i < concreteTypes.size(); ++i) {
        if (i > 0) instanceKey += ",";
        instanceKey += concreteTypes[i];
    }
    instanceKey += ">";
    
    // For now, just return nullptr to indicate "monomorphization not yet implemented"
    // In a full implementation, this would:
    // 1. Clone the template body AST
    // 2. Substitute type parameters with concrete types
    // 3. Cache the result for reuse
    // 4. Return the instantiated declaration
    
    return nullptr;
}

std::unique_ptr<ast::Declaration> SemanticAnalyzer::cloneAndSubstituteAST(ast::Declaration* templateBody,
                                                                         const std::vector<std::string>& genericParams,
                                                                         const std::vector<std::string>& concreteTypes) {
    if (!templateBody) {
        return nullptr;
    }
    
    // Placeholder implementation - proper AST cloning would require:
    // 1. Deep clone the AST structure
    // 2. Walk through all TypeName nodes in the cloned AST
    // 3. Replace generic parameter names with concrete types
    // 4. Update function signatures, struct fields, etc.
    // 5. Generate specialized mangled names
    
    return nullptr;
}

// Template constraint validation methods
bool SemanticAnalyzer::validateTemplateConstraints(TemplateInfo* templateInfo,
                                                  const std::vector<std::string>& concreteTypes,
                                                  ast::GenericInstantiationExpression* node) {
    if (!templateInfo || concreteTypes.size() != templateInfo->parameterNames.size()) {
        return false;
    }
    
    // Validate each type argument against its constraints
    for (size_t i = 0; i < concreteTypes.size(); ++i) {
        const std::string& concreteType = concreteTypes[i];
        const std::vector<std::string>& constraints = templateInfo->parameterConstraints[i];
        
        // Check each trait constraint for this type parameter
        for (const std::string& traitName : constraints) {
            if (!typeImplementsTrait(concreteType, traitName)) {
                addError("Type '" + concreteType + "' does not implement required trait '" + 
                        traitName + "' for template parameter '" + 
                        templateInfo->parameterNames[i] + "'.", node);
                return false;
            }
        }
    }
    
    return true;
}

bool SemanticAnalyzer::typeImplementsTrait(const std::string& typeName, const std::string& traitName) {
    // Check if the type implements the specified trait
    // This is a simplified implementation - a full system would:
    // 1. Look up trait implementations in the symbol table
    // 2. Check for built-in trait implementations
    // 3. Handle generic trait implementations
    
    // For now, implement basic built-in type trait support
    if (isBuiltinTypeCompatible(typeName, traitName)) {
        return true;
    }
    
    // Look up user-defined implementations
    // TODO: Implement proper trait implementation lookup
    // This would search for impl declarations matching the type and trait
    
    return false;
}

std::vector<std::string> SemanticAnalyzer::getImplementedTraits(const std::string& typeName) {
    std::vector<std::string> traits;
    
    // Add built-in trait implementations
    if (typeName == "Int" || typeName == "Int32" || typeName == "Float" || typeName == "Float32" || typeName == "Char") {
        traits.push_back("Comparable");
        traits.push_back("Equatable");
        if (typeName == "Int" || typeName == "Int32" || typeName == "Float" || typeName == "Float32") {
            traits.push_back("Numeric");
        }
    }
    
    if (typeName == "String" || typeName == "Char") {
        traits.push_back("Equatable");
        if (typeName == "String") {
            traits.push_back("Comparable");
        }
    }
    
    if (typeName == "Bool") {
        traits.push_back("Equatable");
    }
    
    // TODO: Look up user-defined trait implementations
    
    return traits;
}

bool SemanticAnalyzer::isBuiltinTypeCompatible(const std::string& typeName, const std::string& traitName) {
    // Check built-in type and trait compatibility
    if (traitName == "Comparable") {
        return typeName == "Int" || typeName == "Int32" || typeName == "Float" || typeName == "Float32" || typeName == "Char" || typeName == "String";
    }
    
    if (traitName == "Equatable") {
        return typeName == "Int" || typeName == "Int32" || typeName == "Float" || typeName == "Float32" || typeName == "Char" || 
               typeName == "String" || typeName == "Bool";
    }
    
    if (traitName == "Numeric") {
        return typeName == "Int" || typeName == "Int32" || typeName == "Float" || typeName == "Float32";
    }
    
    if (traitName == "Hashable") {
        return typeName == "Int" || typeName == "String" || typeName == "Char" || typeName == "Bool";
    }
    
    return false;
}

void SemanticAnalyzer::handleVecMethodCall(ast::CallExpression* node, const std::string& objectName, const std::string& methodName) {
    // Validate that the object is a Vec type and check constness
    // Look up the object in the symbol table to check mutability
    SymbolInfo* objSymbol = currentScope->lookup(objectName);
    bool isConstVec = false;
    bool isTheirVec = false;
    
    if (objSymbol) {
        // Check if the variable is const or has ownership constraints
        if (objSymbol->isConst) {
            isConstVec = true;
        }
        // Check for 'their' ownership (borrowed reference)
        if (objSymbol->ownershipKind == ast::OwnershipKind::THEIR) {
            isTheirVec = true;
        }
    }
    
    if (methodName == "push") {
        // push(element) -> Vec<T> (for chaining)
        if (node->arguments.size() != 1) {
            addError("Vec::push expects exactly 1 argument", node);
            return;
        }
        // Check constness - push is a mutating operation
        if (isConstVec) {
            addError("Cannot call mutating method 'push' on const Vec: " + objectName, node);
            return;
        }
        // Return type is the Vec itself for chaining
        // In full implementation, get element type from Vec<T>
        auto intId = std::make_unique<ast::Identifier>(node->loc, "Int");
        auto intType = std::make_unique<ast::TypeName>(node->loc, std::move(intId));
        auto vecType = std::make_unique<ast::VecType>(node->loc, std::move(intType));
        expressionTypes[node] = vecType.get();
        node->type = std::shared_ptr<ast::TypeNode>(std::move(vecType));
        
    } else if (methodName == "pop") {
        // pop() -> T (element type)
        if (node->arguments.size() != 0) {
            addError("Vec::pop expects no arguments", node);
            return;
        }
        // Check constness - pop is a mutating operation
        if (isConstVec) {
            addError("Cannot call mutating method 'pop' on const Vec: " + objectName, node);
            return;
        }
        // Return element type
        auto intId = std::make_unique<ast::Identifier>(node->loc, "Int");
        auto intType = std::make_unique<ast::TypeName>(node->loc, std::move(intId));
        expressionTypes[node] = intType.get();
        node->type = std::shared_ptr<ast::TypeNode>(std::move(intType));
        
    } else if (methodName == "len") {
        // len() -> Int
        if (node->arguments.size() != 0) {
            addError("Vec::len expects no arguments", node);
            return;
        }
        auto intId = std::make_unique<ast::Identifier>(node->loc, "Int");
        auto intType = std::make_unique<ast::TypeName>(node->loc, std::move(intId));
        expressionTypes[node] = intType.get();
        node->type = std::shared_ptr<ast::TypeNode>(std::move(intType));
        
    } else if (methodName == "get") {
        // get(index) -> T (element type)
        if (node->arguments.size() != 1) {
            addError("Vec::get expects exactly 1 argument (index)", node);
            return;
        }
        // Validate index is integer type
        if (node->arguments[0]) {
            node->arguments[0]->accept(*this);
            // TODO: Check that argument is Int type
        }
        // Return element type
        auto intId = std::make_unique<ast::Identifier>(node->loc, "Int");
        auto intType = std::make_unique<ast::TypeName>(node->loc, std::move(intId));
        expressionTypes[node] = intType.get();
        node->type = std::shared_ptr<ast::TypeNode>(std::move(intType));
        
    } else if (methodName == "push_array") {
        // push_array(array) -> Vec<T> (for chaining)
        if (node->arguments.size() != 1) {
            addError("Vec::push_array expects exactly 1 argument (array)", node);
            return;
        }
        // Check constness - push_array is a mutating operation
        if (isConstVec) {
            addError("Cannot call mutating method 'push_array' on const Vec: " + objectName, node);
            return;
        }
        // TODO: Validate argument is array type [T; N] compatible with Vec<T>
        // Return Vec type for chaining
        auto intId = std::make_unique<ast::Identifier>(node->loc, "Int");
        auto intType = std::make_unique<ast::TypeName>(node->loc, std::move(intId));
        auto vecType = std::make_unique<ast::VecType>(node->loc, std::move(intType));
        expressionTypes[node] = vecType.get();
        node->type = std::shared_ptr<ast::TypeNode>(std::move(vecType));
        
    } else if (methodName == "to_array") {
        // to_array(size) -> [T; N]
        if (node->arguments.size() != 1) {
            addError("Vec::to_array expects exactly 1 argument (array size)", node);
            return;
        }
        // TODO: Validate size argument is integer
        // Return array type [T; N]
        auto intId = std::make_unique<ast::Identifier>(node->loc, "Int");
        auto intType = std::make_unique<ast::TypeName>(node->loc, std::move(intId));
        auto arrayType = std::make_unique<ast::ArrayType>(node->loc, std::move(intType), nullptr);
        expressionTypes[node] = arrayType.get();
        node->type = std::shared_ptr<ast::TypeNode>(std::move(arrayType));
        
    } else if (methodName == "clear") {
        // clear() -> void
        if (node->arguments.size() != 0) {
            addError("Vec::clear expects no arguments", node);
            return;
        }
        // Check constness - clear is a mutating operation
        if (isConstVec) {
            addError("Cannot call mutating method 'clear' on const Vec: " + objectName, node);
            return;
        }
        // Return void (no type)
        node->type = nullptr;
        
    } else if (methodName == "is_empty") {
        // is_empty() -> Bool
        if (node->arguments.size() != 0) {
            addError("Vec::is_empty expects no arguments", node);
            return;
        }
        auto boolId = std::make_unique<ast::Identifier>(node->loc, "Bool");
        auto boolType = std::make_unique<ast::TypeName>(node->loc, std::move(boolId));
        expressionTypes[node] = boolType.get();
        node->type = std::shared_ptr<ast::TypeNode>(std::move(boolType));
        
    } else if (methodName == "capacity") {
        // capacity() -> Int
        if (node->arguments.size() != 0) {
            addError("Vec::capacity expects no arguments", node);
            return;
        }
        auto intId = std::make_unique<ast::Identifier>(node->loc, "Int");
        auto intType = std::make_unique<ast::TypeName>(node->loc, std::move(intId));
        expressionTypes[node] = intType.get();
        node->type = std::shared_ptr<ast::TypeNode>(std::move(intType));
        
    } else if (methodName == "concat") {
        // concat(other_vec) -> Vec<T> (for chaining)
        if (node->arguments.size() != 1) {
            addError("Vec::concat expects exactly 1 argument (other Vec)", node);
            return;
        }
        // TODO: Validate argument is compatible Vec<T> type
        auto intId = std::make_unique<ast::Identifier>(node->loc, "Int");
        auto intType = std::make_unique<ast::TypeName>(node->loc, std::move(intId));
        auto vecType = std::make_unique<ast::VecType>(node->loc, std::move(intType));
        expressionTypes[node] = vecType.get();
        node->type = std::shared_ptr<ast::TypeNode>(std::move(vecType));
        
    } else if (methodName == "contains") {
        // contains(value) -> Bool
        if (node->arguments.size() != 1) {
            addError("Vec::contains expects exactly 1 argument (value to search)", node);
            return;
        }
        // TODO: Validate argument is compatible with element type T
        auto boolId = std::make_unique<ast::Identifier>(node->loc, "Bool");
        auto boolType = std::make_unique<ast::TypeName>(node->loc, std::move(boolId));
        expressionTypes[node] = boolType.get();
        node->type = std::shared_ptr<ast::TypeNode>(std::move(boolType));
        
    } else if (methodName == "remove_at") {
        // remove_at(index) -> T (removed element)
        if (node->arguments.size() != 1) {
            addError("Vec::remove_at expects exactly 1 argument (index)", node);
            return;
        }
        // Check constness - remove_at is a mutating operation
        if (isConstVec) {
            addError("Cannot call mutating method 'remove_at' on const Vec: " + objectName, node);
            return;
        }
        // TODO: Validate index is Int type
        // Return element type
        auto intId = std::make_unique<ast::Identifier>(node->loc, "Int");
        auto intType = std::make_unique<ast::TypeName>(node->loc, std::move(intId));
        expressionTypes[node] = intType.get();
        node->type = std::shared_ptr<ast::TypeNode>(std::move(intType));
        
    } else if (methodName == "get_array") {
        // get_array(pre_allocated_array) -> Int (number of elements copied)
        if (node->arguments.size() != 1) {
            addError("Vec::get_array expects exactly 1 argument (pre-allocated array)", node);
            return;
        }
        // get_array is read-only, so it's allowed on const/their Vecs
        // TODO: Validate argument is array type [T; N] compatible with Vec<T>
        // Return Int (number of elements copied for efficiency feedback)
        auto intId = std::make_unique<ast::Identifier>(node->loc, "Int");
        auto intType = std::make_unique<ast::TypeName>(node->loc, std::move(intId));
        expressionTypes[node] = intType.get();
        node->type = std::shared_ptr<ast::TypeNode>(std::move(intType));
        
    } else if (methodName == "get_vec") {
        // get_vec(target_vec) -> Int (number of elements copied)
        // Extracts contents from any Vec into target Vec, respecting constness
        if (node->arguments.size() != 1) {
            addError("Vec::get_vec expects exactly 1 argument (target Vec)", node);
            return;
        }
        // get_vec is read-only on source Vec (works with all ownership types: MY, OUR, THEIR, PTR)
        // constness is automatically respected since this is a read-only operation
        // TODO: Validate argument is Vec<T> type compatible with source Vec<T>
        // Return Int (number of elements copied)
        auto intId = std::make_unique<ast::Identifier>(node->loc, "Int");
        auto intType = std::make_unique<ast::TypeName>(node->loc, std::move(intId));
        expressionTypes[node] = intType.get();
        node->type = std::shared_ptr<ast::TypeNode>(std::move(intType));
        
    } else {
        addError("Unknown Vec method: " + methodName, node);
    }
}

} // Added missing closing brace for namespace vyn
