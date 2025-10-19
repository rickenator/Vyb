# Phase 6: Generic Trait Methods - Roadmap

## Overview
Phase 6 extends the trait system to support generic method implementations. This builds on Phase 5's monomorphization infrastructure to enable traits like `From<T>` and implementations like `impl<T> From<T> for Box<T>`.

## Current Status (After Phase 5)
✅ **Trait System**:
- Trait declarations with method signatures
- Trait implementations (non-generic)
- Method calls on implementing types
- Generic trait impls parse: `impl<T> Display for Box<T>`

✅ **Monomorphization**:
- Generic struct storage and instantiation
- Type parameter substitution
- LLVM type generation for Box<Int>, Pair<Int, String>, etc.
- Caching to avoid duplicates

❌ **Not Yet Implemented**:
- `Self` type resolution in generic trait impls
- Method body monomorphization
- Generic trait method calls
- Type inference for method calls

## Phase 6 Goals

### 1. Self Type Resolution
**Problem**: In `impl<T> Display for Box<T>`, what is `Self`?

**Solution**:
```vyn
impl<T> Display for Box<T> {
    show(self<Self>)<String> -> {  # Self = Box<T>
        return "Box"
    }
}
```

**Implementation**:
- Modify semantic analyzer's `visit(TraitImpl)`
- When processing generic impl, store mapping: `Self -> Box<T>`
- Resolve `Self` in method parameter types and return types
- Handle `Box<T>` as an unmonomorphized type template

### 2. Method Template Storage
Similar to struct templates in Phase 5:

```cpp
// In codegen.hpp
std::map<std::string, FunctionDeclaration*> genericMethodTemplates;
// Key: "Display::show" or "From<T>::from"
// Value: AST node for method body

// Store during visit(TraitImpl) if impl has generic params
if (!implNode->genericParams.empty()) {
    for (auto& method : implNode->methods) {
        std::string key = traitName + "::" + methodName;
        genericMethodTemplates[key] = method;
    }
}
```

### 3. Method Call Detection & Monomorphization
When encountering a trait method call like `box_int.show()`:

```cpp
// In method call codegen
auto memberType = inferType(box_int);  // Returns: Box_Int
auto originalType = memberType.genericSource;  // Returns: Box<T>
auto typeArgs = memberType.typeArguments;  // Returns: [Int]

// Find method template
std::string methodKey = "Display::show";
if (genericMethodTemplates.find(methodKey) != end) {
    llvm::Function* specialized = monomorphizeMethod(
        methodKey, 
        typeArgs,  // [Int]
        originalType  // Box<T>
    );
    // Call specialized function
}
```

### 4. Method Monomorphization Algorithm
```cpp
llvm::Function* monomorphizeMethod(
    const std::string& methodKey,
    const std::vector<TypeNode*>& typeArgs,
    TypeNode* implType
) {
    // 1. Generate mangled name
    std::string mangledName = mangleMethodName(methodKey, typeArgs);
    // "Display_show_Box_Int"
    
    // 2. Check cache
    if (monomorphizedMethods.find(mangledName) != end) {
        return cached;
    }
    
    // 3. Get template
    FunctionDeclaration* template = genericMethodTemplates[methodKey];
    
    // 4. Build substitution map
    // T -> Int
    // Self -> Box<Int>
    std::map<std::string, TypeNode*> typeMap;
    typeMap["T"] = typeArgs[0];
    typeMap["Self"] = createMonomorphizedType(implType, typeArgs);
    
    // 5. Clone and substitute method body
    FunctionDeclaration* specialized = cloneAndSubstitute(template, typeMap);
    
    // 6. Generate LLVM function
    llvm::Function* func = codegenFunction(specialized);
    
    // 7. Cache
    monomorphizedMethods[mangledName] = func;
    
    return func;
}
```

## Example Test Case

```vyn
# Generic trait with type parameter
trait From<T> {
    from(value<T>)<Self> -> { }
}

# Generic struct
struct Box<T> {
    value<T>
}

# Generic implementation: any Box<U> can be created from a U
impl<U> From<U> for Box<U> {
    from(value<U>)<Self> -> {
        return Box<U> { value = value }
    }
}

main()<Int> -> {
    # Call generic method - triggers monomorphization
    box<Box<Int>> = Box<Int>.from(42)
    
    # Monomorphization creates:
    # fn Display_from_Box_Int(value<Int>)<Box_Int> -> {
    #     return Box_Int { value = value }
    # }
    
    return 0
}
```

**Expected Behavior**:
1. Parse `trait From<T>` and `impl<U> From<U> for Box<U>`
2. Store method template for `from`
3. When `Box<Int>.from(42)` is called:
   - Detect call to generic method
   - Infer type args: `U = Int`
   - Monomorphize method body
   - Generate `Display_from_Box_Int` LLVM function
4. Cache and reuse for subsequent calls

## Implementation Steps

### Step 1: Self Resolution (2-3 hours)
- [ ] Modify `visit(TraitImpl)` in semantic analyzer
- [ ] Create Self -> ImplementingType mapping
- [ ] Update symbol table to resolve Self
- [ ] Test with simple non-generic case first

### Step 2: Method Template Storage (1-2 hours)
- [ ] Add `genericMethodTemplates` map to codegen.hpp
- [ ] Modify `visit(TraitImpl)` in codegen to store templates
- [ ] Test that generic methods don't generate LLVM yet

### Step 3: Call Detection (2-3 hours)
- [ ] Modify method call codegen
- [ ] Detect when receiver type is monomorphized
- [ ] Extract original generic type and type arguments
- [ ] Test with debug output

### Step 4: Monomorphization (4-5 hours)
- [ ] Implement `monomorphizeMethod()`
- [ ] Type substitution for method bodies
- [ ] LLVM function generation
- [ ] Caching logic
- [ ] Test end-to-end

### Step 5: Testing & Documentation (2 hours)
- [ ] Create comprehensive test suite
- [ ] Test multiple instantiations
- [ ] Test cache behavior
- [ ] Document in PHASE_6_SUMMARY.md

**Total Estimate**: 11-15 hours

## Design Considerations

### Challenge 1: Self Type Complexity
`Self` in `impl<T> Display for Box<T>` is `Box<T>`, not `Box<Int>`.
- Need to distinguish between template types and instantiated types
- Self substitution happens in two phases:
  1. Semantic analysis: `Self -> Box<T>`
  2. Monomorphization: `T -> Int`, so `Box<T> -> Box<Int>`

### Challenge 2: Type Inference
For `box.show()`, need to:
1. Infer `box` has type `Box_Int`
2. Trace back to original: `Box<Int>`
3. Extract type args: `[Int]`
4. Find trait impl: `impl<T> Display for Box<T>` matches
5. Substitute: `T = Int`

**Solution**: Store metadata on monomorphized types:
```cpp
struct MonomorphizedTypeInfo {
    std::string originalTemplate;  // "Box"
    std::vector<TypeNode*> typeArguments;  // [Int]
    llvm::StructType* llvmType;  // Box_Int
};
```

### Challenge 3: Multiple Trait Impls
What if we have both:
```vyn
impl<T> Display for Box<T> { ... }
impl<T> From<T> for Box<T> { ... }
```

**Solution**: Method template key must include trait name:
- `"Display::show"` vs `"From::from"`
- Prevents collisions

## Future Enhancements (Post-Phase 6)

### Associated Types
```vyn
trait Iterator {
    type Item
    fn next(self<Self>)<Option<Self.Item>>
}
```

### Type Constraints
```vyn
impl<T: Display> Show for Box<T> {
    # Only implement if T implements Display
}
```

### Generic Functions
```vyn
fn identity<T>(x<T>)<T> -> {
    return x
}
```

### Higher-Kinded Types
```vyn
trait Functor<F<_>> {
    fn map<A, B>(fa<F<A>>, f<A -> B>)<F<B>>
}
```

## Success Criteria

Phase 6 is complete when:
1. ✅ `Self` resolves correctly in generic impls
2. ✅ Generic method templates stored
3. ✅ Method calls trigger monomorphization
4. ✅ Specialized LLVM functions generated
5. ✅ Cache prevents duplicates
6. ✅ Test suite validates all cases
7. ✅ `trait From<T>` example works end-to-end

## Next Phase Preview

**Phase 7: Type Constraints**
- Add `T: Trait` bounds syntax
- Enforce constraints during instantiation
- Enable conditional impl selection

**Phase 8: Advanced Features**
- Associated types
- Default type parameters
- Generic functions
- Const generics (e.g., `Array<T, N>`)
