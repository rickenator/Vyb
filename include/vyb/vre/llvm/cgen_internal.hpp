// SPDX-License-Identifier: Apache-2.0
//
// Shared helpers for the codegen translation units (#347).
//
// cgen_expr.cpp used to be one monolith whose helpers every lowering path could
// reach, because they were file-local statics in it. Splitting the monolith by
// seam means those helpers need one definition several TUs can see: they live
// here as `inline`, so program-wide there is still exactly one definition.
//
// Nothing in this header is new code: every definition below is moved verbatim.

#include "vyb/vre/llvm/codegen.hpp"
#include "vyb/parser/ast.hpp"
#include "vyb/vre/thread_boundary.hpp"
#include "vyb/parser/token.hpp" // For TokenType in BinaryExpression
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/DerivedTypes.h> // For PointerType, StructType
#include <llvm/ADT/APFloat.h>   // For APFloat in FloatLiteral
#include <llvm/ADT/APInt.h>     // For APInt in IntegerLiteral
#include <regex>                // For regex in MemberExpression loaded value handling

namespace vyb {


// issue #343: infer a generic function's type arguments from WHERE the type
// parameter appears in the DECLARED parameter type, instead of from argument
// position. Positional inference binds the whole argument type to the type
// parameter, so `f<T>(b<Box<T>>)` called with `Box<Int>` monomorphized as
// `f_Box_Int` whose signature was `Box<Box<Int>>`, and the call then failed with
// "Argument type mismatch for call to f_Box_Int_. Expected Box_Box_Int but got
// Box_Int". Walk the declared parameter type and the argument's type in
// parallel; every type parameter met inside the pattern owns its own slot.
inline void inferGenericArgsFromPattern(const ast::TypeNode* paramType,
                                        const ast::TypeNode* argType,
                                        const std::vector<std::string>& typeParamNames,
                                        std::vector<std::string>& concreteTypeArgs) {
    if (!paramType || !argType) return;

    if (auto ptn = dynamic_cast<const ast::TypeName*>(paramType)) {
        // A bare type parameter (`x<T>`) unifies with the argument's whole type.
        if (ptn->identifier && ptn->genericArgs.empty()) {
            for (size_t p = 0; p < typeParamNames.size() && p < concreteTypeArgs.size(); ++p) {
                if (typeParamNames[p] == ptn->identifier->name) {
                    if (concreteTypeArgs[p].empty()) {
                        concreteTypeArgs[p] = argType->toString();
                    }
                    return;
                }
            }
        }
        // `Box<T>` against `Box<Int>`: recurse positionally through the generic
        // arguments. The base names must agree and the arity must match.
        if (auto atn = dynamic_cast<const ast::TypeName*>(argType)) {
            if (ptn->identifier && atn->identifier &&
                ptn->identifier->name == atn->identifier->name &&
                ptn->genericArgs.size() == atn->genericArgs.size()) {
                for (size_t k = 0; k < ptn->genericArgs.size(); ++k) {
                    inferGenericArgsFromPattern(ptn->genericArgs[k].get(),
                                                atn->genericArgs[k].get(),
                                                typeParamNames, concreteTypeArgs);
                }
            }
        }
        return;
    }
    if (auto pv = dynamic_cast<const ast::VecType*>(paramType)) {
        if (auto av = dynamic_cast<const ast::VecType*>(argType)) {
            inferGenericArgsFromPattern(pv->elementType.get(), av->elementType.get(),
                                        typeParamNames, concreteTypeArgs);
        }
        return;
    }
    if (auto po = dynamic_cast<const ast::OptionalType*>(paramType)) {
        if (auto ao = dynamic_cast<const ast::OptionalType*>(argType)) {
            inferGenericArgsFromPattern(po->containedType.get(), ao->containedType.get(),
                                        typeParamNames, concreteTypeArgs);
        }
        return;
    }
    // #385/#456: a closure-typed parameter -- `h<T>(f<fn(T) -> T>)` called with
    // `|x<Int>| -> x + 1`. Without this the bare-parameter case above binds T to the
    // argument's WHOLE type ("fn(Int) -> Int"), so monomorphization instantiates a
    // signature with a closure type where Int belongs. Unify the pieces instead.
    if (auto pf = dynamic_cast<const ast::FunctionType*>(paramType)) {
        if (auto af = dynamic_cast<const ast::FunctionType*>(argType)) {
            if (pf->parameterTypes.size() == af->parameterTypes.size()) {
                for (size_t k = 0; k < pf->parameterTypes.size(); ++k) {
                    inferGenericArgsFromPattern(pf->parameterTypes[k].get(), af->parameterTypes[k].get(),
                                                typeParamNames, concreteTypeArgs);
                }
            }
            inferGenericArgsFromPattern(pf->returnType.get(), af->returnType.get(),
                                        typeParamNames, concreteTypeArgs);
        }
        return;
    }
    if (auto pf = dynamic_cast<const ast::FutureType*>(paramType)) {
        if (auto af = dynamic_cast<const ast::FutureType*>(argType)) {
            inferGenericArgsFromPattern(pf->resultType.get(), af->resultType.get(),
                                        typeParamNames, concreteTypeArgs);
        }
        return;
    }
    if (auto pp = dynamic_cast<const ast::PointerType*>(paramType)) {
        if (auto ap = dynamic_cast<const ast::PointerType*>(argType)) {
            inferGenericArgsFromPattern(pp->pointeeType.get(), ap->pointeeType.get(),
                                        typeParamNames, concreteTypeArgs);
        }
        return;
    }
    if (auto pt = dynamic_cast<const ast::TupleTypeNode*>(paramType)) {
        if (auto at = dynamic_cast<const ast::TupleTypeNode*>(argType)) {
            if (pt->memberTypes.size() == at->memberTypes.size()) {
                for (size_t k = 0; k < pt->memberTypes.size(); ++k) {
                    inferGenericArgsFromPattern(pt->memberTypes[k].get(),
                                                at->memberTypes[k].get(),
                                                typeParamNames, concreteTypeArgs);
                }
            }
        }
        return;
    }
}


// Is this a sized unsigned integer type name?
inline bool isUnsignedIntName(const std::string& n) {
    return n == "UInt8" || n == "UInt16" || n == "UInt32" || n == "UInt64" ||
           n == "u8" || n == "u16" || n == "u32" || n == "u64" ||
           n == "Byte" || n == "CUChar" || n == "CUShort" || n == "CUInt" ||
           n == "CULong" || n == "CSize";
}


// #297: a `their<Vec<T>>` receiver that is a CALL result (e.g. `get` on a
// `Vec<Vec<T>>` slot) already carries the borrowed Vec* -- the borrow is the value
// in flight. A storage location instead (an identifier or a field slot) evaluates
// in LHS mode to its slot address (a `Vec**`) and needs one load to recover the
// pointer. Distinguish the two by the receiver's shape.
inline bool receiverIsBorrowValue(const vyb::ast::Expression* e) {
    return dynamic_cast<const vyb::ast::CallExpression*>(e) != nullptr;
}


// A Vec method that only reads the receiver's header, so a *fresh temporary*
// receiver can be released as soon as the call returns (e.g.
// `payload.split("\n").len()` -- the temp's buffer is unreachable afterwards).
// Element accessors (`get`/`first`/`last`/`peek`/`get_vec`) and the mutators
// (`push`/`set`/`insert`/`remove_at`/`clear`) are deliberately excluded: they hand
// the caller a view of, or the address of, the receiver's own buffer, which the
// caller may keep past this statement.
inline bool vecReadMethodSafeForTempReceiver(const std::string& m) {
    return m == "len" || m == "is_empty" || m == "capacity";
}


// #382: an element accessor can qualify for the same reclaim once what it hands
// back stops aliasing the receiver's storage. That holds for `get`/`first`/`last`/
// `peek` on every element kind except a `Vec<Vec<T>>` slot: a primitive is loaded
// by value, a struct element is deep-copied (`generateStructDeepCopy`), and a
// String element is retained by the accessor itself (`handleVecGet`/
// `handleVecLast`). The borrow case cannot be told from the receiver's element type
// alone (`Vec<Vec<T>>` and `Vec<String>` both look like "a Vec"), so the call site
// tests the accessor's *result* type instead: `borrowedVecInnerNode(result) ==
// nullptr` is exactly "this result is not a borrow into the receiver".
inline bool vecElementAccessorValueMethod(const std::string& m) {
    return m == "get" || m == "first" || m == "last" || m == "peek";
}


// True when a channel's payload element type is a Vyb String. Numeric /
// Bool / Char / Float payloads use the int-slot channel runtime; String payloads
// use the refcounted string channel runtime.
inline bool chanElementIsString(const vyb::ast::TypeNode* tn) {
    if (!tn) return false;
    if (auto* nn = dynamic_cast<const vyb::ast::TypeName*>(tn)) {
        if (nn->identifier) {
            const std::string& n = nn->identifier->name;
            if (n == "String" || n == "string") return true;
        }
    }
    return false;
}


// An owning temporary freshly created inline as a call argument is owned solely
// by the call site. `our(...)` (a strong owner) and `soft(x)` (a weak owner)
// return refcounted control blocks: by-value `our`/`mild` parameters retain
// their OWN independent count on entry (a non-consuming copy), so the temporary's
// own count must be released once the callee returns or its control block is
// leaked. A fresh `my(...)` payload (e.g. `my(Struct{...})`) is a new heap
// allocation with no named binding; `my` parameters behave as borrows that do
// not free, so the caller must free the discarded temporary after the call.
// Shared reads (bare identifiers / member borrows) and non-owning values return
// 0 here. Returns 1 = release our, 2 = release mild/soft, 3 = free my payload
// (caller-owned fresh allocation), 0 = none.
inline int freshOwningCallArgKind(ast::Expression* expr) {
    auto* call = dynamic_cast<ast::CallExpression*>(expr);
    if (!call) return 0;
    auto* id = dynamic_cast<ast::Identifier*>(call->callee.get());
    if (!id) return 0;
    if (id->name == "our") return 1;
    if (id->name == "soft") return 2;
    if (id->name == "my") return 3;
    return 0;
}

// ---------------------------------------------------------------------------
// Vec<T> AST-type predicates (#347 seam: promoted from cgen_ownership.cpp so
// the main()-return serializer in cgen_stmt.cpp can ask the same questions).
// ---------------------------------------------------------------------------

// The name of a `TypeName` type node ("" for anything else).
inline std::string ownedFieldTypeBase(const ast::TypeNode* tn) {
    if (!tn) return "";
    if (auto* nn = dynamic_cast<const ast::TypeName*>(tn)) {
        return nn->identifier ? nn->identifier->name : "";
    }
    return "";
}

// Is `tn` a Vec type, written either as a `VecType` or as `Vec<T>`?
inline bool isVecTypeNode(const ast::TypeNode* tn) {
    if (dynamic_cast<const ast::VecType*>(tn)) return true;
    return ownedFieldTypeBase(tn) == "Vec";
}

// Element TypeNode of a `Vec<T>` AST type (`T`), or nullptr when `tn` is not a
// Vec type. Shared by the String and owned-struct element reclaim paths.
inline const ast::TypeNode* vecElementTypeNode(const ast::TypeNode* tn) {
    if (!tn) return nullptr;
    if (auto* vt = dynamic_cast<const ast::VecType*>(tn)) {
        return vt->elementType.get();
    }
    if (auto* name = dynamic_cast<const ast::TypeName*>(tn)) {
        if (name->identifier && name->identifier->name == "Vec" && !name->genericArgs.empty()) {
            return name->genericArgs[0].get();
        }
    }
    return nullptr;
}

} // namespace vyb
