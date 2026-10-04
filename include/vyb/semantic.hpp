// SPDX-License-Identifier: Apache-2.0

#pragma once
/**
 * @file semantic.hpp
 * @brief This is the primary semantic analysis header for the Vyb compiler.
 *
 * IMPORTANT: This file was previously duplicated as include/vyb/vre/semantic.hpp
 * which was removed during cleanup. Do not create duplicate headers!
 *
 * The implementation for this header is in src/vre/semantic.cpp
 */

#include "vyb/parser/ast.hpp"
#include "vyb/vre/analysis_facts.hpp"
#include "vyb/vre/closure_captures.hpp"
// The thread-boundary capability predicate (#365): `Capability` is named by the
// declarations below, and this early include keeps the struct/enum registries and
// the predicate in one place.
#include "vyb/vre/thread_boundary.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <unordered_set>
#include <map>
#include <type_traits>
#include <utility>

namespace vyb {

class Driver; // Forward declaration

namespace ast {
class Node;
class Module;
class ExpressionStatement;
class BlockStatement;
class IfStatement;
class ReturnStatement;
class WhileStatement;
class ForStatement;
class BreakStatement;
class ContinueStatement;
class ImportStatement; // Should be ImportDeclaration
class ExternStatement;
class TryCatchStatement; // Should be TryStatement
class ThrowStatement;
class MatchStatement;
class YieldStatement;
class YieldReturnStatement;
class AssertStatement;
class EmptyStatement;
class FreedomStatement;

// Missing forward declarations for expressions
class Identifier;
class IntegerLiteral;
class UnaryExpression;
class BinaryExpression;
class CallExpression;
class ArrayElementExpression;
class MemberExpression;
class AssignmentExpression;
class LogicalExpression; // Add forward declaration
class ConditionalExpression; // Add forward declaration
class SequenceExpression; // Add forward declaration
class ObjectLiteral;
class ArrayLiteral;
class FunctionExpression; // Add forward declaration
class StringLiteral;
class FloatLiteral;
class BooleanLiteral;
class NilLiteral;
class ThisExpression; // Add forward declaration
class SuperExpression; // Add forward declaration
class AwaitExpression; // Add forward declaration
class ListComprehension;
class GenericInstantiationExpression;
class PointerDerefExpression;
class AddrOfExpression;
class FromIntToLocExpression;
class LocationExpression;
class ConstructionExpression; // Add forward declaration

// Missing forward declarations for declarations
class VariableDeclaration;
class FunctionDeclaration;
class StructDeclaration;
class EnumDeclaration;
class TypeAliasDeclaration;
class AspectDeclaration; // Add forward declaration
class BindDeclaration;
class NamespaceDeclaration; // Add forward declaration

// Missing forward declarations for types
class TypeName; // Add forward declaration
class PointerType; // Add forward declaration
class ArrayType; // Add forward declaration
class FunctionType; // Add forward declaration
class OptionalType; // Add forward declaration
class GenericParameter;

class Visitor; // Forward declaration if it\'s in the ast namespace and defined elsewhere
}

struct BorrowInfo {
    std::string ownerName;
    bool isMutable;
    ast::Node* borrowNode; // The AST node that created the borrow
    // Potentially add ast::TypeNode* of the borrowed type if needed for more complex checks
};

struct SymbolInfo {
    enum class Kind { Variable, Function, Type, TYPE_PARAMETER };
    Kind kind;
    std::string name;
    bool isConst = false;
    ast::OwnershipKind ownershipKind = ast::OwnershipKind::MY; // Default to unique ownership
    std::shared_ptr<ast::TypeNode> type; // Owning (CHECKPOINT B step 2); raw -> shared.
    std::vector<std::string> bounds; // For TYPE_PARAMETER: aspect bounds (e.g., ["Display", "Clone"])
};

// Namespace-scope resolution gate. Holds, per top-level symbol, the module that
// owns it plus the set of names each module's own code may resolve
// (`effectiveScope`). `SymbolTable::lookup` consults this so a consumer never
// sees - or can name - another module's private dependencies, even though those
// declarations are still present in the fused AST so codegen can emit them.
// `currentOwner` is refreshed as the analyzer enters each compiled function.
struct ModuleScopeGate {
    std::unordered_map<std::string, std::string> ownerByName;
    std::unordered_map<std::string, std::unordered_set<std::string>> effectiveScope;
    mutable std::string currentOwner;

    // A module-owned top-level symbol is resolvable only from code owned by a
    // module that scopes it. Invoked only for symbols found in the global
    // (root) scope, so function-local variables are never gated and may safely
    // shadow a module name.
    bool hides(const SymbolInfo& sym, const std::string& name) const {
        auto it = ownerByName.find(name);
        if (it == ownerByName.end()) {
            return false;
        }
        if (currentOwner.empty()) {
            return false;
        }
        auto es = effectiveScope.find(currentOwner);
        bool hidden = es == effectiveScope.end();
        if (!hidden) {
            hidden = es->second.find(name) == es->second.end();
        }
        return hidden;
    }
};

class Scope {
public:
    Scope(Scope* parent_scope = nullptr);
    SymbolInfo* find(const std::string& name);
    SymbolInfo* lookupDirect(const std::string& name); // Added declaration
    void insert(const std::string& name, SymbolInfo* symbol);
    Scope* getParent();

private:
    std::unordered_map<std::string, SymbolInfo*> symbols;
    Scope* parent;
};

class SymbolTable {
public:
    SymbolTable(SymbolTable* parent = nullptr, const ModuleScopeGate* gate = nullptr)
        : parent(parent), scopeGate(gate), isFreedomBlock(false), isLoop(false) {}
    void add(const SymbolInfo& sym) { table[sym.name] = sym; }
    SymbolInfo* lookup(const std::string& name) {
        auto it = table.find(name);
        if (it != table.end()) {
            SymbolInfo* found = &it->second;
            // Module-scope gating applies only to top-level symbols in the
            // global scope; function-local variables always resolve, so a local
            // may shadow a module name without being hidden.
            if (!parent && scopeGate && scopeGate->hides(*found, name)) {
                return nullptr;  // hidden top-level symbol: no global scope parent
            }
            return found;
        }
        if (parent) return parent->lookup(name);
        return nullptr;
    }
    SymbolInfo* lookupDirect(const std::string& name); // Added
    SymbolTable* getParent() { return parent; }
    bool isFreedomBlock;
    bool isLoop;
    const ModuleScopeGate* scopeGate;

private:
    std::unordered_map<std::string, SymbolInfo> table;
    SymbolTable* parent;
};

// Information about a generic aspect binding (e.g., bind<T> Display -> Box<T>)
// Declared before SemanticAnalyzer to allow use in getter return type
struct GenericImplInfo {
    std::string typePattern;           // e.g., "Box<T>"
    std::string traitName;             // e.g., "Display"
    std::vector<std::string> typeParams; // e.g., ["T"]
    bool isBounded = false;            // true when any type parameter declares aspect bounds
    ast::BindDeclaration* declaration; // Original AST node
    std::map<std::string, ast::FunctionDeclaration*> methods; // method name -> AST
    std::map<std::string, ast::TypeNode*> associatedTypeBindings; // associated type name -> assigned type AST

    GenericImplInfo(ast::BindDeclaration* decl) : declaration(decl) {
        if (decl->selfType) {
            typePattern = decl->selfType->toString();
        }
        if (decl->traitType) {
            traitName = decl->traitType->toString();
        }

        // Extract type parameters
        for (const auto& param : decl->genericParams) {
            if (param && param->name) {
                typeParams.push_back(param->name->name);
                if (!param->bounds.empty()) {
                    isBounded = true;
                }
            }
        }

        // Store methods
        for (const auto& method : decl->methods) {
            if (method && method->id) {
                methods[method->id->name] = method.get();
            }
        }

        for (const auto& assocBinding : decl->associatedTypeBindings) {
            if (assocBinding.name && assocBinding.valueType) {
                associatedTypeBindings[assocBinding.name->name] = assocBinding.valueType.get();
            }
        }
    }
};

// Information about aspect/trait methods and the trait itself
// Declared before SemanticAnalyzer for use in public API
struct TraitMethod {
    std::string name;
    std::vector<std::string> parameterNames;
    std::vector<ast::TypeNode*> parameterTypes;
    ast::TypeNode* returnType;
    ast::FunctionDeclaration* declaration; // Original AST node
    bool hasDefaultImpl;
};

struct TraitInfo {
    std::string name;
    std::vector<std::string> genericParams;
    std::vector<std::string> superTraits; // Aspect inheritance
    std::vector<std::string> associatedTypes;
    // Default types for associated types, index-aligned with associatedTypes
    // (nullptr = no default declared).
    std::vector<ast::TypeNode*> associatedTypeDefaults;
    // Aspect bounds for each associated type, index-aligned with associatedTypes
    // (empty vector = no bound declared).
    std::vector<std::vector<ast::TypeNode*>> associatedTypeConstraints;
    std::vector<TraitMethod> methods;
    ast::AspectDeclaration* declaration; // Original AST node

    // Returns the declared default type for an associated type, or nullptr.
    ast::TypeNode* getAssociatedTypeDefault(const std::string& assocName) const {
        for (size_t i = 0; i < associatedTypes.size() && i < associatedTypeDefaults.size(); ++i) {
            if (associatedTypes[i] == assocName) {
                return associatedTypeDefaults[i];
            }
        }
        return nullptr;
    }

    // Returns the declared aspect bounds for an associated type (its types),
    // or nullptr if the associated type has no bound.
    const std::vector<ast::TypeNode*>* getAssociatedTypeConstraints(const std::string& assocName) const {
        for (size_t i = 0; i < associatedTypes.size() && i < associatedTypeConstraints.size(); ++i) {
            if (associatedTypes[i] == assocName) {
                return &associatedTypeConstraints[i];
            }
        }
        return nullptr;
    }

    TraitInfo(ast::AspectDeclaration* decl) : declaration(decl) {
        if (decl->name) {
            name = decl->name->name;
        }

        // Extract generic parameters
        for (const auto& param : decl->genericParams) {
            if (param && param->name) {
                genericParams.push_back(param->name->name);
            }
        }

        // Extract super-aspects
        for (const auto& super : decl->superTypes) {
            if (super) {
                superTraits.push_back(super->name);
            }
        }

        for (size_t i = 0; i < decl->associatedTypes.size(); ++i) {
            const auto& associatedType = decl->associatedTypes[i];
            if (associatedType) {
                associatedTypes.push_back(associatedType->name);
                ast::TypeNode* def = nullptr;
                if (i < decl->associatedTypeDefaults.size()) {
                    def = decl->associatedTypeDefaults[i].get();
                }
                associatedTypeDefaults.push_back(def);

                std::vector<ast::TypeNode*> cons;
                if (i < decl->associatedTypeConstraints.size()) {
                    for (const auto& c : decl->associatedTypeConstraints[i]) {
                        cons.push_back(c.get());
                    }
                }
                associatedTypeConstraints.push_back(std::move(cons));
            }
        }

        // Extract methods
        for (const auto& method : decl->methods) {
            if (method) {
                TraitMethod tm;
                if (method->id) {
                    tm.name = method->id->name;
                }

                // Extract parameters
                for (const auto& param : method->params) {
                    tm.parameterNames.push_back(param.name->name);
                    tm.parameterTypes.push_back(param.typeNode.get());
                }

                tm.returnType = method->returnTypeNode.get();
                tm.declaration = method.get();
                tm.hasDefaultImpl = (method->body != nullptr);

                methods.push_back(std::move(tm));
            }
        }
    }
};

class SemanticAnalyzer : public ast::Visitor {
public:
    explicit SemanticAnalyzer(Driver& driver);
    ~SemanticAnalyzer();
    void analyze(ast::Module* root);
    const std::vector<std::string>& getErrors() const { return errors; }

    // Feed in the registry-computed namespace-scope data (top-level symbol
    // owners + each module's resolvable scope) so identifier resolution hides a
    // consumer from another module's private dependencies.
    void setModuleScoping(
        const std::unordered_map<std::string, std::string>& ownerByName,
        const std::unordered_map<std::string, std::unordered_set<std::string>>& effectiveScope);

    // #292: names proven present by an enclosing `if (x != nil)` guard, so a `T?`
    // can be consumed as its `T` payload inside that branch.
    std::unordered_set<std::string> narrowedNonNil;

    // Access to generic trait implementations for monomorphization
    const std::unordered_map<std::string, std::unordered_map<std::string, std::unique_ptr<GenericImplInfo>>>&
    getGenericTraitImpls() const { return genericTraitImpls; }

    // Access to concrete trait implementations (bind Aspect -> Type)
    const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<ast::FunctionDeclaration*>>>&
    getTraitImpls() const { return traitImpls; }

    // Access to aspect/trait registry
    const std::unordered_map<std::string, std::unique_ptr<TraitInfo>>&
    getTraitRegistry() const { return traitRegistry; }

    std::string resolveAssociatedTypeForType(const std::string& typeName,
                                             const std::string& traitName,
                                             const std::string& typeReference) const {
        return resolveAssociatedTypeReference(typeName, traitName, typeReference);
    }

    // Helper methods
    bool isInLoop();
    bool isInFreedomBlock();
    bool isIntegerType(ast::TypeNode* type);
    bool isFloatType(ast::TypeNode* type);
    bool isRawLocationType(ast::TypeNode* type); // Added to support location type checking
    bool isReservedWord(const std::string& name);
    bool isLValue(ast::Expression* expr);
    std::string borrowedRootName(ast::Expression* expr);
    // Thread-boundary capability (#149, #365). `handoffCapable` answers, for a
    // declared type, "may a value of this type cross a thread boundary as a
    // handoff?": unique owners (`my<T>`), borrows (`their<T>`, `loc<T>`) and raw
    // pointers are never handoff-capable, while shared ownership (`our<T>` /
    // `mild<T>`) and composites (`Vec<T>`, arrays, `T?`, futures, tuples, user
    // structs and enums) are handoff-capable iff every payload is. `viewable`
    // is the weaker "may a second thread read it?" relation, derived
    // structurally in the same pass (see semantic_thread_boundary.cpp).
    //
    // A curated `bind Handoff -> T` (or `bind Viewable -> T`) short-circuits the
    // structural derivation: the explicit bind is the escape hatch for shapes the
    // compiler cannot see through or gets wrong (an FFI `ptr<T>`-holding struct or
    // opaque handle is the motivating case, not a restriction of the feature) and
    // wins over the computed verdict, which registration reports as a warning when
    // the two disagree (#365).
    //
    // The claim is threaded into the walk rather than checked once, so it is asked
    // at every level: `bind Handoff -> Box` also admits `our<Box>`, `Vec<Box>` and a
    // struct field of type `Box` (see thread_boundary::CuratedClaimLookup).
    bool handoffCapable(const ast::TypeNode* type);
    // The tri-state form: the semantic pass cannot decide a type that still
    // mentions a monomorphization type parameter, and DEFERS such captures to
    // codegen (deferredBoundaryCapturesFor) instead of guessing. A curated
    // `bind Handoff -> T` answers Capable outright.
    vyb::thread_boundary::Capability handoffCapability(const ast::TypeNode* type);
    bool viewable(const ast::TypeNode* type);
    // Enforce the thread boundary for the closure argument at a spawn site:
    // reports every capture that is not handoff-capable, and every mutable
    // capture (which holds the defining frame's address). A read-only borrow is
    // admitted only when the closure also captures a *strongly* owned owner of
    // it (rule (b), doc/THREAD_BOUNDARY_SCOPE.md), so the closure environment
    // itself keeps the payload alive for the thread's whole run.
    void checkThreadBoundaryCaptures(ast::FunctionExpression* fe, ast::Node* site,
                                     const std::string& siteName);

    // Rule (b) behind that admission: is `captureName` a read-only borrow whose
    // owner the closure also captures, with the owner held through `our`/`my`
    // (not `mild`, which is weak and does not keep the payload alive) and a
    // handoff-capable payload?
    // Capable when admitted, NotCapable when not, and Undecidable when the
    // payload or the owner still mentions a type parameter (then the capture is
    // deferred to codegen like any other undecidable one).
    vyb::thread_boundary::Capability
    mayCrossBoundaryWithRetainedOwner(const std::string& captureName,
                                      const ast::TypeNode* captureType,
                                      const ast::FunctionExpression* fe);
    // Captures this pass could not decide, keyed by the closure node handed to a
    // spawn site. Codegen looks them up by that same node pointer (the semantic
    // pass and codegen see one shared AST) and judges the substituted type with
    // the registries this pass owns. Null when the closure was never checked.
    const std::vector<std::string>* deferredBoundaryCapturesFor(const ast::FunctionExpression* fe) const;
    bool areTypesCompatible(ast::TypeNode* typeA, ast::TypeNode* typeB); // Added
    // Rejects `return v;` where `v` supplies a different number of values than the
    // enclosing function's declared return arity (e.g. a single value returned from
    // a `()<A, B>` multi-value function). Prevents such malformed programs from
    // reaching codegen, where they previously crashed codegen (LLVM GEP assert).
    void validateReturnArity(ast::ReturnStatement* node);
    // #384 checkpoint b(1): the escape predicate for lifetime-carrying
    // mutable-capture closures. Called at every position where a closure value
    // can outlive its defining frame (a return, a field/element store, a
    // container element, a thread/spawn hand-off). Marks the closure node with
    // analysis::ClosureMutablesBoxed so codegen boxes its mutable captures into
    // environment-owned heap cells instead of storing the frame's stack address.
    // A closure value held in a local is resolved one level through
    // `mutableClosureBindings_` (so `f = || -> {...}; return f` is caught too).
    // `refusableEscape` is set at a position that already refused such a closure
    // before this feature (a direct `return`): there, a mutable capture whose
    // type cannot be boxed soundly is still refused rather than left on the
    // stack path. Everywhere else an unboxable capture keeps the pre-existing
    // stack behaviour.
    void markEscapingClosureValue(ast::Expression* expr, bool refusableEscape = false);
    // Record `name` as a local binding of a mutable-capture closure, so a later
    // escape of the binding can be attributed to the closure node.
    void recordMutableClosureBinding(const std::string& name, ast::Expression* init);
    // #384 b(1): can a mutable capture of this declared type be boxed soundly --
    // i.e. copied into a heap cell that OWNS the value and reclaimed when the
    // closure's environment is dropped? True for primitives, String, Vec, a
    // known struct, `our<T>`/`mild<T>` (reference taken), and closure values
    // (environment retained). Anything else -- a borrow, raw/FFI handle,
    // optional, array, tuple, unresolved name, or `my<T>` (a move capture, which
    // has no frame-side owner to keep in step) -- stays on the existing stack
    // path.
    bool mutableCaptureTypeBoxable(const ast::TypeNode* t) const;
    // Mark `fe` boxed, or refuse/report it according to `refusableEscape` when
    // one of its mutable captures cannot be boxed soundly.
    void markClosureNodeEscaping(const ast::FunctionExpression* fe, const ast::Node* site,
                                 bool refusableEscape);
    std::shared_ptr<ast::TypeNode> cloneTypeNode(ast::TypeNode* type); // Helper to clone type nodes
    ast::TypeNode* substituteSelfType(ast::TypeNode* returnType, const std::string& concreteType); // Substitute Self with concrete type
    // #250: the return type a bind body inherits from the aspect method it
    // implements when the bind omits its own annotation. Returns an owned node,
    // or nullptr when the aspect has no such method / declares no return type
    // (in which case the omitted-signature rule of #212 still applies).
    ast::TypeNode* aspectMethodReturnTypeFor(const std::string& aspectName, const std::string& methodName);
    void handleVecMethodCall(ast::CallExpression* node, const std::string& objectName, const std::string& methodName);
    void handleVecMethodCallOnMember(ast::CallExpression* node, ast::VecType* vecType, const std::string& methodName);
    void handleQualifiedAspectCall(ast::CallExpression* node, const std::string& aspectName, const std::string& methodName);
    void handleChanMethod(ast::CallExpression* node, ast::TypeNode* elem, const std::string& methodName);

    // Statements
    void visit(ast::BlockStatement* node) override;
    void visit(ast::ExpressionStatement* node) override;
    void visit(ast::IfStatement* node) override;
    void visit(ast::ForStatement* node) override;
    void visit(ast::WhileStatement* node) override;
    void visit(ast::ReturnStatement* node) override;
    void visit(ast::PassStatement* node) override;
    void visit(ast::BreakStatement* node) override;
    void visit(ast::ContinueStatement* node) override;
    void visit(ast::TryStatement* node) override;
    void visit(ast::FreedomStatement* node) override;
    void visit(ast::EmptyStatement* node) override;
    void visit(ast::AssertStatement* node) override;
    void visit(ast::MatchStatement* node) override;
    void visit(ast::MatchExpression* node) override;
    void visit(ast::YieldStatement* node) override;
    void visit(ast::YieldReturnStatement* node) override;
    void visit(ast::ExternStatement* node) override; // Added
    void visit(ast::ThrowStatement* node) override; // Added

    // Error Handling
    void visit(ast::FailStatement* node) override;
    void visit(ast::TrapClause* node) override;
    void visit(ast::EnsureClause* node) override;
    void visit(ast::RefailStatement* node) override;
    void visit(ast::PanicStatement* node) override;
    void visit(ast::ExitStatement* node) override;
    void visit(ast::DeferStatement* node) override;
    void visit(ast::TupleDestructureAssignment* node) override;

    // Expressions
    void visit(ast::Identifier* node) override;
    void visit(ast::IntegerLiteral* node) override;
    void visit(ast::UnaryExpression* node) override;
    void visit(ast::BinaryExpression* node) override;
    void visit(ast::CallExpression* node) override;
    void visit(ast::ArrayElementExpression* node) override;
    void visit(ast::MemberExpression* node) override;
    void visit(ast::AssignmentExpression* node) override;
    void visit(ast::LogicalExpression* node) override;
    void visit(ast::ConditionalExpression* node) override;
    void visit(ast::SequenceExpression* node) override;
    void visit(ast::ObjectLiteral* node) override;
    void visit(ast::ArrayLiteral* node) override;
    void visit(ast::FunctionExpression* node) override;
    void visit(ast::StringLiteral* node) override;
    void visit(ast::FloatLiteral* node) override;
    void visit(ast::BooleanLiteral* node) override;
    void visit(ast::NilLiteral* node) override;
    void visit(ast::ThisExpression* node) override;
    void visit(ast::SuperExpression* node) override;
    void visit(ast::AwaitExpression* node) override;
    void visit(ast::RangeExpression* node) override;
    void visit(ast::BlockExpression* node) override;
    void visit(ast::SelectExpression* node) override;
    void visit(ast::ComparisonPattern* node) override;
    void visit(ast::StructPattern* node) override;
    void visit(ast::ListComprehension* node) override;
    void visit(ast::GenericInstantiationExpression* node) override;
    void visit(ast::PointerDerefExpression* node) override;
    void visit(ast::AddrOfExpression* node) override;
    void visit(ast::FromIntToLocExpression* node) override;
    void visit(ast::LocationExpression* node) override;
    void visit(ast::ConstructionExpression* node) override;
    void visit(ast::BorrowExpression* node) override;
    void visit(ast::IfExpression* node) override;
    void visit(ast::ArrayInitializationExpression* node) override;
    void visit(ast::TypeofExpression* node) override;
    void visit(ast::TypenameExpression* node) override;
    void visit(ast::AsExpression* node) override;

    // True if an expression is an Identifier bound as a wildcard trap error (e<?>).
    bool isWildcardErrorExpr(ast::Expression* expr) const;

    // Declarations
    void visit(ast::Module* node) override; // Added declaration
    void visit(ast::VariableDeclaration* node) override;
    void visit(ast::FunctionDeclaration* node) override;
    void visit(ast::ClassDeclaration* node) override; // Add this line
    void visit(ast::StructDeclaration* node) override;
    void visit(ast::EnumDeclaration* node) override;
    void visit(ast::TypeAliasDeclaration* node) override;
    void visit(ast::ImportDeclaration* node) override;
    void visit(ast::AspectDeclaration* node) override; // Uncommented
    void visit(ast::BindDeclaration* node) override; // Ensure this is present
    void visit(ast::NamespaceDeclaration* node) override; // Uncommented
    void visit(ast::FieldDeclaration* node) override;
    void visit(ast::EnumVariant* node) override;
    void visit(ast::GenericParameter* node) override;
    void visit(ast::TemplateDeclaration* node) override;


    // Other
    void visit(ast::TypeNode* node) override;

    // Types
    void visit(ast::TypeName* node) override;
    void visit(ast::PointerType* node) override;
    void visit(ast::ArrayType* node) override;
    void visit(ast::VecType* node) override;
    void visit(ast::FutureType* node) override;
    void visit(ast::FunctionType* node) override;
    void visit(ast::OptionalType* node) override;
    void visit(ast::TupleTypeNode* node) override; // Added for tuple type support

    // Template storage system - public for access
    struct TemplateInfo {
        std::unique_ptr<ast::TemplateDeclaration> declaration;
        std::vector<std::string> parameterNames;
        std::vector<std::vector<std::string>> parameterConstraints; // bounds for each parameter
        std::string templateName;

        TemplateInfo(std::unique_ptr<ast::TemplateDeclaration> decl)
            : declaration(std::move(decl)), templateName(declaration->name->name) {
            for (const auto& param : declaration->genericParams) {
                if (param && param->name) {
                    parameterNames.push_back(param->name->name);

                    // Extract trait bounds for this parameter
                    std::vector<std::string> bounds;
                    for (const auto& bound : param->bounds) {
                        if (bound) {
                            bounds.push_back(bound->toString());
                        }
                    }
                    parameterConstraints.push_back(std::move(bounds));
                }
            }
        }
    };

private:
    Driver& driver_;
    SymbolTable* currentScope;
    ModuleScopeGate scopeGate_;
    std::string defaultOwner_;  // entry/root module key, used for unnamed top-level code
    std::vector<std::string> ownerStack_;
    std::vector<std::string> errors;
    // Non-fatal diagnostics (see addWarning): the curated thread-boundary binds.
    std::vector<std::string> warnings;
    // Thread-boundary captures the semantic pass could not decide (#365 step (c)):
    // closure node -> capture names whose declared type still mentions a type
    // parameter. Codegen judges exactly these on the substituted type.
    std::unordered_map<const ast::FunctionExpression*, std::vector<std::string>> deferredBoundaryCaptures_;
    // TypeTable (node-id -> owned type): CHECKPOINT B. Keyed by the stable
    // Node::typeId() (exprKey() in semantic.cpp; null -> sentinel key 0). Owning
    // shared values; setType/typeOf are its accessors.
    std::unordered_map<unsigned, std::shared_ptr<ast::TypeNode>> expressionTypes;
public:
    // #223 single-authority public TypeTable API over the private map.
    std::shared_ptr<ast::TypeNode> typeOf(const ast::Node* n) const {
        if (!n) return std::shared_ptr<ast::TypeNode>();
        auto it = expressionTypes.find(n->typeId());
        return it == expressionTypes.end() ? std::shared_ptr<ast::TypeNode>() : it->second;
    }
    // #223: smart-pointer receivers (node->id is a unique_ptr) so `typeOf(X)`
    // works for both raw and unique_ptr node receivers during the read migration.
    template <class P>
    std::enable_if_t<
        !std::is_convertible<P, const ast::Node*>::value &&
            std::is_convertible<decltype(std::declval<const P&>().get()),
                                const ast::Node*>::value,
        std::shared_ptr<ast::TypeNode>>
    typeOf(const P& p) const {
        return typeOf(p.get());
    }
    void setType(const ast::Node* n, std::shared_ptr<ast::TypeNode> t) {
        // #223 single authority: the node-id TypeTable is the only type writer.
        if (n) expressionTypes[n->typeId()] = std::move(t);
    }
    // #392 immutable AST: the analysis facts the codegen pass needs (whether a
    // function can fail / lowers to the {T, i8*} error ABI, and which origin a
    // cast/typename operand has) are recorded HERE, keyed by the same stable
    // Node::typeId() as the TypeTable, instead of being written into the parse
    // tree. `factsOf` is what codegen binds (see LLVMCodegen::nodeFacts).
    void markFact(const ast::Node* n, unsigned bit, bool on = true) {
        if (!n) return;
        unsigned& f = nodeFacts_[n->typeId()];
        if (on) f |= bit; else f &= ~bit;
    }
    void markFailable(const ast::Node* n) { markFact(n, analysis::FuncCanFail); }
    void markNeedsErrorReturn(const ast::Node* n) { markFact(n, analysis::FuncNeedsErrorReturn); }
    bool hasFact(const ast::Node* n, unsigned bit) const {
        if (!n) return false;
        auto it = nodeFacts_.find(n->typeId());
        return it != nodeFacts_.end() && (it->second & bit) != 0;
    }
    bool isFailable(const ast::Node* n) const { return hasFact(n, analysis::FuncCanFail); }
    // This one also accepts smart-pointer receivers, mirroring typeOf above.
    template <class P>
    std::enable_if_t<
        !std::is_convertible<P, const ast::Node*>::value &&
            std::is_convertible<decltype(std::declval<const P&>().get()),
                                const ast::Node*>::value,
        bool>
    isFailable(const P& p) const {
        return isFailable(p.get());
    }
    bool functionNeedsErrorReturn(const ast::Node* n) const {
        return hasFact(n, analysis::FuncNeedsErrorReturn);
    }
    unsigned factsOf(const ast::Node* n) const {
        if (!n) return 0u;
        auto it = nodeFacts_.find(n->typeId());
        return it == nodeFacts_.end() ? 0u : it->second;
    }
    // #392: a closure's capture lists, recorded by the semantic pass and read by
    // codegen (environment construction) and by the thread-boundary predicate.
    // Same reason as the facts above: the closure node must not carry mutable
    // analysis state after the pass.
    void resetCaptures(const ast::Node* n) {
        if (n) closureCaptures_[n->typeId()] = analysis::ClosureCaptures{};
    }
    void addCapture(const ast::Node* n, const std::string& name, bool written, bool shared) {
        if (!n) return;
        analysis::ClosureCaptures& c = closureCaptures_[n->typeId()];
        c.captured.push_back(name);
        if (written) c.mutableCaptured.push_back(name);
        if (shared) c.ourCaptured.push_back(name);
    }
    const analysis::ClosureCaptures* capturesOf(const ast::Node* n) const {
        if (!n) return nullptr;
        auto it = closureCaptures_.find(n->typeId());
        return it == closureCaptures_.end() ? nullptr : &it->second;
    }
    // #384 checkpoint (c): which declaration does `name` bind to, searching the
    // active block scopes from innermost outward? A closure that mutably captures a
    // name needs this to mark the DECLARATION (the frame's storage is what promotion
    // changes), since the analyzer reaches the closure after the declaration it
    // captures -- often in a different function, for a closure written inside a
    // nested named function. The marked node is the declaration itself (a
    // VariableDeclaration) or, for a parameter, its name Identifier -- the same node
    // codegen holds at the point where the binding's storage is created. Returns null
    // for a name this analyzer never saw as a local declaration (a global or a field).
    ast::Node* lookupLocalDeclaration(const std::string& name) const {
        for (auto it = declScopes.rbegin(); it != declScopes.rend(); ++it) {
            auto found = it->find(name);
            if (found != it->end()) return found->second;
        }
        return nullptr;
    }
private:
    // #392: per-closure capture lists, node-id keyed like the facts above.
    std::unordered_map<unsigned, analysis::ClosureCaptures> closureCaptures_;
public:
    // #392: analysis facts, node-id keyed (0 when the node records none).
    std::unordered_map<unsigned, unsigned> nodeFacts_;
    // CHECKPOINT B step 2: the synthesis registry is gone. Every synthesized type
    // is owned by its consumer (expressionTypes/TypeTable, node->type, or a
    // SymbolInfo.type). retainType() is a thin "wrap a raw new into an owning
    // shared_ptr" helper (takes ownership of the raw; no global keep-alive).
    std::shared_ptr<ast::TypeNode> retainType(ast::TypeNode* raw) {
        return raw ? std::shared_ptr<ast::TypeNode>(raw) : std::shared_ptr<ast::TypeNode>();
    }

    std::vector<SymbolTable*> scopes;
    std::unordered_set<std::string> reservedWords; // Added for isReservedWord
    std::unordered_map<std::string, std::unique_ptr<TemplateInfo>> templateRegistry;
    std::unordered_map<std::string, std::unique_ptr<ast::Declaration>> instantiatedTemplates;

    // Trait registry for user-defined traits
    std::unordered_map<std::string, std::unique_ptr<TraitInfo>> traitRegistry;

    // Trait implementation registry: type -> trait -> methods
    // Maps "Point" -> "Comparable" -> { method implementations }
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<ast::FunctionDeclaration*>>> traitImpls;

    // Generic trait implementation registry: stores templates like impl<T> Display for Box<T>
    // Maps "Box<T>" -> "Display" -> GenericImplInfo
    std::unordered_map<std::string, std::unordered_map<std::string, std::unique_ptr<GenericImplInfo>>> genericTraitImpls;

    // Associated type assignments for concrete binds: type -> aspect -> associated type -> value type
    std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, ast::TypeNode*>>> traitAssociatedTypeImpls;

    // Struct field type storage for member access resolution
    std::unordered_map<std::string, std::map<std::string, ast::TypeNode*>> structFieldTypes;
    std::unordered_map<std::string, std::vector<std::string>> structGenericParamOrder;

    // Enum type names declared in this module (for identifier and member-expression resolution)
    std::unordered_set<std::string> enumTypeNames;

    // Constant enums (`enum Sock { AF_INET = 2, ... }`): each variant is a
    // compile-time Int constant instead of a nominal enum value. name::variant ->
    // value.
    std::unordered_map<std::string, int64_t> constEnumValues;

    // Payload types for data-carrying (tagged-union) enum variants:
    // enumName -> variantName -> payload TypeNodes (empty vector = unit variant).
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<ast::TypeNodePtr>>> enumVariantPayloadTypes;

    // Generic data enums (tagged unions with type parameters; e.g. `enum Box<T>`
    // { Value(T), Empty }`): keep the template payload types and the type-parameter
    // order so a concrete instantiation like `Box<Int>` can be materialized with
    // substituted payload types under its concrete type string.
    std::unordered_map<std::string, std::vector<std::string>> enumGenericParamOrder;
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<ast::TypeNodePtr>>> enumTemplatePayloadTypes;

    // Materialize payload types for a concrete generic enum instantiation (e.g.
    // `Box<Int>`) into `enumVariantPayloadTypes` under the concrete type string,
    // substituting the type arguments for the enum's type parameters.
    void registerGenericEnumConcrete(const std::string& enumName, const std::string& concreteTypeStr,
                                     const std::vector<ast::TypeNodePtr>& typeArgs);

    // Materialize payload types for `type` (a generic enum instantiation such as
    // `Option<Int>`) into `enumVariantPayloadTypes` if not already present. Used at
    // `match`/`select` scrutinees whose concrete enum arose only from a generic bind
    // return type (so the bind's own `Option<T>` was registered, but not the
    // client-substituted `Option<Int>`).
    void materializeConcreteEnum(ast::TypeNode* type);

    // Context for resolving 'Self' type in aspect implementations
    ast::TypeNode* currentImplType = nullptr;  // Set to Box<T> when processing bind Display -> Box<T>
    std::string currentImplTraitName;          // Set to Display when processing bind Display -> Type
    std::unordered_map<std::string, ast::TypeNode*> currentImplAssociatedTypeBindings;
    bool processingTraitOrBindMethod = false;  // True when visiting methods inside aspect or bind

    // Error handling context
    ast::FunctionDeclaration* currentFunction = nullptr;  // Track current function for error analysis
    int trapDepth = 0;  // Track nesting depth of trap clauses
    std::vector<ast::TypeNode*> activeTrapTypes;  // Stack of error types being trapped

    // Function registry for error propagation analysis
    std::unordered_map<std::string, ast::FunctionDeclaration*> functionRegistry;
    std::unordered_set<std::string> externalFunctionNames;

    // Top-level function names pre-registered by visit(Module) so a call site can
    // reference a function declared later in the same file (forward references)
    // and mutually-recursive functions resolve regardless of declaration order
    // (#211). Cleared and rebuilt per module visit.
    std::unordered_set<std::string> forwardDeclaredFunctions_;

    // Top-level struct / enum / type-alias names pre-registered by visit(Module)
    // (+ their duplicate definitions already reported) so a type can be
    // referenced before its declaration and mutually-referential types resolve.
    std::unordered_set<std::string> forwardDeclaredTypes_;

    struct BorrowState {
        int mutableBorrows = 0;
        int immutableBorrows = 0;
    };
    std::vector<std::unordered_map<std::string, BorrowState>> borrowScopes;
    // Move tracking: per-scope map of variable name -> whether it has been moved from
    struct MoveState {
        bool isMoved = false;
    };
    std::vector<std::unordered_map<std::string, MoveState>> moveScopes;

    // Borrow-lifetime (escape) tracking. For a `their<T>` variable created from a
    // `borrow()/view()` initializer, record the scope-stack depth at which the borrow
    // was recorded and the root identifier it borrows. The variable is only valid
    // while that scope is still open: using it at a shallower depth means the borrow
    // escaped and the pointee may be dead. Parameters are never recorded here and are
    // therefore valid for the whole function body.
    std::unordered_map<std::string, int> theirVarDepth_;
    std::unordered_map<std::string, std::string> theirVarRoot_;

    // Current function's parameter names (a stack for nested closures). A `their<T>`
    // borrowed from a parameter may be returned (the caller owns the pointee); a
    // borrow of a function-local must not be.
    std::vector<std::unordered_set<std::string>> fnParamNamesStack_;

    // #384 checkpoint b(1): closure-typed function-scoped locals that hold a
    // mutable-capture closure, so `return f` / `v.push(f)` / `h.f = f` can mark
    // the underlying closure node as escaping (and hence boxed). Cleared when a
    // named function body starts; deliberately NOT cleared inside a nested
    // lambda, so a lambda that hands an enclosing closure binding outward still
    // attributes the escape to the right node. A stale entry only ever boxes an
    // extra closure (slower, never unsound).
    std::unordered_map<std::string, const ast::FunctionExpression*> mutableClosureBindings_;

    // #384 b(1): the name of the first mutable capture of a closure whose type
    // cannot be boxed soundly (see mutableCaptureTypeBoxable). Recorded when the
    // closure's captures are recorded -- independent of where it later escapes --
    // so the escape predicate can refuse a `return` (which always refused such a
    // closure) instead of boxing it unsoundly.
    std::unordered_map<const ast::FunctionExpression*, std::string> unboxableMutableCapture_;

    // Closure capture detection: while visiting a FunctionExpression body, record
    // the identifiers it references and the names it declares locally, so free
    // variables (resolved from an enclosing scope) can be copied into the
    // closure's environment by codegen. Stacked for nested lambdas.
    struct LambdaCaptureCtx {
        SymbolTable* enclosingScope = nullptr;
        std::unordered_set<std::string> referenced;
        std::unordered_set<std::string> locals;
        // Names the lambda body assigns to (LHS of an assignment). Used to
        // distinguish mutable captures from by-value captures.
        std::unordered_set<std::string> written;
    };
    std::vector<LambdaCaptureCtx> lambdaCaptureStack;
    std::vector<ast::FunctionExpression*> lambdaStack;

    // #385: the closure signature the ENCLOSING context expects -- a typed `fn(...)`
    // binding's declared type, or a call's `fn(...)` parameter type -- used to fill in a
    // closure parameter written without an annotation. Innermost wins; a parameter that
    // carries its own annotation is never rewritten.
    // #385: a closure signature the ENCLOSING context expects -- a typed `fn(...)`
    // binding's declared type, or a Vec combinator's closure parameter -- plus the
    // substitutions that make its type parameters concrete (a combinator's `fn(T) -> T`
    // with T bound to the receiver's element type). Empty substitutions = already concrete.
    struct ExpectedClosureSignature {
        const ast::FunctionType* signature = nullptr;
        std::map<std::string, ast::TypeNode*> substitutions;
    };
    std::vector<ExpectedClosureSignature> expectedClosureSignatures_;

    // #385: for a Vec combinator call (`v.map(f)`, `v.filter(f)`, `v.for_each(f)`) the
    // closure signature its argument must satisfy, carrying the receiver's element type as
    // the substitution for the aspect's type parameter. False when the callee is not one.
    bool closureSignatureForVecCombinatorArg(ast::CallExpression* call,
                                             std::map<std::string, ast::TypeNode*>& substitutions,
                                             const ast::FunctionType*& signature,
                                             size_t& closureArgIndex);

    // #384 checkpoint (c): per-block-scope map from a local name to the node that
    // declares it -- a VariableDeclaration for a local, a parameter's name Identifier for
    // a parameter -- pushed/popped alongside the symbol table, so a closure that mutably
    // captures a name can mark that declaration as promoted to a shared cell.
    std::vector<std::unordered_map<std::string, ast::Node*>> declScopes;

    // Helper methods for move tracking
    void recordMove(const std::string& varName);
    void clearMove(const std::string& varName);
    bool isMoved(const std::string& varName) const;
    // True while visiting the LHS of an assignment, where a write to a moved
    // `my` variable revives it rather than being a use-after-move.
    bool inAssignmentLHS = false;
    bool hasOwnershipKindMY(SymbolInfo* sym) const;
    ast::OwnershipKind getOwnershipKind(SymbolInfo* sym) const;

    // Helper for transitive error propagation
    bool checkCallsFailableFunction(ast::Node* node);

    void enterScope();
    void exitScope();
    void addError(const std::string& message, const ast::Node* node);
    // A non-fatal diagnostic (#365). Curated thread-boundary binds deliberately
    // override the structural verdict, so a contradiction is reported here and
    // printed, but does not fail the compile: the escape hatch is the point.
    void addWarning(const std::string& message, const ast::Node* node);
    // bool isLValue(ast::Expression* expr); // Duplicate declaration removed
    bool isRawLocationType(ast::Expression* expr);
    BorrowState aggregateBorrowState(const std::string& rootName) const;
    void recordBorrow(const std::string& rootName, ast::BorrowKind kind, const ast::Node* node);
    bool hasActiveBorrow(const std::string& rootName) const;
    // Borrow-lifetime helpers: record that `varName` (a their<T>) was created by
    // borrowing `root` at the current scope depth, and reject any later use at a
    // shallower depth (escape).
    void recordTheirBorrowInto(const std::string& varName, const std::string& root);

    // Template management methods
    void registerTemplate(std::unique_ptr<ast::TemplateDeclaration> templateDecl);
    TemplateInfo* findTemplate(const std::string& templateName);
    bool isTemplateInstantiation(const std::string& name);
    std::unique_ptr<ast::Declaration> instantiateTemplate(const std::string& templateName,
                                                         const std::vector<std::string>& typeArgs);

    // Template instantiation helpers
    void handleTemplateInstantiation(ast::Identifier* identifier,
                                   const std::vector<ast::TypeNodePtr>& typeArgs,
                                   ast::GenericInstantiationExpression* node);
    void handleMemberTemplateInstantiation(ast::MemberExpression* memberExpr,
                                         const std::vector<ast::TypeNodePtr>& typeArgs,
                                         ast::GenericInstantiationExpression* node);

    // Builtin Vec constructor (`Vec()`, `Vec(n)`): validate arity, visit the
    // optional size argument, and (if no surrounding annotation propagated a
    // concrete element type) default to `Vec<Int>`.
    void handleVecConstructor(ast::CallExpression* node);

    // Expected-type propagation: inject the parameter type into a bare builtin
    // enum constructor argument (`Some`/`None`/`Ok`/`Err`) when the callee's
    // corresponding parameter is `Option<T>`/`Result<T,E>`, so bare constructors
    // can appear as call arguments (e.g. `unwrap(Some(7))`, `v.push(Some(x))`).
    void injectBareEnumCtorArgTypes(ast::CallExpression* node);

    std::unique_ptr<ast::Declaration> performMonomorphization(TemplateInfo* templateInfo,
                                                             const std::vector<std::string>& concreteTypes);
    std::unique_ptr<ast::Declaration> cloneAndSubstituteAST(ast::Declaration* templateBody,
                                                           const std::vector<std::string>& genericParams,
                                                           const std::vector<std::string>& concreteTypes);

    // Template constraint validation methods
    bool validateTemplateConstraints(TemplateInfo* templateInfo,
                                   const std::vector<std::string>& concreteTypes,
                                   ast::GenericInstantiationExpression* node);
    bool typeImplementsTrait(const std::string& typeName, const std::string& traitName);
    std::vector<std::string> getImplementedTraits(const std::string& typeName);
    bool isBuiltinTypeCompatible(const std::string& typeName, const std::string& traitName);

    // Aspect management methods
    void registerTrait(ast::AspectDeclaration* traitDecl);
    TraitInfo* findTrait(const std::string& traitName);
    void registerTraitImpl(ast::BindDeclaration* implDecl);
    bool validateTraitImpl(const std::string& typeName, const std::string& traitName,
                          const std::vector<std::unique_ptr<ast::FunctionDeclaration>>& methods,
                          const std::vector<ast::BindDeclaration::AssociatedTypeBinding>& associatedTypeBindings,
                          const ast::BindDeclaration* bindDecl);
    bool traitMethodSignatureMatches(const TraitMethod& traitMethod,
                                    ast::FunctionDeclaration* implMethod,
                                    const std::string& traitName,
                                    const std::unordered_map<std::string, std::string>& associatedTypeBindings);
    std::string resolveAssociatedTypeReference(const std::string& typeName,
                                              const std::string& traitName,
                                              const std::string& typeReference,
                                              const std::unordered_map<std::string, ast::TypeNode*>* inlineAssociatedTypeBindings = nullptr) const;
    bool setResolvedTraitReturnType(ast::CallExpression* callNode,
                                    const std::string& concreteTypeName,
                                    const std::string& traitName,
                                    ast::TypeNode* traitReturnType);

    // Pattern matching helper for generic type matching
    // Returns true if concrete type (e.g., "Box<Int>") matches pattern (e.g., "Box<T>")
    bool matchesPattern(const std::string& concreteType, const std::string& pattern);

    // True if the concrete type has a bind (concrete or generic) for the given aspect.
    bool hasAspectBinding(const std::string& typeStr, const std::string& aspectName);

    // True if aspect 'boundAspect' provides aspect 'requestedAspect' directly or
    // via any transitive super-aspect inheritance chain.
    bool boundAspectProvides(const std::string& boundAspect, const std::string& requestedAspect);

    // Finds a method declared by aspect 'aspectName' or any of its transitive
    // super-aspects. On success, 'declaringAspect' is set to the aspect that
    // declares the method and 'outMethod' to the matching method. Returns false
    // if no direct or inherited method matches.
    bool findAspectMethod(const std::string& aspectName,
                          const std::string& methodName,
                          const TraitInfo*& declaringAspect,
                          const TraitMethod*& outMethod);

    // Validates super-aspects exist and that no aspect inheritance cycles exist.
    void validateAspectInheritance();
};

} // namespace vyb
