# Phase 6 Step 5: Remaining Work

**Status:** Core semantic analysis complete, needs monomorphization validation  
**Date:** 2025-10-19

## What's Complete ✅

1. **Parser**: Bounds syntax `<T<Display, Clone>>` works perfectly
2. **Semantic Validation**: Checks bounds are actual aspects
3. **Method Calls**: Bounded type parameters can call aspect methods
4. **Documentation**: Comprehensive docs explaining bounded vs unbounded
5. **Testing**: Test suite with valid/invalid bounds cases

## What's Needed for Full Step 5 Completion

### 1. Monomorphization Bounds Checking (HIGH PRIORITY)

When instantiating a generic with concrete types, validate bounds are satisfied.

**Example Scenario**:
```vyn
# Function with bounded type parameter
printItem<T<Display>>(item<T>)<Void> -> {
    item.show();  # ✅ Already works in semantic analysis
}

# These calls should be validated:
printItem<Point>(point);   # ✅ Should work - Point has Display
printItem<Int>(42);        # ❌ Should fail - Int doesn't have Display
```

**Where to Implement**: In `LLVMCodegen::visit(CallExpression*)` when handling generic functions

**Algorithm**:
```cpp
// When calling printItem<Point>(point)
if (callee is generic function) {
    auto genericParams = function->genericParams;
    auto typeArgs = callExpr->typeArguments;  // [Point]
    
    for (size_t i = 0; i < genericParams.size(); i++) {
        auto param = genericParams[i];  // T
        auto concreteType = typeArgs[i];  // Point
        
        for (auto& bound : param->bounds) {  // [Display]
            if (!typeHasAspect(concreteType, bound)) {
                error("Type " + concreteType + " does not satisfy bound " + bound);
            }
        }
    }
    
    // Proceed with monomorphization...
}
```

**Files to Modify**:
- `src/vre/llvm/cgen_expr.cpp` - CallExpression visitor
- Add helper: `bool LLVMCodegen::typeHasAspect(std::string type, std::string aspect)`
- Query `traitRegistry` to check if aspect is implemented for type

**Test Cases Needed**:
- `test_bounds_satisfied.vyn` - Valid instantiation
- `test_bounds_violated.vyn` - Invalid instantiation should error
- `test_multiple_bounds.vyn` - Check all bounds satisfied

**Estimated Time**: 3-4 hours

### 2. Self Type Resolution (MEDIUM PRIORITY - Not Step 5)

This is actually from earlier Phase 6 steps, but blocking our test:

**Problem**: 
```vyn
impl Display for Box<T> {
    show(self<Self>)<String> -> {
        # Self resolves to Box<T>, not Box<Point>
        return "Box";
    }
}
```

Currently:
- `Self` in parameter types → errors "Unknown struct type: Self"
- `Self` in return types → partially works

**Where to Implement**: `SemanticAnalyzer::visit(TraitImpl*)`

**Solution**:
```cpp
void SemanticAnalyzer::visit(TraitImpl* node) {
    // Already have: currentImplType = "Box<T>"
    
    // When processing method parameters:
    for (auto& param : method->parameters) {
        if (param->type->name == "Self") {
            // Replace with currentImplType
            param->resolvedType = resolveType(currentImplType);
        }
    }
}
```

**Files to Modify**:
- `src/vre/semantic.cpp` - TraitImpl visitor, parameter processing
- Enhance `substituteSelfType` to handle parameters, not just returns

**Estimated Time**: 2-3 hours

### 3. Generic Constructor Inference (LOW PRIORITY - Quality of Life)

**Problem**:
```vyn
box<Box<Point>> = Box { value = point };  # Says "got Box", expected "Box<Point>"
```

**Desired Behavior**:
```vyn
# Infer Box<Point> from:
# 1. Variable type annotation: Box<Point>
# 2. Field initializer type: point is Point
box<Box<Point>> = Box { value = point };  # Should infer Box<Point>
```

**Where to Implement**: `SemanticAnalyzer::visit(StructExpression*)`

**Solution**:
```cpp
void SemanticAnalyzer::visit(StructExpression* node) {
    if (node->structType is generic) {
        // Try to infer type arguments from:
        // 1. Expected type (from variable declaration)
        // 2. Field initializer types
        
        if (expectedType && expectedType->isGeneric()) {
            node->inferredTypeArguments = expectedType->typeArguments;
        }
    }
}
```

**Files to Modify**:
- `src/vre/semantic.cpp` - StructExpression visitor
- Add type inference from context

**Estimated Time**: 4-5 hours

## Priority Order

1. **Monomorphization bounds checking** - Core to Step 5 ⭐⭐⭐
2. **Test with full codegen** - Ensure LLVM generation works
3. **Self type resolution** - Fix older Phase 6 issue (not Step 5)
4. **Constructor inference** - Nice to have, not critical

## What This Enables

Once monomorphization checking is complete:

```vyn
# Define bounded generic function
maxValue<T<Comparable>>(a<T>, b<T>)<T> -> {
    return select(a.compare(b)) -> {
        > 0 -> { pass a },
        ?   -> { pass b }
    };
}

# ✅ Valid: Point implements Comparable
result<Point> = maxValue<Point>(p1, p2);

# ❌ Compile error: String doesn't implement Comparable
# result<String> = maxValue<String>("a", "b");  # ERROR!
```

**Impact**: Complete aspect bounds feature, enabling type-safe generic programming!

## After Step 5

Next steps in Phase 6:
- **Step 6**: Associated types in aspects
- **Step 7**: Type inference without explicit type arguments
- **Step 8**: Multiple aspect bounds precedence rules
- **Step 9**: Default aspect implementations

But first: **Complete monomorphization bounds checking!**
