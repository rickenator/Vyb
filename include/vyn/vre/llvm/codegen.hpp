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
#include <stack>
#include <string>
#include <vector>

#include "vyn/parser/ast.hpp"
#include "vyn/semantic.hpp" // For SourceLocation, UserTypeInfo
#include "vyn/driver.hpp"   // Added to resolve Driver type

// Forward declarations
namespace llvm {
    class Value;
    class Type;
    class Function;
    class BasicBlock;
    class StructType;
    class AllocaInst;
}

namespace vyn {
    class Driver; // Forward declaration might also work if full include causes issues
}


namespace vyn {

// Helper struct for storing information about user-defined types
struct UserTypeInfo {
    llvm::StructType* llvmType;
    std::map<std::string, unsigned> fieldIndices; // Map field name to index
    bool isStruct; // True if struct, false if class (or could be enum later)
    // Potentially: vtable, parent type info, etc.
};

// Helper struct to manage loop context
struct LoopContext {
    llvm::BasicBlock *loopHeader; // Block for the loop condition check
    llvm::BasicBlock *loopBody;   // Block for the loop body
    llvm::BasicBlock *loopUpdate; // Block for the loop increment/update
    llvm::BasicBlock *loopExit;   // Block after the loop
};

// Helper struct to manage select expression context
struct SelectContext {
    llvm::BasicBlock *endBlock;   // Block after the select
    llvm::AllocaInst *resultAlloca; // Alloca for storing the result
};

class LLVMCodegen : public ast::Visitor {
public:
    // explicit LLVMCodegen(); // Old constructor
    explicit LLVMCodegen(Driver& driver); // Constructor expects a Driver reference
    virtual ~LLVMCodegen(); // Add virtual destructor declaration

    void generate(vyn::ast::Module* astModule, const std::string& outputFilename); // Add declaration
    void dumpIR() const; // Add declaration
    std::unique_ptr<llvm::Module> releaseModule(); // Add declaration
    std::unique_ptr<llvm::LLVMContext> releaseContext(); // Add declaration for context release
    llvm::Module* getModule() const { return module.get(); } // Add method to get module pointer without releasing

private:
    Driver& driver_; // Add a Driver reference
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    
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
    llvm::Type* stringType; // Placeholder for Vyn's string type representation

    // Current state
    llvm::Type* m_currentLLVMType = nullptr; // Initialize

    llvm::Value* m_currentLLVMValue = nullptr; // Unified value propagation

    // Scope and symbol management
    llvm::Function* currentFunction = nullptr; // Initialize
    llvm::StructType* currentClassType = nullptr; // Initialize
    LoopContext currentLoopContext;
    std::vector<LoopContext> loopStack;
    std::vector<SelectContext> selectStack;  // Track nested select expressions
    bool infer_types_only = false;  // Flag for type inference without codegen
    std::map<std::string, llvm::AllocaInst*> m_currentFunctionNamedValues;


    // Global and type information
    std::map<std::string, llvm::Value*> namedValues;
    std::map<std::string, UserTypeInfo> userTypeMap;
    std::map<std::string, llvm::Type*> typeParameterMap;
    std::map<std::string, llvm::Type*> typeAliasMap; // Maps type alias names to their underlying LLVM types
    std::map<vyn::ast::TypeNode*, llvm::Type*> m_typeCache;
    std::map<llvm::Value*, std::shared_ptr<vyn::ast::TypeNode>> valueTypeMap; // Maps LLVM values to AST types
    vyn::ast::TypeNode* m_currentImplTypeNode = nullptr; // Initialize
    vyn::ast::Module* m_currentVynModule = nullptr;
    bool m_isLHSOfAssignment = false;
    bool verbose = false;  // Controls detailed warning output
    bool m_isMemberAccessBase = false; // Controls Identifier behavior for member access

    // Ownership and scope tracking
    struct ScopeVariable {
        std::string name;
        llvm::Value* allocaInst;  // The alloca instruction for the variable
        llvm::Value* value;       // Current value (may be loaded from alloca)
        ast::OwnershipKind ownership;
        bool needsCleanup;
        llvm::Type* type;
        bool isVecWithMallocData; // Tracks if this is a Vec that owns malloc'd data
    };
    std::vector<std::vector<ScopeVariable>> scopeStack;
    std::map<std::string, uint32_t> refCounts; // For our<T> reference counting
    std::map<std::string, llvm::Value*> refCountStorage; // Storage for refcount variables

    // Monomorphization: Generic type instantiation
    std::map<std::string, vyn::ast::StructDeclaration*> genericStructTemplates; // Store generic struct AST nodes (e.g., Box<T>)
    std::map<std::string, llvm::StructType*> monomorphizedStructs; // Cache instantiated types (e.g., "Box<Int>" -> Box_Int LLVM type)
    
    // Helper methods
    llvm::Type* codegenType(vyn::ast::TypeNode* typeNode); // Converts vyn::TypeNode to llvm::Type
    std::string mangleGenericTypeName(const std::string& baseName, const std::vector<vyn::ast::TypeNodePtr>& typeArgs); // Generate mangled name like Box_Int
    llvm::StructType* monomorphizeStruct(const std::string& baseName, const std::vector<vyn::ast::TypeNodePtr>& typeArgs); // Generate specialized struct
    llvm::Function* getCurrentFunction();
    llvm::BasicBlock* getCurrentBasicBlock();
    void createFunctionForwardDeclaration(vyn::ast::FunctionDeclaration* funcDecl); // Forward declaration helper

    // Error and warning reporting
    void logError(const SourceLocation& loc, const std::string& message);
    void logWarning(const SourceLocation& loc, const std::string& message); // Added this line
    llvm::Value* createEntryBlockAlloca(llvm::Function* func, const std::string& varName, llvm::Type* type);
    llvm::AllocaInst* createEntryBlockAlloca(llvm::Type* type, const std::string& name);


    // Type system helpers
    std::string getTypeName(llvm::Type* type);
    llvm::Type* getPointeeTypeInfo(llvm::Value* ptr);
    llvm::Function* getLitConversionFunction();
    bool isLitIntrinsicCall(vyn::ast::Expression* expr);
    bool functionBodyReturnsLitIntrinsic(vyn::ast::BlockStatement* body);
    std::string extractOriginalTypeNameFromSemantics(vyn::ast::Expression* expr);
    std::string extractOriginalTypeNameFromAST(vyn::ast::Expression* expr);

    llvm::Value* tryCast(llvm::Value* value, llvm::Type* targetType, const vyn::SourceLocation& loc);
    
    // String operations
    llvm::Value* generateStringConcatenation(llvm::Value* leftStr, llvm::Value* rightStr, SourceLocation loc);
    
    // Array serialization
    llvm::Value* generateArraySerialization(llvm::Value* arrayPtr, vyn::ast::ArrayType* arrayType);
    llvm::Value* generateGenericSerialization(llvm::Value* objPtr, vyn::ast::TypeNode* typeNode);
    llvm::Value* generateIntToString(llvm::Value* intValue);
    llvm::Value* generateFloatToString(llvm::Value* floatValue);
    llvm::Value* generateBoolToString(llvm::Value* boolValue);
    llvm::Function* getSprintfFunction();
    
    // ToString conversion helpers for mixed-type string concatenation  
    llvm::Value* generateToStringCall(llvm::Value* value, llvm::Type* valueType, vyn::ast::TypeNode* astType, SourceLocation loc);
    llvm::Value* generateMixedStringConcatenation(llvm::Value* leftValue, llvm::Value* rightValue, 
                                                vyn::ast::TypeNode* leftTypeNode, vyn::ast::TypeNode* rightTypeNode, SourceLocation loc);
    std::string resolveTypeAliasToBaseName(vyn::ast::TypeNode* typeNode);
    
    // IO operations
    llvm::Function* getPrintlnFunction();
    llvm::Function* getVynPrintlnFunction();
    llvm::Function* getSerializeToJsonFunction();
    
    // Vec operations
    void handleVecMethod(vyn::ast::CallExpression* node, const std::string& objectName, const std::string& methodName);
    void handleVecMethodOnValue(vyn::ast::CallExpression* node, llvm::Value* vecValue, const std::string& methodName, vyn::ast::Expression* objectExpr);
    void handleVecPush(vyn::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);
    void handleVecPop(vyn::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);
    void handleVecLen(vyn::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);
    void handleVecGet(vyn::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);
    void handleVecPushArray(vyn::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);
    void handleVecToArray(vyn::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);
    void handleVecClear(vyn::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);
    void handleVecIsEmpty(vyn::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);
    void handleVecCapacity(vyn::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);
    void handleVecConcat(vyn::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);
    void handleVecContains(vyn::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);
    void handleVecRemoveAt(vyn::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);
    void handleVecResize(vyn::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);
    void handleVecGetArray(vyn::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);
    void handleVecGetVec(vyn::ast::CallExpression* node, llvm::Value* vecPtr, llvm::Type* vecStructType);

    // String type methods
    void handleStringMethod(vyn::ast::CallExpression* node, const std::string& objectName, const std::string& methodName);
    void handleStringLen(vyn::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType);
    void handleStringConcat(vyn::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType);
    void handleStringSubstring(vyn::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType);
    void handleStringCharAt(vyn::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType);
    void handleStringToBytes(vyn::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType);
    void handleStringFromBytes(vyn::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType);
    void handleStringStartsWith(vyn::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType);
    void handleStringEndsWith(vyn::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType);
    void handleStringContains(vyn::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType);
    void handleStringToUpper(vyn::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType);
    void handleStringToLower(vyn::ast::CallExpression* node, llvm::Value* strPtr, llvm::Type* strStructType);

    // Scope and ownership management
    void enterScope();
    void exitScope();
    void registerVariable(const std::string& name, llvm::Value* allocaInst, llvm::Value* value, ast::OwnershipKind ownership, llvm::Type* type, bool needsCleanup = false);
    void cleanupVariable(const ScopeVariable& var);
    void incrementRefCount(const std::string& name);
    void decrementRefCount(const std::string& name);
    llvm::Function* getOrCreateFreeFunction();
    llvm::Function* getOrCreateMallocFunction();
    llvm::Function* getOrCreateMemsetFunction();

    // Async/await support
    struct AsyncState {
        llvm::Function* asyncFunction;
        llvm::Function* stateMachineFunction;
        llvm::StructType* stateStructType;
        llvm::Value* stateStructInstance;
        llvm::Value* currentStateValue;
        llvm::BasicBlock* resumeBlock;
        llvm::Value* futureValue;
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
                       futureValue(nullptr), stateCounter(0), isAsync(false),
                       stateDebugVar(nullptr), futureDebugVar(nullptr) {}
    };
    
    AsyncState currentAsyncState;
    llvm::Function* getOrCreateScheduleTaskFunction();
    llvm::Function* getOrCreateAwaitTaskFunction();
    llvm::Function* getOrCreateCreateFutureFunction();
    llvm::StructType* createFutureStructType(llvm::Type* resultType);

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
    void pushLoop(llvm::BasicBlock* header, llvm::BasicBlock* body, llvm::BasicBlock* update, llvm::BasicBlock* exit);
    void popLoop();

    // Struct field access
    int getStructFieldIndex(llvm::StructType* structType, const std::string& fieldName);

public:
    // Visitor methods overridden from vyn::Visitor, corrected to match ast.hpp
    // Literals
    void visit(vyn::ast::Identifier* node) override;
    void visit(vyn::ast::IntegerLiteral* node) override;
    void visit(vyn::ast::FloatLiteral* node) override;
    void visit(vyn::ast::StringLiteral* node) override;
    void visit(vyn::ast::BooleanLiteral* node) override;
    void visit(vyn::ast::ObjectLiteral* node) override;
    void visit(vyn::ast::NilLiteral* node) override;

    // Expressions
    void visit(vyn::ast::UnaryExpression* node) override;
    void visit(vyn::ast::BinaryExpression* node) override;
    void visit(vyn::ast::CallExpression* node) override; // Ensure this is declared
    void visit(vyn::ast::MemberExpression* node) override; // Ensure this is declared
    void visit(vyn::ast::AssignmentExpression* node) override; // Ensure this is declared
    void visit(vyn::ast::ArrayLiteral* node) override;
    void visit(vyn::ast::BorrowExpression* node) override;
    void visit(vyn::ast::PointerDerefExpression* node) override;
    void visit(vyn::ast::AddrOfExpression* node) override;
    void visit(vyn::ast::FromIntToLocExpression* node) override;
    void visit(vyn::ast::ArrayElementExpression* node) override;
    void visit(vyn::ast::LocationExpression* node) override; 
    void visit(vyn::ast::ListComprehension* node) override;
    void visit(vyn::ast::IfExpression* node) override; // Added this line
    void visit(vyn::ast::ConstructionExpression* node) override; // Existing "Added"
    void visit(vyn::ast::ArrayInitializationExpression* node) override; // Existing "Added
    void visit(vyn::ast::LogicalExpression* node) override;
    void visit(vyn::ast::ConditionalExpression* node) override;
    void visit(vyn::ast::SequenceExpression* node) override;
    void visit(vyn::ast::FunctionExpression* node) override;
    void visit(vyn::ast::ThisExpression* node) override;
    void visit(vyn::ast::SuperExpression* node) override;
    void visit(vyn::ast::AwaitExpression* node) override;
    void visit(vyn::ast::RangeExpression* node) override;
    void visit(vyn::ast::BlockExpression* node) override;
    void visit(vyn::ast::SelectExpression* node) override;
    void visit(vyn::ast::ComparisonPattern* node) override;

    // Add missing visit methods for expressions from ast.hpp if they are defined there
    // and are causing linker errors.
    // Based on linker errors, AssignmentExpression is already declared.
    // CallExpression and MemberExpression need to be checked if they are part of ast::Visitor
    // and implemented in LLVMCodegen.
    // It seems CallExpression and MemberExpression were expected by the vtable.

    // Statements
    void visit(vyn::ast::BlockStatement* node) override;
    void visit(vyn::ast::ExpressionStatement* node) override;
    void visit(vyn::ast::IfStatement* node) override;
    void visit(vyn::ast::WhileStatement* node) override;
    void visit(vyn::ast::ForStatement* node) override;
    void visit(vyn::ast::ReturnStatement* node) override;
    void visit(vyn::ast::PassStatement* node) override;
    void visit(vyn::ast::BreakStatement* node) override;
    void visit(vyn::ast::ContinueStatement* node) override;
    void visit(vyn::ast::UnsafeStatement* node) override;
    void visit(vyn::ast::EmptyStatement* node) override;
    void visit(vyn::ast::ExternStatement* node) override;
    void visit(vyn::ast::YieldStatement* node) override;
    void visit(vyn::ast::YieldReturnStatement* node) override;
    void visit(vyn::ast::MatchStatement* node) override; // Added this line
    void visit(vyn::ast::TryStatement* node) override; // Added this line

    // Declarations
    void visit(vyn::ast::VariableDeclaration* node) override;
    void visit(vyn::ast::FunctionDeclaration* node) override;
    void visit(vyn::ast::TypeAliasDeclaration* node) override;
    void visit(vyn::ast::ImportDeclaration* node) override;
    void visit(vyn::ast::StructDeclaration* node) override;
    void visit(vyn::ast::ClassDeclaration* node) override;
    void visit(vyn::ast::FieldDeclaration* node) override;
    void visit(vyn::ast::ImplDeclaration* node) override;
    void visit(vyn::ast::EnumDeclaration* node) override;
    void visit(vyn::ast::EnumVariant* node) override;
    void visit(vyn::ast::GenericParameter* node) override;
    void visit(vyn::ast::TemplateDeclaration* node) override;
    void visit(vyn::ast::TraitDeclaration* node) override;
    void visit(vyn::ast::NamespaceDeclaration* node) override;
    void visit(vyn::ast::Module* node) override;
    void visit(vyn::ast::GenericInstantiationExpression* node) override;
    void visit(vyn::ast::ThrowStatement* node) override;
    void visit(vyn::ast::TypeNode* node) override;
    void visit(vyn::ast::AssertStatement* node) override;
    void visit(vyn::ast::TypeName* node) override;
    void visit(vyn::ast::PointerType* node) override;
    void visit(vyn::ast::ArrayType* node) override;
    void visit(vyn::ast::VecType* node) override;
    void visit(vyn::ast::FutureType* node) override;
    void visit(vyn::ast::FunctionType* node) override;
    void visit(vyn::ast::OptionalType* node) override;
    void visit(vyn::ast::TupleTypeNode* node) override;

};

} // namespace vyn
