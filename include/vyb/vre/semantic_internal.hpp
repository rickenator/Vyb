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
#include <set>
#include <string>

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



// Ownership-wrapped types whose runtime representation is a plain scalar value.
// At codegen time, ownership wrappers over primitives keep the underlying type
// (e.g. my<Int> is just an i64), so semantic analysis treats them as transparent.
inline const std::set<std::string> ownershipWrapperTypes = {
    "my", "our", "their", "mild", "view", "borrow"
};


inline const std::set<std::string> primitiveValueTypes = {
    "Int", "Int8", "Int16", "Int32", "Int64",
    "UInt8", "UInt16", "UInt32", "UInt64",
    "Float", "Float32", "Float64", "Bool", "Char", "Rune",
    "i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64", "f32", "f64",
    "CChar", "CUChar", "CShort", "CUShort", "CInt", "CUInt", "CLong", "CULong",
    "CSize", "CSSize", "CFloat", "CDouble", "CVoid", "CString"
};


// If `type` is an ownership wrapper over a primitive value type, return the
// underlying primitive type; otherwise return `type` unchanged.
inline ast::TypeNode* unwrapPrimitiveOwnershipType(ast::TypeNode* type) {
    if (!type) return nullptr;
    auto* typeName = dynamic_cast<ast::TypeName*>(type);
    if (!typeName || !typeName->identifier || typeName->genericArgs.size() != 1 ||
        !ownershipWrapperTypes.count(typeName->identifier->name)) {
        return type;
    }
    ast::TypeNode* inner = typeName->genericArgs[0].get();
    auto* innerName = dynamic_cast<ast::TypeName*>(inner);
    if (innerName && innerName->identifier &&
        primitiveValueTypes.count(innerName->identifier->name)) {
        return inner;
    }
    return type;
}


// Strip a leading ownership wrapper (`their<T>` / `my<T>` / `view<T>` / `our<T>` /
// `borrow<T>` / `mild<T>`) from a rendered type string, returning the inner type.
// Aspect/bind dispatch is keyed on the base struct, so a borrowed receiver must
// resolve through the same trait-impl tables as an owned one. Bracket depth is
// tracked so `their<Card<Int>>` unwraps to `Card<Int>` (not the truncated
// `Card<Int`), and repeated wrappers (`their<their<T>>`) are peeled too.
inline std::string unwrapOwnershipTypeString(const std::string& type) {
    std::string result = type;
    while (true) {
        size_t anglePos = result.find('<');
        if (anglePos == std::string::npos) break;
        if (!ownershipWrapperTypes.count(result.substr(0, anglePos))) break;
        // Find the '>' matching the wrapper's opening '<'.
        int depth = 0;
        size_t closeAnglePos = std::string::npos;
        for (size_t i = anglePos; i < result.size(); ++i) {
            if (result[i] == '<') {
                ++depth;
            } else if (result[i] == '>') {
                if (--depth == 0) { closeAnglePos = i; break; }
            }
        }
        if (closeAnglePos == std::string::npos || closeAnglePos <= anglePos + 1) break;
        result = result.substr(anglePos + 1, closeAnglePos - anglePos - 1);
    }
    return result;
}

} // namespace vyb
