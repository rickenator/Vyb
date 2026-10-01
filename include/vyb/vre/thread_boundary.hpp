// SPDX-License-Identifier: Apache-2.0
//
// Thread-boundary capability as a structural property of a type (#149, #365).
//
// `handoffCapable` asks, for a declared type, "may a value of this type cross a
// thread boundary as a handoff?": unique owners (`my<T>`), borrows (`their<T>`,
// `loc<T>`), raw pointers and anything unresolved are never capable; shared
// ownership (`our<T>`, `mild<T>`) and composites (`Vec<T>`, arrays, `T?`,
// futures, tuples, generic named types) are capable iff every payload is; a
// named struct or enum is capable iff every field / variant payload is.
//
// The predicate is deliberately registry-parameterized. Both compiler passes ask
// this question at different times -- the semantic pass when it checks a
// spawn-site closure, and codegen at the same call sites once a generic
// function's type parameters have been substituted. The semantic pass owns the
// structural registries (they are built while its AST walk runs); codegen has
// only the substitutable type strings. Passing the registries in keeps ONE
// implementation instead of a copy that can drift.
//
// A null registry means "no structural view available": named structs and enums
// are then treated as unresolved and stay permissive, exactly as an unresolved
// type parameter does. This is the codegen-side limit today -- a substituted type
// parameter standing for a *named* struct whose fields include a `my<T>` is still
// accepted there; closing that needs the registries shared with codegen
// (doc/THREAD_BOUNDARY_SCOPE.md, (c)).

#pragma once

#include "vyb/parser/ast.hpp"

#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace vyb {
namespace thread_boundary {

// Struct name -> field name -> declared field type. Borrowed from
// SemanticAnalyzer::structFieldTypes; pointers stay valid for the pass's lifetime.
using StructFieldRegistry = std::unordered_map<std::string, std::map<std::string, ast::TypeNode*>>;

// Enum name (or concrete type string) -> variant name -> payload types.
using EnumPayloadRegistry =
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<ast::TypeNodePtr>>>;

// The curated claims (`bind Handoff -> T` / `bind Viewable -> T`, `core::aspects`):
// the reviewed escape hatch for a shape the structural walk cannot see through or
// gets wrong about. The walk asks these at EVERY level, not only at the top, so a
// claim on `Box` also covers `our<Box>`, `Vec<Box>` and a struct field of type
// `Box`. Before that, the recursion consulted no claims at all, so a wrapper hid
// the claim and `our<Box>` -- the natural way to share a curated type -- stayed
// refused even though `Box` was bound.
//
// Each predicate is asked with the type string exactly as written at that level,
// which is how registration keyed the claim (the analyzer also matches generic
// patterns, so `bind Handoff -> Box<T>` covers `Box<Int>`).
//
// Null means "this pass has no curated claims"; a null member means the same for
// that aspect. `capability` consults `handoff` only -- a `Viewable` claim is the
// weaker relation and must never make a value handoff-capable.
struct CuratedClaimLookup {
    std::function<bool(const std::string&)> handoff;
    std::function<bool(const std::string&)> viewable;
};

// May a value of `type` cross a thread boundary as a handoff? (See the header
// comment for the rules and for what a null registry means.)
bool handoffCapable(const ast::TypeNode* type,
                    const StructFieldRegistry* structs,
                    const EnumPayloadRegistry* enums,
                    const CuratedClaimLookup* claims = nullptr);

// The same question, but able to say "I cannot decide": the semantic pass runs
// before monomorphization, so a type that still mentions a type parameter is not
// decidable *there*, while codegen -- which holds the substitutions -- can decide
// it. `Undecidable` therefore also covers a named type that is neither a known
// struct/enum nor a plain value type, because a later pass with the same
// registries cannot resolve it either.
//
// Callers use this to *defer* rather than to guess: the semantic pass records the
// capture for codegen to judge on the substituted type, instead of reporting an
// error it cannot justify or admitting something it cannot check.
enum class Capability {
    Capable,
    NotCapable,
    Undecidable,
};

Capability capability(const ast::TypeNode* type,
                      const StructFieldRegistry* structs,
                      const EnumPayloadRegistry* enums,
                      const CuratedClaimLookup* claims = nullptr);

// The weaker relation: may a second thread *read* a value of this type without
// taking ownership? A handoff-capable value is viewable, and a borrow of a
// handoff-capable payload is viewable because a reader only needs the address.
bool viewable(const ast::TypeNode* type,
              const StructFieldRegistry* structs,
              const EnumPayloadRegistry* enums,
              const CuratedClaimLookup* claims = nullptr);

} // namespace thread_boundary
} // namespace vyb
