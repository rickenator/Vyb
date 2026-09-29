// SPDX-License-Identifier: Apache-2.0
//
// Seam extracted from src/vre/semantic.cpp (#347): the member definitions below are moved
// VERBATIM -- no reformatting, no renames -- so the split stays behaviour-neutral.
// Everything that is file-local (static helpers, macros) travels with them or is
// promoted into the shared internal header, never duplicated.

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
void SemanticAnalyzer::addError(const std::string& message, const ast::Node* node) {
    errors.push_back(message);
}
void SemanticAnalyzer::addWarning(const std::string& message, const ast::Node* node) {
    warnings.push_back(message);
    std::cerr << "warning: " << message << "\n";
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
bool SemanticAnalyzer::isInFreedomBlock() {
    SymbolTable* scope = currentScope;
    while (scope) {
        if (scope->isFreedomBlock) {
            return true;
        }
        scope = scope->getParent();
    }
    return false;
}
bool SemanticAnalyzer::isReservedWord(const std::string& name) {
    return reservedWords.count(name);
}

} // namespace vyb
