# Phase 6 Step 1: Self Type Resolution

## Overview
Implemented Self type resolution in generic trait implementations, enabling code like:
```vyn
impl<T> Display for Box<T> {
    show(self<Self>)<String> -> { ... }
}
```
where `Self` correctly resolves to `Box<T>` during semantic analysis.

## Implementation Details

### 1. Semantic Analyzer Enhancement
**File:** `include/vyn/semantic.hpp`
- Added `ast::TypeNode* currentImplType` field to track the implementing type

**File:** `src/vre/semantic.cpp`

#### Context Tracking
In `visit(ImplDeclaration*)`:
```cpp
ast::TypeNode* previousImplType = currentImplType;
currentImplType = node->selfType.get();  // Set to Box<T>
// ... process methods ...
currentImplType = previousImplType;  // Restore context
```

#### Self Resolution
In `visit(TypeName*)`:
```cpp
if (typeNameStr == "Self") {
    if (currentImplType) {
        // Inside impl block - resolve to implementing type
        std::cout << "DEBUG: Resolving Self to " << currentImplType->toString() << std::endl;
        expressionTypes[node] = currentImplType;
        return;
    } else {
        // In trait declaration - treat as placeholder
        std::cout << "DEBUG: Self used as placeholder in trait declaration" << std::endl;
        node->type = std::shared_ptr<ast::TypeNode>(node->clone());
        return;
    }
}
```

### 2. Codegen Improvements
**File:** `src/vre/llvm/cgen_decl.cpp`

#### Skip Trait Declarations
Trait declarations are interfaces with placeholder types (like `Self`):
```cpp
void LLVMCodegen::visit(ast::TraitDeclaration* node) {
    // Skip visiting trait method signatures - they contain placeholder types like 'Self'
    // which have no concrete LLVM representation
    std::cout << "DEBUG: Trait '" << traitName << "' declaration skipped in codegen" << std::endl;
    m_currentLLVMValue = nullptr;
}
```

#### Skip Generic Impl Blocks
Generic impl blocks are templates that don't generate code until instantiated:
```cpp
void LLVMCodegen::visit(vyn::ast::ImplDeclaration* node) {
    // Check if this is a generic impl block
    if (!node->genericParams.empty()) {
        std::cout << "DEBUG: Skipping generic impl block for " << node->selfType->toString() 
                  << " - codegen happens on instantiation" << std::endl;
        m_currentLLVMValue = nullptr;
        return;
    }
    // ... handle non-generic impl blocks ...
}
```

## Design Rationale

### Why Track Current Impl Type?
The `currentImplType` field provides context during semantic analysis:
- When entering an impl block, we know what type is being implemented
- When visiting method signatures, we can resolve `Self` references
- When exiting, we restore the previous context (supports nested scenarios)

### Why Different Handling for Traits vs Impls?
- **Trait declarations**: `Self` is a placeholder (like a type parameter)
  - No resolution needed
  - Set `node->type` to clone of itself
  - Skip in codegen (no concrete type exists)

- **Impl blocks**: `Self` refers to the implementing type
  - Resolution to `Box<T>`, `Vec<T>`, etc.
  - Stored in `expressionTypes` map
  - Generic impls skipped in codegen (templates)
  - Non-generic impls generate actual methods

### Why Skip Generic Impls in Codegen?
Generic impl blocks like `impl<T> Display for Box<T>` are **templates**, not concrete implementations:
- `T` is a type parameter, not a real type
- Can't generate LLVM IR for `Box<T>` until `T` is known
- Code generation happens during **instantiation** (e.g., `Box<Int>.show()`)
- This is analogous to C++ template functions

## Test Case
**File:** `test/trait/test_self_resolution.vyn`
```vyn
// Generic struct
struct Box<T> {
    value<T>
}

// Trait with Self in signature
trait Display {
    show(self<Self>)<String> -> { }
}

// Generic impl using Self
impl<T> Display for Box<T> {
    show(self<Self>)<String> -> {
        return "Box"
    }
}

main()<Int> -> {
    return 0
}
```

### Test Results
✅ **Parsing:** Successfully parses `impl<T> Display for Box<T>`  
✅ **Semantic Analysis:** Resolves `Self` to `Box<T>` in method signature  
✅ **Codegen:** Skips generic impl, generates main function  
✅ **Execution:** Program runs without errors

### Debug Output
```
DEBUG: Registering trait: Display
DEBUG: Trait 'Display' registered successfully with 1 methods
DEBUG: Processing generic impl with 1 type parameters
DEBUG: Set currentImplType to Box<T> for Self resolution
DEBUG: Resolving Self to Box<T>
DEBUG: Successfully registered impl Display for Box<T> with 1 methods
Semantic analysis completed
DEBUG: Trait 'Display' declaration skipped in codegen (interface only)
DEBUG: Skipping generic impl block for Box<T> - codegen happens on instantiation
LLVM IR generation completed
```

## Next Steps

### Phase 6 Step 2: Method Monomorphization Infrastructure
Goal: Detect method calls and trigger monomorphization
- Add `genericMethodTemplates` map to store method AST nodes
- Store methods during impl processing: `genericMethodTemplates["Box<T>"]["show"] = methodNode`
- Detect method calls: When codegen sees `box.show()` and `box` has type `Box<Int>`
- Trigger monomorphization: Check if specialized version exists, if not, create it

### Phase 6 Step 3: Method Body Type Substitution
Goal: Clone and specialize method bodies
- Clone method AST node
- Create type substitution map: `{ "T": Int, "Self": Box<Int> }`
- Walk cloned AST, replacing type parameters
- Generate LLVM function: `Box_Int_show(Box_Int* self)`
- Cache to avoid duplicates

### Phase 6 Step 4: End-to-End Testing
Goal: Verify complete generic trait method workflow
```vyn
trait From<T> {
    from(value<T>)<Self> -> { }
}

impl<U> From<U> for Box<U> {
    from(value<U>)<Self> -> {
        return Box<U> { value = value }
    }
}

main()<Int> -> {
    box<Box<Int>> = Box<Int>.from(42)  // Should work!
    return 0
}
```

## Challenges Overcome

### 1. shared_ptr vs Raw Pointer
**Problem:** Initially tried `node->type = currentImplType` but got compilation error:
```
no match for operator= (operand types are std::shared_ptr<ast::TypeNode> and ast::TypeNode*)
```

**Solution:** Use `expressionTypes[node] = currentImplType` instead, storing the mapping separately.

### 2. Trait Method Codegen
**Problem:** Codegen tried to generate LLVM types for trait method signatures containing `Self`.

**Solution:** Skip visiting trait method signatures entirely - they're just interface definitions.

### 3. Generic Impl Codegen
**Problem:** Codegen tried to monomorphize `Box<T>` where `T` is a type parameter.

**Solution:** Check `node->genericParams` and skip generic impl blocks - they're templates.

## Benefits

1. **Type Safety:** `Self` references are validated during semantic analysis
2. **Flexibility:** Enables writing generic trait implementations
3. **Performance:** No runtime overhead - all resolution happens at compile time
4. **Foundation:** Prepares for full method monomorphization in Step 2

## Related Commits
- `41a1562` - Phase 6 Step 1: Implement Self type resolution in generic trait impls

## Documentation Updated
- `doc/TRAIT_SYSTEM_DESIGN.md` - Documents Self type usage
- `doc/ROADMAP.md` - Marks Phase 6 Step 1 complete
- `TODO.md` - Updated with Phase 6 progress
