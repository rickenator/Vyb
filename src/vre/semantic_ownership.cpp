// SPDX-License-Identifier: Apache-2.0
//
// Ownership / borrow / move state helpers, extracted verbatim from semantic.cpp
// (#347, checkpoint 3): lvalue/borrow-root queries, borrow-state bookkeeping,
// ownership-kind lookups and move-scope tracking. The file-local type-predicate
// helpers this cluster used (`isMyOwnershipType`, `isTheirType`,
// `isVecTypeNodeAst`, `vecGetResultType`) were promoted to
// vyb/vre/semantic_internal.hpp, because semantic.cpp itself still calls them.
// Same member definitions declared in vyb/semantic.hpp; no behaviour change.

#include "vyb/semantic.hpp"
#include "vyb/parser/ast.hpp"
#include "vyb/vre/semantic_internal.hpp"

#include <string>
#include <algorithm>

namespace vyb {

bool SemanticAnalyzer::isLValue(ast::Expression* expr) {
    return dynamic_cast<ast::Identifier*>(expr) != nullptr ||
           dynamic_cast<ast::MemberExpression*>(expr) != nullptr ||
           dynamic_cast<ast::ArrayElementExpression*>(expr) != nullptr ||
           dynamic_cast<ast::PointerDerefExpression*>(expr) != nullptr;
}

std::string SemanticAnalyzer::borrowedRootName(ast::Expression* expr) {
    if (auto* ident = dynamic_cast<ast::Identifier*>(expr)) {
        return ident->name;
    }
    if (auto* member = dynamic_cast<ast::MemberExpression*>(expr)) {
        return borrowedRootName(member->object.get());
    }
    if (auto* element = dynamic_cast<ast::ArrayElementExpression*>(expr)) {
        return borrowedRootName(element->array.get());
    }
    return "";
}

SemanticAnalyzer::BorrowState SemanticAnalyzer::aggregateBorrowState(const std::string& rootName) const {
    BorrowState total;
    for (const auto& scope : borrowScopes) {
        auto it = scope.find(rootName);
        if (it != scope.end()) {
            total.mutableBorrows += it->second.mutableBorrows;
            total.immutableBorrows += it->second.immutableBorrows;
        }
    }
    return total;
}

bool SemanticAnalyzer::hasActiveBorrow(const std::string& rootName) const {
    BorrowState state = aggregateBorrowState(rootName);
    return state.mutableBorrows > 0 || state.immutableBorrows > 0;
}

void SemanticAnalyzer::recordBorrow(const std::string& rootName, ast::BorrowKind kind, const ast::Node* node) {
    if (rootName.empty()) {
        return;
    }
    if (borrowScopes.empty()) {
        borrowScopes.emplace_back();
    moveScopes.emplace_back();
    }

    BorrowState active = aggregateBorrowState(rootName);
    VYB_CDBG << "DEBUG: recordBorrow " << rootName
             << " kind=" << (kind == ast::BorrowKind::MUTABLE_BORROW ? "borrow" : "view")
             << " mutable=" << active.mutableBorrows
             << " immutable=" << active.immutableBorrows << std::endl;
    if (kind == ast::BorrowKind::MUTABLE_BORROW) {
        if (active.mutableBorrows > 0 || active.immutableBorrows > 0) {
            addError("Cannot take mutable borrow of '" + rootName + "' while it is already borrowed.", node);
            return;
        }
        borrowScopes.back()[rootName].mutableBorrows++;
        return;
    }

    if (active.mutableBorrows > 0) {
        addError("Cannot take view of '" + rootName + "' while it has an active mutable borrow.", node);
        return;
    }
    borrowScopes.back()[rootName].immutableBorrows++;
}

// record the borrow-lifetime of a `their<T>` variable: it was created by borrowing
// `root` at the current scope depth, so it is only valid within (or nested under)
// the current scope. Later code that uses it at a shallower depth (after this
// scope has exited) is a dangling-reference escape.
void SemanticAnalyzer::recordTheirBorrowInto(const std::string& varName, const std::string& root) {
    if (varName.empty() || root.empty()) return;
    theirVarDepth_[varName] = static_cast<int>(borrowScopes.size());
    theirVarRoot_[varName] = root;
}

// Move tracking helpers
bool SemanticAnalyzer::hasOwnershipKindMY(SymbolInfo* sym) const {
    if (!sym || !sym->type) return false;
    // Check if the type name starts with "my<" - this indicates MY ownership
    std::string typeStr = sym->type->toString();
    return typeStr.rfind("my<", 0) == 0;
}

ast::OwnershipKind SemanticAnalyzer::getOwnershipKind(SymbolInfo* sym) const {
    if (!sym) return ast::OwnershipKind::MY;
    return sym->ownershipKind;
}

// True if a type node denotes a MY-owned wrapper (`my<T>`).

// True if a type node denotes a `their<T>` borrowed reference type.

// #297: does `t` denote a Vec (i.e. an element of a `Vec<Vec<...>>` slot)?

// #297: the result type of a Vec element accessor (`get`). An element that is
// itself a Vec lives inside the container's data buffer, so `get` hands back a
// *borrow* of it (`their<Vec<T>>`) rather than a value -- the compiler then states
// the aliasing instead of codegen emulating a view. Scalars and structs keep the
// existing by-value result. Returns a fresh TypeNode.

void SemanticAnalyzer::recordMove(const std::string& varName) {
    if (!moveScopes.empty()) {
        moveScopes.back()[varName].isMoved = true;
    }
}

void SemanticAnalyzer::clearMove(const std::string& varName) {
    if (!moveScopes.empty()) {
        moveScopes.back()[varName].isMoved = false;
    }
}

bool SemanticAnalyzer::isMoved(const std::string& varName) const {
    // Walk scopes from innermost to outermost
    for (auto it = moveScopes.rbegin(); it != moveScopes.rend(); ++it) {
        auto vit = it->find(varName);
        if (vit != it->end()) {
            return vit->second.isMoved;
        }
    }
    return false;
}




} // namespace vyb
