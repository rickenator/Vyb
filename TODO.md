# Vyn Language v0.3.7 Development Roadmap

## Current Implementation Status

Based on comprehensive analysis of documentation (EBNF grammar, AST specs, tests), approximately **30-40%** of the documented Vyn language is implemented. The parser layer is more complete than semantic analysis and codegen phases.

## ✅ IMPLEMENTED FEATURES (Working)

### Core Language Elements
- **Lexer/Tokenizer**: Complete with all documented token types
- **Basic function declarations**: `fn<ReturnType> name() -> { ... }`
- **Basic variable declarations**: `var<Type> name = value`
- **Struct declarations**: Both colon (`x: Int`) and angle bracket (`x<Int>`) syntax
- **Struct construction**: `StructName{field: value}`
- **Integer/Float/String literals**: Basic literal parsing
- **Return statements**: `return expression`
- **Block statements**: `{ ... }`
- **Basic expressions**: Identifiers, literals, assignments
- **LLVM Integration**: Basic codegen and JIT execution
- **Unsafe blocks**: `unsafe { ... }` (parsing and basic semantic support)

### Type System (Partial)
- **Basic type names**: `Int`, `String`, `Float`, `Bool`
- **Type parsing**: Basic TypeNode structure exists
- **Ownership wrapper tokens**: `my`, `our`, `their`, `ptr` (tokenized but not fully implemented)

## ❌ MISSING/UNIMPLEMENTED FEATURES

### 🔥 HIGH PRIORITY - Core Language Features

#### 1. **Control Flow Statements** ⚠️ CRITICAL
- [ ] `if/else` statements and expressions  
- [ ] `for` loops (`for item in iterable`)
- [ ] `while` loops
- [ ] `match` statements (pattern matching)
- [ ] `break`/`continue` statements

#### 2. **Expression Types** ⚠️ CRITICAL  
- [ ] Binary expressions (`+`, `-`, `*`, `/`, `==`, `!=`, `<`, `>`, etc.)
- [ ] Unary expressions (`!`, `-`, `+`)
- [ ] Call expressions (function calls)
- [ ] Member access (`obj.field`, `obj->field`)
- [ ] Array access (`arr[index]`)
- [ ] Conditional expressions (ternary operator)

#### 3. **Advanced Type System** ⚠️ CRITICAL
- [ ] Array types (`[Type; Size]`, `[Type]`)
- [ ] Tuple types (`(Type1, Type2)`)
- [ ] Function types (`fn<ReturnType>(ParamTypes)`)
- [ ] Optional types (`Type?`)
- [🔶] **Ownership wrappers** - PARTIALLY IMPLEMENTED
  - [x] `my<Type>`, `our<Type>`, `their<Type>` (parsing works)
  - [ ] `ptr<Type>` (missing from type parser)
  - [ ] Semantic ownership enforcement
  - [ ] LLVM codegen for ownership

#### 4. **Memory Management** ⚠️ CRITICAL  
- [🔶] **Unsafe blocks** - PARTIALLY IMPLEMENTED
  - [x] `unsafe { ... }` parsing and tokenization
  - [ ] Semantic validation (operations only allowed in unsafe)
- [ ] **Memory intrinsics** - NOT IMPLEMENTED
  - [ ] `loc<T>()` pointer creation intrinsic
  - [ ] `at()` pointer dereferencing intrinsic
  - [ ] `from<loc<T>>()` type conversion intrinsic
- [ ] **Borrowing intrinsics** - PARSING ONLY
  - [x] `borrow()`, `view()` tokenization
  - [ ] Semantic validation and type generation

#### 5. **Multi-Value Returns** ⚠️ IMPORTANT
- [ ] `fn<Type1, Type2>` syntax (parsing may exist)
- [ ] Comma-separated return values (`return a, b`)
- [ ] Multiple assignment (`var<T1> a, var<T2> b = func()`)

### 🔶 MEDIUM PRIORITY - Advanced Features

#### 6. **Object-Oriented Features**
- [ ] `class` declarations
- [ ] `impl` blocks (parsing exists, semantic missing)
- [ ] Method calls
- [ ] Inheritance (`extends`)
- [ ] Traits/interfaces (`trait`, `implements`)

#### 7. **Generic Programming**
- [ ] Generic parameters (`<T>`, `<T: Bound>`)
- [ ] Generic constraints
- [ ] Template declarations

#### 8. **Module System**
- [ ] `import` statements (parsing exists)
- [ ] `smuggle` statements
- [ ] Module resolution
- [ ] Path resolution (`module::item`)

#### 9. **Pattern Matching**
- [ ] Pattern syntax in `match` arms
- [ ] Destructuring assignments
- [ ] Enum variant patterns

### 🔷 LOWER PRIORITY - Nice-to-Have

#### 10. **Advanced Control Flow**
- [ ] `try`/`catch`/`finally` (AST exists)
- [ ] `defer` statements  
- [ ] `async`/`await`
- [ ] `throw` statements

#### 11. **Advanced Expressions**
- [ ] List comprehensions (`[expr for item in iterable]`)
- [ ] Range expressions (`a..b`, `a..=b`)
- [ ] Lambda expressions

#### 12. **Serialization System**
- [ ] Auto-serialization for `main()` returns
- [ ] `lit()`, `notype()`, `bare()`, `deserial()` intrinsics

## 🐛 KNOWN ISSUES TO FIX

### Active Issues
- [x] **LLVM Type Mismatches**: Return types show as `i32` instead of actual types *(IN PROGRESS)*
- [x] **Unsafe Keyword**: ~~Tokenized as `UNKNOWN` instead of `KEYWORD_UNSAFE`~~ *(FIXED - works correctly in function context)*
- [ ] **Semantic Analysis**: Incomplete type checking and validation
- [ ] **Error Handling**: Parser error recovery needs improvement

### LLVM Verification Errors
Current error pattern:
```
Function return type does not match operand type of return inst!
  ret i32 0
 i64Error at unknown_file:1:1: LLVM module verification failed.
```

## 📋 v0.3.7 DEVELOPMENT ROADMAP

### Phase 1: Fix Critical Issues (Week 1)
1. [x] ~~Fix unsafe keyword tokenization~~ *(COMPLETED - works in proper context)*
2. [ ] **Fix LLVM return type generation** *(IN PROGRESS)*
3. [ ] Implement basic binary/unary expressions
4. [ ] Implement basic if/else statements

### Phase 2: Core Language (Weeks 2-3)  
1. [ ] Implement for/while loops
2. [ ] Implement function calls
3. [ ] Implement member access
4. [ ] Implement array types and access

### Phase 3: Memory & Types (Week 4)
1. [ ] **Complete ownership type system**
   - [ ] Fix `ptr<Type>` parsing (missing from type parser)
   - [ ] Implement semantic ownership enforcement
   - [ ] Add LLVM codegen for ownership types
2. [ ] **Implement unsafe memory operations**
   - [ ] Complete `loc<T>()`, `at()`, `from<loc<T>>()` intrinsics
   - [ ] Semantic validation for unsafe-only operations
3. [ ] Implement basic generics

### Phase 4: Advanced Features (Weeks 5-6)
1. [ ] Multi-value returns
2. [ ] Pattern matching basics
3. [ ] Module system basics

## 🎯 IMMEDIATE NEXT STEPS

1. **Fix LLVM Type Issues** *(Current Priority)*
   - Investigate why return types default to i32
   - Fix struct return type handling
   - Ensure proper type verification

2. **Implement Binary Expressions**
   - Add parser support for `+`, `-`, `*`, `/`, `==`, `!=`, `<`, `>`, etc.
   - Add semantic analysis for binary operations
   - Add LLVM codegen for binary operations

3. **Implement Control Flow**
   - Add if/else statement parsing and codegen
   - Add basic loop constructs

## 📊 TESTING STATUS

### Working Test Files
- ✅ `simple_fn_test.vyn` - Basic function execution
- ✅ `test_struct_colon_syntax.vyn` - Struct declarations and construction
- ✅ Functions with unsafe blocks (verified working)

### Test Files Needing Updates
- Many `.vyn` files in `test/` directory use syntax variations
- Some expect features not yet implemented
- Need systematic audit and updates

## 📚 REFERENCE DOCUMENTATION

### Key Files for Implementation
- **EBNF Grammar**: `include/vyn/vyn.hpp` (lines 10-200+)
- **AST Definitions**: `include/vyn/parser/ast.hpp`
- **Documentation**: `doc/AST_*.md` files
- **Parser Implementation**: `src/parser/*.cpp`
- **Semantic Analysis**: `src/vre/semantic.cpp`
- **LLVM Codegen**: `src/vre/llvm/codegen.cpp`

### Architecture Notes
- **Parser**: Recursive descent parser with separate modules for declarations, statements, expressions, types
- **AST**: Comprehensive node hierarchy with visitor pattern
- **Semantic**: Type checking and validation layer
- **Codegen**: LLVM-based code generation with JIT execution

---

*Last Updated: October 16, 2025*
*Branch: v0.3.7*
*Status: Active Development - Focus on Core Language Completion*