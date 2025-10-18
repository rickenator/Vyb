# Reference Syntax Unification for Vyn v0.4.0

## Problem Statement

Currently, Vyn has **three different syntaxes** for ownership and borrowing operations, causing confusion and inconsistency:

### Current Inconsistent Syntaxes:

1. **Function-style constructors** (examples/main.vyn, examples/binary_tree.vyn):
   ```vyn
   my(Expression)     // Used in examples
   our(Expression)    
   their(Expression)
   ```

2. **make_* functions** (README.md, doc/RUNTIME.md):
   ```vyn
   my("data")    // Documented in README/docs
   our("data")
   view "data"  
   ```

3. **Keyword-style** (README.md examples):
   ```vyn
   view <expr>        // Creates their<T const>
   borrow <expr>      // Creates their<T>
   ```

## Unified Canonical Syntax

This document establishes the **single, canonical syntax** for all ownership and borrowing operations in Vyn v0.4.0:

### ✅ **Canonical Ownership Creation Syntax**

```vyn
// OWNERSHIP TYPES - These are type annotations
name<my<Type>>      = ...    // Unique ownership type
name<our<Type>>     = ...    // Shared ownership type  
name<their<Type>>   = ...    // Borrowed reference type

// OWNERSHIP CREATION - These are value constructors  
my(expression)              // Create unique ownership
our(expression)             // Create shared ownership (reference counted)

// BORROWING OPERATIONS - These are temporary reference operators
borrow expression           // Create mutable borrow -> their<T>
view expression             // Create immutable borrow -> their<T const>
```

> **Syntax Rationale**: The operator syntax `view expr` and `borrow expr` (without parentheses) distinguishes zero-cost borrowing operations from allocation constructors `my(expr)` and `our(expr)`. This design emphasizes the semantic difference between creating owned values (which may allocate) and creating lightweight references (which are zero-cost). The parenthesized alternatives `view(expr)` and `borrow(expr)` remain under consideration for potential adoption based on parser implementation complexity and developer ergonomics.

### ✅ **Complete Example with Canonical Syntax**

```vyn
// Type declarations use ownership types
process_data(input<my<String>>)<their<String const>> -> {
    // Create owned values with constructors
    owned_copy<my<String>> = my(input.clone());
    shared_ref<our<String>> = our(input.clone());
    
    // Create borrowed references with operators
    immutable_view<their<String const>> = view owned_copy;
    mutable_borrow<their<String>> = borrow owned_copy;
    
    return view owned_copy;  // Return immutable view
}

main()<Void> -> {
    data<my<String>> = my("Hello, World!");
    result<their<String const>> = process_data(data);
    println((view result));
}
```

## Implementation Status

### ✅ **Already Implemented:**
- `my<T>`, `our<T>`, `their<T>` ownership types in type system
- `borrow` and `view` keywords and expressions in parser
- `my()`, `our()` function-style constructors in examples

### ❌ **Legacy Syntax to Remove:**
- `make_my()`, `make_our()`, `make_their()` functions
- Inconsistent documentation examples

## Migration Guide

### **For Code:**
```vyn
// OLD (inconsistent):
data<my<String>> = my("text");      // ❌ Remove
shared<our<String>> = our("text");  // ❌ Remove

// NEW (canonical):
data<my<String>> = my("text");           // ✅ Use this
shared<our<String>> = our("text");       // ✅ Use this
```

### **For Documentation:**
All documentation must use the canonical syntax consistently.

## Parsing Rules

The parser recognizes these constructs with the following precedence:

1. **Type Context**: `my<T>`, `our<T>`, `their<T>` → Ownership type wrappers
2. **Expression Context**: 
   - `my(expr)` → Ownership constructor (function call)
   - `our(expr)` → Ownership constructor (function call)  
   - `borrow expr` → Borrowing operator (prefix unary)
   - `view expr` → Borrowing operator (prefix unary)

## Benefits of Unification

1. **Consistency**: Single syntax reduces cognitive load
2. **Clarity**: Clear distinction between types (`my<T>`) and values (`my(expr)`)
3. **Intuitive**: Mirrors the VRE implementation concepts directly
4. **Concise**: Shorter than `make_*` functions
5. **Composable**: Works naturally with complex expressions

## Action Items

1. **✅ Parser Implementation**: Already supports canonical syntax
2. **🔧 Documentation Update**: Update all docs to use canonical syntax
3. **🔧 Example Migration**: Update examples to use canonical syntax
4. **🔧 Remove Legacy**: Remove `make_*` references from documentation
5. **🔧 Test Updates**: Ensure all tests use canonical syntax

---

**This document serves as the authoritative reference for all ownership and borrowing syntax in Vyn v0.4.0.**