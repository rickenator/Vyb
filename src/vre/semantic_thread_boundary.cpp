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
// This translation unit derives the capability structurally from the type graph
// (scalars are capable; shared ownership is capable iff its payload is;
// composites are capable iff every payload is; unique owners, borrows and raw
// pointers are never capable). The predicate itself lives in
// `vyb::thread_boundary` and is registry-parameterized, so codegen can run the
// SAME implementation at the spawn call sites once a generic function's type
// parameters have been substituted -- step (c) of #365; see
// vyb/vre/thread_boundary.hpp. This file adds the semantic-pass gate on top:
// every capture of a closure handed to a site whose closure leaves the spawner's
// OS thread is checked, and the rule-(b) case (a read-only borrow whose owner the
// closure also captures) is admitted.
//
// Vocabulary is borrowed from the ownership rules the language already has -- a
// value is `handoff`-capable when it can be handed to another thread, and
// `viewable` when a second thread may read it -- rather than from another
// language's trait names. The gate is error-only; the provisional wording
// ("not handoff-capable") is deliberate while the drop-semantics row for
// propagation paths is still open.

#include "vyb/semantic.hpp"
#include "vyb/parser/ast.hpp"
#include "vyb/vre/thread_boundary.hpp"

#include <set>
#include <string>
#include <vector>

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

namespace thread_boundary {

// The structural capability predicate (see the header for the contract).
//
// Rules, in order:
//   * no type / raw pointer            -> NotCapable
//   * my<T>, their<T>, loc<T>, ptr<T>  -> NotCapable (unique owner or borrowed
//       address, and for `loc<T>` the address belongs to the defining frame)
//   * our<T>, mild<T>                  -> Capable iff T is (shared ownership is
//       safe to hand over, but a shared owner of a unique owner is not)
//   * Vec<T>, [T], T?, future<T>, tuple -> iff every payload is
//   * Result<T, E>, Pair<A, B>, ...    -> iff every generic argument is
//   * a known enum                     -> iff every variant payload is
//   * a known struct                   -> iff every field is
//   * a plain value type               -> Capable
//   * `bind Handoff -> T` at any level -> Capable (the curated claim: the reviewed
//       escape hatch, asked with the type string as written at every level of the
//       walk, so a claim on `Box` also covers `our<Box>` and `Vec<Box>`)
//   * anything neither registry knows  -> Undecidable (a type parameter before
//       monomorphization, or an opaque handle in either pass). The semantic pass
//       DEFERS those captures to codegen rather than admitting them silently;
//       codegen judges the substituted type with the same registries and stays
//       permissive only if it is still undecidable. The gate is error-only and
//       must never reject a program it cannot reason about.
Capability capability(const ast::TypeNode* type,
                      const StructFieldRegistry* structs,
                      const EnumPayloadRegistry* enums,
                      const CuratedClaimLookup* claims) {
    using C = Capability;

    if (!type) return C::NotCapable;
    if (dynamic_cast<const ast::PointerType*>(type)) return C::NotCapable;

    const std::string ts = type->toString();
    const std::string base = baseNameOf(ts);

    if (base == "my" || base == "their" || base == "loc" || base == "ptr") return C::NotCapable;

    // The curated claim (`bind Handoff -> T`), asked at EVERY level of this walk --
    // not only at the top -- so a claim on `Box` also covers `our<Box>`, `Vec<Box>`
    // and a struct field of type `Box`. It is asked after the refusal wrappers
    // above, so a claim rescues a *composition* without ever turning a `my<T>` /
    // `their<T>` / `loc<T>` / `ptr<T>` written as the type itself into a handoff.
    if (claims && claims->handoff && claims->handoff(ts)) return C::Capable;

    if (base == "our" || base == "mild") {
        const auto args = namedTypeArgs(type);
        if (args.empty()) return C::Undecidable;
        bool undecidable = false;
        for (const auto* a : args) {
            const C c = capability(a, structs, enums, claims);
            if (c == C::NotCapable) return C::NotCapable;
            if (c == C::Undecidable) undecidable = true;
        }
        return undecidable ? C::Undecidable : C::Capable;
    }

    // A list of payload types is capable iff every payload is, and undecidable as
    // soon as one payload is -- the aggregate never guesses.
    auto aggregate = [&](const std::vector<const ast::TypeNode*>& payloads) -> C {
        bool undecidable = false;
        for (const auto* p : payloads) {
            const C c = capability(p, structs, enums, claims);
            if (c == C::NotCapable) return C::NotCapable;
            if (c == C::Undecidable) undecidable = true;
        }
        return undecidable ? C::Undecidable : C::Capable;
    };

    if (auto* v = dynamic_cast<const ast::VecType*>(type)) {
        return capability(v->elementType.get(), structs, enums, claims);
    }
    if (auto* a = dynamic_cast<const ast::ArrayType*>(type)) {
        return capability(a->elementType.get(), structs, enums, claims);
    }
    if (auto* f = dynamic_cast<const ast::FutureType*>(type)) {
        return capability(f->resultType.get(), structs, enums, claims);
    }
    if (auto* t = dynamic_cast<const ast::TupleTypeNode*>(type)) {
        std::vector<const ast::TypeNode*> members;
        for (const auto& m : t->memberTypes) members.push_back(m.get());
        return aggregate(members);
    }
    if (auto* o = dynamic_cast<const ast::OptionalType*>(type)) {
        return capability(o->containedType.get(), structs, enums, claims);
    }

    // Any other generic named type (`Result<T, E>`, `Pair<A, B>`, a user generic
    // struct/enum): structural over its arguments.
    {
        const auto args = namedTypeArgs(type);
        if (!args.empty()) return aggregate(args);
    }

    // Enums: keyed by the concrete type string (`Box<Int>`) or the bare name.
    if (enums) {
        auto eit = enums->find(ts);
        if (eit == enums->end()) eit = enums->find(base);
        if (eit != enums->end()) {
            std::vector<const ast::TypeNode*> payloads;
            for (const auto& variant : eit->second) {
                for (const auto& payload : variant.second) payloads.push_back(payload.get());
            }
            return aggregate(payloads);
        }
    }

    // Structs: keyed by name; fields are inspected structurally.
    if (structs) {
        auto sit = structs->find(ts);
        if (sit == structs->end()) sit = structs->find(base);
        if (sit != structs->end()) {
            std::vector<const ast::TypeNode*> fields;
            for (const auto& field : sit->second) fields.push_back(field.second);
            return aggregate(fields);
        }
    }

    if (isPlainValueType(base)) return C::Capable;

    // A named type neither registry knows: a type parameter before
    // monomorphization, or an opaque handle in either pass. Undecidable -- the
    // semantic pass defers it to codegen, and codegen stays permissive.
    return C::Undecidable;
}

bool handoffCapable(const ast::TypeNode* type,
                    const StructFieldRegistry* structs,
                    const EnumPayloadRegistry* enums,
                    const CuratedClaimLookup* claims) {
    return capability(type, structs, enums, claims) == Capability::Capable;
}

// May a second thread read a value of this type without taking ownership?
// The weaker relation: a handoff-capable value is also viewable, and a borrow
// (`their<T>` / `loc<T>`) of a handoff-capable payload is viewable because a
// reader only needs the address, not the owner.
//
// A curated claim is asked for the type as written, and the walk below is then run
// in "reading" mode -- a lookup in which `Handoff` OR `Viewable` counts -- so a
// `Viewable` claim on the payload of a wrapper (`our<W>`, `Vec<W>`) is seen too.
// Reading mode exists only here: `capability` consults `Handoff` alone, so a
// `Viewable` claim can never make a value handoff-capable.
bool viewable(const ast::TypeNode* type,
              const StructFieldRegistry* structs,
              const EnumPayloadRegistry* enums,
              const CuratedClaimLookup* claims) {
    if (!type) return false;
    const std::string ts = type->toString();
    if (claims) {
        if (claims->viewable && claims->viewable(ts)) return true;
        if (claims->handoff && claims->handoff(ts)) return true;
    }

    CuratedClaimLookup reading;
    const CuratedClaimLookup* readClaims = nullptr;
    if (claims) {
        reading.handoff = [claims](const std::string& t) {
            return (claims->handoff && claims->handoff(t)) ||
                   (claims->viewable && claims->viewable(t));
        };
        readClaims = &reading;
    }

    const std::string base = baseNameOf(ts);
    if (base == "their" || base == "loc") {
        const auto args = namedTypeArgs(type);
        if (args.empty()) return false;
        return handoffCapable(args.front(), structs, enums, readClaims);
    }
    return handoffCapable(type, structs, enums, readClaims);
}

} // namespace thread_boundary

// Semantic-pass entry points: thin wrappers that hand the predicate the
// registries this pass built while walking the AST.
//
// A curated bind short-circuits both: `bind Handoff -> T` is the reviewed escape
// hatch for a shape the structural derivation rejects (an FFI struct holding a
// `ptr<T>`, an opaque handle), so the explicit claim wins. Registration reports
// the contradiction as a warning (registerTraitImpl), which keeps the override
// honest without making it impossible.
//
// The claim travels down as a CuratedClaimLookup instead of being checked only at
// the top, so the structural walk asks it at EVERY level: `bind Handoff -> Box`
// also admits `our<Box>`, `Vec<Box>` and a struct holding a `Box`. The pre-fix
// behaviour consulted the claim for the type as written alone, so a wrapper hid it
// and `our<Box>` -- the idiomatic way to share a curated type -- stayed refused.
thread_boundary::Capability SemanticAnalyzer::handoffCapability(const ast::TypeNode* type) {
    if (!type) return thread_boundary::Capability::NotCapable;
    // The curated claim is a decision, not a deferral: the bind author has
    // spoken for the type regardless of what the derivation can see.
    if (hasAspectBinding(type->toString(), "Handoff")) return thread_boundary::Capability::Capable;
    thread_boundary::CuratedClaimLookup claims;
    claims.handoff = [this](const std::string& ts) { return hasAspectBinding(ts, "Handoff"); };
    claims.viewable = [this](const std::string& ts) {
        // `Handoff` implies `Viewable`: a value that may be handed over may also
        // be read by another thread.
        return hasAspectBinding(ts, "Viewable") || hasAspectBinding(ts, "Handoff");
    };
    return thread_boundary::capability(type, &structFieldTypes, &enumVariantPayloadTypes, &claims);
}

bool SemanticAnalyzer::handoffCapable(const ast::TypeNode* type) {
    return handoffCapability(type) == thread_boundary::Capability::Capable;
}

bool SemanticAnalyzer::viewable(const ast::TypeNode* type) {
    if (!type) return false;
    const std::string ts = type->toString();
    // `Handoff` implies `Viewable`: a value that may be handed over may also be
    // read by another thread.
    if (hasAspectBinding(ts, "Viewable") || hasAspectBinding(ts, "Handoff")) return true;
    thread_boundary::CuratedClaimLookup claims;
    claims.handoff = [this](const std::string& t) { return hasAspectBinding(t, "Handoff"); };
    claims.viewable = [this](const std::string& t) {
        return hasAspectBinding(t, "Viewable") || hasAspectBinding(t, "Handoff");
    };
    return thread_boundary::viewable(type, &structFieldTypes, &enumVariantPayloadTypes, &claims);
}

// Rule (b): a read-only borrow may cross the thread boundary when the closure
// also captures the owner it borrows from, so the closure environment itself
// keeps the payload alive for as long as the thread runs. The five conditions
// are spelled out in doc/THREAD_BOUNDARY_SCOPE.md; the load-bearing one is the
// owner carrying *strong* ownership (`our<X>` retains a reference in the env,
// `my<X>` moves the heap object into it). A `mild<X>` (weak) capture does not
// keep the payload alive on its own, and a plain by-value owner hands the
// closure a copy -- neither qualifies.
thread_boundary::Capability
SemanticAnalyzer::mayCrossBoundaryWithRetainedOwner(const std::string& captureName,
                                                    const ast::TypeNode* captureType,
                                                    const ast::FunctionExpression* fe) {
    using C = thread_boundary::Capability;
    if (!captureType || !fe) return C::NotCapable;

    // 1. The capture is a borrow (`their<X>` / `view<X>` / `borrow<X>`).
    const std::string base = baseNameOf(captureType->toString());
    if (base != "their" && base != "view" && base != "borrow") return C::NotCapable;

    // 2. The closure does not write it. A written borrow reaches this function
    //    only through the mutable-capture list, which is reported separately;
    //    be explicit here so the rule stands on its own.
    static const analysis::ClosureCaptures kNoCaptures{};
    const analysis::ClosureCaptures* capsPtr = capturesOf(fe);
    const analysis::ClosureCaptures& caps = capsPtr ? *capsPtr : kNoCaptures;
    for (const std::string& cap : caps.mutableCaptured) {
        if (cap == captureName) return C::NotCapable;
    }

    // 3. The closure also captures the owner the borrow came from.
    auto rootIt = theirVarRoot_.find(captureName);
    if (rootIt == theirVarRoot_.end() || rootIt->second.empty()) return C::NotCapable;
    const std::string& ownerName = rootIt->second;
    bool capturesOwner = false;
    for (const std::string& cap : caps.captured) {
        if (cap == ownerName) capturesOwner = true;
    }
    for (const std::string& cap : caps.mutableCaptured) {
        if (cap == ownerName) capturesOwner = true;
    }
    if (!capturesOwner) return C::NotCapable;

    // 4. That owner carries strong ownership, so the closure keeps it alive.
    SymbolInfo* ownerSym = currentScope ? currentScope->lookup(ownerName) : nullptr;
    if (!ownerSym || !ownerSym->type) return C::NotCapable;
    const std::string ownerBase = baseNameOf(ownerSym->type->toString());
    if (ownerBase != "our" && ownerBase != "my") return C::NotCapable;
    const C ownerCap = handoffCapability(ownerSym->type.get());
    if (ownerCap == C::NotCapable) return C::NotCapable;
    if (ownerCap == C::Undecidable) return C::Undecidable;

    // 5. The borrowed payload is itself handoff-capable, so concurrent reads of
    //    it are race-free by construction.
    const auto args = namedTypeArgs(captureType);
    if (args.empty()) return C::NotCapable;
    bool undecidable = false;
    for (const auto* a : args) {
        const C c = handoffCapability(a);
        if (c == C::NotCapable) return C::NotCapable;
        if (c == C::Undecidable) undecidable = true;
    }
    return undecidable ? C::Undecidable : C::Capable;
}

// Reject a closure handed to another thread that captures anything it may not.
void SemanticAnalyzer::checkThreadBoundaryCaptures(ast::FunctionExpression* fe,
                                                   ast::Node* site,
                                                   const std::string& siteName) {
    if (!fe) return;

    std::set<std::string> reported;

    static const analysis::ClosureCaptures kNoCaptures{};
    const analysis::ClosureCaptures* capsPtr = capturesOf(fe);
    const analysis::ClosureCaptures& caps = capsPtr ? *capsPtr : kNoCaptures;

    // A mutable capture holds the defining frame's address, so it is not a
    // handoff no matter what the variable's type is.
    for (const std::string& cap : caps.mutableCaptured) {
        if (!reported.insert(cap).second) continue;
        addError(siteName + ": closure captures '" + cap +
                     "' mutably -- a mutable capture holds the defining frame's address and "
                     "cannot cross a thread boundary (not handoff-capable). Capture it by "
                     "value, or share it as our(" + cap + ").",
                 site);
    }

    for (const std::string& cap : caps.captured) {
        if (reported.count(cap)) continue;
        if (!reported.insert(cap).second) continue;

        SymbolInfo* csym = currentScope ? currentScope->lookup(cap) : nullptr;
        if (!csym || !csym->type) continue;
        const ast::TypeNode* ty = csym->type.get();
        const thread_boundary::Capability capC = handoffCapability(ty);
        if (capC == thread_boundary::Capability::Capable) continue;

        // Rule (b): a read-only borrow may cross when the closure also captures
        // the owner it borrows from, so the closure environment keeps the
        // payload alive. `viewable` alone deliberately does NOT relax this site:
        // a borrow addresses the *spawner's frame*, the borrow model is lexical,
        // and nothing proves that frame outlives a detached thread's read --
        // letting a read-only borrow through unconditionally reopened the
        // dangling read pinned by `test/ownership/thread_send_their.vyb` (#149).
        // See doc/THREAD_BOUNDARY_SCOPE.md.
        const thread_boundary::Capability retained = mayCrossBoundaryWithRetainedOwner(cap, ty, fe);
        if (retained == thread_boundary::Capability::Capable) continue;

        // Nothing to decide here: the type still mentions a monomorphization type
        // parameter (or the owner/payload does), so the verdict belongs to codegen,
        // which holds the substitutions. Record the capture against the closure
        // node -- codegen sees the same node -- and report nothing now. This is
        // what keeps the two passes from disagreeing: a capture the semantic pass
        // *can* decide (including the rule-(b) admission above) is never re-judged
        // at codegen, and one it cannot is never silently admitted.
        if (capC == thread_boundary::Capability::Undecidable ||
            retained == thread_boundary::Capability::Undecidable) {
            deferredBoundaryCaptures_[fe].push_back(cap);
            continue;
        }

        const std::string ts = ty->toString();
        addError(siteName + ": '" + cap + "' is " + ts +
                     " -- it cannot cross a thread boundary (not handoff-capable). "
                     "Hand it off as shared ownership (our(" + cap + ")) or pass a copy.",
                 site);
    }
}

// Codegen asks by the closure node it is lowering. The semantic pass and codegen
// walk one shared AST, so the pointer matches; a closure that was never checked
// returns null and codegen judges nothing (permissive), which is the same shape
// as a build that stops before codegen (`--semantic-only`, doc generation).
const std::vector<std::string>*
SemanticAnalyzer::deferredBoundaryCapturesFor(const ast::FunctionExpression* fe) const {
    if (!fe) return nullptr;
    auto it = deferredBoundaryCaptures_.find(fe);
    return it == deferredBoundaryCaptures_.end() ? nullptr : &it->second;
}

} // namespace vyb
