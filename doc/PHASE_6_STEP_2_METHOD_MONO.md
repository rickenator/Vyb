# Phase 6 Step 2: Method Monomorphization Infrastructure - IN PROGRESS

## Overview
Adding infrastructure to store generic trait implementations separately from concrete ones, enabling method monomorphization when called on specialized types.

## Completed: Storage Infrastructure

### 1. GenericImplInfo Struct
**File:** `include/vyn/semantic.hpp`

New struct to store generic trait implementation metadata:
```cpp
struct GenericImplInfo {
    std::string typePattern;           // e.g., "Box<T>"
    std::string traitName;             // e.g., "Display"
    std::vector<std::string> typeParams; // e.g., ["T"]
    ast::ImplDeclaration* declaration; // Original AST node
    std::map<std::string, ast::FunctionDeclaration*> methods; // method name -> AST
};
```

**Purpose:**
- Stores template information for generic implementations
- Keeps reference to original AST nodes for monomorphization
- Maps method names to their AST for later instantiation

### 2. Generic Trait Impl Registry
**File:** `include/vyn/semantic.hpp`

Added new map alongside existing `traitImpls`:
```cpp
// Generic trait implementation registry
// Maps "Box<T>" -> "Display" -> GenericImplInfo
std::unordered_map<std::string, std::unordered_map<std::string, 
    std::unique_ptr<GenericImplInfo>>> genericTraitImpls;
```

**Design:**
- **Key 1:** Type pattern (e.g., `"Box<T>"`)
- **Key 2:** Trait name (e.g., `"Display"`)
- **Value:** GenericImplInfo containing methods and metadata

### 3. Updated registerTraitImpl()
**File:** `src/vre/semantic.cpp`

Modified to distinguish generic vs concrete implementations:
```cpp
void SemanticAnalyzer::registerTraitImpl(ast::ImplDeclaration* implDecl) {
    bool isGeneric = !implDecl->genericParams.empty();
    
    if (isGeneric) {
        // Store in genericTraitImpls
        auto genericInfo = std::make_unique<GenericImplInfo>(implDecl);
        genericTraitImpls[typeName][traitName] = std::move(genericInfo);
    } else {
        // Store in traitImpls (existing behavior)
        traitImpls[typeName][traitName] = implMethods;
    }
}
```

**Logic:**
- Check if `impl` has generic parameters (e.g., `impl<T>`)
- Generic impls → stored in `genericTraitImpls`
- Concrete impls → stored in `traitImpls` (unchanged)

## Test Results
✅ **Storage Working:**
```
DEBUG: Storing generic trait impl: Display for Box<T>
DEBUG: Successfully registered impl Display for Box<T> with 1 methods
```

The generic impl is correctly identified and stored separately!

## Next: Method Call Detection & Monomorphization

### Goal
When codegen encounters `box.show()` where `box: Box<Int>`:
1. Detect that `box` has type `Box<Int>`
2. Extract base pattern: `Box<T>`
3. Find generic impl: `impl<T> Display for Box<T>`
4. Create type substitution: `T` → `Int`
5. Monomorphize method: clone AST, substitute types
6. Generate specialized function: `Box_Int_show()`
7. Cache for reuse

### Implementation Plan

#### Step 2A: Type Pattern Matching
**Challenge:** Match `Box<Int>` to pattern `Box<T>`

**Solution:**
```cpp
bool matchesPattern(const std::string& concreteType, const std::string& pattern,
                   std::map<std::string, std::string>& substitutions) {
    // Parse "Box<Int>" and "Box<T>"
    // Extract: base="Box", args=["Int"] vs base="Box", params=["T"]
    // Create mapping: substitutions["T"] = "Int"
    // Return: true if bases match
}
```

#### Step 2B: Method Lookup in CallExpression
**File:** `src/vre/llvm/cgen_expr.cpp` (around line 820)

Current code:
```cpp
// Check if this is a trait method call
if (auto memberExpr = dynamic_cast<vyn::ast::MemberExpression*>(node->callee.get())) {
    // Get object type
    std::string typeName = ...;
    // Look up method function
    llvm::Function* implFunc = module->getFunction(methodName);
}
```

**Enhancement needed:**
```cpp
// Check if this is a trait method call
if (auto memberExpr = dynamic_cast<vyn::ast::MemberExpression*>(node->callee.get())) {
    std::string concreteType = getFullTypeName(memberExpr->object);  // "Box<Int>"
    std::string methodName = getMethodName(memberExpr->property);     // "show"
    
    // Try to find monomorphized version
    std::string mangledName = concreteType + "_" + methodName; // "Box_Int_show"
    llvm::Function* func = module->getFunction(mangledName);
    
    if (!func) {
        // Not yet monomorphized - find generic impl and instantiate
        func = monomorphizeTraitMethod(concreteType, methodName);
    }
    
    // Generate call to monomorphized function
}
```

#### Step 2C: Access to Semantic Data
**Challenge:** Codegen needs access to `genericTraitImpls` from SemanticAnalyzer

**Options:**
1. **Pass SemanticAnalyzer reference through Driver**
   - Modify Driver to hold SemanticAnalyzer*
   - LLVMCodegen can access via driver_

2. **Pass data structures directly**
   - Add `setGenericImpls()` method to LLVMCodegen
   - Copy maps before codegen phase

3. **Store in Module metadata**
   - Serialize impl info as LLVM metadata
   - Access during codegen

**Recommended:** Option 1 (cleanest architecture)

#### Step 2D: Method Monomorphization Function
**New file:** `src/vre/llvm/cgen_trait_mono.cpp`

```cpp
llvm::Function* LLVMCodegen::monomorphizeTraitMethod(
    const std::string& concreteType,
    const std::string& methodName) 
{
    // 1. Extract base pattern from concrete type
    std::string basePattern;
    std::map<std::string, std::string> typeSubst;
    if (!extractPattern(concreteType, basePattern, typeSubst)) {
        return nullptr;  // Not a generic type
    }
    
    // 2. Find generic impl
    auto& impls = driver_.getSemantic Analyzer().genericTraitImpls;
    auto patternIt = impls.find(basePattern);
    if (patternIt == impls.end()) {
        return nullptr;  // No generic impl for this pattern
    }
    
    // 3. Find the method in the generic impl
    GenericImplInfo* implInfo = findImplWithMethod(patternIt->second, methodName);
    if (!implInfo) {
        return nullptr;  // Method not found
    }
    
    // 4. Clone method AST
    ast::FunctionDeclaration* originalMethod = implInfo->methods[methodName];
    auto clonedMethod = cloneMethodAST(originalMethod);
    
    // 5. Substitute types in cloned AST
    substituteTypes(clonedMethod.get(), typeSubst);
    
    // 6. Generate LLVM function
    std::string mangledName = concreteType + "_" + methodName;
    llvm::Function* func = codegenMethod(clonedMethod.get(), mangledName);
    
    // 7. Cache for future use
    monomorphizedMethods[concreteType][methodName] = func;
    
    return func;
}
```

### Test Case
**File:** `test/trait/test_method_call.vyn`

```vyn
struct Box<T> {
    value<T>
}

trait Display {
    show(self<Self>)<String> -> { }
}

impl<T> Display for Box<T> {
    show(self<Self>)<String> -> {
        return "Box"
    }
}

main()<Int> -> {
    b<Box<Int>> = Box<Int> { value = 42 }
    
    # This should trigger:
    # 1. Detect b: Box<Int>
    # 2. Match to impl<T> Display for Box<T>
    # 3. Substitute T -> Int
    # 4. Generate Box_Int_show()
    # 5. Call it
    str<String> = b.show()
    
    return 0
}
```

## Challenges

### 1. Type Name Extraction
**Problem:** Getting full type name `Box<Int>` from AST expression

**Current State:** Type info stored in `expressionTypes` map by semantic analyzer

**Solution:** Add helper to extract type name from Expression:
```cpp
std::string getFullTypeName(ast::Expression* expr) {
    if (auto ident = dynamic_cast<ast::Identifier*>(expr)) {
        if (ident->type) {
            return ident->type->toString();
        }
    }
    // Fallback to expression type lookup
}
```

### 2. Pattern Matching
**Problem:** Matching `Box<Int>` to `Box<T>` and extracting substitutions

**Complexity:**
- Handle multiple type parameters: `Map<K, V>` vs `Map<String, Int>`
- Handle nested generics: `Box<Vec<Int>>` vs `Box<T>`
- Handle partial specialization (future)

**Solution:** Parse type strings:
```cpp
struct TypePattern {
    std::string base;                    // "Box"
    std::vector<std::string> args;       // ["Int"] or ["T"]
    
    static TypePattern parse(const std::string& typeStr);
    bool matches(const TypePattern& concrete,
                std::map<std::string, std::string>& subst) const;
};
```

### 3. AST Cloning with Type Substitution
**Problem:** Deep clone AST and replace type references

**Current State:** Some clone methods exist (from monomorphization)

**Enhancement needed:**
```cpp
class TypeSubstitutor : public ast::Visitor {
    std::map<std::string, std::string> substitutions;  // "T" -> "Int"
    
    void visit(ast::TypeName* node) override {
        if (substitutions.count(node->identifier->name)) {
            // Replace with concrete type
        }
    }
};
```

### 4. Method Name Mangling
**Problem:** Generate unique function names for specialized methods

**Current State:** Structs use `_` separator (e.g., `Box_Int`)

**Consistency:** Use same scheme for methods
- `Box_Int_show` for `show()` method on `Box<Int>`
- `Vec_String_push` for `push()` on `Vec<String>`

### 5. Passing Semantic Data to Codegen
**Problem:** Codegen doesn't have access to `genericTraitImpls`

**Solution:** Enhance Driver:
```cpp
class Driver {
    SemanticAnalyzer* semanticAnalyzer = nullptr;
    
    void setSemanticAnalyzer(SemanticAnalyzer* sa) { semanticAnalyzer = sa; }
    SemanticAnalyzer& getSemanticAnalyzer() { return *semanticAnalyzer; }
};
```

Then in main workflow:
```cpp
SemanticAnalyzer semantic(driver);
driver.setSemanticAnalyzer(&semantic);
semantic.analyze(astModule);

LLVMCodegen codegen(driver);  // Can now access semantic data
codegen.generate(astModule);
```

## Progress Status

✅ **Completed:**
- GenericImplInfo struct definition
- genericTraitImpls registry
- registerTraitImpl() updated to store generic impls
- Test verification (generic impls being stored)

⏳ **In Progress:**
- Type pattern matching logic
- Method call detection enhancement
- SemanticAnalyzer access from codegen

❌ **Not Started:**
- AST cloning for methods
- Type substitution in method bodies
- Method code generation
- Caching of monomorphized methods

## Next Session Tasks

1. **Enhance Driver** (5 min)
   - Add SemanticAnalyzer* field
   - Add getter/setter methods
   - Update main.cpp to set reference

2. **Add Type Pattern Matching** (20 min)
   - Create `TypePattern` struct
   - Implement `parse()` and `matches()`
   - Test with Box<Int> vs Box<T>

3. **Update CallExpression** (15 min)
   - Extract concrete type name
   - Check for monomorphized function
   - Call monomorphization if needed

4. **Implement Method Monomorphization** (30 min)
   - Clone method AST
   - Substitute types
   - Generate LLVM function
   - Cache result

5. **Test End-to-End** (10 min)
   - Run test_method_call.vyn
   - Verify Box_Int_show() generated
   - Verify method call works

## Related Commits
- `ef76dee` - Phase 6 Step 2: Add generic trait impl storage infrastructure

## Documentation
- `doc/PHASE_6_STEP_1_SELF_RESOLUTION.md` - Prerequisites
- `doc/PHASE_6_STEP_2_METHOD_MONO.md` - This file (in progress)
