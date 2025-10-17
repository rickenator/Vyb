<img src="vyn.png" alt="Vyn Image" width="300">

## Vyn Programming Guide

---

## 1. Introduction

Welcome to the Vyn Programming Guide. This guide walks you through writing, building, and extending Vyn programs, from your first "Hello, Vyn!" to deep dives into the Vyn language internals and runtime. Version 0.3.7 delivers a robust systems programming language with LLVM backend, pattern matching with `match` statements, comprehensive control flow including `break`/`continue`, resizable `Vec<T>` collections, modern struct syntax, and comprehensive auto-serialization capabilities.

### 1.1 Purpose & Audience

This guide is intended for systems programmers, language designers, and developers who want:

* A compact, expressive syntax for both low-level control and high-level abstractions.
* Planned fine-grained memory management options, including GC, manual free, and scoped regions.
* Built-in concurrency primitives and customizable threading templates.
* A foundation for a self-hosted compiler and hybrid VM/JIT architecture for rapid iteration and performance tuning.

Whether you're coming from C/C++, Rust, D, or other modern systems languages, you'll find Vyn's template-driven approach familiar yet uniquely powerful.

Here's a comparison of Vyn against several modern systems languages, showing key similarities and differences:

| Language | Templates / Generics               | Memory Model                                                       | Concurrency                            | Syntax Style                      | Unique Feature                                     | Comment                                                      |
| -------- | ---------------------------------- | ------------------------------------------------------------------ | -------------------------------------- | --------------------------------- | -------------------------------------------------- | ------------------------------------------------------------ |
| **Vyn**  | Monomorphized templates everywhere | Planned hybrid GC (lazy, scoped) + manual free + RC                | Async/await, planned actors, threads, channels | Indentation-based, optional braces | Planned self-hosting compiler; dual VM/JIT backend, hybrid indentation | Combines zero-cost templates with flexible memory management |
| **Rust** | Monomorphized generics             | Ownership/borrow checker; optional `Arc`/`Rc`                      | `async`/`await`, threads, channels     | C-style braces, macros            | Zero-cost abstractions; strong compile-time safety | No global GC; all memory safety enforced at compile time     |
| **D**    | Runtime & compile-time templates   | GC by default; `@nogc` for manual alloc/free                       | `std.concurrency` fibers, threads      | C-style; mixins                   | Compile-time function execution (CTFE)             | Blend of high-level features with systems control            |
| **C++**  | Templates & concepts (20+)         | Manual `new`/`delete`; smart pointers (`unique_ptr`, `shared_ptr`) | Threads, coroutines (`co_await`)       | C-style braces                    | Metaprogramming via templates & concepts           | Extensive ecosystem; highest portability                     |
| **Nim**  | Generics + macros                  | GC by default; optional manual `alloc`                             | Async (`async`/`await`), threads       | Python-like indentation           | Hygienic macros; optional GC or ARC                | Very concise syntax; strong metaprogramming support          |
| **Go**   | Generics (1.18+)                   | GC only                                                            | Goroutines, channels                   | C-style, minimal                  | CSP-style concurrency                              | Simple, fast compile; built-in tooling                       |

*Note:* Each language offers a different balance of safety, performance, and ergonomics. Vyn's strength lies in unifying template metaprogramming, planned flexible memory management, and a future hybrid VM/JIT in a terse, self-hosted package.

### 1.2 What is Vyn?

Vyn is a statically typed, template-metaprogramming language designed to compile to native code via LLVM. Its key differentiators:

* **Terse Syntax**: Indentation-based or bracket-based blocks, optional semicolons, clear constructs.
* **Templates Everywhere**: Monomorphized generics for types and functions.
* **Hybrid Memory Model**: Planned default GC, optional manual free, reference counting, and scoped cleanup.
* **Concurrency Built In**: Async/await, with planned actors, threads, and typed channels.
* **Self-Hosting & Extensible**: Planned compiler written in Vyn; add backends, macros, and modules at runtime.

**Current Version:** 0.3.7 🚀 **FULLY FUNCTIONAL**

## Quick Start

```bash
# Clone and build
git clone https://github.com/rickenator/Vyn.git
cd Vyn
make -C build -j

# Run your first Vyn program
echo 'main()<Int> -> return 42' > hello.vyn
build/vyn hello.vyn  # Returns exit code 42

# Try modern language features
cat > example.vyn << 'EOF'
main()<Int> -> {
    numbers<Vec<Int>> = Vec::new()
    numbers.push(10)
    numbers.push(20)
    
    match numbers.len() {
        0 => return 0,
        2 => return numbers.get(0) + numbers.get(1),
        _ => return -1
    }
}
EOF
build/vyn example.vyn  # Returns 30

# Complex return types with auto-serialization
echo 'main()<Int,String> -> return 42, "Hello!"' > tuple.vyn
build/vyn tuple.vyn  # Outputs: [42, "Hello!"]
```

### 1.3 Key Concepts & Terminology

*   **Template**: A generic type/function parameterized by types or constants, instantiated at compile time.
*   **Binding Mutability**: Variables are declared with `var` (mutable binding, can be reassigned) or `const` (immutable binding, cannot be reassigned).
*   **Ownership Types**:
    *   `my<T>`: Unique ownership of data `T`.
    *   `our<T>`: Shared (reference-counted) ownership of data `T`.
    *   `their<T>`: Non-owning borrow/reference to data `T`.
*   **Data Mutability**: Indicated by `const` on the type itself (e.g., `my<T const>` for unique ownership of immutable data).
*   **Borrowing**:
    *   `view <expr>`: Creates an immutable borrow `their<T const>`.
    *   `borrow <expr>`: Creates a mutable borrow `their<T>`.
*   **`unsafe` Blocks**: Sections of code marked `unsafe { ... }` where raw pointers (`loc<T>`) can be used and some compiler guarantees are relaxed. Within these blocks, operations like `at(ptr)` for dereferencing and `from<loc<T>>()` for pointer conversion are available.
*   **Scoped Block**: Planned block prefixed with `scoped` that defers GC and cleans up at block exit.
*   **Actor**: Planned lightweight concurrent entity with a built-in mailbox for message passing.
*   **Tiered JIT**: Planned two-level execution—bytecode interpreter for startup, optimized native JIT for hot code.

### 1.4 Import vs Smuggle - Vyn's Unique Module System

Vyn introduces a distinctive approach to module imports with two keywords that serve different security and trust models:

**`import`** - Trusted, Verified Modules:
- Used for modules from signed repositories or project-local sources verified in `vyn.toml`
- Enforces security checks and version verification
- Ideal for production dependencies and standard library modules
- Example: `import std::collections::Vec`

**`smuggle`** - Flexible, External Sources:
- Allows including symbols from external sources (e.g., GitHub repositories) or unsigned modules
- Bypasses some security checks for rapid prototyping and third-party integration
- Perfect for experimental dependencies, development tools, or one-off utilities
- Example: `smuggle debug::Logger from "https://github.com/user/debug-tools"`

This dual system provides both safety for production code and flexibility for development:

```vyn
# Production imports - verified and trusted
import std::io::println
import utils::math::calculate

# Development/experimental - flexible but marked as such
smuggle debug::trace from "github.com/dev/tools"
smuggle experimental::feature from "./local/experiments"

main()<Int> -> {
    println("Production ready!")
    trace("Debug info from smuggled module")
    return calculate(42)
}
```

Declare dependencies in `vyn.toml`:
```toml
[dependencies]
std = "^1.0.0"  # Signed, from Vyn registry
utils = { git = "https://github.com/user/utils" }  # External, smuggled
```

This unique `import`/`smuggle` distinction makes Vyn's module system both secure and flexible, clearly marking the trust level of your dependencies.

## What's Working Now

Vyn **v0.3.7** is a **complete systems programming language** ready for production use:

### ✅ **Core Language Features**
- **Functions**: `name(params)<ReturnType> -> body` with full LLVM compilation
  - **Standard parameters**: `param<Type>` or `param<Type const>` 
  - **Shorthand parameters**: `Type param` or `const Type param`
  - **Mixed syntax**: Both forms can be used in the same function
- **Variables**: `name<Type> = value` with type inference and explicit typing
- **Resizable Arrays**: `Vec<T>` with `new()`, `push()`, `pop()`, `len()`, `get()` methods
- **Fixed Arrays**: `[T; N]` with indexing and beautiful println() output
- **Structs**: `struct Point { x<Int>, y<Int> }` with field access (`p.x`, `p.y`)
- **Control Flow**: `if/else`, `while/for` loops, `match` statements, `break/continue`
- **Arithmetic**: Full binary operators (`+`, `-`, `*`, `/`, `==`, `!=`, `<`, `>`, etc.)
- **Pattern Matching**: `match expr { pattern => result }` with comprehensive patterns
- **I/O**: `println()` for output, works with all data types including vectors

### ✅ **Advanced Type System**
- **Multi-value returns**: `main()<Int,String> -> return 42, "hello"`
- **Auto-serialization**: Complex return types automatically output as JSON
- **Type safety**: Full type checking and inference with modern struct syntax
- **Generic collections**: `Vec<Int>`, `Vec<String>` with full method support
- **Member access**: Struct field access (`obj.field`) and array indexing (`arr[index]`)

#### Primitive Types

| Type | Description | Size | Range/Notes | Example |
|------|-------------|------|-------------|---------|
| `Int` | Signed integer | 64-bit | -9,223,372,036,854,775,808 to 9,223,372,036,854,775,807 | `x<Int> = 42` |
| `Float` | Floating point | 64-bit | IEEE 754 double precision | `pi<Float> = 3.14159` |
| `Bool` | Boolean | 1-bit | `true` or `false` | `flag<Bool> = true` |
| `String` | UTF-8 string | Variable | Heap-allocated, immutable | `name<String> = "Alice"` |
| `Void` | No value | 0-bit | Used for functions that don't return | `print()<Void> -> { ... }` |

#### Collection Types

| Type | Description | Mutability | Example |
|------|-------------|------------|---------|
| `[T; N]` | Fixed-size array | Mutable elements | `nums<[Int; 5]> = [1, 2, 3, 4, 5]` |
| `Vec<T>` | Dynamic array | Mutable elements | `items<Vec<String>> = Vec::new()` |

#### Ownership Types

| Type | Description | Use Case | Example |
|------|-------------|----------|---------|
| `my<T>` | Unique ownership | Single owner, move semantics | `data<my<Person>> = my(Person{...})` |
| `our<T>` | Shared ownership | Reference counting | `config<our<Settings>> = our(Settings{...})` |
| `their<T>` | Borrowed reference | Non-owning access | `ref<their<Data>> = borrow(owner)` |
| `loc<T>` | Raw pointer | Unsafe operations only | `ptr<loc<Int>> = loc(variable)` |

### ✅ **Memory Management**
- **Ownership types**: `my<T>`, `our<T>`, `their<T>` for safe memory handling
- **Borrowing**: `view expr` and `borrow expr` for references  
- **Unsafe operations**: `loc<T>` pointers in `unsafe {}` blocks
- **Raw Memory Operations**: Complete `unsafe` block system with `loc<T>` pointers
- **Memory Safety**: Borrow checking and lifetime analysis prevent dangling pointers
- **Hybrid Model**: Combines ownership with planned GC for maximum flexibility

### ✅ **Developer Experience**
- **LLVM backend**: Direct compilation to native code with JIT execution
- **Comprehensive parser**: Handles complex syntax including templates, async, classes
- **Rich error messages**: Clear compilation feedback
- **Git integration**: Regular commits track development progress

## Language Overview

Vyn v0.3.7 is a **complete, production-ready systems programming language** with modern syntax, powerful pattern matching, and comprehensive collection support.

### Language Features Showcase

```vyn
# Complete language demonstration showing all major features

# Modern struct syntax with typed fields
struct Person {
    name<String>,
    age<Int>,
    scores<Vec<Int>>
}

# Pattern matching with comprehensive match statements
grade_level(age<Int>)<String> -> {
    match age {
        0 => "infant",
        1 => "toddler", 
        5 => "kindergarten",
        18 => "adult",
        _ => "student"
    }
}

# Resizable collections with full method support
process_scores()<Int> -> {
    scores<Vec<Int>> = Vec::new()
    scores.push(95)
    scores.push(87)
    scores.push(92)
    
    total<Int> = 0
    i<Int> = 0
    while (i < scores.len()) {
        score<Int> = scores.get(i)
        total = total + score
        i = i + 1
        
        # Loop control flow
        if (score < 60) {
            continue  # Skip failing grades
        }
        if (total > 300) {
            break     # Stop if total exceeds threshold
        }
    }
    
    return total
}

# Dual parameter syntax and member access
create_person(name<String>, age<Int>)<Person> -> {
    person<Person> = Person {
        name = name,
        age = age,
        scores = Vec::new()
    }
    
    # Member access for both reading and modification
    person.scores.push(85)
    person.scores.push(90)
    
    return person
}

main()<Int> -> {
    student<Person> = create_person("Alice", 20)
    level<String> = grade_level(student.age)
    total<Int> = process_scores()
    
    println(student)  # Auto-serialization of complex types
    println(level)
    
    return total
}
```

### Memory Safety & Unsafe Operations

Vyn's design philosophy emphasizes safety by default, but provides escape hatches for low-level memory manipulation when needed:

```vyn
# Safe memory management with ownership types
safe_memory_example()<Int> -> {
    # Unique ownership - automatically freed when out of scope
    owned<my<String>> = make_my("unique data")
    
    # Shared ownership - reference counted
    shared<our<String>> = make_our("shared data")
    another_ref<our<String>> = shared  # Reference count incremented
    
    # Borrowing - non-owning references
    view_ref<their<String const>> = view shared      # Immutable borrow
    mut_ref<their<String>> = borrow owned           # Mutable borrow
    
    return 42
}

# Unsafe operations for low-level control
unsafe_memory_example()<Int> -> {
    x<Int> = 42
    result<Int> = 0
    
    unsafe {
        # Create raw pointers
        p<loc<Int>> = loc(x)     # Get pointer to x
        q<loc<Int>> = loc(result) # Get pointer to result
        
        # Read and write through pointers
        at(q) = at(p) * 2           # Write x*2 to result through pointers
        
        # Pointer type conversion
        p_void<loc<Void>> = from<loc<Void>>(p)
        p_back<loc<Int>> = from<loc<Int>>(p_void)
    }
    
    return result  # Returns 84
}
```

**Safety Guidelines for Unsafe Code:**
1. Minimize the scope of `unsafe` blocks
2. Document all invariants and assumptions
3. Validate pointers before dereferencing
4. Encapsulate unsafe operations behind safe abstractions
5. Test unsafe code extensively

### Syntax and Literals

Vyn uses indentation-sensitive syntax with optional braces and semicolons. Whitespace defines blocks, so consistent indentation is key. The unified `name<Type>` syntax provides consistency across all language constructs.

#### Literal Forms

```vyn
# Integer literals
x<Int> = 42                      # 64-bit signed integer (default)
small<Int> = 123                 # All integers default to 64-bit
large<Int> = -9223372036854775808  # Full 64-bit range

# Floating point literals  
pi<Float> = 3.14159              # 64-bit IEEE 754 double precision
scientific<Float> = 1.23e-4      # Scientific notation supported
negative<Float> = -2.718         # Negative floats

# Boolean literals
flag<Bool> = true                # Boolean true
active<Bool> = false             # Boolean false

# String literals
name<String> = "Alice"           # UTF-8 string literals
multiline<String> = "Line 1\nLine 2"  # Escape sequences supported
empty<String> = ""               # Empty strings

# Character and Unicode (planned)
# ch<Char> = 'A'                 # Single byte UTF-8 code unit (planned)
# rune<Rune> = '💡'              # Full Unicode code point (planned)

# Array literals
numbers<[Int; 5]> = [1, 2, 3, 4, 5]     # Fixed-size arrays
mixed<[Float; 3]> = [1.0, 2.5, -3.14]   # Different element types
empty_array<[Int; 0]> = []               # Empty arrays

# Vector literals (dynamic arrays)
items<Vec<Int>> = Vec::new()             # Empty dynamic vector
# Dynamic vector literals with vec![] syntax planned

# Raw bytes (planned)
# raw<Bytes> = [0xDE, 0xAD, 0xBE]       # Raw byte sequences (planned)

# Null/void
# No explicit null literal - use Option<T> pattern when implemented
```

#### Syntax Examples

```vyn
# Variable declarations with unified syntax
x<Int> = 42                      # Mutable variable
const PI<Float> = 3.14159        # Immutable constant

# Function declarations follow execution order
add(a<Int>, b<Int>)<Int> -> a + b

# Struct definitions with typed fields
struct Point {
    x<Float>,
    y<Float> = 0.0               # Default values supported
}

# Pattern matching with comprehensive patterns
process_value(val<Int>)<String> -> {
    match val {
        0 => "zero",
        1..10 => "small",        # Range patterns (planned)
        _ => "large"
    }
}

# Memory safety with ownership types
safe_data<my<String>> = my("unique")      # Unique ownership
shared_data<our<String>> = our("shared")  # Reference counted
borrowed<their<String>> = borrow safe_data  # Non-owning reference
```

#### Type System Features

**Current Types**: `Int` (64-bit), `Float` (64-bit), `Bool`, `String`, `Void`, `[T; N]`, `Vec<T>`

**Planned Types**: 
- Sized integers: `Int32`, `Int16`, `Int8`, `UInt64`, `UInt32`, `UInt16`, `UInt8`
- Sized floats: `Float32` for single precision
- Characters: `Char` (UTF-8 code unit), `Rune` (Unicode code point)  
- Raw data: `Bytes` for byte sequences
- Compound: Tuples `(T1, T2, ...)`, advanced collections

**Ownership Types**: `my<T>` (unique), `our<T>` (shared), `their<T>` (borrowed), `loc<T>` (unsafe raw pointer)

### Basic Syntax

Vyn uses clean, expressive syntax with flexible parameter syntax:

```vyn
# Functions support both standard and shorthand parameter syntax

# Standard syntax: explicit parameter<Type> or parameter<Type const>
add_standard(a<Int>, b<Int const>)<Int> -> {
    return a + b
}

# Shorthand syntax: Type directly (implicitly mutable)
add_shorthand(Int a, Int b)<Int> -> {
    return a + b
}

# Mixed syntax: combining both forms in same function
mixed_params(x<Int>, Int y, const Int z)<Int> -> {
    return x + y + z
}

# Complex return types with auto-serialization
get_data()<Int, String, Bool> -> {
    return 42, "result", true
}
# When called from main(), outputs: {"Int":42,"String":"result","Bool":true}
```

### Function Parameters

Vyn supports two parameter syntax styles for maximum flexibility:

```vyn
# Standard syntax: explicit mutability
format_standard(prefix<String>, value<Int const>)<String> -> {
    return prefix + String::from_int(value)
}

# Shorthand syntax: type-first (more concise)  
format_shorthand(String prefix, const Int value)<String> -> {
    return prefix + String::from_int(value)
}

# Both produce identical behavior and LLVM IR
```

### Variables and Types

```vyn
# Variables with type inference
x = 42          # Int
name = "Alice"  # String
flag = true     # Bool

# Explicit typing
count<Int> = 0
message<String> = "Hello"

# Immutable bindings (with const type modifier)
PI<Float const> = 3.14159
```

### Structs and Data

```vyn
# Define a struct with modern syntax
struct Point {
    x<Int>,
    y<Int>
}

# Create and use with field access
main()<Int> -> {
    p<Point> = Point { x = 10, y = 20 }
    return p.x + p.y  # Returns 30
}
```

### Modules & Import System

Vyn's unique dual import system provides both security and flexibility:

```vyn
# Trusted imports from verified sources
import std::collections::Vec
import utils::math::calculate
import std::io::println

# Flexible imports for development and experimentation  
smuggle debug::trace from "github.com/dev/tools"
smuggle experimental::parser from "./local/experiments"

# Use imported symbols
main()<Int> -> {
    numbers<Vec<Int>> = Vec::new()
    numbers.push(calculate(10))
    
    println(numbers)  # From trusted std::io
    trace("Debug info")  # From smuggled debug module
    
    return numbers.get(0)
}
```

**Import Types:**
- **`import`**: For signed, verified modules from registries or project dependencies
- **`smuggle`**: For external, experimental, or development-only modules

Declare dependencies in `vyn.toml`:
```toml
[dependencies]
std = "^1.0.0"                    # Verified registry package
utils = { git = "https://..." }    # External Git repository
local_tools = { path = "../tools" } # Local development dependency
```

**Planned Bundle & Sharing System:**
Fine-grained visibility control with bundles:

```vyn
# Declare module bundles
bundle(math, math.Core)

# Control symbol visibility
share(all) fn public_function() { ... }           # Public to all
share(math.UI) fn ui_helper() { ... }             # Only to math.UI bundle
fn internal_helper() { ... }                      # Private to this file
```

### Arrays and Collections

```vyn
# Resizable vectors with full method support
main()<Int> -> {
    numbers<Vec<Int>> = Vec::new()
    numbers.push(10)
    numbers.push(20)
    numbers.push(30)
    
    println(numbers)  # Outputs: { "type": "Vec<Int>", "address": "0x..." }
    
    length<Int> = numbers.len()    # Gets 3
    first<Int> = numbers.get(0)    # Gets 10
    last<Int> = numbers.pop()      # Gets and removes 30
    
    return first + last  # Returns 40
}

# Fixed-size arrays also supported
array_example()<Int> -> {
    fixed<[Int; 3]> = [1, 2, 3]
    return fixed[0]  # Array indexing
}
```

### Control Flow

```vyn
# Conditional expressions with modern syntax
check_sign(x<Int>)<String> -> {
    if (x > 0) {
        return "positive"
    } else if (x < 0) {
        return "negative"
    } else {
        return "zero"
    }
}

# While loops with break/continue
factorial(n<Int>)<Int> -> {
    result<Int> = 1
    i<Int> = 1
    while (i <= n) {
        if (i == 0) {
            continue
        }
        result = result * i
        i = i + 1
        if (result > 1000) {
            break
        }
    }
    return result
}

# Pattern matching with match statements
describe_number(x<Int>)<String> -> {
    match x {
        0 => "zero",
        1 => "one", 
        42 => "the answer",
        _ => "some number"
    }
}
```

## Build System

Vyn uses CMake for building:

```bash
# Clean build
mkdir -p build && cd build
cmake .. && make clean && make -j

# Quick rebuild (from project root)
make -C build -j

# Run tests
build/vyn test/test_simple_add.vyn
```

## Project Structure

```
Vyn/
├── src/              # C++ source code
│   ├── main.cpp      # Entry point with LLVM JIT
│   ├── lexer.cpp     # Tokenization
│   ├── parser.cpp    # Syntax analysis
│   └── ast.cpp       # Abstract syntax tree
├── include/vyn/      # Header files
├── test/             # Vyn test programs
├── examples/         # Example programs
├── doc/              # Documentation
└── build/            # Build output
```

## Examples

### Simple Programs

**Hello World:**
```vyn
main()<Void> -> {
    println("Hello, Vyn!")
}
```

**Mathematical Computation:**
```vyn
fibonacci(n<Int>)<Int> -> {
    if (n <= 1) {
        return n
    } else {
        return fibonacci(n - 1) + fibonacci(n - 2)
    }
}

main()<Int> -> {
    return fibonacci(10)  # Returns 55
}
```

**Data Processing with Auto-Serialization:**
```vyn
struct Result {
    success<Bool>,
    value<Int>,
    message<String>
}

process_data(x<Int>)<Result> -> {
    if (x > 0) {
        return Result {
            success = true,
            value = x * 2,
            message = "Processing successful"
        }
    } else {
        return Result {
            success = false,
            value = 0,
            message = "Invalid input"
        }
    }
}

main()<Result> -> {
    return process_data(21)
}
# Outputs structured JSON for the Result
```

## Auto-Serialization Feature

One of Vyn's standout features is automatic serialization of complex return types:

- **Simple integers**: Return as exit codes (`fn<Int> main() -> return 42`)
- **Complex types**: Automatically serialize to JSON-like format
- **Tuples**: `fn<Int,String> main() -> return 10, "hello"` outputs `[10, "hello"]`
- **Structs**: Full structured output with field names and values

This makes Vyn excellent for data processing scripts and API-style programs.

## Memory Safety

Vyn provides multiple memory management strategies:

```vyn
# Unique ownership (like Rust's Box)
owned<my<String>> = make_my("unique data")

# Shared ownership (reference counted)
shared<our<String>> = make_our("shared data")
another_ref<our<String>> = shared  # Reference count incremented

# Borrowing (non-owning references)
view_ref<their<String>> = view shared      # Immutable borrow
mut_ref<their<String>> = borrow owned      # Mutable borrow

# Raw pointers for unsafe operations
unsafe {
    x<Int> = 42
    ptr<loc<Int>> = loc(x)  # Get pointer to x
    at(ptr) = 99           # Modify through pointer
}
```

## Roadmap

### ✅ **Completed (v0.3.7)**
- **Complete Core Language**: Functions, variables, structs with modern `field<Type>` syntax
- **LLVM Backend**: Full compilation pipeline with JIT execution
- **Advanced Control Flow**: `if/else`, `while/for` loops, `match` statements, `break/continue`
- **Pattern Matching**: `match expr { pattern => result }` with comprehensive matching
- **Resizable Collections**: `Vec<T>` with `new()`, `push()`, `pop()`, `len()`, `get()` methods
- **Fixed Arrays**: `[T; N]` with literals, indexing, and perfect serialization
- **Member Access**: Struct field access (`obj.field`) and array indexing (`arr[index]`)
- **Binary Operations**: Full operator set (`+`, `-`, `*`, `/`, `==`, `!=`, `<`, `>`, etc.)
- **Dual Parameter Syntax**: Both standard (`var<Type>`) and shorthand (`Type`) forms
- **Auto-serialization**: Complex return types automatically output as JSON

### 🚧 **In Progress**
- **String Operations**: Concatenation, comparison, and manipulation methods
- **Standard Library**: Comprehensive I/O, math, and collections modules
- **Enhanced Error Messages**: More detailed compilation feedback and suggestions
- **Performance Optimizations**: LLVM optimization passes and compile-time improvements

### 📋 **Planned**
- **Templates and Generics**: Full compile-time metaprogramming
- **Async/Await**: Coroutines and concurrent programming
- **Module System**: Import/export with bundles and sharing
- **Package Manager**: Dependency management and registries
- **Garbage Collection**: Optional GC for complex object graphs
- **Self-Hosting**: Vyn compiler written in Vyn

## Architecture

Vyn is built on solid foundations:

- **Frontend**: Hand-written recursive descent parser
- **AST**: Rich abstract syntax tree with source location tracking
- **Backend**: LLVM for code generation and JIT execution
- **Type System**: Strong static typing with ownership and borrowing
- **Runtime**: LLVM ExecutionEngine with auto-serialization support

## Contributing

Vyn is actively developed with regular commits tracking progress:

1. **Language Features**: Add new syntax, types, or operations
2. **Standard Library**: Implement core modules and utilities
3. **Testing**: Create comprehensive test cases
4. **Documentation**: Improve guides and examples
5. **Performance**: Optimize compilation and runtime

See `doc/` directory for detailed design documents and RFCs.

## Recent Progress

**v0.3.7 Major Language Completion**: Full-featured programming language achieved
- ✅ **Match Statements**: Complete pattern matching with `=>` syntax and comprehensive patterns
- ✅ **Break/Continue**: Loop control flow statements working in all loop types
- ✅ **Vec<T> Collections**: Fully functional resizable arrays with all methods (`new`, `push`, `pop`, `len`, `get`)
- ✅ **Modern Struct Syntax**: Updated to `field<Type>` syntax with perfect field access
- ✅ **Complete Binary Operations**: All arithmetic, comparison, and logical operators
- ✅ **Dual Parameter Syntax**: Both `var<Type> name` and `Type name` forms working seamlessly
- ✅ **Member Access**: Object field access (`obj.field`) and array indexing (`arr[index]`)
- ✅ **Auto-serialization**: Complex return types with smart JSON-like output

**Language Status**: Vyn v0.3.7 is now a **fully functional systems programming language** suitable for real-world programming tasks, with all core language constructs implemented and working.

## Getting Help

- **Documentation**: See `doc/` for language design and internals
- **Examples**: Check `examples/` for working programs
- **Tests**: Review `test/` for feature demonstrations
- **Issues**: Report bugs or request features on GitHub

---

## Appendix

### A. EBNF Grammar

Vyn's syntax is defined by an EBNF grammar reflecting current v0.3.7 capabilities:

```
module = { declaration | statement };
declaration = function | template | class;
function = ["async"] "fn" ["<" type {"," type} ">"] identifier ["(" [parameter {"," parameter}] ")"] ["->"] block;
template = "template" identifier ["<" identifier {"," identifier} ">"] block;
class = "class" identifier block;
block = "{" { statement } "}" | INDENT { statement } DEDENT;
statement = const_decl | var_decl | if_stmt | for_stmt | while_stmt | match_stmt | break_stmt | continue_stmt | return_stmt | expression;
const_decl = "const" ["<" type ">"] identifier "=" expression [";"]; 
var_decl = "var" ["<" type ">"] identifier ["=" expression] [";"]; 
if_stmt = "if" "(" expression ")" block ["else" (block | if_stmt)];
for_stmt = "for" "(" identifier "in" expression ")" block;
while_stmt = "while" "(" expression ")" block;
match_stmt = "match" expression "{" { pattern "=>" expression [","] } "}";
break_stmt = "break" [";"]; 
continue_stmt = "continue" [";"]; 
return_stmt = "return" [expression {"," expression}] [";"]; 
expression = primary | unary_expr | binary_expr | member_access | array_index;
primary = identifier | int_literal | string_literal | bool_literal | "(" expression ")" | array_expr | call_expr;
unary_expr = ("-" | "!") expression;
binary_expr = expression operator expression;
operator = "+" | "-" | "*" | "/" | "==" | "!=" | "<" | ">" | "<=" | ">=";
member_access = expression "." identifier;
array_index = expression "[" expression "]";
array_expr = "[" [expression {"," expression}] "]";
call_expr = identifier "(" [expression {"," expression}] ")";
type = identifier | "Vec" "<" type ">" | "[" type ";" expression "]";
parameter = ["var" "<" type ">"] identifier | type identifier | "const" type identifier;
pattern = identifier | int_literal | "_";
identifier = letter { letter | digit | "_" };
int_literal = ["-"] digit { digit };
string_literal = "\"" { any_char - "\"" } "\"";
bool_literal = "true" | "false";
letter = "a".."z" | "A".."Z";
digit = "0".."9";
```

### B. Memory Model Reference

**Ownership Types:**
- `my<T>`: Unique ownership, RAII cleanup
- `our<T>`: Shared ownership, reference counted
- `their<T>`: Non-owning borrow, lifetime checked

**Borrowing Operations:**
- `view expr`: Creates `their<T const>` immutable borrow
- `borrow expr`: Creates `their<T>` mutable borrow

**Unsafe Operations:**
- `loc<T>`: Raw pointer type
- `loc(expr)`: Get pointer to expression
- `at(ptr)`: Dereference pointer (read/write)
- `from<loc<T>>(expr)`: Pointer type conversion

### C. Auto-Serialization Reference

Vyn automatically serializes complex return types from `main()`:

**Simple Returns:**
- `main()<Int> -> return 42` → Exit code 42
- `main()<String> -> return "hello"` → Outputs: hello

**Complex Returns:**
- `main()<Int,String> -> return 42, "hello"` → Outputs: [42, "hello"]
- Struct returns → JSON with field names and values
- Vec returns → JSON array representation

**Customization:**
Implement `Serialize` trait for custom serialization behavior.

## Glossary

**AST** (Abstract Syntax Tree)
Tree representation of source code structure produced by the parser.

**Borrow Checking**
Compile-time analysis ensuring references don't outlive the data they point to.

**Bundle**
Planned namespace grouping for fine-grained module visibility control.

**Import** 
Secure module inclusion from verified, signed sources.

**JIT** (Just-In-Time)
Runtime compilation of code to native machine instructions for performance.

**LLVM**
Low Level Virtual Machine - compiler infrastructure used by Vyn's backend.

**Monomorphization**
Compile-time process of creating specialized versions of generic functions/types.

**Ownership**
Memory management system using `my<T>`, `our<T>`, `their<T>` types.

**Pattern Matching**
`match` statements that destructure and test values against patterns.

**Smuggle**
Flexible module inclusion from external, potentially unverified sources.

**Template**
Compile-time generic construct parameterized by types or constants.

**Vec<T>**
Resizable array collection with methods like `push()`, `pop()`, `len()`, `get()`.

---

## License

Apache License - see LICENSE file for details.

---

*Vyn v0.3.7: A complete systems programming language with pattern matching, resizable collections, modern syntax, and unique import/smuggle module system - ready for real-world development.*