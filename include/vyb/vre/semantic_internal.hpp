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
#include <algorithm>
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



inline const char* kernelIntrinsicReturnType(const std::string& name) {
    if (name == "tid_x" || name == "tid_y" || name == "tid_z" ||
        name == "blk_x" || name == "blk_y" || name == "blk_z" ||
        name == "dim_x" || name == "dim_y" || name == "dim_z" ||
        name == "grid_x" || name == "grid_y" ||
        name == "lane_id" || name == "warp_size" ||
        name == "ld_i64") return "Int";
    if (name == "ld_f64" || name == "ld_f32" || name == "ld_f16" ||
        name == "ld_bf16" || name == "deq_q4_0" ||
        name == "ld_shared_f64" || name == "atomic_add_f64") return "Float";
    if (name == "ld_i32" || name == "atomic_add_i32") return "CInt";
    if (name == "ld_i8") return "Int8";
    if (name == "ld_u8") return "UInt8";
    if (name == "ld_i16") return "Int16";
    if (name == "ld_u16") return "UInt16";
    if (name == "st_f64" || name == "st_f32" || name == "st_i64" ||
        name == "st_i32" || name == "st_i8" || name == "st_u8" ||
        name == "st_i16" || name == "st_u16" || name == "st_f16" ||
        name == "st_bf16" || name == "st_shared_f64" ||
        name == "kernel_barrier") return "Void";
    return nullptr;
}


inline std::string intCanonicalNameForName(const std::string& name) {
    if (name == "Int" || name == "Int64" || name == "i64" ||
        name == "CLong" || name == "CSSize") return "s64";
    if (name == "UInt64" || name == "u64" || name == "CULong" || name == "CSize") return "u64";
    if (name == "Int32" || name == "i32" || name == "CInt" ||
        name == "Rune") return "s32";
    if (name == "UInt32" || name == "u32" || name == "CUInt") return "u32";
    if (name == "Int16" || name == "i16" || name == "CShort") return "s16";
    if (name == "UInt16" || name == "u16" || name == "CUShort") return "u16";
    if (name == "Int8" || name == "i8" || name == "Char" || name == "CChar") return "s8";
    if (name == "UInt8" || name == "u8" || name == "CUChar" || name == "Byte") return "u8";
    return "";
}



inline int integerTypeWidthForName(const std::string& name) {
    if (name == "Int" || name == "Int64" || name == "i64" ||
        name == "UInt64" || name == "u64" || name == "ULong") return 64;
    if (name == "Int32" || name == "i32" || name == "UInt32" || name == "u32") return 32;
    if (name == "Int16" || name == "i16" || name == "UInt16" || name == "u16") return 16;
    if (name == "Int8" || name == "i8" || name == "UInt8" || name == "u8" ||
        name == "Char" || name == "Byte") return 8;
    return 0;
}

inline int integerTypeWidth(ast::TypeNode* type) {
    if (!type) return 0;
    auto* tn = dynamic_cast<ast::TypeName*>(type);
    if (!tn || !tn->identifier) return 0;
    return integerTypeWidthForName(tn->identifier->name);
}

inline std::string intCanonicalName(ast::TypeNode* type) {
    if (!type) return "";
    auto* tn = dynamic_cast<ast::TypeName*>(type);
    if (!tn || !tn->identifier) return "";
    return intCanonicalNameForName(tn->identifier->name);
}

inline bool intCanonicalRange(const std::string& c, __int128& lo, __int128& hi) {
    int bits = 0;
    bool s = false;
    if (c == "s64" || c == "u64") { bits = 64; s = (c == "s64"); }
    else if (c == "s32" || c == "u32") { bits = 32; s = (c == "s32"); }
    else if (c == "s16" || c == "u16") { bits = 16; s = (c == "s16"); }
    else if (c == "s8" || c == "u8") { bits = 8; s = (c == "s8"); }
    else return false;
    if (s && bits == 64) {
        lo = INT64_MIN; hi = INT64_MAX;
    } else if (s) {
        lo = -(1LL << (bits - 1));
        hi = (1LL << (bits - 1)) - 1;
    } else if (bits == 64) {
        lo = 0; hi = (__int128)UINT64_MAX;
    } else {
        lo = 0; hi = (1LL << bits) - 1;
    }
    return true;
}

inline bool intConstantValueFull(ast::Expression* e, __int128& out, bool& isUnsigned) {
    if (!e) return false;
    if (auto lit = dynamic_cast<ast::IntegerLiteral*>(e)) {
        if (lit->isUnsigned) {
            out = (__int128)lit->uvalue;
            isUnsigned = true;
        } else {
            out = (__int128)lit->value;
            isUnsigned = false;
        }
        return true;
    }
    if (auto un = dynamic_cast<ast::UnaryExpression*>(e)) {
        if (un->op.type == TokenType::MINUS) {
            __int128 v;
            bool u;
            if (intConstantValueFull(un->operand.get(), v, u)) { out = -v; isUnsigned = u; return true; }
        }
    }
    return false;
}

inline std::string i128ToString(__int128 v) {
    if (v == 0) return "0";
    bool neg = v < 0;
    unsigned __int128 u = neg ? (unsigned __int128)(-(v + 1)) + 1 : (unsigned __int128)v;
    std::string s;
    while (u) {
        s.push_back(static_cast<char>('0' + (u % 10)));
        u /= 10;
    }
    if (neg) s.push_back('-');
    std::reverse(s.begin(), s.end());
    return s;
}



// Result of the explicit integer-assignment check at assignment/init sites.
enum class IntAssignCode { NotInteger, Ok, NeedExplicitCast, ConstantOutOfRange };

struct IntAssignCheck {
    IntAssignCode code;
    std::string message;
};

} // namespace vyb
