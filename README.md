<img src="vyn.png" alt="Vyn Image" width="300">

# Vyn Programming Language

A modern systems programming language with zero-cost abstractions, safe memory management, and powerful metaprogramming.

**Current Version:** 0.3.7

## Quick Start

```bash
# Clone and build
git clone https://github.com/rickenator/Vyn.git
cd Vyn
make -C build -j

# Run your first Vyn program
echo 'fn<Int> main() -> return 42' > hello.vyn
build/vyn hello.vyn  # Returns exit code 42

# Try complex return types with auto-serialization
echo 'fn<Int,String> main() -> return 42, "Hello!"' > tuple.vyn
build/vyn tuple.vyn  # Outputs: [42, "Hello!"]
```

## What's Working Now

Vyn **v0.3.7** is a functional programming language with impressive capabilities:

### ✅ **Core Language Features**
- **Functions**: `fn<ReturnType> name(params) -> body` with full LLVM compilation
  - **Standard parameters**: `var<Type> name` or `const<Type> name` 
  - **Shorthand parameters**: `Type name` or `const Type name`
  - **Mixed syntax**: Both forms can be used in the same function
- **Variables**: `var x = 42` with type inference and explicit typing
- **Arrays**: `[T; N]` fixed-size arrays with indexing and beautiful println() output
- **Structs**: `struct Point { x: Int, y: Int }` with field access
- **Control Flow**: `if/else`, `while` loops, `return` statements
- **Arithmetic**: `+`, `-`, `*`, `/` with proper precedence
- **I/O**: `println()` for output, works with strings, integers, and arrays

### ✅ **Advanced Type System**
- **Multi-value returns**: `fn<Int,String> main() -> return 42, "hello"`
- **Auto-serialization**: Complex return types automatically output as JSON
- **Type safety**: Full type checking and inference
- **Structs with methods**: Object-oriented programming support

### ✅ **Memory Management**
- **Ownership types**: `my<T>`, `our<T>`, `their<T>` for safe memory handling
- **Borrowing**: `view expr` and `borrow expr` for references
- **Unsafe operations**: `loc<T>` pointers in `unsafe {}` blocks

### ✅ **Developer Experience**
- **LLVM backend**: Direct compilation to native code with JIT execution
- **Comprehensive parser**: Handles complex syntax including templates, async, classes
- **Rich error messages**: Clear compilation feedback
- **Git integration**: Regular commits track development progress

## Language Overview

### Basic Syntax

Vyn uses clean, expressive syntax with flexible parameter syntax:

```vyn
# Functions support both standard and shorthand parameter syntax

# Standard syntax: explicit var<Type> or const<Type>
fn<Int> add_standard(var<Int> a, const<Int> b) -> {
    return a + b
}

# Shorthand syntax: Type directly (implicitly mutable)
fn<Int> add_shorthand(Int a, Int b) -> {
    return a + b
}

# Mixed syntax: combining both forms in same function
fn<Int> mixed_params(var<Int> x, Int y, const<Int> z) -> {
    return x + y + z
}

# Complex return types with auto-serialization
fn<Int, String, Bool> get_data() -> {
    return 42, "result", true
}
# When called from main(), outputs: {"Int":42,"String":"result","Bool":true}
```

### Function Parameters

Vyn supports two parameter syntax styles for maximum flexibility:

```vyn
# Standard syntax: explicit mutability
fn<String> format_standard(var<String> prefix, const<Int> value) -> {
    return prefix + value.to_string()
}

# Shorthand syntax: type-first (more concise)
fn<String> format_shorthand(String prefix, const Int value) -> {
    return prefix + value.to_string()
}

# Both produce identical behavior and LLVM IR
```

### Variables and Types

```vyn
# Variables with type inference
var x = 42          # Int
var name = "Alice"  # String
var flag = true     # Bool

# Explicit typing
var<Int> count = 0
var<String> message = "Hello"

# Immutable bindings
const PI = 3.14159
```

### Structs and Data

```vyn
# Define a struct
struct Point {
    x: Int,
    y: Int
}

# Create and use
fn<Int> main() -> {
    var p = Point { x: 10, y: 20 }
    return p.x + p.y  # Returns 30
}
```

### Arrays and Collections

```vyn
# Fixed-size arrays with beautiful output
fn<Int> main() -> {
    var<[Int; 3]> numbers = [10, 20, 30]
    var<[Int; 1]> single = [42]
    
    println(numbers)  # Outputs: [10, 20, 30]
    println(single)   # Outputs: [42]
    
    # Array indexing and functions
    var first = numbers[0]  # Gets 10
    return first
}
```

### Control Flow

```vyn
# Conditional expressions
fn<String> check_sign(x: Int) -> {
    if x > 0 {
        return "positive"
    } else if x < 0 {
        return "negative"
    } else {
        return "zero"
    }
}

# While loops
fn<Int> factorial(n: Int) -> {
    var result = 1
    var i = 1
    while i <= n {
        result = result * i
        i = i + 1
    }
    return result
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
fn<Void> main() -> {
    println("Hello, Vyn!")
}
```

**Mathematical Computation:**
```vyn
fn<Int> fibonacci(n: Int) -> {
    if n <= 1 {
        return n
    } else {
        return fibonacci(n - 1) + fibonacci(n - 2)
    }
}

fn<Int> main() -> {
    return fibonacci(10)  # Returns 55
}
```

**Data Processing with Auto-Serialization:**
```vyn
struct Result {
    success: Bool,
    value: Int,
    message: String
}

fn<Result> process_data(x: Int) -> {
    if x > 0 {
        return Result {
            success: true,
            value: x * 2,
            message: "Processing successful"
        }
    } else {
        return Result {
            success: false,
            value: 0,
            message: "Invalid input"
        }
    }
}

fn<Result> main() -> {
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
var<my<String>> owned = make_my("unique data")

# Shared ownership (reference counted)
var<our<String>> shared = make_our("shared data")
var<our<String>> another_ref = shared  # Reference count incremented

# Borrowing (non-owning references)
var<their<String>> view_ref = view shared      # Immutable borrow
var<their<String>> mut_ref = borrow owned      # Mutable borrow

# Raw pointers for unsafe operations
unsafe {
    var<Int> x = 42
    var<loc<Int>> ptr = loc(x)  # Get pointer to x
    at(ptr) = 99               # Modify through pointer
}
```

## Roadmap

### ✅ **Completed (v0.3.7)**
- Core language parser and AST
- LLVM backend with JIT execution
- Basic type system and inference
- Functions, variables, structs
- **Fixed-size arrays**: `[T; N]` with literals, indexing, and perfect serialization
- Control flow (if/else, while)
- Auto-serialization for complex returns
- Memory safety with ownership types

### 🚧 **In Progress**
- **Dynamic Collections**: `Vec<T>` for resizable data management
- **For Loops**: Parser tokens exist, need runtime completion
- **String Operations**: Concatenation, comparison, manipulation
- **Standard Library**: I/O, math, collections modules

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

**v0.3.7 Breakthrough**: Auto-serialization implementation
- ✅ Fixed tuple segmentation faults completely
- ✅ Smart return type detection (simple vs complex)
- ✅ JSON-like output for complex data structures
- ✅ Preserved all existing functionality
- ✅ Foundation for full JSON serialization system

**v0.3.6**: Enhanced parser capabilities
- Advanced syntax support (async, templates, classes)
- Improved error handling and recovery
- Comprehensive test validation

## Getting Help

- **Documentation**: See `doc/` for language design and internals
- **Examples**: Check `examples/` for working programs
- **Tests**: Review `test/` for feature demonstrations
- **Issues**: Report bugs or request features on GitHub

## License

MIT License - see LICENSE file for details.

---

*Vyn: Where performance meets productivity*