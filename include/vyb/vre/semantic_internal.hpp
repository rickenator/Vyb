// SPDX-License-Identifier: Apache-2.0
//
// Internal helpers shared by the semantic-analysis translation units
// (`semantic.cpp` and its extracted siblings). Not a public header: nothing
// outside `src/vre` includes it.
//
// Promoted from `semantic.cpp` (#347, checkpoint 3) so the ownership/borrow
// helper cluster can move into its own translation unit while `semantic.cpp`
// keeps calling the same helpers. Declared `inline` so each translation unit
// gets the definition without violating ODR; symbol visibility is unchanged
// from the old file-local `static` (nothing outside this directory sees them).

#pragma once

#include "vyb/parser/ast.hpp"

#include <iostream>

namespace vyb {
// Debug tracing toggle for the semantic pass. Declared here (rather than in a
// codegen header) so every semantic translation unit can use VYB_CDBG without
// pulling in the LLVM headers; it is defined by the driver.
extern bool g_debug_codegen;
} // namespace vyb

// Conditional debug stream: `VYB_CDBG << ...`.
#ifndef VYB_CDBG
#define VYB_CDBG if (::vyb::g_debug_codegen) std::cerr
#endif

namespace vyb {

// Map an AST node (possibly null) to its stable TypeTable key.
// `Node::typeId()` starts at 1, so 0 is a safe sentinel for a null node key
// (a null key previously hit `map.find(nullptr) -> end`; now it maps to key 0).
inline unsigned exprKey(const vyb::ast::Node* n) { return n ? n->typeId() : 0; }

inline bool isMyOwnershipType(const ast::TypeNode* type) {
    if (!type) return false;
    std::string typeStr = type->toString();
    return typeStr.rfind("my<", 0) == 0;
}

inline bool isTheirType(const ast::TypeNode* type) {
    if (!type) return false;
    return type->toString().rfind("their<", 0) == 0;
}

inline bool isVecTypeNodeAst(const ast::TypeNode* t) {
    if (!t) return false;
    if (dynamic_cast<const ast::VecType*>(t)) return true;
    if (auto* tn = dynamic_cast<const ast::TypeName*>(t))
        return tn->identifier && tn->identifier->name == "Vec";
    return false;
}

inline std::shared_ptr<ast::TypeNode> vecGetResultType(const SourceLocation& loc,
                                                       ast::TypeNode* element) {
    if (!element) return nullptr;
    if (isVecTypeNodeAst(element)) {
        std::vector<ast::TypeNodePtr> args;
        args.push_back(ast::TypeNodePtr(element->clone()));
        auto theirName = std::make_unique<ast::TypeName>(
            loc, std::make_unique<ast::Identifier>(loc, "their"), std::move(args));
        return std::shared_ptr<ast::TypeNode>(std::move(theirName));
    }
    return std::shared_ptr<ast::TypeNode>(element->clone());
}

} // namespace vyb
