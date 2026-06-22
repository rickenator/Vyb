# VyB Intrinsics & Core Syntax

This document covers VyB’s built-in intrinsics, variable declaration syntax (including type inference), and function declaration syntax, all aligned with the `<T>`‑first style.

---

## 1. Variable & Constant Declarations

VyB uses two primary declaration forms:

```ebnf
Declaration ::= "var" "<" Type ">" Identifier [ "=" Expression ]
              | "var" "auto" Identifier "=" Expression
              | "const" "<" Type ">" Identifier [ "=" Expression ]
```

- **`var<T> name [= expr]`**
  Mutable binding of type `T`.
- **`var auto name = expr`**
  Mutable binding with type `T` inferred from `expr`.
- **`const<T> name [= expr]`**
  Immutable binding of type `T`.

> **Note:** `const auto` is not supported; use explicit `const<T>` for immutable bindings.

### Examples

```vyb
var<Int> x             // mutable Int, uninitialized
var<Int> y = 42        // mutable Int, initialized

var auto tree = BTree<Int, String, 3>::new()
// `tree` inferred as BTree<Int, String, 3>

const<String> s = "hello"  // immutable String
```

Ownership-aware declarations:

```vyb
var<my<Task>> task = my<Task>(Task { id: 1, payload: "foo" })
var<our<Config>> cfg  = our<Config>(Config { debug: true })
var<their<Foo>> b     = their<Foo>(owner)       // mutable borrow
var<their<Foo const>> v = their<Foo const>(owner)  // immutable borrow
```

Pointer declarations (inside `freedom`):

```vyb
freedom {
  var<loc<Int>> p = loc(x)
  at(p) = 99
}
```

---

## 2. Function Declaration Syntax

Functions follow a `<ReturnType>`‑first style, with a mandatory `->` separator. Braces are optional for single-expression bodies.

```ebnf
FunctionDecl ::= "fn" "<" Type ">" Identifier "(" ParamList ")" "->" Body

ParamList    ::= [ Param { "," Param } ]
Param        ::= ("var" | "const") "<" Type ">" Identifier

Body         ::= Block
               | Expression

Block        ::= "{" Statement* [ Expression ] "}"
Expression   ::= <any single VyB expression>
```

- **Return type**: declared in `<Type>` after `fn`.
- **Parameters**: `var<T>` or `const<T>` before each name.
- **`->`**: mandatory separator between signature and body.
- **Braces** `{}` optional only for single-expression bodies.

### Examples

```vyb
class Node {
  var<Bool> is_leaf

  // Constructor with block body
  fn<Node> new(const<Bool> is_leaf_param) -> {
    Node { is_leaf: is_leaf_param }
  }

  // Concise single-expression function
  fn<Int> double(var<Int> x) -> x * 2
}
```

---

## 3. Intrinsics Overview

Intrinsics are compiler-handled operations, split into **stable** (Sections 4–8) and **proposed/experimental** (Section 9).

### 3.1 Reserved Keywords

These names are reserved and cannot be used as identifiers:

- **Declarations**: `var`, `auto`, `const`
- **Ownership & borrowing**: `my`, `our`, `their`, `borrow`, `view`
- **Pointer & address**: `loc`, `at`, `addr`, `from`
- **Type metadata**: `sizeof`, `alignof`, `offsetof`
- **Visibility/macros**: `import`, `smuggle`, `share`

---

## 4. Ownership Intrinsics

### 4.1 Core Wrappers

```vyb
fn my<T>(value: T) -> my<T>
fn our<T>(value: T) -> our<T>
fn their<T>(owner: my<T> | our<T>) -> their<T>
fn their<T const>(owner: my<T> | our<T>) -> their<T const>
```

- **`my<T>(value)`**: wrap `value` in a unique-owned `my<T>`.
- **`our<T>(value)`**: wrap `value` in a shared-owned `our<T>`.
- **`their<T>(owner)`**: create a mutable borrow `their<T>` of `owner`.
- **`their<T const>(owner)`**: create an immutable borrow `their<T const>` of `owner`.

### 4.2 Shorthand Borrowing

For convenience, the compiler provides **inferred** shorthand intrinsics:

```vyb
fn borrow(owner) -> their<T>
fn view(owner)   -> their<T const>
```

- **`borrow(owner)`** infers `T` from `owner` and returns `their<T>`.
- **`view(owner)`** infers `T` from `owner` and returns `their<T const>`.

---

## 5. Memory Intrinsics (`freedom` required)

```vyb
freedom fn loc<T>(expr: T) -> loc<T>
freedom fn at<T>(pointer: loc<T>) -> T
freedom fn addr<T>(pointer: loc<T>) -> Int64
freedom fn from<P>(addr: Int64) -> P
```

- **`loc<T>(expr)`**: address‑of a value → `loc<T>`.
- **`at<T>(pointer)`**: dereference pointer → `T` (l-value/r-value).
- **`addr<T>(pointer)`**: pointer → raw `Int64`.
- **`from<P>(addr)`**: raw `Int64` → pointer `P`.

---

## 6. Type Metadata Intrinsics (safe)

```vyb
fn sizeof<T>() -> UInt
fn alignof<T>() -> UInt
fn offsetof<T>(field: identifier) -> UInt
```

- **`sizeof<T>()`**: size of `T` in bytes.
- **`alignof<T>()`**: alignment of `T`.
- **`offsetof<T>(field)`**: byte offset of `field` in `T`.

---

## 7. Print & String Conversion Intrinsics (stable)

### 7.1 Generic Print Intrinsics

`println` and `print` accept **any type** and convert it to a string automatically:

```vyb
println(value)    // print with newline (auto-stringifies any type)
print(value)      // print without newline (auto-stringifies any type)
```

- Works with `Int`, `Float`, `Bool`, `String`, arrays, Vec, and structs.
- No type-specific variants like `println_int` or `println_bool` are needed.
- String concatenation with `+` auto-coerces non-string operands when either side is a `String`.
- Example: `println("Hello" + 1000)` prints `Hello1000`.

#### Examples

```vyb
i<Int> = 42
println(i)         // 42

f<Float> = 3.14
println(f)         // 3.14

b<Bool> = true
println(b)         // true

s<String> = "hello"
println(s)         // hello
```

### 7.2 String Concatenation with `+`

The `+` operator supports **automatic string coercion**: when at least one operand is a `String`, non-string values are automatically converted via `to_string()`. Both explicit and implicit forms work:

```vyb
i<Int> = 42
// Explicit: call to_string() yourself
println("Value: " + i.to_string())   // Value: 42

// Implicit: println handles conversion automatically
println("Value: " + i)               // Value: 42 (i auto-converted)

f<Float> = 3.14
println("Pi ≈ " + f.to_string())     // Pi ≈ 3.14
```

### 7.3 `to_string()` Method

Every primitive type has a `to_string()` method returning a `String`:

```vyb
fn<String> Int.to_string()    -> String
fn<String> Float.to_string()  -> String
fn<String> Bool.to_string()   -> String
fn<String> String.to_string() -> String
```

#### Examples

```vyb
x<Int> = 99
s<String> = x.to_string()     // "99"

pi<Float> = 3.14
ps<String> = pi.to_string()   // "3.14"

flag<Bool> = false
fs<String> = flag.to_string() // "false"
```

---

## 8. Auto-Serialization Intrinsics (stable)

VyB provides built-in serialization support for automatic JSON generation of data structures, particularly for values returned from `main()`. These intrinsics are stable and ready for production use.

### 7.1 Serialization Mode Intrinsics

```vyb
fn lit(value: T) -> T
fn notype(value: T) -> T
fn bare(value: T) -> T
fn deserial(value: T) -> T
```

- **`lit(value)`**: Emits raw JSON literals without type wrapping. Converts strings to raw JSON values, numbers to unquoted numbers, and booleans to literal true/false. Restricted to primitive values (Int, Float, String, Bool).

- **`notype(value)`**: Removes `<Type>` suffixes from field names in struct serialization. For structs, this produces cleaner JSON field names without type annotations. Only valid for structs.

- **`bare(value)`**: Emits only raw field values as JSON array, removing all type and field metadata. For structs, outputs values in field declaration order as a JSON array. Only valid for structs.

- **`deserial(json_string)`**: Deserializes JSON string back to typed VyB values. Used for converting JSON input back to VyB data structures.

#### Examples

**lit() Intrinsic:**
```vyb
fn<String> main() -> {
    return lit("42");     // Output: 42 (number, not string)
}

fn<String> main() -> {
    return lit("true");   // Output: true (boolean, not string)
}

fn<String> main() -> {
    return lit("hello");  // Output: "hello" (quoted string)
}
```

**notype() Intrinsic:**
```vyb
struct Person {
    Int id,
    String name
}

fn<Person> main() -> {
    var<Person> p = Person(id=123, name="Alice");
    return notype(p);
    // Output: {"id":123,"name":"Alice"}
    // instead of {"id<Int>":123,"name<String>":"Alice"}
}
```

**bare() Intrinsic:**
```vyb
struct Point {
    Float x,
    Float y
}

fn<Point> main() -> {
    var<Point> point = Point(x=3.5, y=4.2);
    return bare(point);
    // Output: [3.5, 4.2]
    // instead of {"x<Float>":3.5,"y<Float>":4.2}
}
```

**Multi-Value with Mixed Intrinsics:**
```vyb
struct Config {
    String name,
    Int version
}

fn<Config, Int> main() -> {
    var<Config> cfg = Config(name="app", version=1);
    return notype(cfg), lit("42");
    // Output: [{"name":"app","version":1}, 42]
}
```

### 7.2 JSON Serialization Intrinsics

The following intrinsics are provided for manual JSON construction and are used internally by the auto-serialization system:

```vyb
fn __vyb_serialize_to_json(value: any) -> String
fn __vyb_serialize_struct_with_names(value: any) -> String
fn __vyb_json_array_start() -> String
fn __vyb_json_array_append(current: String, item: String) -> String
fn __vyb_json_array_end(current: String) -> String
fn __vyb_json_object_start() -> String
fn __vyb_json_object_append_field(current: String, name: String, value: String) -> String
fn __vyb_json_object_end(current: String) -> String
```

- **`__vyb_serialize_to_json(value)`**: serialize any value to JSON string.
- **`__vyb_serialize_struct_with_names(value)`**: serialize struct with field names included.
- **JSON Array functions**: manually construct JSON arrays with proper formatting.
- **JSON Object functions**: manually construct JSON objects with field names and values.

### 7.3 Auto-Serialization Usage

The auto-serialization system automatically activates when `main()` returns a structured value:

```vyb
// Simple value return - auto-serialized to JSON
fn<Int> main() -> {
    return 42  // Output: 42
}

// Struct return - auto-serialized with field names
struct Point {
    var<Float> x
    var<Float> y
}

fn<Point> main() -> {
    return Point { x: 3.5, y: 4.2 }
    // Output: {"x": 3.5, "y": 4.2}
}

// Custom serialization with mode intrinsics
fn<String> main() -> {
    var<String> name = "example"
    return lit(name)  // Output: example (without quotes)
}
```

### 7.4 Serialization Guidelines

1. **Return Types**: `main()` can return any serializable type; JSON output is automatic.
2. **Mode Control**: Use `lit()`, `notype()`, `bare()`, `deserial()` to customize serialization behavior.
3. **Manual Construction**: Use `__vyb_json_*` functions for complex manual JSON building.
4. **Performance**: Auto-serialization is optimized for common cases; manual intrinsics available for edge cases.

For comprehensive documentation on auto-serialization capabilities and configuration, see `doc/Auto_Serialization_Main_Returns.md`.

---

## 9. Proposed/Experimental Intrinsics

```vyb
fn offset<T>(ptr: loc<T>, count: Int) -> loc<T>
fn is_null<T>(ptr: loc<T>) -> Bool
fn aligned<T>(ptr: loc<T>) -> Bool
fn mem_copy(dst: loc<UInt8>, src: loc<UInt8>, n: UInt) -> Void
fn mem_set(ptr: loc<UInt8>, value: UInt8, n: UInt) -> Void
// Atomic & volatile operations
```

---

## 10. Usage Guidelines

1. **Declarations**: choose explicit (`var<T>`) or inferred (`var auto`).
2. **Functions**: return in `<Type>`, arrow mandatory, braces optional for single expressions.
3. **Ownership**: use `my<T>`, `our<T>`, `their<T>`, with canonical `borrow(expr)` / `view(expr)` borrowing.
4. **Intrinsics**: memory ops only in `freedom`, metadata always safe.
5. **Print**: use generic `println(value)` for any type; prefer `to_string()` for explicit conversion.
6. **Serialization**: use auto-serialization for `main()` returns; mode intrinsics for customization.
7. **Stability**: Sections 4–8 are stable; Section 9 is experimental.