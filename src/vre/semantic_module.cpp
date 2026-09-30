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

void SemanticAnalyzer::setModuleScoping(
    const std::unordered_map<std::string, std::string>& ownerByName,
    const std::unordered_map<std::string, std::unordered_set<std::string>>& effectiveScope) {
    scopeGate_.ownerByName = ownerByName;
    scopeGate_.effectiveScope = effectiveScope;
    defaultOwner_ = "";
    auto rootIt = ownerByName.find("main");
    if (rootIt != ownerByName.end()) {
        defaultOwner_ = rootIt->second;
    }
    scopeGate_.currentOwner = defaultOwner_;
}
void SemanticAnalyzer::analyze(ast::Module* root) {
    if (root) {
        root->accept(*this);
    }
}
void SemanticAnalyzer::visit(ast::Module* node) {
    // Two-pass analysis for error propagation:
    // Pass 1: Build function registry and detect explicit fail statements
    // Pass 2: Propagate failability transitively through function calls

    // First pass: Build registry and visit all declarations
    functionRegistry.clear();
    externalFunctionNames.clear();
    for (auto& item : node->body) {
        if (auto funcDecl = dynamic_cast<ast::FunctionDeclaration*>(item.get())) {
            if (funcDecl->id) {
                functionRegistry[funcDecl->id->name] = funcDecl;
            }
        } else if (auto namespaceDecl = dynamic_cast<ast::NamespaceDeclaration*>(item.get())) {
            if (namespaceDecl->name && namespaceDecl->name->name == "__extern_C") {
                for (auto& member : namespaceDecl->members) {
                    if (auto funcDecl = dynamic_cast<ast::FunctionDeclaration*>(member.get())) {
                        if (funcDecl->id) {
                            externalFunctionNames.insert(funcDecl->id->name);
                        }
                    }
                }
            }
        }
    }

    // Forward-reference pre-pass (#211): register every top-level function in
    // the module's global scope BEFORE any function body is analyzed, so a call
    // site can reference a function declared later in the same file and pairs
    // of mutually-recursive functions resolve regardless of declaration order.
    // This mirrors the LLVM codegen forward-declaration pass
    // (LLVMCodegen::visit(Module)) and separates name resolution from
    // implementation order. A genuine duplicate (two function declarations with
    // the same name in this scope) is reported here, once.
    forwardDeclaredFunctions_.clear();
    for (auto& item : node->body) {
        if (auto* funcDecl = dynamic_cast<ast::FunctionDeclaration*>(item.get())) {
            if (!funcDecl->id) continue;
            const std::string& fname = funcDecl->id->name;
            if (!forwardDeclaredFunctions_.insert(fname).second) {
                addError("Redefinition of function \"" + fname + "\" in the same scope.", funcDecl->id.get());
                continue;
            }
            currentScope->add(SymbolInfo{SymbolInfo::Kind::Function, fname,
                                         false, ast::OwnershipKind::MY, nullptr});
        }
    }

    // Forward-type-name pre-pass (#211): register every top-level struct,
    // non-constant enum, and type-alias NAME as a Type symbol in the module
    // scope before any declaration body is analyzed, so a type can be
    // referenced before its declaration and mutually-referential types resolve.
    // Mirrors the LLVM codegen type-ordering pass and the function pre-pass
    // above. Struct/alias duplicate definitions are reported here once (their
    // visitors would false-positive on every single pre-registered definition);
    // enums keep their prior overwrite-tolerant behavior.
    forwardDeclaredTypes_.clear();
    for (auto& item : node->body) {
        if (auto* sd = dynamic_cast<ast::StructDeclaration*>(item.get())) {
            if (!sd->name) continue;
            if (!forwardDeclaredTypes_.insert(sd->name->name).second) {
                addError("Redefinition of struct \"" + sd->name->name + "\" in the same scope.", sd->name.get());
                continue;
            }
            currentScope->add(SymbolInfo{SymbolInfo::Kind::Type, sd->name->name, false,
                ast::OwnershipKind::MY,
                retainType(new ast::TypeName(sd->name->loc, std::make_unique<ast::Identifier>(sd->name->loc, sd->name->name)))});
        } else if (auto* ed = dynamic_cast<ast::EnumDeclaration*>(item.get())) {
            if (!ed->name) continue;
            // Constant enums (every variant carries `= value`) register no Type
            // symbol (their visitor returns early) — they are namespace /
            // Int-constant sources, not types. Skip them to mirror that.
            bool anyValue = false;
            for (auto& v : ed->variants) if (v && v->hasValue) { anyValue = true; break; }
            if (anyValue) continue;
            if (!forwardDeclaredTypes_.insert(ed->name->name).second) continue; // overwrite-tolerant like the enum visitor
            currentScope->add(SymbolInfo{SymbolInfo::Kind::Type, ed->name->name, false,
                ast::OwnershipKind::MY,
                retainType(new ast::TypeName(ed->name->loc, std::make_unique<ast::Identifier>(ed->name->loc, ed->name->name)))});
        } else if (auto* ta = dynamic_cast<ast::TypeAliasDeclaration*>(item.get())) {
            if (!ta->name) continue;
            if (!forwardDeclaredTypes_.insert(ta->name->name).second) {
                addError("Redefinition of type alias \"" + ta->name->name + "\" in the same scope.", ta->name.get());
                continue;
            }
            currentScope->add(SymbolInfo{SymbolInfo::Kind::Type, ta->name->name, false,
                ast::OwnershipKind::MY,
                retainType(new ast::TypeName(ta->name->loc, std::make_unique<ast::Identifier>(ta->name->loc, ta->name->name)))});
        }
    }

    // Visit all top-level TYPE declarations (struct/enum/alias) first so their
    // records (fields, variants, type params, constants) are available to every
    // function body, signature, and top-level statement regardless of
    // declaration order (#211). Mirrors codegen, which emits type decls before
    // functions.
    {
        auto visitOwned = [&](auto& item2) {
            std::string owner = defaultOwner_;
            if (std::string name = topLevelDeclarationName(item2); !name.empty()) {
                auto ownerIt = scopeGate_.ownerByName.find(name);
                if (ownerIt != scopeGate_.ownerByName.end()) owner = ownerIt->second;
            }
            ownerStack_.push_back(scopeGate_.currentOwner);
            scopeGate_.currentOwner = owner;
            item2->accept(*this);
            scopeGate_.currentOwner = ownerStack_.back();
            ownerStack_.pop_back();
        };
        for (auto& item2 : node->body) {
            if (!item2) continue;
            if (dynamic_cast<ast::StructDeclaration*>(item2.get()) ||
                dynamic_cast<ast::EnumDeclaration*>(item2.get()) ||
                dynamic_cast<ast::TypeAliasDeclaration*>(item2.get())) {
                visitOwned(item2);
            }
        }
    }

    // Visit all remaining declarations (this detects explicit fail statements). Top-level
    // non-function code resolves against the owning module's scope so a global
    // initializer/expression in the entry file cannot name another module's
    // private dependency; function declarations manage their own owner.
    for (auto& item : node->body) {
        if (!item) continue;
        if (dynamic_cast<ast::FunctionDeclaration*>(item.get())) {
            item->accept(*this);
            continue;
        }
        if (dynamic_cast<ast::StructDeclaration*>(item.get()) ||
            dynamic_cast<ast::EnumDeclaration*>(item.get()) ||
            dynamic_cast<ast::TypeAliasDeclaration*>(item.get())) {
            continue;  // already visited in the type pass above
        }
        std::string owner = defaultOwner_;
        if (std::string name = topLevelDeclarationName(item); !name.empty()) {
            auto ownerIt = scopeGate_.ownerByName.find(name);
            if (ownerIt != scopeGate_.ownerByName.end()) {
                owner = ownerIt->second;
            }
        }
        ownerStack_.push_back(scopeGate_.currentOwner);
        scopeGate_.currentOwner = owner;
        item->accept(*this);
        scopeGate_.currentOwner = ownerStack_.back();
        ownerStack_.pop_back();
    }

    // Second pass: Propagate failability transitively
    bool changed = true;
    int iterations = 0;
    const int MAX_ITERATIONS = 100; // Prevent infinite loops

    while (changed && iterations < MAX_ITERATIONS) {
        changed = false;
        iterations++;

        for (auto& item : node->body) {
            if (auto funcDecl = dynamic_cast<ast::FunctionDeclaration*>(item.get())) {
                // Skip main function - it's the entry point and should handle errors explicitly
                if (funcDecl->id && funcDecl->id->name == "main") {
                    continue;
                }

                if (!funcDecl->canFail) {
                    // Check if this function calls any failable functions
                    bool callsFailableFunction = checkCallsFailableFunction(funcDecl->body.get());
                    if (callsFailableFunction) {
                        funcDecl->canFail = true;
                        funcDecl->needsErrorReturn = true;
                        if (funcDecl->errorTypes.empty()) {
                            funcDecl->errorTypes.push_back("Error");
                        }
                        changed = true;
                        VYB_CDBG << "DEBUG: Marked function '" << funcDecl->id->name
                                  << "' as failable (calls failable function)" << std::endl;
                    }
                }
            }
        }
    }

    if (iterations >= MAX_ITERATIONS) {
        std::cerr << "Warning: Error propagation analysis hit maximum iterations" << std::endl;
    }

    // Third pass: reject untrapped calls to failable functions from callers that
    // are still non-failable after propagation analysis.
    std::function<void(ast::Node*, bool, ast::FunctionDeclaration*)> validateCalls;
    validateCalls = [&](ast::Node* n, bool trapProtected, ast::FunctionDeclaration* owner) {
        if (!n) return;

        if (auto* call = dynamic_cast<ast::CallExpression*>(n)) {
            if (!trapProtected && owner && !owner->canFail) {
                if (auto* calleeIdent = dynamic_cast<ast::Identifier*>(call->callee.get())) {
                    auto it = functionRegistry.find(calleeIdent->name);
                    if (it != functionRegistry.end() && it->second && it->second->canFail) {
                        addError(
                            "Call to failable function '" + calleeIdent->name + "' from non-failable function '" +
                            owner->id->name + "' requires a trap block or marking the caller as failable.",
                            call
                        );
                    }
                }
            }
            for (auto& arg : call->arguments) {
                validateCalls(arg.get(), trapProtected, owner);
            }
            validateCalls(call->callee.get(), trapProtected, owner);
            return;
        }

        if (auto* block = dynamic_cast<ast::BlockStatement*>(n)) {
            for (auto& stmt : block->body) validateCalls(stmt.get(), trapProtected, owner);
            return;
        }

        if (auto* exprStmt = dynamic_cast<ast::ExpressionStatement*>(n)) {
            validateCalls(exprStmt->expression.get(), trapProtected, owner);
            return;
        }

        if (auto* varDecl = dynamic_cast<ast::VariableDeclaration*>(n)) {
            validateCalls(varDecl->init.get(), trapProtected, owner);
            return;
        }

        if (auto* retStmt = dynamic_cast<ast::ReturnStatement*>(n)) {
            validateCalls(retStmt->argument.get(), trapProtected, owner);
            return;
        }

        if (auto* ifStmt = dynamic_cast<ast::IfStatement*>(n)) {
            validateCalls(ifStmt->test.get(), trapProtected, owner);
            validateCalls(ifStmt->consequent.get(), trapProtected, owner);
            validateCalls(ifStmt->alternate.get(), trapProtected, owner);
            return;
        }

        if (auto* whileStmt = dynamic_cast<ast::WhileStatement*>(n)) {
            validateCalls(whileStmt->test.get(), trapProtected, owner);
            validateCalls(whileStmt->body.get(), trapProtected, owner);
            return;
        }

        if (auto* forStmt = dynamic_cast<ast::ForStatement*>(n)) {
            validateCalls(forStmt->init.get(), trapProtected, owner);
            validateCalls(forStmt->test.get(), trapProtected, owner);
            validateCalls(forStmt->update.get(), trapProtected, owner);
            validateCalls(forStmt->body.get(), trapProtected, owner);
            return;
        }

        if (auto* blockExpr = dynamic_cast<ast::BlockExpression*>(n)) {
            bool protectedBody = trapProtected || !blockExpr->trapClauses.empty();
            validateCalls(blockExpr->block.get(), protectedBody, owner);
            for (auto& trapClause : blockExpr->trapClauses) {
                if (trapClause && trapClause->handler) {
                    validateCalls(trapClause->handler.get(), trapProtected, owner);
                }
            }
            if (blockExpr->ensureClause && blockExpr->ensureClause->cleanupBlock) {
                validateCalls(blockExpr->ensureClause->cleanupBlock.get(), trapProtected, owner);
            }
            return;
        }
    };

    for (auto& item : node->body) {
        if (auto* funcDecl = dynamic_cast<ast::FunctionDeclaration*>(item.get())) {
            validateCalls(funcDecl->body.get(), false, funcDecl);
        }
    }

    // Validate aspect inheritance (super-aspect existence, no cycles) after all
    // declaratons have been visited and registered.
    validateAspectInheritance();
}

} // namespace vyb
