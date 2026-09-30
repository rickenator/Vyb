// SPDX-License-Identifier: Apache-2.0
//
// Seam extracted from src/vre/semantic.cpp (#347): the member definitions below
// are moved VERBATIM -- no reformatting, no renames -- so the split stays
// behaviour-neutral. Helpers that other code still calls are promoted into
// include/vyb/vre/semantic_internal.hpp as `inline`, never duplicated.

#include "vyb/semantic.hpp"
#include "vyb/parser/token.hpp"
#include "vyb/parser/ast.hpp"
#include "vyb/driver.hpp"
#include "vyb/vre/semantic_internal.hpp"
#include "vyb/vre/thread_boundary.hpp"
#include <stdexcept>
#include <memory>
#include <unordered_set>
#include <string>
#include <map>
#include <set>
#include <functional>
#include <algorithm>

namespace vyb {

void SemanticAnalyzer::handleVecConstructor(ast::CallExpression* node) {
    if (node->arguments.size() > 1) {
        addError("Vec() accepts at most 1 argument (optional size)", node);
        return;
    }
    if (node->arguments.size() == 1 && node->arguments[0]) {
        node->arguments[0]->accept(*this);
    }
    // If a surrounding annotation (e.g. `v<Vec<String>> = Vec()`) already
    // propagated the concrete Vec<T> element type, use it.
    if (expressionTypes.count(exprKey(node)) && expressionTypes[exprKey(node)]) {
        return;
    }
    // Fall back to Vec<Int> when no element type is inferable.
    auto intId = std::make_unique<ast::Identifier>(node->loc, "Int");
    auto intType = std::make_unique<ast::TypeName>(node->loc, std::move(intId));
    auto vecType = std::make_unique<ast::VecType>(node->loc, std::move(intType));
    setType(node,  vecType ? std::shared_ptr<ast::TypeNode>(vecType->clone()) : nullptr);
    setType(node,  std::shared_ptr<ast::TypeNode>(std::move(vecType)));
}
void SemanticAnalyzer::visit(ast::VecType* node) {
    if (node && node->elementType) {
        node->elementType->accept(*this);
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

    // Look up user-defined trait implementations
    auto typeIt = traitImpls.find(typeName);
    if (typeIt != traitImpls.end()) {
        for (const auto& traitEntry : typeIt->second) {
            traits.push_back(traitEntry.first);
        }
    }

    return traits;
}
void SemanticAnalyzer::handleVecMethodCall(ast::CallExpression* node, const std::string& objectName, const std::string& methodName) {
    // Validate that the object is a Vec type and check constness
    // Look up the object in the symbol table to check mutability
    SymbolInfo* objSymbol = currentScope->lookup(objectName);
    bool isConstVec = false;
    bool isTheirVec = false;

    // By-ref receivers are typed `their<Vec<T>>` (a TypeName whose single
    // generic argument is a Vec). Record the ownership kind so mutating calls
    // are permitted, then treat `objSymbol->type` as that inner Vec throughout
    // the element-type / Vec-type return inference below.
    ast::TypeNode* vecTypeNode = objSymbol ? objSymbol->type.get() : nullptr;
    if (objSymbol && objSymbol->type) {
        if (auto wrap = dynamic_cast<ast::TypeName*>(objSymbol->type.get())) {
            if (wrap->identifier &&
                (wrap->identifier->name == "their" || wrap->identifier->name == "my" ||
                 wrap->identifier->name == "our" || wrap->identifier->name == "view" ||
                 wrap->identifier->name == "borrow") && wrap->genericArgs.size() == 1) {
                ast::TypeNode* inner = wrap->genericArgs[0].get();
                bool innerIsVec = dynamic_cast<ast::VecType*>(inner) != nullptr;
                if (!innerIsVec) {
                    if (auto innerTN = dynamic_cast<ast::TypeName*>(inner)) {
                        innerIsVec = innerTN->identifier && innerTN->identifier->name == "Vec";
                    }
                }
                if (innerIsVec) {
                    vecTypeNode = inner;
                    if (wrap->identifier->name == "their" || wrap->identifier->name == "my" ||
                        wrap->identifier->name == "our" || wrap->identifier->name == "borrow") {
                        objSymbol->ownershipKind = ast::OwnershipKind::THEIR;
                    }
                }
            }
        }
    }

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
        setType(node,  vecType ? std::shared_ptr<ast::TypeNode>(vecType->clone()) : nullptr);
        setType(node,  std::shared_ptr<ast::TypeNode>(std::move(vecType)));

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
        // Return element type (resolve T from Vec<T> receiver; mirrors get())
        if (vecTypeNode) {
            if (auto* vty = dynamic_cast<ast::VecType*>(vecTypeNode)) {
                if (vty->elementType) {
                    std::shared_ptr<ast::TypeNode> et = cloneTypeNode(vty->elementType.get());
                    setType(node,  et ? std::shared_ptr<ast::TypeNode>(et->clone()) : nullptr);
                    setType(node,  et);
                    return;
                }
            } else if (auto* tny = dynamic_cast<ast::TypeName*>(vecTypeNode)) {
                if (tny->identifier && tny->identifier->name == "Vec" && !tny->genericArgs.empty()) {
                    std::shared_ptr<ast::TypeNode> et = tny->genericArgs[0]->clone();
                    setType(node,  et ? std::shared_ptr<ast::TypeNode>(et->clone()) : nullptr);
                    setType(node,  et);
                    return;
                }
            }
        }
        auto intId = std::make_unique<ast::Identifier>(node->loc, "Int");
        auto intType = std::make_unique<ast::TypeName>(node->loc, std::move(intId));
        setType(node,  intType ? std::shared_ptr<ast::TypeNode>(intType->clone()) : nullptr);
        setType(node,  std::shared_ptr<ast::TypeNode>(std::move(intType)));

    } else if (methodName == "len") {
        // len() -> Int
        if (node->arguments.size() != 0) {
            addError("Vec::len expects no arguments", node);
            return;
        }
        auto intId = std::make_unique<ast::Identifier>(node->loc, "Int");
        auto intType = std::make_unique<ast::TypeName>(node->loc, std::move(intId));
        setType(node,  intType ? std::shared_ptr<ast::TypeNode>(intType->clone()) : nullptr);
        setType(node,  std::shared_ptr<ast::TypeNode>(std::move(intType)));

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

        // Extract element type from Vec<T>
        // Look up the object in the symbol table to get its Vec type
        SymbolInfo* objSymbol = currentScope->lookup(objectName);
        if (objSymbol && objSymbol->type) {
            // Check if the type is a VecType
            if (auto* vecType = dynamic_cast<ast::VecType*>(vecTypeNode)) {
                // Clone the element type for the return type
                if (vecType->elementType) {
                    // #297: a Vec element accessor yields a borrow when the
                    // element is itself a Vec (see vecGetResultType).
                    std::shared_ptr<ast::TypeNode> clonedElementType = vecGetResultType(node->loc, vecType->elementType.get());
                    setType(node,  clonedElementType ? std::shared_ptr<ast::TypeNode>(clonedElementType->clone()) : nullptr);
                    setType(node,  clonedElementType);
                    return;
                }
            }
            // Also handle TypeName "Vec<T>" (e.g., function parameters)
            if (auto* typeName = dynamic_cast<ast::TypeName*>(vecTypeNode)) {
                if (typeName->identifier && typeName->identifier->name == "Vec" && !typeName->genericArgs.empty()) {
                    if (typeName->genericArgs[0]) {
                        std::shared_ptr<ast::TypeNode> clonedElementType = vecGetResultType(node->loc, typeName->genericArgs[0].get());
                        setType(node,  clonedElementType ? std::shared_ptr<ast::TypeNode>(clonedElementType->clone()) : nullptr);
                        setType(node,  clonedElementType);
                        return;
                    }
                }
            }
        }

        // Fallback to Int if we couldn't determine the element type
        auto intId = std::make_unique<ast::Identifier>(node->loc, "Int");
        auto intType = std::make_unique<ast::TypeName>(node->loc, std::move(intId));
        setType(node,  intType ? std::shared_ptr<ast::TypeNode>(intType->clone()) : nullptr);
        setType(node,  std::shared_ptr<ast::TypeNode>(std::move(intType)));

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
        setType(node,  vecType ? std::shared_ptr<ast::TypeNode>(vecType->clone()) : nullptr);
        setType(node,  std::shared_ptr<ast::TypeNode>(std::move(vecType)));

    } else if (methodName == "to_array") {
        // to_array(size) -> [T; N]
        if (node->arguments.size() != 1) {
            addError("Vec::to_array expects exactly 1 argument (array size)", node);
            return;
        }
        // Return array type [T; N]: element type taken from the Vec<T> receiver
        // so `typeOf(node)` names a real array and codegen can build `[N x T]`.
        ast::TypeNodePtr elemType = nullptr;
        if (auto vt = dynamic_cast<ast::VecType*>(vecTypeNode)) {
            elemType = vt->elementType ? vt->elementType->clone() : nullptr;
        } else if (auto tn = dynamic_cast<ast::TypeName*>(vecTypeNode)) {
            if (tn->identifier && tn->identifier->name == "Vec" && !tn->genericArgs.empty())
                elemType = tn->genericArgs[0]->clone();
        }
        if (!elemType)
            elemType = std::make_unique<ast::TypeName>(
                node->loc, std::make_unique<ast::Identifier>(node->loc, "Int"));
        auto arrayType = std::make_unique<ast::ArrayType>(node->loc, std::move(elemType));
        setType(node,  arrayType ? std::shared_ptr<ast::TypeNode>(arrayType->clone()) : nullptr);
        setType(node,  std::shared_ptr<ast::TypeNode>(std::move(arrayType)));

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
        setType(node,  nullptr);

    } else if (methodName == "is_empty") {
        // is_empty() -> Bool
        if (node->arguments.size() != 0) {
            addError("Vec::is_empty expects no arguments", node);
            return;
        }
        auto boolId = std::make_unique<ast::Identifier>(node->loc, "Bool");
        auto boolType = std::make_unique<ast::TypeName>(node->loc, std::move(boolId));
        setType(node,  boolType ? std::shared_ptr<ast::TypeNode>(boolType->clone()) : nullptr);
        setType(node,  std::shared_ptr<ast::TypeNode>(std::move(boolType)));

    } else if (methodName == "capacity") {
        // capacity() -> Int
        if (node->arguments.size() != 0) {
            addError("Vec::capacity expects no arguments", node);
            return;
        }
        auto intId = std::make_unique<ast::Identifier>(node->loc, "Int");
        auto intType = std::make_unique<ast::TypeName>(node->loc, std::move(intId));
        setType(node,  intType ? std::shared_ptr<ast::TypeNode>(intType->clone()) : nullptr);
        setType(node,  std::shared_ptr<ast::TypeNode>(std::move(intType)));

    } else if (methodName == "concat") {
        // concat(other_vec) -> Vec<T> (for chaining)
        if (node->arguments.size() != 1) {
            addError("Vec::concat expects exactly 1 argument (other Vec)", node);
            return;
        }
        // Return Vec<T> (element type taken from Vec<T> receiver; mirrors get()).
        ast::TypeNodePtr elemType = nullptr;
        if (auto vty = dynamic_cast<ast::VecType*>(vecTypeNode)) {
            elemType = vty->elementType ? vty->elementType->clone() : nullptr;
        } else if (auto tny = dynamic_cast<ast::TypeName*>(vecTypeNode)) {
            if (tny->identifier && tny->identifier->name == "Vec" && !tny->genericArgs.empty())
                elemType = tny->genericArgs[0]->clone();
        }
        if (!elemType)
            elemType = std::make_unique<ast::TypeName>(
                node->loc, std::make_unique<ast::Identifier>(node->loc, "Int"));
        auto vecType = std::make_unique<ast::VecType>(node->loc, std::move(elemType));
        setType(node,  vecType ? std::shared_ptr<ast::TypeNode>(vecType->clone()) : nullptr);
        setType(node,  std::shared_ptr<ast::TypeNode>(std::move(vecType)));

    } else if (methodName == "contains") {
        // contains(value) -> Bool
        if (node->arguments.size() != 1) {
            addError("Vec::contains expects exactly 1 argument (value to search)", node);
            return;
        }
        // TODO: Validate argument is compatible with element type T
        auto boolId = std::make_unique<ast::Identifier>(node->loc, "Bool");
        auto boolType = std::make_unique<ast::TypeName>(node->loc, std::move(boolId));
        setType(node,  boolType ? std::shared_ptr<ast::TypeNode>(boolType->clone()) : nullptr);
        setType(node,  std::shared_ptr<ast::TypeNode>(std::move(boolType)));

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
        // Return element type (resolve T from Vec<T> receiver; mirrors get())
        if (vecTypeNode) {
            if (auto* vty = dynamic_cast<ast::VecType*>(vecTypeNode)) {
                if (vty->elementType) {
                    std::shared_ptr<ast::TypeNode> et = cloneTypeNode(vty->elementType.get());
                    setType(node,  et ? std::shared_ptr<ast::TypeNode>(et->clone()) : nullptr);
                    setType(node,  et);
                    return;
                }
            } else if (auto* tny = dynamic_cast<ast::TypeName*>(vecTypeNode)) {
                if (tny->identifier && tny->identifier->name == "Vec" && !tny->genericArgs.empty()) {
                    std::shared_ptr<ast::TypeNode> et = tny->genericArgs[0]->clone();
                    setType(node,  et ? std::shared_ptr<ast::TypeNode>(et->clone()) : nullptr);
                    setType(node,  et);
                    return;
                }
            }
        }
        auto intId = std::make_unique<ast::Identifier>(node->loc, "Int");
        auto intType = std::make_unique<ast::TypeName>(node->loc, std::move(intId));
        setType(node,  intType ? std::shared_ptr<ast::TypeNode>(intType->clone()) : nullptr);
        setType(node,  std::shared_ptr<ast::TypeNode>(std::move(intType)));

    } else if (methodName == "set") {
        // set(index, value) -> void (overwrite element in place)
        if (node->arguments.size() != 2) {
            addError("Vec::set expects exactly 2 arguments (index, value)", node);
            return;
        }
        if (isConstVec) {
            addError("Cannot call mutating method 'set' on const Vec: " + objectName, node);
            return;
        }
        if (node->arguments[0]) node->arguments[0]->accept(*this);
        if (node->arguments[1]) node->arguments[1]->accept(*this);
        setType(node,  nullptr);

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
        setType(node,  intType ? std::shared_ptr<ast::TypeNode>(intType->clone()) : nullptr);
        setType(node,  std::shared_ptr<ast::TypeNode>(std::move(intType)));

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
        setType(node,  intType ? std::shared_ptr<ast::TypeNode>(intType->clone()) : nullptr);
        setType(node,  std::shared_ptr<ast::TypeNode>(std::move(intType)));

    } else if (methodName == "last" || methodName == "peek") {
        // last()/peek() -> T (last element, read-only: no removal, no mutation).
        if (node->arguments.size() != 0) {
            addError("Vec::" + methodName + " expects no arguments", node);
            return;
        }
        if (vecTypeNode) {
            if (auto* vty = dynamic_cast<ast::VecType*>(vecTypeNode)) {
                if (vty->elementType) {
                    std::shared_ptr<ast::TypeNode> et = cloneTypeNode(vty->elementType.get());
                    setType(node,  et ? std::shared_ptr<ast::TypeNode>(et->clone()) : nullptr);
                    setType(node,  et);
                    return;
                }
            } else if (auto* tny = dynamic_cast<ast::TypeName*>(vecTypeNode)) {
                if (tny->identifier && tny->identifier->name == "Vec" && !tny->genericArgs.empty()) {
                    std::shared_ptr<ast::TypeNode> et = tny->genericArgs[0]->clone();
                    setType(node,  et ? std::shared_ptr<ast::TypeNode>(et->clone()) : nullptr);
                    setType(node,  et);
                    return;
                }
            }
        }
        auto intId = std::make_unique<ast::Identifier>(node->loc, "Int");
        auto intType = std::make_unique<ast::TypeName>(node->loc, std::move(intId));
        setType(node,  intType ? std::shared_ptr<ast::TypeNode>(intType->clone()) : nullptr);
        setType(node,  std::shared_ptr<ast::TypeNode>(std::move(intType)));

    } else {
        addError("Unknown Vec method: " + methodName, node);
    }
}
void SemanticAnalyzer::handleVecMethodCallOnMember(ast::CallExpression* node, ast::VecType* vecType, const std::string& methodName) {
    // Handle Vec method calls on member expressions (e.g., tree.nodes.push())
    // This variant doesn't need to look up the object in the symbol table since we already have the type

    // Get the element type from the Vec<T>
    ast::TypeNode* elementType = vecType->elementType.get();

    // IMPORTANT: vecType may point to a temporary unique_ptr in the caller (e.g. tempVecType).
    // To avoid use-after-free when that temporary is destroyed, always clone types into typeOf(node)
    // FIRST, then store typeOf(node).get() in expressionTypes so the pointer is stable.

    if (methodName == "push") {
        // push(element) -> Vec<T> (for chaining)
        if (node->arguments.size() != 1) {
            addError("Vec::push expects exactly 1 argument", node);
            return;
        }
        // Accept the argument to ensure it gets analyzed
        node->arguments[0]->accept(*this);

        // Clone type into typeOf(node) first, then store stable pointer
        setType(node,  std::shared_ptr<ast::TypeNode>(vecType->clone()));
        setType(node,  typeOf(node) ? std::shared_ptr<ast::TypeNode>(typeOf(node)->clone()) : nullptr);

    } else if (methodName == "pop") {
        // pop() -> T (element type)
        if (node->arguments.size() != 0) {
            addError("Vec::pop expects no arguments", node);
            return;
        }
        // Return element type
        if (elementType) {
            setType(node,  std::shared_ptr<ast::TypeNode>(elementType->clone()));
            setType(node,  typeOf(node) ? std::shared_ptr<ast::TypeNode>(typeOf(node)->clone()) : nullptr);
        }

    } else if (methodName == "len") {
        // len() -> Int
        if (node->arguments.size() != 0) {
            addError("Vec::len expects no arguments", node);
            return;
        }
        auto intId = std::make_unique<ast::Identifier>(node->loc, "Int");
        auto intType = std::make_unique<ast::TypeName>(node->loc, std::move(intId));
        setType(node,  intType ? std::shared_ptr<ast::TypeNode>(intType->clone()) : nullptr);
        setType(node,  std::shared_ptr<ast::TypeNode>(std::move(intType)));

    } else if (methodName == "get") {
        // get(index) -> T (element type)
        if (node->arguments.size() != 1) {
            addError("Vec::get expects exactly 1 argument (index)", node);
            return;
        }
        // Validate index argument
        if (node->arguments[0]) {
            node->arguments[0]->accept(*this);
        }
        // Return element type
        if (elementType) {
            // #297: a Vec element accessor yields a borrow when the element is
            // itself a Vec (see vecGetResultType).
            setType(node,  vecGetResultType(node->loc, elementType));
            setType(node,  typeOf(node) ? std::shared_ptr<ast::TypeNode>(typeOf(node)->clone()) : nullptr);
        }

    } else if (methodName == "last" || methodName == "peek") {
        // last()/peek() -> T (last element, read-only: no removal, no mutation).
        if (node->arguments.size() != 0) {
            addError("Vec::" + methodName + " expects no arguments", node);
            return;
        }
        if (elementType) {
            setType(node,  std::shared_ptr<ast::TypeNode>(elementType->clone()));
            setType(node,  typeOf(node) ? std::shared_ptr<ast::TypeNode>(typeOf(node)->clone()) : nullptr);
        }

    } else if (methodName == "set") {
        // set(index, value) -> void (overwrite element in place)
        if (node->arguments.size() != 2) {
            addError("Vec::set expects exactly 2 arguments (index, value)", node);
            return;
        }
        if (node->arguments[0]) node->arguments[0]->accept(*this);
        if (node->arguments[1]) node->arguments[1]->accept(*this);
        setType(node,  nullptr);

    } else if (methodName == "clear") {
        // clear() -> void
        if (node->arguments.size() != 0) {
            addError("Vec::clear expects no arguments", node);
            return;
        }
        setType(node,  nullptr);

    } else if (methodName == "is_empty") {
        // is_empty() -> Bool
        if (node->arguments.size() != 0) {
            addError("Vec::is_empty expects no arguments", node);
            return;
        }
        auto boolId = std::make_unique<ast::Identifier>(node->loc, "Bool");
        auto boolType = std::make_unique<ast::TypeName>(node->loc, std::move(boolId));
        setType(node,  boolType ? std::shared_ptr<ast::TypeNode>(boolType->clone()) : nullptr);
        setType(node,  std::shared_ptr<ast::TypeNode>(std::move(boolType)));

    } else if (methodName == "remove_at" || methodName == "remove") {
        // remove_at(index) -> T (removed element). Mutating: valid with a mutable member.
        if (node->arguments.size() != 1) {
            addError("Vec::" + methodName + " expects exactly 1 argument (index)", node);
            return;
        }
        if (node->arguments[0]) node->arguments[0]->accept(*this);
        if (elementType) {
            setType(node,  std::shared_ptr<ast::TypeNode>(elementType->clone()));
            setType(node,  typeOf(node) ? std::shared_ptr<ast::TypeNode>(typeOf(node)->clone()) : nullptr);
        }

    } else if (methodName == "capacity") {
        // capacity() -> Int
        if (node->arguments.size() != 0) {
            addError("Vec::capacity expects no arguments", node);
            return;
        }
        auto capId = std::make_unique<ast::Identifier>(node->loc, "Int");
        auto capType = std::make_unique<ast::TypeName>(node->loc, std::move(capId));
        setType(node,  capType ? std::shared_ptr<ast::TypeNode>(capType->clone()) : nullptr);
        setType(node,  std::shared_ptr<ast::TypeNode>(std::move(capType)));

    } else if (methodName == "contains") {
        // contains(element) -> Bool
        if (node->arguments.size() != 1) {
            addError("Vec::contains expects exactly 1 argument", node);
            return;
        }
        if (node->arguments[0]) node->arguments[0]->accept(*this);
        auto containId = std::make_unique<ast::Identifier>(node->loc, "Bool");
        auto containType = std::make_unique<ast::TypeName>(node->loc, std::move(containId));
        setType(node,  containType ? std::shared_ptr<ast::TypeNode>(containType->clone()) : nullptr);
        setType(node,  std::shared_ptr<ast::TypeNode>(std::move(containType)));

    } else {
        addError("Unknown Vec method: " + methodName, node);
    }
}

} // namespace vyb
