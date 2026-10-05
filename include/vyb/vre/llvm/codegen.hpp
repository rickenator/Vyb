// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/Support/raw_ostream.h>
#include <map>
#include <memory>
#include <functional>
#include <type_traits>
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "vyb/parser/ast.hpp"
#include "vyb/semantic.hpp" // For SourceLocation, UserTypeInfo
#include "vyb/driver.hpp"   // Added to resolve Driver type

// Forward declarations
namespace llvm {
    class Value;
    class Type;
    class Function;
    class BasicBlock;
    class StructType;
    class AllocaInst;
}

namespace vyb {
    class Driver; // Forward declaration might also work if full include causes issues
}


namespace vyb {

// Global flag: when false (the default), all "DEBUG: ..." codegen prints are suppressed.
// Enable with --debug-codegen CLI flag.
extern bool g_debug_codegen;

// Global flag: true when compiling under `--kernel` (issue #198). In kernel mode a
// module is lowered as pure device code for NVIDIA GPUs: no `main`, no host-runtime
// calls, no __vyb_* intrinsic externs. The codegen subsets what it emits accordingly.
extern bool g_kernel_mode;

// Convenience macro: use VYB_CDBG in place of std::cerr for DEBUG-level codegen output.
// The entire chained << expression is skipped when g_debug_codegen is false.
#define VYB_CDBG if (vyb::g_debug_codegen) std::cerr

// Helper struct for storing information about user-defined types
struct UserTypeInfo {
    llvm::StructType* llvmType;
    std::map<std::string, unsigned> fieldIndices; // Map field name to index
    bool isStruct; // True if struct, false if class (or could be enum later)
    bool isReprC = false;
    // Potentially: vtable, parent type info, etc.
};

// Helper struct to manage loop context
struct LoopContext {
    llvm::BasicBlock *loopHeader; // Block for the loop condition check
    llvm::BasicBlock *loopBody;   // Block for the loop body
    llvm::BasicBlock *loopUpdate; // Block for the loop increment/update
    llvm::BasicBlock *loopExit;   // Block after the loop
    std::string label;            // loop label ("" = none) for labeled break/continue
    size_t scopeBaseline = 0;     // Scope-stack depth at loop entry; a break/continue
                                  // releases only scopes stacked strictly above this.
};

// Helper struct to manage value-yielding contexts (select expressions and
// match-as-value expressions). Both `select` arms and `match` block arms yield
// a value via the `pass` statement, which stores into resultAlloca and branches
// to endBlock.
struct YieldContext {
    llvm::BasicBlock *endBlock;   // Block after the yielding expression
    llvm::AllocaInst *resultAlloca; // Alloca for storing the result
};

class LLVMCodegen : public ast::Visitor {
public:
    // explicit LLVMCodegen(); // Old constructor
    explicit LLVMCodegen(Driver& driver); // Constructor expects a Driver reference
    virtual ~LLVMCodegen(); // Add virtual destructor declaration

    // issue #348: the IR dump is opt-in. `writeIR` defaults to false so an ordinary
    // compile/run never litters `<source>.vyb.ll` beside the program; the --emit-llvm
    // `outputFilename` names the emitted `.ll`/object; `sourceFilename` (when given)
    // is the Vyb source the compile unit's DWARF file table must name, so a debugger
    // can resolve `break <file>.vyb:<line>` (#390).
    void generate(vyb::ast::Module* astModule, const std::string& outputFilename,
                  bool writeIR = false,
                  const std::string& sourceFilename = ""); // Add declaration
    void dumpIR() const; // Add declaration
    std::unique_ptr<llvm::Module> releaseModule(); // Add declaration
    std::unique_ptr<llvm::LLVMContext> releaseContext(); // Add declaration for context release
    llvm::Module* getModule() const { return module.get(); } // Add method to get module pointer without releasing
    // True when main()'s return argument is a lit(...) call (raw JSON passthrough).
    bool isMainReturnLitRaw() const { return m_mainReturnIsLitRaw; }
    // #223: resolve a node's AST type from the semantic TypeTable (bound from the
    // Driver's SemanticAnalyzer in the constructor). The TypeTable is the single
    // authority; the retired node->type field is no longer consulted.
    std::shared_ptr<vyb::ast::TypeNode> typeOfNode(const vyb::ast::Node* n) const {
        return nodeTypeOf_ ? nodeTypeOf_(n) : std::shared_ptr<vyb::ast::TypeNode>();
    }
    // #223: same query for smart-pointer receivers (unique_ptr/shared_ptr) so
    // call sites like `node->id->type` become `typeOfNode(node->id)` unchanged
    // in shape. SFINAE keeps it to objects whose get() yields a Node*; a raw
    // or non-Node receiver (LLVM Value*, ScopeVariable*, ...) is intentionally
    // NOT matched here so the raw-pointer overload above or a compile error
    // (for truly non-Node receivers) surfaces at the call site.
    template <class P>
    std::enable_if_t<
        !std::is_convertible<P, const vyb::ast::Node*>::value &&
            std::is_convertible<decltype(std::declval<const P&>().get()),
                                const vyb::ast::Node*>::value,
        std::shared_ptr<vyb::ast::TypeNode>>
    typeOfNode(const P& p) const {
        const vyb::ast::Node* n = p.get();
        return nodeTypeOf_ ? nodeTypeOf_(n) : std::shared_ptr<vyb::ast::TypeNode>();
    }
    // #392 immutable AST: the analysis facts the semantic pass recorded for a
    // node (bound from the Driver's analyzer). Facts for a node codegen
    // synthesized itself are overlaid from synthFacts_ -- the analyzer never saw
    // those. The AST no longer carries them (they were mutable fields that both
    // passes read and wrote).
    unsigned nodeFacts(const vyb::ast::Node* n) const {
        if (!n) return 0u;
        unsigned f = nodeFactsOf_ ? nodeFactsOf_(n) : 0u;
        auto it = synthFacts_.find(n->typeId());
        if (it != synthFacts_.end()) f |= it->second;
        return f;
    }
    bool nodeCanFail(const vyb::ast::Node* n) const {
        return (nodeFacts(n) & vyb::analysis::FuncCanFail) != 0;
    }
    bool nodeNeedsErrorReturn(const vyb::ast::Node* n) const {
        return (nodeFacts(n) & vyb::analysis::FuncNeedsErrorReturn) != 0;
    }
    bool nodeOperandFromWildcardError(const vyb::ast::Node* n) const {
        return (nodeFacts(n) & vyb::analysis::OperandWildcardError) != 0;
    }
    bool nodeOperandFromTypeValue(const vyb::ast::Node* n) const {
        return (nodeFacts(n) & vyb::analysis::OperandFromTypeValue) != 0;
    }
    bool nodeOperandIsWildcardError(const vyb::ast::Node* n) const {
        return (nodeFacts(n) & vyb::analysis::OperandIsWildcardError) != 0;
    }
    // #384 b(1): the semantic escape predicate decided this closure's mutable
    // captures must be boxed into environment-owned heap cells because the
    // closure value can outlive its defining frame.
    bool nodeClosureMutablesBoxed(const vyb::ast::Node* n) const {
        return (nodeFacts(n) & vyb::analysis::ClosureMutablesBoxed) != 0;
    }
    // #384 checkpoint (c): this declaration's binding is promoted to a shared,
    // refcounted heap cell (see analysis::CellPromotedBinding).
    bool nodeCellPromotedBinding(const vyb::ast::Node* n) const {
        return (nodeFacts(n) & vyb::analysis::CellPromotedBinding) != 0;
    }
    // Record a fact for a node codegen built itself (the async worker cloned
    // from a failable function is the case that needs it).
    void setSynthFact(const vyb::ast::Node* n, unsigned bit, bool on = true) {
        if (!n) return;
        if (on) synthFacts_[n->typeId()] |= bit; else synthFacts_[n->typeId()] &= ~bit;
    }
    // #392: the capture lists the semantic pass recorded for a closure (bound
    // from the same analyzer). Empty when the closure was never analyzed --
    // identical to the retired AST vectors' empty state.
    const vyb::analysis::ClosureCaptures& nodeCaptures(const vyb::ast::Node* n) const {
        static const vyb::analysis::ClosureCaptures kEmpty{};
        const vyb::analysis::ClosureCaptures* c = nodeCapturesOf_ ? nodeCapturesOf_(n) : nullptr;
        return c ? *c : kEmpty;
    }
    // True when a call expression's type is `mild<...>` (a shared borrow with
    // retained-on-stow semantics). Made a member (#223) so it resolves the type
    // through typeOfNode instead of the node->type field.
    bool isMildTransferExpr(ast::Expression* expr);

    // Hard codegen failures: the generated module must not be run or linked.
    // Used for a defect that would otherwise let a binary execute with degraded
    // resolution -- an enum variant whose payload type never resolved (#251),
    // where the module still verifies and `main` still returns 0.
    void flagHardCodegenError() { m_hardCodegenError = true; }
    bool hasHardCodegenError() const { return m_hardCodegenError; }

private:
    Driver& driver_; // Add a Driver reference
    // Bound to the semantic analyzer's TypeTable query (#223). null when no
    // analyzer is registered (fallback to node->type above keeps it working).
    std::function<std::shared_ptr<vyb::ast::TypeNode>(const vyb::ast::Node*)> nodeTypeOf_;
    // #392: the analyzer's fact + capture queries, bound alongside nodeTypeOf_.
    // null when no analyzer is registered (test/JIT paths): nodeFacts() then
    // answers 0 and nodeCaptures() answers an empty record.
    std::function<unsigned(const vyb::ast::Node*)> nodeFactsOf_;
    std::function<const vyb::analysis::ClosureCaptures*(const vyb::ast::Node*)> nodeCapturesOf_;
    // Facts recorded for nodes codegen synthesizes (not present in the AST).
    std::unordered_map<unsigned, unsigned> synthFacts_;

    // Thread-boundary capability (#365 step (c)): bound to the semantic analyzer,
    // so codegen asks the question with the registries the semantic pass owns
    // (struct fields, enum variant payloads) and its curated `bind Handoff -> T`
    // overrides -- the predicate itself is the same implementation in
    // vyb/vre/thread_boundary.hpp. Null in test/JIT paths with no analyzer, where
    // codegen falls back to the registry-free predicate (permissive for named
    // types).
    std::function<bool(const vyb::ast::TypeNode*)> boundaryCapable_;
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    // Set by flagHardCodegenError(): the module reached codegen through a
    // failure that only the verifier would otherwise catch (#251).
    bool m_hardCodegenError = false;

    // Debug information support
    std::unique_ptr<llvm::DIBuilder> debugBuilder;
    llvm::DICompileUnit* debugCompileUnit;
    llvm::DIFile* debugFile;
    std::stack<llvm::DIScope*> debugScopeStack;

    // Basic LLVM types
    llvm::Type* voidType;
    llvm::Type* int1Type; // For booleans
    llvm::Type* int8Type;
    llvm::Type* int32Type;
    llvm::Type* int64Type;
    llvm::Type* floatType;
    llvm::Type* doubleType;
    llvm::Type* int8PtrType; // Generic pointer type (char*)
    llvm::StructType* rttiStructType; // For RTTI objects
    llvm::Type* stringType; // Placeholder for Vyb's string type representation

    // Current state
    llvm::Type* m_currentLLVMType = nullptr; // Initialize

    llvm::Value* m_currentLLVMValue = nullptr; // Unified value propagation

    // Scope and symbol management
    llvm::Function* currentFunction = nullptr; // Initialize
    vyb::ast::FunctionDeclaration* currentFunctionAST = nullptr; // Track AST node for error propagation
    bool m_currentFunctionFailable = false; // True inside a failable lambda body
    size_t m_functionScopeBaseline = 0;   // Scope-stack depth at function entry
    llvm::StructType* currentClassType = nullptr; // Initialize
    LoopContext currentLoopContext;
    std::vector<LoopContext> loopStack;
    std::vector<YieldContext> yieldContextStack_;  // Track nested select/match yield expressions
    bool infer_types_only = false;  // Flag for type inference without codegen
    std::map<std::string, llvm::AllocaInst*> m_currentFunctionNamedValues;

    // Defer support: stack of deferred statement lists, one per function scope
    std::vector<std::vector<vyb::ast::Statement*>> m_deferStack;


    // Global and type information
    std::map<std::string, llvm::Value*> namedValues;
    // Module-level global variables, kept visible to generated function bodies
    // even though each function starts codegen from an isolated namedValues
    // (functions swap the module scope out and back in on entry/exit).
    std::map<std::string, llvm::GlobalVariable*> globalValues_;
    // Module-level globals whose initializers are not pure compile-time
    // constants (they reference other globals, or compute a value at runtime).
    // Their value is stored in __vyb_module_init before main body runs.
    std::vector<std::pair<llvm::GlobalVariable*, vyb::ast::Expression*>> pendingGlobalInits_;
    // For mutable captures, maps the captured variable name to the address of
    // the *outer* variable's alloca, so writes inside a lambda can propagate
    // back to the enclosing scope. Populated only while generating a lambda.
    std::map<std::string, llvm::Value*> mutableCaptureOuterPointers;
    // A lambda's mutable captures are read/written through a per-call snapshot
    // alloca (`closure.cap.<name>`); an assignment propagates back through
    // mutableCaptureOuterPointers, but an in-place mutation (`v.push(3)`, a
    // subscript store) does not. Each lambda exit flushes these snapshots back into
    // the captured storage, so a mutation made through a mutable capture is visible
    // to the defining frame and to later calls.
    struct PendingCaptureFlush {
        llvm::Value* outerPtr;      // the captured storage (frame alloca or heap cell)
        llvm::Value* snapshotAlloca; // this call's snapshot of that storage
        llvm::Type* valueType;
    };
    std::vector<PendingCaptureFlush> pendingCaptureFlush;
    void flushPendingCaptures();
    std::map<std::string, UserTypeInfo> userTypeMap;
    std::map<std::string, llvm::Type*> typeParameterMap;
    std::map<std::string, llvm::Type*> typeAliasMap; // Maps type alias names to their underlying LLVM types
    // Memo of AST TypeNode -> LLVM type. Keyed by the raw TypeNode pointer but
    // each entry also records the node's `toString()` at store time and the hit
    // is validated against the current node's string. Transient substitution
    // clones (created in `monomorphizeStruct` etc.) are freed and their heap
    // addresses reused, so a raw-pointer key alone produced stale false hits
    // (e.g. `Bool` false-resolving to `Vec`); the string check makes a stale
    // entry for a *different* type at a reused address a miss. Distinct nodes
    // with the same string are still keyed separately, preserving context that
    // can change how a type resolves (e.g. `Self` inside trait binds).
    std::map<vyb::ast::TypeNode*, std::pair<llvm::Type*, std::string>> m_typeCache;
    std::map<llvm::Value*, std::shared_ptr<vyb::ast::TypeNode>> valueTypeMap; // Maps LLVM values to AST types
    std::map<std::string, llvm::FunctionType*> localLambdaTypes; // Maps lambda variable name to its function type
    // Uniform closure representation: every lambda (capturing or not) is a
    // `struct { ptr env; ptr fn }`. The env is null for non-capturing lambdas.
    llvm::StructType* getClosureStructType();
    // User-facing function signature of the most recently generated lambda
    // (without the hidden environment parameter), for localLambdaTypes.
    llvm::FunctionType* lastLambdaFuncType = nullptr;
    vyb::ast::TypeNode* m_currentImplTypeNode = nullptr; // Initialize
    std::string m_currentImplTraitName;
    vyb::ast::Module* m_currentVybModule = nullptr;
    bool m_isLHSOfAssignment = false;
    bool verbose = false;  // Controls detailed warning output
    bool m_isMemberAccessBase = false; // Controls Identifier behavior for member access
    // Auto-serialization: when main() has a non-Int, non-Void, non-String return type,
    // its LLVM return type is changed to void and the value is serialized and printed.
    // This member holds the original return type so cgen_stmt knows how to serialize.
    llvm::Type* m_mainAutoSerializeOrigRetType = nullptr;
    // True when `main()`'s return argument is a `lit(...)` intrinsic call. `lit()`
    // produces an already-serialized raw JSON fragment that must pass through to
    // stdout verbatim (no JSON escaping, no surrounding quotes), unlike a genuine
    // user String return which is escaped+quoted. The JIT runner and the standalone
    // wrapper consult this to decide raw-passthrough vs escaped output.
    bool m_mainReturnIsLitRaw = false;
    llvm::Type* m_asyncResultType = nullptr;  // Result type T for Future<T> in async context
    llvm::Type* m_currentCallResultType = nullptr;  // Result type from most recent function call

    // Ownership and scope tracking
    struct ScopeVariable {
        std::string name;
        llvm::Value* allocaInst;  // The alloca instruction for the variable
        llvm::Value* value;       // Current value (may be loaded from alloca)
        ast::OwnershipKind ownership;
        bool needsCleanup;
        llvm::Type* type;
        bool isVecWithMallocData; // Tracks if this is a Vec that owns malloc'd data
        bool isOwnedStruct;       // Tracks if this is a struct binding owning Vec/String fields
        // #384 checkpoint (c): the base of the refcounted cell this binding was
        // promoted to (null when it was not). The binding's reads/writes go through
        // the cell's value field; this is the owner handle scope exit releases, and it
        // replaces the ordinary per-type reclaim so the payload is reclaimed exactly
        // once, by the cell's destructor when the last owner drops.
        llvm::Value* ownedCellBase = nullptr;
    };
    std::vector<std::vector<ScopeVariable>> scopeStack;
    // #384 checkpoint (c): a promoted binding's storage is the value field of its shared
    // cell -- a struct GEP, not an alloca. Everywhere codegen discriminates "storage slot
    // (load from / store to)" from "value", both kinds must answer yes, so those sites ask
    // through these helpers rather than `dyn_cast<llvm::AllocaInst>` directly. A promoted
    // slot is registered when the cell is created.
    std::set<llvm::Value*> promotedValueSlots_;
    bool isStorageSlot(llvm::Value* v) const {
        if (!v) return false;
        if (llvm::isa<llvm::AllocaInst>(v)) return true;
        return promotedValueSlots_.count(v) != 0;
    }
    llvm::Type* storageSlotValueType(llvm::Value* v) const {
        if (auto* ai = llvm::dyn_cast<llvm::AllocaInst>(v)) return ai->getAllocatedType();
        if (auto* gep = llvm::dyn_cast<llvm::GetElementPtrInst>(v)) {
            return gep->getResultElementType();
        }
        return v ? v->getType() : nullptr;
    }
    // Counter for synthetic owned-struct receiver-temp alloca names (#192).
    int m_recvStructTempCounter = 0;
    std::map<std::string, uint32_t> refCounts; // For our<T> reference counting
    std::map<std::string, llvm::Value*> refCountStorage; // Storage for refcount variables

    // Error handling state
    struct TrapContext {
        llvm::BasicBlock* landingPad;        // Landing pad for error handling
        llvm::BasicBlock* resumeBlock;       // Block to resume to after handling
        llvm::Value* errorSlot;              // Heap-allocated slot for error pointer
        ast::TypeNode* errorType;            // Expected error type
        std::string errorVarName;            // Name of error variable
        llvm::BasicBlock* ensureBlock;       // Ensure block to run before resuming (if any)
        llvm::AllocaInst* resultAlloca;      // Result alloca for storing handler return values
        bool disabled = false;               // Set while this trap's own handler body
                                              // is being generated, so a `fail` raised
                                              // there propagates outward instead of
                                              // re-entering the same handler.
        bool errorHandedOff = false;         // Set when this handler's `refail` re-raised
                                              // the caught error: ownership of the error
                                              // object transfers outward, so the handler
                                              // must NOT free it on exit.
    };
    std::vector<TrapContext> trapStack;      // Stack of active trap contexts
    bool inTrapHandler = false;           // True when executing trap handler body
    int currentTrapHandlerIndex = -1;     // Index of current trap handler being executed
    std::vector<llvm::BasicBlock*> ensureBlocks; // Ensure cleanup blocks to execute
    std::vector<llvm::Value*> trapHandlerReturnValues; // Return values from trap handlers (for PHI node)
    llvm::AllocaInst* currentErrorSlot = nullptr; // Current error being handled

    // Stack trace capture for error handling (Phase 6.4)
    struct CallStackFrame {
        std::string functionName;       // Vyb function name
        SourceLocation location;        // Source location of function definition
        llvm::Function* llvmFunction;   // LLVM function pointer
    };
    std::vector<CallStackFrame> callStack; // Runtime call stack for error reporting

    // Monomorphization: Generic type instantiation
    std::map<std::string, vyb::ast::StructDeclaration*> genericStructTemplates; // Store generic struct AST nodes (e.g., Box<T>)
    std::map<std::string, llvm::StructType*> monomorphizedStructs; // Cache instantiated types (e.g., "Box<Int>" -> Box_Int LLVM type)

    // #383: values that came through `bare(...)`. `bare(x)` forwards its argument
    // (the inner value is untouched), so the fact that it was asked for raw field
    // values has to travel with the value: the serialization site records it here
    // and passes a marked type name down to the runtime, which then emits the
    // struct's field values in declaration order instead of a keyed object.
    std::vector<llvm::Value*> bareSerializationValues;
    std::map<std::string, vyb::ast::EnumDeclaration*> genericEnumTemplates;   // Generic data-enum AST nodes (enum Box<T> { ... })
    std::map<std::string, llvm::GlobalVariable*> typeMetadataGlobals; // Type metadata for JSON serialization
    std::map<std::string, llvm::GlobalVariable*> enumMetadataGlobals; // Enum metadata for JSON serialization

    // Generic function templates
    std::map<std::string, vyb::ast::FunctionDeclaration*> genericFunctionTemplates; // Store generic function AST nodes (e.g., printItem<T>)
    std::map<std::string, llvm::Function*> monomorphizedFunctions; // Cache instantiated functions (e.g., "printItem_Point" -> Function*)
    // Declared struct constructors: struct name -> list of (arity, ctor fn name).
    // `HashMap<K,V>(n)` dispatches to the matching constructor generic function.
    std::map<std::string, std::vector<std::pair<unsigned, std::string>>> structConstructors;

    // Enum variant integer constants: "EnumName::VariantName" -> constant i64
    std::map<std::string, llvm::Constant*> enumVariantValues;
    // Set of declared enum type names (for quick lookup)
    std::set<std::string> enumTypeNames;

    // Tagged-union layout for enums that carry data variants (e.g. enum Shape { Circle(Float) }).
    // Represented as a value-semantics struct { i64 tag, [N x i8] data } where N is the
    // largest payload (in bytes) among the variants. C-like enums with no data variants
    // are represented by a single scalar i64 tag (isScalar=true) so that `Enum` values
    // interoperate with C integer-backed enums across the FFI boundary.
    struct TaggedEnumInfo {
        llvm::StructType* llvmType = nullptr;          // { i64 tag, [N x i8] data }
        unsigned payloadBytes = 0;                     // N
        bool isScalar = false;                         // C-like enum: a single i64 tag, no struct
        std::map<std::string, unsigned> variantTags;   // VariantName -> tag value
        std::map<std::string, llvm::StructType*> variantPayloadTypes; // VariantName -> payload struct (absent = unit variant)
    };
    std::map<std::string, TaggedEnumInfo> taggedEnumInfo;

    // Helper methods
    llvm::Type* codegenType(vyb::ast::TypeNode* typeNode); // Converts vyb::TypeNode to llvm::Type
    const TaggedEnumInfo* findTaggedEnum(llvm::Type* structTy) const;
    const TaggedEnumInfo* findTaggedEnum(vyb::ast::TypeNode* typeNode); // Resolve by concrete AST type name
    llvm::Value* buildTaggedEnumValue(const std::string& enumName, const std::string& variantName,
                                      std::vector<llvm::Value*> payloadVals);
    llvm::Value* extractEnumVariantField(llvm::Value* enumVal, llvm::StructType* payloadTy, unsigned fieldIdx);
    // Equality for tagged-union enum values (`==` / `!=`): compare the i64 tag
    // and, when the tags match, the payload fields of the matched variant.
    // Previously the EQEQ/NOTEQ path fed a `{ i64 tag, [N x i8] data }` struct
    // straight into ICmp, which LLVM rejects with an assert crash (#181).
    llvm::Value* generateTaggedEnumEquality(llvm::Value* L, llvm::Value* R, vyb::TokenType op,
                                            const TaggedEnumInfo& info);
    // Emit a Vec value for the builtin Vec constructor (`Vec()`, `Vec(n)`), sharing
    // the codegen between the bare `Vec(...)` and legacy `Vec::new(...)` forms.
    void emitVecConstructor(vyb::ast::CallExpression* node);
    // Kernel-mode (issue #198 P4) device intrinsic lowering: thread indexing
    // (tid_x/blk_x/dim_x/... -> NVPTX special-register reads) and device-global
    // load/store (ld_f64/st_i32/... -> global memory access). Returns true when
    // `node` was a kernel intrinsic and was fully lowered. Only valid in kernel mode.
    bool emitKernelIntrinsic(vyb::ast::CallExpression* node);
    std::string mangleGenericTypeName(const std::string& baseName, const std::vector<vyb::ast::TypeNodePtr>& typeArgs); // Generate mangled name like Box_Int
    llvm::StructType* monomorphizeStruct(const std::string& baseName, const std::vector<vyb::ast::TypeNodePtr>& typeArgs); // Generate specialized struct
    llvm::StructType* monomorphizeEnum(const std::string& baseName, const std::vector<vyb::ast::TypeNodePtr>& typeArgs);   // Generate specialized tagged-union enum
    std::vector<vyb::ast::TypeNodePtr> applyTypeSubstitutions(const std::vector<vyb::ast::TypeNodePtr>& typeArgs);
    void generateTypeMetadata(const std::string& typeName, vyb::ast::StructDeclaration* structDecl); // Generate type metadata for JSON/reflection
    void generateEnumTypeMetadata(const std::string& typeName, vyb::ast::EnumDeclaration* enumDecl);  // Generate enum metadata for JSON round-trip
    void registerTypeMetadata(); // Register all type metadata at program startup
    void registerTypeNames(); // Register all compile-time-known type names in the runtime type registry
    llvm::Function* getCurrentFunction();
    llvm::BasicBlock* getCurrentBasicBlock();
    void createFunctionForwardDeclaration(vyb::ast::FunctionDeclaration* funcDecl); // Forward declaration helper

    // Error and warning reporting
    void logError(const SourceLocation& loc, const std::string& message);
    // Speculative type probes (e.g. "is this enum's payload sized yet?") must not
    // emit diagnostics; the enum is re-processed without suppression afterwards,
    // so a genuine failure is still reported exactly once.
    bool m_suppressLogging = false;
    void logWarning(const SourceLocation& loc, const std::string& message); // Added this line
    llvm::Value* createEntryBlockAlloca(llvm::Function* func, const std::string& varName, llvm::Type* type);
    llvm::AllocaInst* createEntryBlockAlloca(llvm::Type* type, const std::string& name);


    // Type system helpers
    std::string getTypeName(llvm::Type* type);
    llvm::Type* getPointeeTypeInfo(llvm::Value* ptr);
    llvm::Function* getLitConversionFunction();
    bool isLitIntrinsicCall(vyb::ast::Expression* expr);
    bool functionBodyReturnsLitIntrinsic(vyb::ast::BlockStatement* body);
    std::string extractOriginalTypeNameFromSemantics(vyb::ast::Expression* expr);
    std::string extractOriginalTypeNameFromAST(vyb::ast::Expression* expr);

    llvm::Value* tryCast(llvm::Value* value, llvm::Type* targetType, const vyb::SourceLocation& loc);

    // Store a produced value into a result slot, wrapping a raw char* (e.g. the
    // result of a primitive .to_string()) into a String { ptr, i64 } struct so the
    // length field is set. Without the wrap, a char* stored into a String slot
    // leaves the length at zero and the String compares unequal / reports length 0.
    void storeIntoResultSlot(llvm::Value* value, llvm::AllocaInst* slot,
                             const vyb::SourceLocation& loc);

    // Generic trait method monomorphization
    struct TypePattern {
        std::string base;                    // e.g., "Box"
        std::vector<std::string> args;       // e.g., ["Int"] or ["T"]

        static TypePattern parse(const std::string& typeStr);
        bool matchesPattern(const TypePattern& concrete, std::map<std::string, std::string>& substitutions) const;
        std::string toMangled() const;       // e.g., "Box<Int>" -> "Box_Int"
    };

    llvm::Function* monomorphizeTraitMethod(const std::string& concreteType,
                                           const std::string& traitName,
                                           const std::string& methodName);
    // Bind methods whose bodies were generated ON DEMAND from a forward reference
    // inside another verb of the same bind (#344). The eager bind pass visits the
    // same AST node afterwards and must not report it as a redefinition.
    std::set<std::string> onDemandGeneratedMethods;
    // #344: generate a CONCRETE (non-generic) bind method on demand. Bind methods
    // are emitted in declaration order, so a verb whose body calls a SIBLING verb
    // declared later ("forward reference") has no LLVM function to look up yet --
    // codegen reported "Function self.<verb> not found.", returned undef, and the
    // program still ran with exit 0 and a garbage value. Generating the sibling
    // here makes the call resolve independently of declaration order.
    llvm::Function* generateConcreteBindMethodOnDemand(const std::string& concreteType,
                                                      const std::string& traitName,
                                                      const std::string& methodName);
    std::string extractBasePattern(const std::string& concreteType);
    std::string getFullTypeName(vyb::ast::Expression* expr);
    vyb::ast::TypeNodePtr typePatternToTypeNode(const TypePattern& pattern,
                                                const vyb::SourceLocation& loc);

    // Generic function monomorphization
    llvm::Function* monomorphizeGenericFunction(const std::string& functionName,
                                               const std::vector<std::string>& concreteTypeArgs);
    std::string mangleGenericFunctionName(const std::string& baseName,
                                          const std::vector<std::string>& typeArgs);
    // Register a struct's declared constructors as synthetic generic functions
    // (`__ctor_<Struct>_<N>`) and record their arities for construction dispatch.
    void registerStructConstructors(vyb::ast::StructDeclaration* node);

    // Helper methods for monomorphization with type substitution
    llvm::Type* resolveTypeForMonomorphization(const TypePattern& pattern,
                                               const std::map<std::string, std::string>& substitutions);
    llvm::Type* resolveParameterTypeWithSubstitution(vyb::ast::TypeNode* typeNode,
                                                     const std::map<std::string, std::string>& substitutions);
    llvm::Type* resolveReturnTypeWithSubstitution(vyb::ast::TypeNode* typeNode,
                                                  const std::map<std::string, std::string>& substitutions);
    std::string replaceTypeTokens(const std::string& s, const std::string& token, const std::string& repl);

    // Current type substitutions active during monomorphization
    std::map<std::string, std::string> currentTypeSubstitutions;

    // Cache for monomorphized trait methods: "Box<Int>::show" -> Function*
    std::map<std::string, llvm::Function*> monomorphizedMethods;

    // String operations
    llvm::Value* generateStringConcatenation(llvm::Value* leftStr, llvm::Value* rightStr, SourceLocation loc);
    llvm::Value* generateStringComparison(llvm::Value* leftStr, llvm::Value* rightStr, vyb::TokenType op);

    // Array serialization
    llvm::Value* generateArraySerialization(llvm::Value* arrayPtr, vyb::ast::ArrayType* arrayType);
    llvm::Value* generateGenericSerialization(llvm::Value* objPtr, vyb::ast::TypeNode* typeNode);
    llvm::Value* generateIntToString(llvm::Value* intValue);
    llvm::Value* generateFloatToString(llvm::Value* floatValue);
    llvm::Value* generateBoolToString(llvm::Value* boolValue);
    llvm::Function* getSprintfFunction();

    // ToString conversion helpers for mixed-type string concatenation
    llvm::Value* generateToStringCall(llvm::Value* value, llvm::Type* valueType, vyb::ast::TypeNode* astType, SourceLocation loc);
    // Serialize one value as JSON for main()'s auto-serialized return. The AST type
    // drives the shape: Vec<T> -> JSON array (elements serialized recursively),
    // a named struct -> the metadata serializer's object, a String -> JSON string
    // literal, scalars -> their number/bool form. Returns a `char*` fragment, or
    // nullptr when the shape is unsupported (the caller then emits "null").
    llvm::Value* serializeMainReturnJson(llvm::Value* value, const vyb::ast::TypeNode* astType,
                                         llvm::Type* llvmType, SourceLocation loc);
    llvm::Value* generateTaggedEnumToString(llvm::Value* value, const TaggedEnumInfo& info, const std::string& typeName, SourceLocation loc);
    llvm::Value* generateMixedStringConcatenation(llvm::Value* leftValue, llvm::Value* rightValue,
                                                vyb::ast::TypeNode* leftTypeNode, vyb::ast::TypeNode* rightTypeNode,
                                                SourceLocation loc, bool freeLeftOwnedTemp = false, bool freeRightOwnedTemp = false);
    std::string resolveTypeAliasToBaseName(vyb::ast::TypeNode* typeNode);

    // IO operations
    llvm::Function* getPrintlnFunction();
    llvm::Function* getVybPrintlnFunction();
    llvm::Function* getVybPrintFunction();   // print() - no newline
    llvm::Function* getVybPrintlnIntFunction();  // println_int()
    llvm::Function* getVybPrintIntFunction();    // print_int()
    llvm::Function* getVybPrintlnBoolFunction(); // println_bool()
    llvm::Function* getVybPrintBoolFunction();   // print_bool()
    llvm::Function* getSerializeToJsonFunction();

    // Error handling runtime functions
    llvm::Function* getVybPanicFunction();
    llvm::Function* getVybUntrappedErrorFunction();

    // Error handling helpers
    void setupTrapContext(ast::BlockExpression* blockExpr, llvm::BasicBlock* continueBB);
    void cleanupTrapContext();
    llvm::Value* createErrorValue(ast::Expression* errorExpr, ast::TypeNode* errorType);
    llvm::Value* buildRuntimeErrorFromValue(const std::string& typeName, llvm::Value* errorValue, const SourceLocation& loc);
    void forwardError(llvm::Value* errorPtr, const SourceLocation& loc);
    void emitDeferredStatementsForCurrentFunction();
    void emitPropagatingErrorReturn(llvm::Value* errorPtr);

    // Stack trace helpers (Phase 6.4)
    void pushCallStackFrame(const std::string& functionName, const SourceLocation& loc, llvm::Function* llvmFunc);
    void popCallStackFrame();
    llvm::GlobalVariable* createCallStackGlobal();
    void generatePushFrameCall(const std::string& functionName, const SourceLocation& loc);
    void generatePopFrameCall();

    // Vec operations
    void handleVecMethod(vyb::ast::CallExpression* node, const std::string& objectName, const std::string& methodName);
    void handleVecMethodOnValue(vyb::ast::CallExpression* node, llvm::Value* vecValue, const std::string& methodName, vyb::ast::Expression* objectExpr);
    void handleVecPush(vyb::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);

    // #281: the *declared* element type of the Vec a method is called on
    // (`Vec<T>` -> T), so push/set lay elements out with the same stride that
    // get/set read them with. Returns nullptr when the receiver's type is not
    // known, letting callers fall back to the value's own type.
    llvm::Type* vecElementTypeFromReceiver(vyb::ast::CallExpression* node);

    // The AST element type of the Vec a method is called on (`Vec<T>` -> T node).
    vyb::ast::TypeNode* vecElementNodeFromReceiver(vyb::ast::CallExpression* node);

    // #284: deep-copy a `Vec` value used as an element of another Vec -- a fresh
    // buffer holding a copy of the payload -- so the outer slot does not share the
    // inner buffer with the source binding (dangling reads, double frees).
    // innerSizeBytes is the stride of ONE element of the copied Vec.
    llvm::Value* deepCopyVecElement(llvm::Value* vecVal, llvm::Type* vecStructTy,
                                    uint64_t innerSizeBytes);

    // #284: element stride for a type spelled as a name (the semantic layer hands
    // back strings like "Vec<Int>" / "UInt8" for expression types), used where no
    // VecType node is available.
    uint64_t elementStrideForTypeName(const std::string& typeName);

    // #297: the inner `Vec<...>` TypeNode when `tn` is a bare borrow of a Vec
    // (`their<Vec<T>>`), else nullptr. `get` on a `Vec<Vec<T>>` slot is typed as
    // such a borrow, and the value in flight is the address of the slot's
    // { ptr, i64, i64 } header rather than a struct copy.
    vyb::ast::TypeNode* borrowedVecInnerNode(const vyb::ast::TypeNode* tn);
    void handleVecPop(vyb::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);
    void handleVecLen(vyb::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);
    void handleVecGet(vyb::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);
    // #308: a Vec index operand must reach the bounds compare and the offset
    // multiply as the receiver's index type (`i64`). A chained receiver
    // (`outer.get(k).len()`) evaluates the inner call's arguments while
    // m_isLHSOfAssignment is still set by the OUTER call's pointer-mode receiver
    // evaluation, so an Identifier index yields its alloca (a `ptr`) instead of a
    // load -- the IR then fails verification ("icmp ult ptr ..., i64 ...") and the
    // process dies. Coerce here so every index site is correct regardless of how
    // the operand was lowered.
    llvm::Value* coerceVecIndexToI64(llvm::Value* index);
    void handleVecLast(vyb::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);
    void handleVecSet(vyb::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);
    void handleVecPushArray(vyb::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);
    llvm::Value* normalizeVecStringElement(llvm::Value* value);
    void handleVecToArray(vyb::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);
    void handleVecClear(vyb::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);
    void handleVecIsEmpty(vyb::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);
    void handleVecCapacity(vyb::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);
    void handleVecConcat(vyb::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);
    void handleVecContains(vyb::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);
    void handleVecRemoveAt(vyb::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);
    void handleVecResize(vyb::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);
    void handleVecGetArray(vyb::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);
    void handleVecGetVec(vyb::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);

    // String type methods
    void handleStringMethod(vyb::ast::CallExpression* node, const std::string& objectName, const std::string& methodName);
    void handleStringMethodOnValue(vyb::ast::CallExpression* node, llvm::Value* strPtr, const std::string& methodName);
    void emitChannelMethod(vyb::ast::CallExpression* node, llvm::Value* handle, const std::string& methodName, bool isString, const vyb::ast::TypeNode* elem);
    llvm::Value* encodeChannelScalar(llvm::Value* payload, const vyb::ast::TypeNode* elem);
    llvm::Value* decodeChannelScalar(llvm::Value* raw, const vyb::ast::TypeNode* elem);

    void dispatchStringMethod(vyb::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType, const std::string& methodName);
    void handleStringLen(vyb::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType);
    void handleStringConcat(vyb::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType);
    void handleStringSubstring(vyb::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType);
    void handleStringCharAt(vyb::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType);
    void handleStringToBytes(vyb::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType);
    void handleStringFromBytes(vyb::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType);
    void handleStringStartsWith(vyb::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType);
    void handleStringEndsWith(vyb::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType);
    void handleStringContains(vyb::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType);
    void handleStringToUpper(vyb::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType);
    void handleStringToLower(vyb::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType);
    void handleStringTrim(vyb::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType);
    void handleStringReplace(vyb::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType);
    void handleStringFormat(vyb::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType);

    // Scope and ownership management
    void enterScope();
    void exitScope();
    void exitToFunctionBaseline();
    void cleanupScopesToBaseline(size_t baseline);
    void registerVariable(const std::string& name, llvm::Value* allocaInst, llvm::Value* value, ast::OwnershipKind ownership, llvm::Type* type, bool needsCleanup = false, llvm::Value* ownedCellBase = nullptr);
    // #384 checkpoint (c): promote a binding an escaping closure mutably captures to a
    // refcounted cell `{ i64 refcount; ptr dtor; T value }`, shared with every
    // environment that captures it. Returns the cell base (the owner handle) and the
    // value field pointer the frame's reads and writes go through; a null valuePtr
    // means the binding is not promoted. `localSlot` is the binding's frame storage as
    // codegen currently has it (the cell is initialised from it), and `astType` is the
    // declared type used for the cell's payload reclaim.
    struct PromotedBindingCell {
        llvm::Value* base = nullptr;
        llvm::Value* valuePtr = nullptr;
    };
    PromotedBindingCell maybePromoteBinding(const std::string& name,
                                            const vyb::ast::Node* declNode,
                                            llvm::Value* localSlot,
                                            const vyb::ast::TypeNode* astType,
                                            llvm::Type* varType);
    // #384 checkpoint (c): the per-cell destructor for a promoted binding's payload
    // (`{ i64 refcount; ptr cap_dtor; T value }` -- the closure env layout, so
    // __vyb_closure_release drives it). Null when the payload needs no reclaim, in
    // which case the runtime frees the block on the last release.
    llvm::Function* generatePromotedCellDtor(llvm::StructType* cellTy,
                                             const std::string& tag,
                                             const vyb::ast::TypeNode* payloadAst);
    void cleanupVariable(const ScopeVariable& var);
    void incrementRefCount(const std::string& name);
    void decrementRefCount(const std::string& name);
    llvm::Function* getOrCreateFreeFunction();
    llvm::Function* getOrCreateVybStringFreeFunction();
    llvm::Function* getOrCreateVybStringRetainFunction();
    llvm::Function* getOrCreateVybStringRegisterFunction();
    llvm::Function* getOrCreateMallocFunction();
    llvm::Function* getOrCreateClosureRetainFunction();
    llvm::Function* getOrCreateClosureReleaseFunction();
    bool isClosureStructType(llvm::Type* type);       // `{ ptr env, ptr fn }`
    bool isFnTypeNode(const vyb::ast::TypeNode* tn) const; // true for `fn` types
    void retainClosureValue(llvm::Value* closureVal);  // +1 on a copied closure value
    void releaseClosureValue(llvm::Value* closureVal); // -1 on a closure value
    // #384 checkpoint (c): a promoted binding's shared cell reuses the closure
    // environment header (`{ i64 refcount; ptr cap_dtor; T value }`) but is not a
    // closure value, so it is retained/released through the raw runtime entry points
    // with the cell BASE pointer (the refcount lives there, not at the value field).
    void retainClosureEnvPtr(llvm::Value* envPtr);
    void releaseClosureEnvPtr(llvm::Value* envPtr);
    // The cell's value field address is what the frame and every capturing environment
    // hold; its BASE is what carries the refcount, so recover it where needed.
    llvm::Value* promotedCellBaseFromValuePtr(llvm::Value* valuePtr, llvm::StructType* cellTy);
    // #439: is this Vec element / struct field (or accessor RESULT) a closure
    // VALUE? Both the declared `fn ...` type and the closure struct
    // `{ ptr env, ptr fn }` are required, so a coincidental two-pointer value is
    // not reference counted.
    bool isClosureElementType(llvm::Type* elementLLVMType, const vyb::ast::TypeNode* astElemType);
    // #439: retain the environment a storage location (a Vec slot, a struct field)
    // or an accessor's caller now references -- the location owns one reference of
    // its own. No-op unless the value really is a closure value. When
    // `validIncoming` is non-null it is set to the block the retain left the
    // builder in -- a retain emits its own null-check blocks, so a merge PHI built
    // afterwards must name that block.
    void retainClosureRef(llvm::Value* value, llvm::Type* valueLLVMType,
                          const vyb::ast::TypeNode* astType,
                          llvm::BasicBlock** validIncoming);
    // #427 defects 6/7: is this type a `mild<T>`/`our<T>` handle? Answered from the type
    // NAME with the active monomorphization substitutions applied, because inside a
    // generic body (a declaration `x<T>` instantiated as `mild<A>`; `VecHigherOps::filter`
    // pushes into `Vec<T>`) the AST still names the type parameter -- and a storage
    // location whose retain misses while the matching release fires drives the count
    // negative instead of balancing it. `elementTypeIsMildHandle` is the `mild` case.
    bool elementTypeIsHandle(const vyb::ast::TypeNode* elemAst, const std::string& wrapper);
    bool elementTypeIsMildHandle(const vyb::ast::TypeNode* elemAst);
    bool elementTypeIsOwnedHandle(const vyb::ast::TypeNode* elemAst);
    // A Vec accessor call (`get`/`first`/`last`/`peek`) on a weak-handle slot: its result
    // is an owned reference the consuming store must drop after taking its own.
    bool argIsWeakHandleAccessorTemp(vyb::ast::Expression* arg);
    // The same "a storage location owns one reference" rule for a Vec slot / an accessor
    // result: a closure environment (#439) or a `mild` handle's weak count (#427 defects
    // 6/7, `weakSlot`). The closure case is decided by the LLVM type + AST; `weakSlot` is
    // decided by the caller, which can resolve substitutions. No-op for anything else.
    void retainElementRef(llvm::Value* value, llvm::Type* valueLLVMType,
                          const vyb::ast::TypeNode* astType, bool weakSlot,
                          llvm::BasicBlock** validIncoming);
    // The matching release for retainElementRef, used where a location drops the element
    // it held (a `set` overwrite) and for the per-element reclaim loop.
    void releaseElementValue(llvm::Value* value, llvm::Type* valueLLVMType,
                             const vyb::ast::TypeNode* astType, bool weakSlot,
                             const std::string& tag);
    // The per-element loop that retains or releases every reference-counted element in a
    // Vec's element buffer -- one implementation for the deep copy (retain) and the
    // reclaim sites (release). Leaves the builder in the loop-exit block.
    void emitElementRefLoop(llvm::Value* dataPtr, llvm::Value* count, llvm::Type* elemTy,
                            bool retain, const std::string& tag,
                            const vyb::ast::TypeNode* astElemType = nullptr,
                            bool weakSlot = false);
    void releaseClosureAlloca(llvm::Value* allocaInst); // load closure from an alloca, then -1
    // Build the per-layout destructor for a closure capture environment that
    // owns transferred `my<Struct>` payloads; reclaims each captured pointee's
    // owned fields and frees the heap block. Returns the function (or null).
    llvm::Function* generateClosureEnvDtor(
        llvm::StructType* envTy, const std::string& tag,
        const std::vector<std::pair<size_t, const vyb::ast::TypeNode*>>& ownedFields,
        const std::vector<std::pair<size_t, const vyb::ast::TypeNode*>>& boxedFields,
        // #384 checkpoint (c): capture fields holding the value-field address of a
        // PROMOTED binding's shared cell (frame and environment co-own the cell). The
        // destructor releases the cell instead of reclaiming and freeing the block.
        const std::vector<std::pair<size_t, llvm::StructType*>>& sharedCellFields = {});
    // #384 b(1): reclaim the payload held inside a boxed mutable-capture cell
    // (the cell holds a value of `astType` inline) before the cell block itself
    // is freed -- a captured String/Vec/my<T>/our<T>/struct owns storage that
    // would otherwise leak.
    void reclaimBoxedCapturePayload(llvm::Value* cellPtr, const vyb::ast::TypeNode* astType,
                                    const std::string& tag);
    // #384 b(1): the value a boxed mutable-capture cell will OWN -- a deep copy
    // for a Vec / owned struct, a fresh heap object for `my<Struct>`, a retained
    // reference for a String / `our<T>` / `mild<T>` / closure value, identity for
    // a scalar. Mirrors reclaimBoxedCapturePayload so exactly what the cell took
    // is what the cell's destructor releases.
    llvm::Value* copyCaptureValueForCell(llvm::Value* value, const vyb::ast::TypeNode* astType,
                                         llvm::Type* ty);
    // Describes one owned parameter field inside an async-task environment.
    struct AsyncEnvField {
        size_t fieldIx = 0;                    // env struct field index to reclaim
        bool isString = false;                 // a Vyb String value
        bool isVec = false;                    // a Vec<T> value
        bool vecIsString = false;              // Vec<String>: release String elements before freeing data
        bool isOur = false;                    // an `our<T>` control-block pointer (release on cleanup)
        bool isStruct = false;                 // an inline struct value (deep-copied into the env)
        bool isClosure = false;                // a closure `{ ptr env, ptr fn }` value (release env on cleanup)
        const vyb::ast::TypeNode* structType = nullptr; // AST type of the struct field (for reclaim)
    };
    // Build the per-layout destructor for an async-task environment that holds
    // inline owned param fields (String / Vec<T>): release each String buffer
    // reference or reclaim each Vec's storage, then free the heap block. Returns
    // the function (or null).
    llvm::Function* generateAsyncEnvDtor(
        llvm::StructType* envTy, const std::string& tag,
        const std::vector<AsyncEnvField>& fields);
    void retainStringValue(llvm::Value* strVal);          // +1 on a copied String value
    void releaseStringValue(llvm::Value* strVal);         // -1 on a String value
    // Take a reference / clone the heap data of `value` so a binding that
    // establishes a new owner (a match pattern binding an enum payload) reclaims
    // exactly what it holds (#345).
    llvm::Value* copyOwnedValueForBinding(llvm::Value* value, const vyb::ast::TypeNode* astType,
                                          llvm::Type* ty);
    void releaseStringAlloca(llvm::Value* allocaInst);    // load a String from an alloca, then -1
    void releaseStringElements(llvm::Value* dataPtr, llvm::Value* count); // -1 per String element in a Vec buffer
    void retainStringElements(llvm::Value* dataPtr, llvm::Value* count);  // +1 per String element in a Vec buffer
    llvm::Function* getOrCreateVybStringReleaseEachFunction();
    llvm::Function* getOrCreateVybStringRetainEachFunction();
    llvm::Function* getOrCreateMemsetFunction();
    llvm::Function* getOrCreateMemcpyFunction();
    llvm::StructType* getControlBlockType(llvm::Type* objectPtrType);

    // A handle whose payload is a String (`my<String>`, `our<String>`, `mild<String>`,
    // `their<String>`) is represented as a POINTER to the String block, so a value
    // context that wants the string itself must load through it: printing or
    // concatenating the handle used to feed the pointer to the to_string path and the
    // program emitted heap bytes (#427 defect 10). Returns `v` unchanged when `tn` is
    // not such a handle or `v` is not a pointer.
    llvm::Value* loadHandleStringPayload(llvm::Value* v, const vyb::ast::TypeNode* tn);
    bool isVecStructType(llvm::Type* type); // Check if LLVM type matches Vec{T, i64, i64} layout
    bool isVybStringStructType(llvm::Type* type); // `{ ptr, i64 }` Vyb String layout
    bool isOptionalStructType(llvm::Type* type); // literal `{ T, i1 }` native `T?`
    llvm::Value* generateOptionalEquality(llvm::Value* L, llvm::Value* R, vyb::TokenType op); // presence+payload == / !=
    bool exprProducesOwnedStringTemp(vyb::ast::Expression* expr); // String expr yielding a fresh owned heap buffer
    // A Vec expression that builds a fresh buffer with no named owner: a
    // Vec-returning call (`payload.split("\n")`, `v5(a, b)`) or a `Vec<T>()`
    // literal. A read of a binding/field is owned by that location's own cleanup;
    // element accessors that borrow from the receiver's buffer (`get`, `first`,
    // `last`, `get_vec`, `pop`) and the mutators that hand that same buffer back
    // (`push`, `set`, `insert`, `remove_at`, `clear`, `push_array`, `resize`,
    // `concat`) are not fresh allocations and return false here.
    bool exprProducesFreshVecTemp(vyb::ast::Expression* expr);
    bool exprIsStringTransfer(vyb::ast::Expression* expr); // String value whose single ref transfers on stow
    bool exprIsOurTransfer(vyb::ast::Expression* expr);   // `our`/`grab`/fn-call value whose fresh strong ref transfers on stow
    bool exprIsMildTransfer(vyb::ast::Expression* expr);  // `soft(...)` value whose fresh weak ref transfers on stow
    // Deep-copy a Vec struct value (clones malloc'd data so caller and callee are independent).
    // Returns an updated Vec struct value with a freshly malloc'd data buffer.
    // When `astElemType` is supplied and names a struct that owns heap data, each
    // element is deep-copied individually (retaining String buffers, cloning inner
    // Vecs) instead of a shallow memcpy, so the clone's owned fields never alias the
    // source (which would double-free when both are reclaimed).
    llvm::Value* generateVecDeepCopy(llvm::Value* vecStructValue, llvm::Type* elemType, llvm::Type* vecStructType,
                                     const vyb::ast::TypeNode* astElemType = nullptr);
    // Deep-copy a struct value into an independent owned copy: retain String
    // buffers and our/mild control blocks, clone Vec buffers and `my<Struct>`
    // heap blocks, and recurse into nested structs (scalars copied by value).
    // Mirrors reclaimStructOwnedFieldsAt so reclaiming the result balances every
    // action taken here. Returns an updated struct value.
    llvm::Value* generateStructDeepCopy(llvm::Value* structValue,
                                        const vyb::ast::TypeNode* astType,
                                        llvm::StructType* llvmTy);
    // Deep-copy a standalone `my<Struct>` heap payload so the new owner holds data
    // independent of the caller's. Mirrors generateStructDeepCopy's field handling.
    llvm::Value* deepCopyMyStruct(llvm::Value* myPtr,
                                  const vyb::ast::TypeNode* pointeeAst,
                                  llvm::StructType* pointeeTy);

    // Owned-field introspection + reclaim for struct-typed storage. Resolves a
    // struct's concrete field type nodes (substituting generic args) in layout
    // order; reclaims each owned field's heap buffer/String reference on scope exit.
    bool collectStructConcreteFieldTypes(const vyb::ast::TypeNode* astType,
                                         std::vector<vyb::ast::TypeNodePtr>& out) const;
    bool isVecOfStringTypeNode(const vyb::ast::TypeNode* tn) const;
    bool isKnownStructTypeNode(const vyb::ast::TypeNode* tn) const;
    bool isMyOwnedStructTypeNode(const vyb::ast::TypeNode* tn) const;
    const vyb::ast::TypeNode* myPointeeOf(const vyb::ast::TypeNode* tn) const;
    bool structTypeHasOwnedFields(const vyb::ast::TypeNode* astType) const;
    bool scopeVarIsOwnedStruct(const ScopeVariable& var) const;
    void reclaimOwnedStructAt(llvm::Value* structPtr, const vyb::ast::TypeNode* astType,
                              llvm::StructType* llvmTy);
    void reclaimStructOwnedFieldsAt(llvm::Value* structPtr, const vyb::ast::TypeNode* astType,
                                    llvm::StructType* llvmTy, std::set<std::string>& visited);
    // Release the storage owned by the *elements* of a Vec whose element type is
    // itself a Vec (any nesting depth): frees each level's inner buffers and drops
    // the string references of the innermost level. The caller frees the buffer it
    // passed in. Leaves the builder at a fresh continuation block.
    void emitInnerVecCleanup(llvm::Value* dataPtr, llvm::Value* elemCount,
                             const vyb::ast::TypeNode* vecAst, const std::string& tag);

    // Release the storage a Vec value owns: drop every String element reference,
    // reclaim owned struct fields per element and every nested-Vec level, then free
    // the element buffer. Leaves the builder at a fresh continuation block. Used by
    // scope-exit cleanup AND by the fresh-temporary reclaim at a call site (a Vec
    // expression result nobody can reach afterwards, e.g. `payload.split("\n").len()`
    // or `rows.push(v2(a, b))`). `vecValue` is either the Vec struct value or a
    // pointer to it; a null data pointer is a no-op. Never call this on a borrow:
    // it frees the storage, so the caller must own it.
    void reclaimVecStorage(llvm::Value* vecValue, llvm::Type* vecStructTy,
                           const vyb::ast::TypeNode* vecAst, const std::string& tag);

    // Reclaim the ORIGINAL owned buffers of a fresh owned-struct TEMP argument
    // after it was deep-copied into a Vec slot (Vec.push/set). Same leak class as
    // pendingStructTempReclaims in cgen_expr (#192) but on the Vec special-case
    // handler. A named/binding source is owned by its own cleanup and untouched.
    void reclaimFreshStructArgTemp(vyb::ast::CallExpression* node, unsigned argIdx,
                                   llvm::Value* value, llvm::Type* elementType);

    bool isOurRefType(const vyb::ast::TypeNode* tn) const;   // `our<...>` wrapper type node
    bool isMildRefType(const vyb::ast::TypeNode* tn) const;  // `mild<...>` wrapper type node

    // Pointee type node `T` of an `our<T>` / `mild<T>` wrapper (or null when the
    // node is not that ref wrapper). Used to reclaim a struct payload's owned
    // fields when its strong count drops to zero.
    const vyb::ast::TypeNode* refPointeeOf(const vyb::ast::TypeNode* tn, const std::string& kind) const;
    const vyb::ast::TypeNode* ourPointeeOf(const vyb::ast::TypeNode* tn) const;
    const vyb::ast::TypeNode* mildPointeeOf(const vyb::ast::TypeNode* tn) const;

    // The pointer a `borrow(x)` / `view(x)` must yield for its operand. A borrow
    // addresses the OBJECT, not the slot that holds it: for a plain stack struct
    // the object *is* the slot, so the slot address is the borrow; for an
    // ownership-wrapped operand the slot holds the pointee (`my`/`their`/
    // `borrow`/`view`/`ptr`) or a control-block pointer whose payload pointer
    // (field 3) is the object (`our`/`mild`). Returning the raw slot made
    // `borrow(x).field` read the pointer itself (#365 step 0).
    llvm::Value* borrowTargetPointer(llvm::Value* operandValue,
                                     const vyb::ast::TypeNode* operandType,
                                     const std::string& kw);

    // Step (c) of #365. The semantic pass judges a spawn-site closure's captures
    // from their *declared* types, so a capture typed as a generic function's
    // type parameter (`T`) stays permissive there -- that pass holds no
    // monomorphization data. Codegen does hold it (`currentTypeSubstitutions`),
    // so at the spawn call sites it re-runs the SAME structural predicate on the
    // substituted concrete type (vyb/vre/thread_boundary.hpp), which is what
    // makes the verdict real for generic code.
    void checkSpawnHandoffWithSubstitutions(vyb::ast::FunctionExpression* fe,
                                           vyb::ast::Node* site,
                                           const std::string& siteName);
    // The substituted type of a capture at this point in codegen, or null when
    // codegen has no recorded type for it. Used by the deferred thread-boundary
    // check: the semantic pass recorded which captures it could not decide, and
    // this resolves each of those to the concrete type of the instantiation.
    vyb::ast::TypeNodePtr substitutedCaptureType(const std::string& name,
                                                const vyb::SourceLocation& loc);

    // Retain an `our`/`mild` refcount control block: bump the strong (our) or
    // weak (mild) count on the shared block so a new storage location that will
    // release on scope exit holds its own reference. `controlBlockPtr` may be null.
    void retainOurControlBlock(llvm::Value* controlBlockPtr, const std::string& tag);
    // Release an `our`/`mild` refcount control block (shared by top-level
    // bindings and struct fields). `controlBlockPtr` may be null. When the strong
    // count drops to zero, the payload struct is freed; if the pointee type is a
    // struct with owned fields, those fields are reclaimed first so nested
    // resources (inner our/mild refs, Vec storage, String buffers, my blocks) are
    // not leaked. `pointeeAst`/`pointeeLlvm` may be null (treated as scalar/no-op).
    // `visited`, when supplied, is the caller's reclaim set: a RECURSIVE payload
    // type (e.g. `struct Node { next<our<Node>> }`) must share it or the reclaim
    // expansion recurses without bound in the compiler (#391 defect 5).
    void releaseOurControlBlock(llvm::Value* controlBlockPtr, const std::string& tag,
                                const vyb::ast::TypeNode* pointeeAst = nullptr,
                                llvm::Type* pointeeLlvm = nullptr,
                                std::set<std::string>* visited = nullptr);
    void releaseMildControlBlock(llvm::Value* controlBlockPtr, const std::string& tag);
    // Retain a `mild` weak-count control block: bump the weak count so a new
    // storage location that will release on scope exit holds its own weak ref.
    // `controlBlockPtr` may be null.
    void retainMildControlBlock(llvm::Value* controlBlockPtr, const std::string& tag);

    // Data-carrying built-in enums (Option<T>, Result<T, E>) whose payload is an
    // `our<T>` reference own a strong count on a shared control block. Copies
    // (Some(owner)) must retain, and scope exit must release, so the control
    // block is reclaimed once the last owner drops.
    bool enumPayloadHoldsOurRef(const vyb::ast::TypeNode* astType) const;
    void reclaimEnumOurPayload(llvm::Value* enumPtr, const vyb::ast::TypeNode* astType,
                               bool retain);
    // Retain/release the `our<T>` strong ref inside an `Optional<our<T>>`
    // ({ value(0)=cb, hasValue(1) }). Releases a grabbed `our<T>?` on scope exit.
    void reclaimOptionalOurPayload(llvm::Value* optPtr, const vyb::ast::TypeNode* optAst,
                                   llvm::StructType* optLlvm, bool retain);
    // The `mild` twin: drop the one WEAK count an accessor took when it handed back
    // a `mild<A>?` (`get`/`last`/an iterator's `next()`). Layout is the same
    // ({ value(0)=cb, hasValue(1) }); only the count decremented differs.
    void reclaimOptionalMildPayload(llvm::Value* optPtr, llvm::StructType* optLlvm);
    // Trap matching (#157): aspect name -> concrete type names that bind it (from the
    // module's `bind <Aspect> -> <Type>` declarations). An aspect-typed trap matches
    // every such concrete type. Built lazily on first trap dispatch.
    std::map<std::string, std::vector<std::string>> m_aspectBindTypes;
    bool m_aspectBindTypesBuilt = false;
    void ensureAspectBindTypes();
    bool trapTypeIsAspect(const vyb::ast::TypeNode* tn);
    bool enumInitIsOurTransfer(vyb::ast::Expression* init);

    // Async/await support
    struct AsyncState {
        llvm::Function* asyncFunction;
        llvm::Function* stateMachineFunction;
        llvm::StructType* stateStructType;
        llvm::Value* stateStructInstance;
        llvm::Value* currentStateValue;
        llvm::BasicBlock* resumeBlock;
        llvm::Value* futureValue;
        llvm::Type* futureResultType;  // Result type T for Future<T> (opaque pointer tracking)
        int stateCounter;
        bool isAsync;

        // Debug support for async state machines
        llvm::DILocalVariable* stateDebugVar;
        llvm::DILocalVariable* futureDebugVar;
        std::map<int, llvm::DILocation*> suspensionPointLocations;
        std::map<int, std::string> stateDescriptions;

        AsyncState() : asyncFunction(nullptr), stateMachineFunction(nullptr),
                       stateStructType(nullptr), stateStructInstance(nullptr),
                       currentStateValue(nullptr), resumeBlock(nullptr),
                       futureValue(nullptr), futureResultType(nullptr), stateCounter(0), isAsync(false),
                       stateDebugVar(nullptr), futureDebugVar(nullptr) {}
    };

    AsyncState currentAsyncState;
    llvm::Function* getOrCreateScheduleTaskFunction();
    llvm::Function* getOrCreateAwaitTaskFunction();
    llvm::Function* getOrCreateCreateFutureFunction();
    /// Cache of Future<T> struct types keyed by their result type so that all
    /// references to a given Future<T> (async fn return, explicit variable type,
    /// launcher) share one canonical LLVM struct type instead of distinct ones.
    std::map<llvm::Type*, llvm::StructType*> futureStructCache;
    llvm::StructType* createFutureStructType(llvm::Type* resultType);
    /// Phase-1/2 real async: an `async fn(...)<Future<Int>>` runs its body as a
    /// task on the cooperative event loop. Splits the declaration into a public
    /// launcher (`fn(params...)` returns a Future, spawning the task with a
    /// closure env snapshotting the args) and a hidden worker
    /// (`<fn>$__async_body` is a plain `fn(params...) -> Int`) that runs the
    /// body, dispatched from an `i64(void*)` entry trampoline that unpacks env.
    void codegenAsyncTask(vyb::ast::FunctionDeclaration* node);
    /// Async lambda: `async |x| -> await process(x)`. The lambda body is compiled
    /// as a normal closure (reused worker); the outer closure value launches a
    /// cooperative task that runs that closure and returns a Future<T> to the
    /// caller. Mirrors the worker + env snapshot + entry + launcher split that
    /// real async functions use, composed with the closure {env,fn} value.
    void codegenAsyncLambda(vyb::ast::FunctionExpression* node);

    // Ensure all core intrinsic functions are declared
    void ensureCoreIntrinsicFunctions();

    // Debug information support
    void initializeDebugInfo(const std::string& filename);
    void finalizeDebugInfo();
    llvm::DISubprogram* createDebugFunctionInfo(llvm::Function* function, const std::string& name,
                                                const SourceLocation& loc, bool isAsync = false);
    void setDebugLocation(const SourceLocation& loc);
    void pushDebugScope(llvm::DIScope* scope);
    void popDebugScope();
    llvm::DIType* getDebugType(llvm::Type* llvmType, const std::string& typeName = "");
    llvm::DILocalVariable* createDebugVariableInfo(const std::string& varName, llvm::DIType* debugType,
                                                   const SourceLocation& loc, llvm::DIScope* scope = nullptr);
    void insertDebugVariableDeclaration(llvm::DILocalVariable* debugVar, llvm::Value* alloca,
                                        const SourceLocation& loc);

    // Async state machine debug support
    void initializeAsyncStateDebugInfo(const std::string& functionName, const SourceLocation& loc);
    void createSuspensionPointDebugInfo(int stateNumber, const SourceLocation& loc, const std::string& description);
    void insertAsyncStateTransitionDebugInfo(int fromState, int toState, const SourceLocation& loc);
    void insertContinuationDebugMarker(int stateNumber, const SourceLocation& loc);

    // RTTI (Run-Time Type Information)
    llvm::StructType* getOrCreateRTTIStructType();
    llvm::Value* generateRTTIObject(const std::string& typeName, int typeId); // typeId for distinguishing types

    // Loop handling
    void pushLoop(llvm::BasicBlock* header, llvm::BasicBlock* body, llvm::BasicBlock* update, llvm::BasicBlock* exit, const std::string& label = "");
    void popLoop();

    // Struct field access
    int getStructFieldIndex(llvm::StructType* structType, const std::string& fieldName);
    void bindStructPatternFields(const vyb::ast::StructPattern* node, llvm::Value* matchValue);

public:
    // Visitor methods overridden from vyb::Visitor, corrected to match ast.hpp
    // Literals
    void visit(vyb::ast::Identifier* node) override;
    void visit(vyb::ast::IntegerLiteral* node) override;
    void visit(vyb::ast::FloatLiteral* node) override;
    void visit(vyb::ast::StringLiteral* node) override;
    void visit(vyb::ast::BooleanLiteral* node) override;
    void visit(vyb::ast::ObjectLiteral* node) override;
    void visit(vyb::ast::NilLiteral* node) override;

    // Expressions
    void visit(vyb::ast::UnaryExpression* node) override;
    void visit(vyb::ast::BinaryExpression* node) override;
    void visit(vyb::ast::CallExpression* node) override; // Ensure this is declared
    void visit(vyb::ast::MemberExpression* node) override; // Ensure this is declared
    void visit(vyb::ast::AssignmentExpression* node) override; // Ensure this is declared
    void visit(vyb::ast::ArrayLiteral* node) override;
    void visit(vyb::ast::BorrowExpression* node) override;
    void visit(vyb::ast::PointerDerefExpression* node) override;
    void visit(vyb::ast::AddrOfExpression* node) override;
    void visit(vyb::ast::FromIntToLocExpression* node) override;
    void visit(vyb::ast::ArrayElementExpression* node) override;
    void visit(vyb::ast::LocationExpression* node) override;
    void visit(vyb::ast::ListComprehension* node) override;
    void visit(vyb::ast::IfExpression* node) override; // Added this line
    void visit(vyb::ast::ConstructionExpression* node) override; // Existing "Added"
    void visit(vyb::ast::ArrayInitializationExpression* node) override; // Existing "Added
    void visit(vyb::ast::TypeofExpression* node) override;
    void visit(vyb::ast::TypenameExpression* node) override;
    void visit(vyb::ast::AsExpression* node) override;
    void visit(vyb::ast::LogicalExpression* node) override;
    void visit(vyb::ast::ConditionalExpression* node) override;
    void visit(vyb::ast::SequenceExpression* node) override;
    void visit(vyb::ast::FunctionExpression* node) override;
    void visit(vyb::ast::ThisExpression* node) override;
    void visit(vyb::ast::SuperExpression* node) override;
    void visit(vyb::ast::AwaitExpression* node) override;
    void visit(vyb::ast::RangeExpression* node) override;
    void visit(vyb::ast::BlockExpression* node) override;
    void visit(vyb::ast::SelectExpression* node) override;
    void visit(vyb::ast::ComparisonPattern* node) override;
    void visit(vyb::ast::StructPattern* node) override;

    // Add missing visit methods for expressions from ast.hpp if they are defined there
    // and are causing linker errors.
    // Based on linker errors, AssignmentExpression is already declared.
    // CallExpression and MemberExpression need to be checked if they are part of ast::Visitor
    // and implemented in LLVMCodegen.
    // It seems CallExpression and MemberExpression were expected by the vtable.

    // Statements
    void visit(vyb::ast::BlockStatement* node) override;
    void visit(vyb::ast::ExpressionStatement* node) override;
    void visit(vyb::ast::IfStatement* node) override;
    void visit(vyb::ast::WhileStatement* node) override;
    void visit(vyb::ast::ForStatement* node) override;
    void visit(vyb::ast::ReturnStatement* node) override;
    void visit(vyb::ast::PassStatement* node) override;
    void visit(vyb::ast::BreakStatement* node) override;
    void visit(vyb::ast::ContinueStatement* node) override;
    void visit(vyb::ast::FreedomStatement* node) override;
    void visit(vyb::ast::EmptyStatement* node) override;
    void visit(vyb::ast::ExternStatement* node) override;
    void visit(vyb::ast::YieldStatement* node) override;
    void visit(vyb::ast::YieldReturnStatement* node) override;
    void visit(vyb::ast::MatchStatement* node) override; // Added this line
    void visit(vyb::ast::MatchExpression* node) override;
    void codegenMatch(vyb::ast::MatchStatement* node, llvm::AllocaInst* resultAlloca);
    void visit(vyb::ast::TryStatement* node) override; // Added this line

    // Declarations
    void visit(vyb::ast::VariableDeclaration* node) override;
    void visit(vyb::ast::FunctionDeclaration* node) override;
    void visit(vyb::ast::TypeAliasDeclaration* node) override;
    void visit(vyb::ast::ImportDeclaration* node) override;
    void visit(vyb::ast::StructDeclaration* node) override;
    void visit(vyb::ast::ClassDeclaration* node) override;
    void visit(vyb::ast::FieldDeclaration* node) override;
    void visit(vyb::ast::BindDeclaration* node) override;
    void visit(vyb::ast::EnumDeclaration* node) override;
    void visit(vyb::ast::EnumVariant* node) override;
    void visit(vyb::ast::GenericParameter* node) override;
    void visit(vyb::ast::TemplateDeclaration* node) override;
    void visit(vyb::ast::AspectDeclaration* node) override;
    void visit(vyb::ast::NamespaceDeclaration* node) override;
    void visit(vyb::ast::Module* node) override;
    void visit(vyb::ast::GenericInstantiationExpression* node) override;
    void visit(vyb::ast::ThrowStatement* node) override;

    // Error Handling
    void visit(vyb::ast::FailStatement* node) override;
    void visit(vyb::ast::TrapClause* node) override;
    void visit(vyb::ast::EnsureClause* node) override;
    void visit(vyb::ast::RefailStatement* node) override;
    void visit(vyb::ast::PanicStatement* node) override;
    void visit(vyb::ast::ExitStatement* node) override;
    void visit(vyb::ast::DeferStatement* node) override;
    void visit(vyb::ast::TupleDestructureAssignment* node) override;

    void visit(vyb::ast::TypeNode* node) override;
    void visit(vyb::ast::AssertStatement* node) override;
    void visit(vyb::ast::TypeName* node) override;
    void visit(vyb::ast::PointerType* node) override;
    void visit(vyb::ast::ArrayType* node) override;
    void visit(vyb::ast::VecType* node) override;
    void visit(vyb::ast::FutureType* node) override;
    void visit(vyb::ast::FunctionType* node) override;
    void visit(vyb::ast::OptionalType* node) override;
    void visit(vyb::ast::TupleTypeNode* node) override;

};

} // namespace vyb
