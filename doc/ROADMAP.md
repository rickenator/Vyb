# Vyn Language Roadmap

**Last Updated:** October 16, 2025

This document outlines the completed features, ongoing development, and future considerations for the Vyn programming language.

## Current Status (v0.4.1)

**✅ MAJOR ACHIEVEMENTS COMPLETED:**
1.  **LLVM Backend:** Fully functional LLVM IR generation and JIT execution
2.  **Auto-Serialization:** Complex return types automatically serialize to JSON-like output
3.  **Type System:** Working functions, variables, structs, control flow
4.  **Memory Safety:** Ownership types (`my<T>`, `our<T>`, `their<T>`) with borrowing
5.  **Parser:** Comprehensive syntax support including templates, async, classes
6.  **✅ Pattern Matching (COMPLETED):** Match statements with `->` arrow syntax and `?` wildcard; no-match continues as NOP
7.  **✅ Loop Control Flow (COMPLETED):** Break and continue statements working in all loop constructs
8.  **✅ Resizable Collections (COMPLETED):** Vec<T> with full method support (new, push, pop, len, get)
9.  **✅ Vec Iteration (COMPLETED v0.4.1):** `for (item in vec)` with mandatory parentheses, full break/continue support
10. **✅ Range-Based For Loops (COMPLETED v0.4.1):** `for (i in 0..10)` inclusive ranges with optional step
11. **✅ Member Access (COMPLETED):** Object field access (obj.field) and array indexing (arr[index])
12. **✅ Binary Operations (COMPLETED):** Complete operator precedence system with all arithmetic, comparison, and logical operators
13. **✅ Variadic Tuples (COMPLETED v0.4.0):** `Tuple<T,U,V,...>` with full 1-N type parameter support and dual syntax

### Variadic Tuple System (v0.4.0)

Vyn now supports **fully variadic tuple types** with comprehensive 1-to-N type parameter support:

**Features Implemented:**
- ✅ **Dual Syntax Support:**
  - Inline syntax: `main()<Int, String, Bool>` 
  - Generic syntax: `main()<Tuple<Int, String, Bool>>`
  - Both produce identical LLVM anonymous struct types
  
- ✅ **Complete Variadic Range:**
  - Single-element tuples: `Tuple<Int>` with auto-wrapping
  - Multi-element tuples: 2-7+ elements tested and verified
  - No theoretical upper limit on element count
  
- ✅ **Type System Integration:**
  - Generic type parameter processing in `cgen_types.cpp`
  - Sequence expression handling in `cgen_expr.cpp`
  - Return statement auto-wrapping in `cgen_stmt.cpp`
  - Full type safety and compile-time checking
  
- ✅ **Complex Type Support:**
  - Primitive types: Int, Float, Bool
  - String types with proper memory management
  - Collection types: Vec<T> in tuples
  - Custom struct types (planned)
  
- ✅ **Comprehensive Testing:**
  - 9 test files covering edge cases
  - Single-element tuple edge case verified
  - Mixed type combinations tested
  - All tests pass without crashes or type errors

**LLVM Representation:**
- Tuples compile to anonymous struct types: `{ T1, T2, ..., TN }`
- Efficient memory layout with no overhead
- Compatible with C ABI for struct returns

**Current Limitations** (future enhancements):
- ⏳ Tuple element access (`.0`, `.1`, `.2` syntax)
- ⏳ Tuple variables (only return values currently)
- ⏳ Tuple destructuring (`let (a, b, c) = tuple`)
- ⏳ Tuple serialization/output (compiles but no JSON output yet)
- ⏳ Tuple pattern matching in match expressions

See `test/tuples/README.md` for detailed documentation and examples.

## Core Development Focus
The current primary focus is on:
1.  **Standard Library Expansion:** Building core modules for collections, I/O, math
2.  **✅ For Loop Implementation (COMPLETED v0.4.1):** Vec iteration and range-based for loops with mandatory parentheses
3.  **✅ Arrays and Collections (COMPLETED v0.3.7):** Fixed-size arrays `[T; N]` fully implemented with beautiful serialization
4.  **✅ Dynamic Collections (COMPLETED v0.3.7):** Vec<T> resizable data structures with full method support
5.  **✅ Vec Iteration (COMPLETED v0.4.1):** `for (item in vec)` with break/continue support
6.  **✅ String Operations (COMPLETED v0.4.0):** Complete String type with natural literal syntax and 11 methods
7.  **Enhanced Error Messages:** More detailed compilation feedback and suggestions
8.  **Tuple Element Access:** `.0`, `.1`, `.2` syntax for accessing tuple elements
9.  **⏳ Select Expressions (MID PRIORITY):** Expression-based pattern matching with `pass` keyword for explicit returns

## Project Structure and Organization

### Proposed Refactoring (Future Task)
As the Vyn project grows, a more structured directory layout will be beneficial for maintainability, readability, and scalability.

-   **Goal:** Improve overall project organization and clearly separate components.
-   **Proposed Source Structure (`src/`):**
    -   `ast/`: Implementations of AST (Abstract Syntax Tree) nodes.
    -   `parser/`: Lexer, parser logic, and related utilities.
    -   `sema/` or `analyzer/`: Semantic analysis, type checking.
    -   `codegen/` or `llvm_backend/`: LLVM IR generation and compilation logic.
    -   `vre/`: Vyn Runtime Environment components (core runtime, standard library C++ implementations).
    -   `core/` or `common/`: Common utilities, data structures, error reporting used across the compiler.
    -   `main/`: Main executable entry point, REPL implementation.
-   **Proposed Include Structure (`include/vyn/`):**
    -   `ast/`: Header files for AST node definitions.
    -   `parser/`: Header files for parser interfaces, token definitions.
    -   `sema/` or `analyzer/`: Header files for semantic analysis components.
    -   `codegen/` or `llvm_backend/`: Header files for code generation interfaces.
    -   `vre/`: Header files for VRE interfaces and public APIs.
    -   `core/` or `common/`: Header files for common utilities.
-   **Considerations:**
    -   This refactoring will require significant updates to `CMakeLists.txt` to correctly identify source files and manage include directories.
    -   All `#include` paths within the project files will need to be updated.
    -   **Timing:** This is a desirable refactoring but should likely be undertaken after initial VRE/LLVM milestones are achieved, or during a dedicated refactoring phase, to avoid disrupting the current development momentum on core features.

#### Codebase Cleanup and Organization

-   **Address Duplicate Files**: Resolve the duplication of header and source files identified in `/include/vyn/` and `/src/` directories by consolidating them into the `/parser/` subdirectories as planned.
-   **Implement Proposed Directory Structure**: Refactor the codebase to follow the proposed `src/` and `include/vyn/` directory layouts (`ast/`, `parser/`, `sema/`, `codegen/`, `vre/`, `core/`).
-   **Review for Stale/Unused Code**: Conduct a thorough review of the codebase to identify and remove any other stale or unused files and code segments.

### Build System and Includes

-   **`vyn.hpp` as a Central Include:**
    -   The file `include/vyn/vyn.hpp` was initially conceived not just to house the EBNF grammar but to serve as a primary, top-level include file for essential Vyn definitions, core types, and widely used utilities.
    -   Utilities such as `source_location.hpp` and potentially `token.hpp` should ideally be directly included by `vyn.hpp` or be part of a core module that `vyn.hpp` exposes. This approach simplifies include management for different parts of the Vyn compiler and for any external tools that might interact with Vyn's core components.
    -   The EBNF grammar, if kept in `vyn.hpp`, should be clearly demarcated (e.g., within a large comment block) if the file also serves as an active header for code. Alternatively, the EBNF could reside purely in a design document like `AST.md`.

## Future Language & System Considerations

### Type System Implementation

-   **Complete Type Library**: Implement and optimize the full type system including:
    -   **Core Primitives**: Complete implementation of `Int` variants (`Int8`, `Int16`, `Int32`), `Float` variants (`Float32`, `Float64`), `Char`, `Rune`, `Bool`, `Bytes`, and `Void`.
    -   **Compound Types**: Complete implementation of tuples `(T1, T2, ...)`, fixed-size arrays `[T; N]`, dynamic vectors `Vec<T>`.
    -   **✅ String Type (COMPLETED v0.4.0)**: Base `String` type complete with fat pointer struct `{ptr: *i8, len: i64}`, natural literal syntax, and comprehensive methods (len, substring, char_at, starts_with, ends_with, contains, to_upper, to_lower, +). Future: UTF-8 operations, `String<Char>` for raw code units, `String<Rune>` for Unicode support.
-   **Performance Optimization**: Investigate performance optimizations for primitive types, particularly in tight loops and math-intensive operations.
-   **Memory Layout**: Define and document memory layout guarantees for all types, ensuring consistent behavior across platforms.
-   **FFI Compatibility**: Ensure all primitive types have well-defined mappings to C/C++ equivalents for FFI interoperability.

### Refinement of Memory Model and Unsafe Operations

-   **Complete LLVM Codegen**: Finish the implementation and refinement of LLVM code generation for `LocationExpression`, `PointerDerefExpression`, `AddrOfExpression`, and `FromIntToLocExpression` in `src/vre/llvm/cgen_expr.cpp`.
-   **Enhance Semantic Analysis**: Strengthen semantic checks related to memory operations, including more robust type validation and analysis within `unsafe` blocks.
-   **Runtime Checks (Optional)**: Investigate adding optional runtime checks for null pointer dereferences and out-of-bounds access, potentially controlled by build flags.

### Formalization of Compiler Intrinsics

-   **Document Intrinsics Mechanism**: Clearly document the process by which compiler intrinsics (like memory operations) are recognized and handled throughout the compiler pipeline (parsing, semantic analysis, code generation).
-   **Define Interface for New Intrinsics**: Establish a clear interface or pattern for adding new compiler intrinsics in the future.

### Standard Library Development

-   **Core Modules**: Begin implementing core standard library modules for common data structures (e.g., lists, maps), I/O operations, threading primitives, and utility functions.
-   **Integrate with Compiler**: Ensure seamless integration between standard library components and the compiler, especially concerning ownership, borrowing, and generics.

### Improved Error Handling and Reporting

-   **Develop Error Framework**: Implement a more structured error handling and reporting framework beyond simple console logging.
-   **Contextual Error Messages**: Provide more detailed and contextual error messages, including source locations, to aid developers in debugging.

### Testing Infrastructure

-   **Expand Test Coverage**: Write comprehensive unit and integration tests for all language features, with a focus on complex areas like memory management, ownership, concurrency, and generics.
-   **Automated Testing**: Set up automated testing in the build pipeline.

### Documentation Improvements

-   **User Guides and Tutorials**: Create user-friendly guides and tutorials covering language features, build process, and using the standard library.
-   **API Reference**: Generate or manually create a reference documentation for the standard library and core compiler interfaces.
-   **Compiler Internals Documentation**: Add more detailed documentation on compiler internals, including the AST, semantic analysis, and code generation phases.

### Select Expressions

A powerful expression-based pattern matching construct that complements the existing `match` statement:

- **Core Concept**: `select` is an **expression** (not a statement) that pattern-matches on a value and returns a result
  - Auto-infers return type from context or defaults to `Void`
  - Uses `pass` keyword to explicitly return values from match arms
  - Syntactic sugar for complex if-then-else chains

- **Syntax**:
  ```vyn
  result<String> = select(value) -> {
      pattern1 -> { pass expression1 }
      pattern2 -> { pass expression2 }
      ?        -> { pass default_expression }
  }
  ```

- **Key Features**:
  - **Type Inference**: Return type inferred from LHS variable declaration
  - **Expression-Based**: Can be used anywhere an expression is valid
  - **Explicit Returns**: `pass` keyword makes value flow explicit
  - **Value Equality**: Pattern matching uses value equality (requires careful Struct comparison semantics)
  - **String Concatenation**: Supports `+` operator for mixed-type concatenation (likely using JSON representation)

- **Example Use Cases**:
  ```vyn
  // Auto-typed from LHS
  message<String> = select(frequency) -> {
      0.0 -> { pass "This " + frequency }
      1.0 -> { pass "That " + frequency }
      ?   -> { increment_other(); pass "The other" }
  }
  
  // Void return (side effects only)
  select(status_code) -> {
      200 -> { log("Success") }
      404 -> { log("Not Found") }
      ?   -> { log("Unknown: " + status_code) }
  }
  
  // Inline simple cases
  value<Int> = select(x) -> { 0 -> { pass 10 }, 1 -> { pass 20 }, ? -> { pass 0 } }
  ```

- **Implementation Requirements**:
  1. **Type System**: Implement value-based equality for all types including Structs
  2. **String Concatenation**: Auto-conversion of non-String types (probably via JSON serialization)
  3. **`pass` Keyword**: New keyword for explicit value returns in select arms
  4. **Type Inference**: Analyze LHS to determine select expression's return type
  5. **Codegen**: Generate optimal LLVM IR (similar to match but with return values)

- **Distinctions from Match**:
  - `match`: Statement-based, used for control flow, no return value
  - `select`: Expression-based, returns a value, can be assigned
  - `match` arms use `->` for direct execution
  - `select` arms use `-> { pass value }` for explicit returns

- **Benefits**:
  - More expressive than nested ternary operators
  - Cleaner than long if-else-if chains
  - Type-safe pattern matching with value returns
  - Natural fit for functional-style programming

**Status**: Proposed for mid-priority implementation after current core features stabilize.

#### Range/Comparison Patterns

An enhancement to both `match` and `select` to support comparison-based patterns:

- **Motivation**: Enable elegant handling of numeric ranges and ordered comparisons without full class/interface system
  - Works with any comparable type (Int, Float, String with lexical ordering)
  - Reduces verbose if-else chains for range-based logic
  - Complements exact-match patterns with relational operators

- **Syntax** (using `select` as example):
  ```vyn
  label<String> = select(age) -> {
      == 0  -> { pass "infant" }
      >= 1  -> { pass "toddler" }
      >= 5  -> { pass "kindergarten" }
      >= 18 -> { pass "adult" }
      ?     -> { pass "student" }
  }
  
  grade<String> = select(score) -> {
      >= 90 -> { pass "A" }
      >= 80 -> { pass "B" }
      >= 70 -> { pass "C" }
      >= 60 -> { pass "D" }
      ?     -> { pass "F" }
  }
  
  // String lexical comparison (requires String comparison operators)
  category<String> = select(name) -> {
      < "M"  -> { pass "First Half" }
      >= "M" -> { pass "Second Half" }
  }
  ```

- **Supported Operators**:
  - `==` - Exact equality (same as current behavior)
  - `!=` - Not equal
  - `<`  - Less than
  - `<=` - Less than or equal
  - `>`  - Greater than
  - `>=` - Greater than or equal

- **Evaluation Semantics**:
  - Patterns evaluated **top-to-bottom** (order matters!)
  - First matching pattern wins (early exit)
  - Wildcard `?` acts as catch-all (should be last)
  - All comparison operators use the matched value as right operand: `pattern_op value`
  - Example: `>= 18` means "is the matched value >= 18?"

- **Type Requirements**:
  - Type must support the comparison operator used
  - **Current Support**: Int, Float (have built-in comparison)
  - **Future Support**: String (requires lexical comparison implementation)
  - **Struct Support**: Requires explicit comparison operator overloading (future polymorphism feature)

- **Implementation Prerequisites**:
  1. Extend pattern AST to include comparison operators
  2. Implement comparison codegen for all supported types
  3. Add String comparison operators (`<`, `<=`, `>`, `>=`) for lexical ordering
  4. Ensure top-to-bottom evaluation order in codegen
  5. Type checking to verify comparison operator compatibility

- **Same Syntax for Match**:
  ```vyn
  match (temperature) {
      < 0   -> println("Freezing")
      < 20  -> println("Cold")
      < 30  -> println("Comfortable")
      >= 30 -> println("Hot")
  }
  ```

- **Benefits**:
  - Natural expression of range-based logic
  - Avoids verbose chained comparisons
  - Works without complex type system (interfaces/traits)
  - Clear, readable code for categorization problems

- **Edge Cases**:
  - Overlapping patterns: first match wins (explicit ordering requirement)
  - Missing wildcard: if no pattern matches, `select` behavior TBD (error? default value? Void?)
  - Non-comparable types: compile-time error

**Status**: Future extension to match/select, depends on String comparison operators.

### Bundles & Sharing System

A planned feature to introduce fine-grained control over module visibility using bundles and sharing:

- **Core Concepts**:
  - **`bundle(...)`**: Declares which bundles (packages/subsystems) a module belongs to
  - **`share(...)`**: Controls which bundles can see each declaration
  - **Import validation**: Ensures modules import only symbols shared with their bundles
  
- **Visibility Control**:
  - `bundle(sort, sort.Core)` at file level declares package membership
  - `share(all)` exports a symbol to all modules
  - `share(sort.UI, sort.Database)` exports only to specific bundles
  - No `share` prefix keeps symbols private to the file

- **Import Rules**:
  - `import` succeeds only if your bundle list overlaps the target's share list
  - `smuggle` bypasses visibility checks for special cases

- **Benefits**:
  - Fine-grained modular packaging without external configuration
  - Self-documenting module boundaries
  - Compile-time validation of dependencies
  - Flexible opt-out via `smuggle`

See `doc/bundles_and_sharing.md` for detailed documentation.

### Auto-Serialization & Runner Behavior

✅ **IMPLEMENTED in v0.3.7** - Zero-boilerplate JSON serialization for data structures returned from `main()`:

- **Core Functionality**: The Vyn compiler/runtime automatically:
  - Calls `main()` and inspects the return type `T`
  - If `T` is a simple integer, uses it as the process exit code
  - If `T` is complex (tuples, structs), outputs structured JSON-like data
  - Prevents segmentation faults through smart type detection
  - Handles multi-value returns like `main()<Int,String>` perfectly

- **Built-in Auto-Derive**:
  - Primitives (`Int`, `Float`, `Bool`, `String`) will have built-in `to_json`
  - Collections (`[T]`, `Map<K,V>`, `Maybe<T>`) will auto-serialize if their elements do
  - Structs and classes will have auto-derived `ToJson` implementations

- **Customization**:
  - `#[jsonIgnore]` attribute to skip specific fields
  - `#[jsonName="..."]` to specify custom key names
  - Manual implementation of `ToJson` to override auto-derivation

- **Type Safety**:
  - Non-serializable types (functions, raw pointers) will trigger compile-time errors
  - Clear error messages guiding developers to convert or extract serializable data

- **Runner Behavior**:
  - Standard mode: `vyn run program.vyn` will auto-print JSON or exit code
  - Explicit JSON mode: `vyn run --json program.vyn` (future feature)
  - Proper error handling for exceptions and serialization failures

This feature will enable scripts and API-style binaries to return structured data without manual printing logic, enhancing Vyn's utility for data processing and service development.

### Function Syntax Investigation

✅ **IMPLEMENTED in v0.4.0** - **Unified Syntax Revolution**: Successfully replaced keyword-heavy syntax with clean `name<Type>` patterns:
    ```vyn
    example()<Int> -> { ... }  // Modern unified syntax (v0.4.0+)
    fn<Int> example() -> { ... }  // Legacy syntax (still supported)
    ```
-   **Casting Operations**: Formalize and implement casting operations, including:
    -   `cast<T>(expr)`: Explicit casting of expression to type T
    -   Safe versus unsafe casts, with appropriate compiler warnings or errors
    -   Rules for implicit conversions between compatible types

### Polymorphism and Type System

-   **Advanced Polymorphism**: Define and implement advanced polymorphic features including:
    -   Dynamic dispatch mechanisms for trait objects
    -   Performance considerations for virtual method tables versus monomorphization
    -   Trait object safety rules

### Other Language Considerations

The following points were previously noted in `ROADMAP.txt` and are retained here for future planning:

-   **Template Placement**: Explore allowing template/generic declarations in more contexts beyond just module-level (e.g., within classes, functions) if deemed beneficial for advanced metaprogramming scenarios. Currently, templates are primarily module-level items.
-   **Dedicated Header Files (`.vyh` or similar)**: Investigate the potential need for dedicated header files (e.g., `.vyh`) for separating public interfaces, public template/generic definitions, and type declarations from implementation files (`.vyn`). This could improve organization, reduce compilation dependencies, and potentially speed up compile times for larger Vyn projects by allowing for more explicit module boundaries.

## Documentation
Key design and planning documents for Vyn:

-   `doc/AST.md`: Detailed description of the Vyn Abstract Syntax Tree nodes and structure.
-   `doc/VRE.md`: Preliminary design for the Vyn Runtime Environment.
-   `doc/ROADMAP.md`: This document, outlining project direction and future plans.

---
*This roadmap is a living document and is subject to change as development progresses.*
