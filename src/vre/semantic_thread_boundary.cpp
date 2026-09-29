// SPDX-License-Identifier: Apache-2.0
//
// Thread-boundary capability (#149, #365).
//
// #149 shipped the first cut of the cross-thread check as a *name* test: it
// looked at a captured variable's declared type string and rejected anything
// starting with `my<` / `their<`. That misses the interesting cases, because
// the wrapper is usually not at the top level:
//
//   * `Vec<my<T>>`, `my<T>?`, `Result<my<T>, E>`, a struct or enum with a
//     `my<T>` / `their<T>` / raw-pointer payload -- all crossed the boundary;
//   * a *mutable* capture, which holds the defining frame's address, regardless
//     of the type of the variable;
//   * nothing at all was said about the closure's own captures when the same
//     closure was handed to a spawn site other than `thread_spawn`.
//
// This translation unit derives the capability structurally from the type
// graph (scalars are capable; shared ownership is capable iff its payload is;
// composites are capable iff every payload is; unique owners, borrows and raw
// pointers are never capable) and enforces it at the spawn site. It also
// borrows its vocabulary from the ownership rules the language already has --
// a value is `handoff`-capable when it can be handed to another thread, and
// `viewable` when a second thread may read it -- rather than from another
// language's trait names.
//
// Behaviour of the fix mirrors the docs gate the maintainer landed for it:
// renames only, no new syntax. The gate is error-only for now; the provisional
// wording ("not handoff-capable") is deliberate while the drop-semantics row
// for propagation paths is still open.

#include "vyb/semantic.hpp"
#include "vyb/parser/ast.hpp"

#include <set>
#include <string>

namespace vyb {

namespace {

// Strip generic arguments from a type string: `Box<Int>` -> `Box`.
std::string baseNameOf(const std::string& typeStr) {
    const auto lt = typeStr.find('<');
    return lt == std::string::npos ? typeStr : typeStr.substr(0, lt);
}

// Plain value types: no heap owner, no borrowed address -- trivially
// handoff-capable (a copy of the value crosses the boundary).
bool isPlainValueType(const std::string& base) {
    static const std::set<std::string> plain = {
        "Int", "Int8", "Int16", "Int32", "Int64", "Int128",
        "UInt", "UInt8", "UInt16", "UInt32", "UInt64",
        "Float", "Float32", "Float64", "Bool", "Char", "Rune",
        "String", "Type", "Byte", "Bytes", "Unit", "Void", "Nothing",
    };
    return plain.count(base) > 0;
}

// Nearest named-type arguments: `Box<Int, String>` -> {Int, String}.
std::vector<const ast::TypeNode*> namedTypeArgs(const ast::TypeNode* type) {
    std::vector<const ast::TypeNode*> args;
    if (auto* tn = dynamic_cast<const ast::TypeName*>(type)) {
        for (const auto& a : tn->genericArgs) args.push_back(a.get());
    }
    return args;
}

} // namespace

// May a value of this type cross a thread boundary as a handoff?
//
// Rules, in order:
//   * no type / raw pointer            -> no
//   * my<T>, their<T>, loc<T>, ptr<T>  -> no (unique owner or borrowed address,
//       and for `loc<T>` the address belongs to the defining frame)
//   * our<T>, mild<T>                  -> yes iff T is (shared ownership is
//       safe to hand over, but a shared owner of a unique owner is not)
//   * Vec<T>, [T], T?, future<T>, tuple -> yes iff every payload is
//   * Result<T, E>, Pair<A, B>, ...    -> yes iff every generic argument is
//   * a known enum                     -> yes iff every variant payload is
//   * a known struct                   -> yes iff every field is
//   * a plain value type               -> yes
//   * anything unresolved (a type parameter `T`, an opaque handle) -> yes,
//       conservatively: the gate is error-only and must not reject programs
//       it cannot reason about (resolution of type parameters through a
//       monomorphized bind is a tracked follow-up).
bool SemanticAnalyzer::handoffCapable(const ast::TypeNode* type) const {
    if (!type) return false;
    if (dynamic_cast<const ast::PointerType*>(type)) return false;

    const std::string ts = type->toString();
    const std::string base = baseNameOf(ts);

    if (base == "my" || base == "their" || base == "loc" || base == "ptr") return false;
    if (base == "our" || base == "mild") {
        const auto args = namedTypeArgs(type);
        if (args.empty()) return false;
        for (const auto* a : args) {
            if (!handoffCapable(a)) return false;
        }
        return true;
    }

    if (auto* v = dynamic_cast<const ast::VecType*>(type)) {
        return handoffCapable(v->elementType.get());
    }
    if (auto* a = dynamic_cast<const ast::ArrayType*>(type)) {
        return handoffCapable(a->elementType.get());
    }
    if (auto* f = dynamic_cast<const ast::FutureType*>(type)) {
        return handoffCapable(f->resultType.get());
    }
    if (auto* t = dynamic_cast<const ast::TupleTypeNode*>(type)) {
        for (const auto& m : t->memberTypes) {
            if (!handoffCapable(m.get())) return false;
        }
        return true;
    }

    // Optional is usually parsed as a TypeName carrying one generic argument
    // (`Int?`), but both spellings are handled.
    if (auto* o = dynamic_cast<const ast::OptionalType*>(type)) {
        return handoffCapable(o->containedType.get());
    }

    {
        const auto args = namedTypeArgs(type);
        if (!args.empty()) {
            // Any other generic named type (Result<T, E>, Pair<A, B>, a user
            // generic struct/enum): structural over its arguments.
            for (const auto* a : args) {
                if (!handoffCapable(a)) return false;
            }
        }
    }

    // Enums: keyed by the concrete type string (`Box<Int>`) or the bare name.
    auto eit = enumVariantPayloadTypes.find(ts);
    if (eit == enumVariantPayloadTypes.end()) eit = enumVariantPayloadTypes.find(base);
    if (eit != enumVariantPayloadTypes.end()) {
        for (const auto& variant : eit->second) {
            for (const auto& payload : variant.second) {
                if (!handoffCapable(payload.get())) return false;
            }
        }
        return true;
    }

    // Structs: keyed by name; fields are inspected structurally.
    auto sit = structFieldTypes.find(ts);
    if (sit == structFieldTypes.end()) sit = structFieldTypes.find(base);
    if (sit != structFieldTypes.end()) {
        for (const auto& field : sit->second) {
            if (!handoffCapable(field.second)) return false;
        }
        return true;
    }

    if (isPlainValueType(base)) return true;

    // Unresolved named type (type parameter, opaque handle): stay permissive.
    return true;
}

// May a second thread read a value of this type without taking ownership?
// The weaker relation: a handoff-capable value is also viewable, and a borrow
// (`their<T>` / `loc<T>`) of a handoff-capable payload is viewable because a
// reader only needs the address, not the owner.
bool SemanticAnalyzer::viewable(const ast::TypeNode* type) const {
    if (!type) return false;
    const std::string base = baseNameOf(type->toString());
    if (base == "their" || base == "loc") {
        const auto args = namedTypeArgs(type);
        if (args.empty()) return false;
        return handoffCapable(args.front());
    }
    return handoffCapable(type);
}

// Reject a closure handed to another thread that captures anything it may not.
void SemanticAnalyzer::checkThreadBoundaryCaptures(ast::FunctionExpression* fe,
                                                   ast::Node* site,
                                                   const std::string& siteName) {
    if (!fe) return;

    std::set<std::string> reported;

    // A mutable capture holds the defining frame's address, so it is not a
    // handoff no matter what the variable's type is.
    for (const std::string& cap : fe->mutableCapturedVariables) {
        if (!reported.insert(cap).second) continue;
        addError(siteName + ": closure captures '" + cap +
                     "' mutably -- a mutable capture holds the defining frame's address and "
                     "cannot cross a thread boundary (not handoff-capable). Capture it by "
                     "value, or share it as our(" + cap + ").",
                 site);
    }

    for (const std::string& cap : fe->capturedVariables) {
        if (reported.count(cap)) continue;
        if (!reported.insert(cap).second) continue;

        SymbolInfo* csym = currentScope ? currentScope->lookup(cap) : nullptr;
        if (!csym || !csym->type) continue;
        const ast::TypeNode* ty = csym->type.get();
        if (handoffCapable(ty)) continue;
        // `viewable` deliberately does NOT relax this site on its own. A borrow
        // (`their<T>` / `loc<T>`) addresses the *spawner's frame*: the borrow
        // model is lexical, so nothing proves the frame outlives a detached
        // thread's read, and letting a read-only borrow through unconditionally
        // would reopen the dangling read that
        // `test/ownership/thread_send_their.vyb` pins (#149).
        //
        // Rule (b) -- admitting a read-only borrow whose owner the closure also
        // captures -- is *scoped but not landed*: with the owner retained the
        // lifetime argument holds, but the closure-capture lowering reads a
        // captured `their<T>` as the address rather than through it, so the
        // borrow still produces garbage in the thread (probe:
        // `thread: ro.n=99157966951504` while `hold.n=7`). Landing (b) requires
        // fixing that capture lowering first; see
        // doc/THREAD_BOUNDARY_SCOPE.md.

        const std::string ts = ty->toString();
        addError(siteName + ": '" + cap + "' is " + ts +
                     " -- it cannot cross a thread boundary (not handoff-capable). "
                     "Hand it off as shared ownership (our(" + cap + ")) or pass a copy.",
                 site);
    }
}

} // namespace vyb
