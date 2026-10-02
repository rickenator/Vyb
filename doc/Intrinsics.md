# Vyb Intrinsics & Core Syntax

This document covers Vyb’s built-in intrinsics, variable declaration syntax (including type inference), and function declaration syntax, all aligned with the `<T>`‑first style.

---

## 1. Variable Bindings

A binding names a value with its type:

```ebnf
Binding      ::= Identifier "<" Type ">" [ "=" Expression ]
               | Identifier "=" Expression     # type inferred from the initializer
```

- **`name<Type> = expr`** — the canonical form: a binding of type `Type`.
- **`name = expr`** — the same, with `Type` inferred from the initializer
  (`auto name = expr` is the optional explicit spelling of the same thing).
- **`name<Type const> = expr`** / **`const<Type> name = expr`** — an immutable
  binding; `const` is a type modifier, so there is no separate declaration keyword.

### Declaration Examples

```vyb
x<Int> = 42            # a binding of type Int
y = 42                 # the same, with the type inferred from the initializer
s<String> = "hello"    # a binding of type String
tree<BTreeMap<Int, String>> = BTreeMap<Int, String>()   # a container binding
```

Ownership-aware declarations:

```vyb
task<my<Task>> = my(Task { id: 1, payload: "foo" })
cfg<our<Config>> = our(Config { debug: true })
b<their<Foo>> = borrow(owner)        # mutable borrow
v<their<Foo const>> = view(owner)    # read-only borrow
```

Pointer declarations (inside `freedom`):

```vyb
freedom {
  p<loc<Int>> = loc(x)
  at(p) = 99
}
```

---

## 2. Function Declaration Syntax

A declaration is `name(params)<ReturnType> -> body`. The return type follows the
parameter list, the `->` separator is mandatory, and braces are optional for a
single-expression body. Type parameters, when present, precede the parameter list.

```ebnf
FunctionDecl ::= Identifier [ "<" GenericParam { "," GenericParam } ">" ]
                 "(" ParamList ")" [ "<" ReturnType " >" ] "->" Body

GenericParam ::= Identifier [ "<" Type ">" ]     # optional bound, e.g. K<Equatable>
ParamList    ::= [ Param { "," Param } ]
Param        ::= Identifier "<" Type ">"

Body         ::= Block
               | Expression

Block        ::= "{" Statement* [ Expression ] "}"
Expression   ::= <any single Vyb expression>
```

- **Return type**: `name(params)<ReturnType> ->`; omit it for a `Void` procedure.
- **Parameters**: `name<Type>` for each one, comma-separated.
- **Type parameters**: `name<T>(a<T>)<T> ->`, with an optional bound (`T<Equatable>`).
- **`->`**: mandatory separator between signature and body.
- **Braces** `{}`: optional only for a single-expression body.

### Function Declaration Examples

```vyb
struct Node {
    is_leaf<Bool>
}

# A constructor is an ordinary function that returns the struct.
new_node(is_leaf<Bool>)<Node> -> {
    return Node { is_leaf: is_leaf }
}

# Concise single-expression body (braces optional).
double_it(x<Int>)<Int> -> x * 2
```

---

## 3. Intrinsics Overview

Intrinsics are compiler-handled operations, split into **stable** (Sections 4–8) and **proposed/experimental** (Section 9).

### 3.1 Reserved Keywords

These names are reserved and cannot be used as identifiers:

- **Declarations**: `struct`, `enum`, `type`, `aspect`, `bind`, `share`, `import`, `smuggle`; the
  binding modifiers `auto` and `const` (and the legacy `var` prefix) are accepted in
  binding position only.
- **Ownership & borrowing**: `my`, `our`, `their`, `borrow`, `view`
- **Pointer & address**: `loc`, `at`, `addr`, `from`
- **Type metadata**: `sizeof`, `alignof`, `offsetof`
- **Visibility/macros**: `import`, `smuggle`, `share`

---

## 4. Ownership Intrinsics

### 4.1 Core Wrappers

```vyb
my(value<T>)<my<T>> ->
our(value<T>)<our<T>> ->
borrow(owner)<their<T>> ->
view(owner)<their<T const>> ->
```

- **`my<T>(value)`**: wrap `value` in a unique-owned `my<T>`.
- **`our<T>(value)`**: wrap `value` in a shared-owned `our<T>`.
- **`their<T>(owner)`**: create a mutable borrow `their<T>` of `owner`.
- **`their<T const>(owner)`**: create an immutable borrow `their<T const>` of `owner`.

### 4.2 Shorthand Borrowing

For convenience, the compiler provides **inferred** shorthand intrinsics:

```vyb
borrow(owner)<their<T>> ->        # `T` inferred from `owner`
view(owner)<their<T const>> ->    # read-only borrow of `owner`
```

- **`borrow(owner)`** infers `T` from `owner` and returns `their<T>`.
- **`view(owner)`** infers `T` from `owner` and returns `their<T const>`.

---

## 5. Memory Intrinsics (`freedom` required)

```vyb
loc(expr<T>)<loc<T>> ->        # requires a `freedom` block
at(pointer<loc<T>>)<T> ->
addr(pointer<loc<T>>)<Int64> ->
from<P>(addr<Int64>)<P> ->
```

- **`loc<T>(expr)`**: address‑of a value → `loc<T>`.
- **`at<T>(pointer)`**: dereference pointer → `T` (l-value/r-value).
- **`addr<T>(pointer)`**: pointer → raw `Int64`.
- **`from<P>(addr)`**: raw `Int64` → pointer `P`.

---

## 6. Type Metadata Intrinsics (safe)

```vyb
sizeof<T>()<UInt> ->
alignof<T>()<UInt> ->
offsetof<T>(field)<UInt> ->      # `field` is a field name, not a value
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

#### Generic Print Examples

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
to_string(self<Int>)<String> ->
to_string(self<Float>)<String> ->
to_string(self<Bool>)<String> ->
to_string(self<String>)<String> ->
```

#### to_string Examples

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

Vyb provides built-in serialization support for automatic JSON generation of data structures, particularly for values returned from `main()`. These intrinsics are stable and ready for production use.

### 7.1 Serialization Mode Intrinsics

```vyb
lit(value<T>)<T> ->
notype(value<T>)<T> ->
bare(value<T>)<T> ->
```

- **`lit(value)`**: Emits raw JSON literals without type wrapping. Converts strings to raw JSON values, numbers to unquoted numbers, and booleans to literal true/false. Restricted to primitive values (Int, Float, String, Bool).

- **`notype(value)`**: Removes `<Type>` suffixes from field names in struct serialization. For structs, this produces cleaner JSON field names without type annotations. Only valid for structs.

- **`bare(value)`**: Emits only raw field values as JSON array, removing all type and field metadata. For structs, outputs values in field declaration order as a JSON array. Only valid for structs.

- **`deserial(json_string)`**: **refused** rather than forwarded (#383) — it raises
  `deserial() is not implemented` at the call site. Parse JSON with the `json` module, or
  rebuild the value with the type's own `T::from_string(json)`, which is length- and
  bounds-checked (see the Programmer's Guide, § serialization).

#### Serialization Examples

**lit() Intrinsic:**

```vyb
main()<String> -> {
    return lit("42");     // Output: 42 (number, not string)
}

main()<String> -> {
    return lit("true");   // Output: true (boolean, not string)
}

main()<String> -> {
    return lit("hello");  // Output: "hello" (quoted string)
}
```

**notype() Intrinsic:**

```vyb
struct Person {
    id<Int>,
    name<String>
}

main()<Person> -> {
    p<Person> = Person { id: 123, name: "Alice" };
    return notype(p);
    // Output: {"id": 123, "name": "Alice"}
    // (the old <Type>-suffixed names are not implemented -- see the Programmer's Guide)
}
```

**bare() Intrinsic:**

```vyb
struct Point {
    x<Float>,
    y<Float>
}

main()<Point> -> {
    point<Point> = Point { x: 3.5, y: 4.2 };
    return bare(point);
    // Output: [3.5, 4.2]
}
```

**Multi-Value with Mixed Intrinsics:**

```vyb
struct Config {
    name<String>,
    version<Int>
}

main()<Config, Int> -> {
    cfg<Config> = Config { name: "app", version: 1 }
    return notype(cfg), 42
    // Output: [{"name": "app", "version": 1}, 42]
    // A `lit(...)` element inside a MULTI-value return is not codegen-clean on this
    // build; the single-value `lit()` programs above are.
}
```

### 7.2 JSON Serialization Intrinsics

The following intrinsics are provided for manual JSON construction and are used internally by the auto-serialization system:

```vyb
__vyb_serialize_to_json(value<any>)<String> ->
__vyb_serialize_struct_with_names(value<any>)<String> ->
__vyb_json_array_start()<String> ->
__vyb_json_array_append(current<String>, item<String>)<String> ->
__vyb_json_array_end(current<String>)<String> ->
__vyb_json_object_start()<String> ->
__vyb_json_object_append_field(current<String>, name<String>, value<String>)<String> ->
__vyb_json_object_end(current<String>)<String> ->
```

- **`__vyb_serialize_to_json(value)`**: serialize any value to JSON string.
- **`__vyb_serialize_struct_with_names(value)`**: serialize struct with field names included.
- **JSON Array functions**: manually construct JSON arrays with proper formatting.
- **JSON Object functions**: manually construct JSON objects with field names and values.

### 7.3 Auto-Serialization Usage

The auto-serialization system automatically activates when `main()` returns a structured value:

```vyb
// Simple value return - auto-serialized to JSON
main()<Int> -> {
    return 42  // Output: 42
}

// Struct return - auto-serialized with field names
struct Point {
    x<Float>,
    y<Float>
}

main()<Point> -> {
    return Point { x: 3.5, y: 4.2 }
    // Output: {"x": 3.5, "y": 4.2}
}

// Custom serialization with mode intrinsics
main()<String> -> {
    name<String> = "example"
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
offset<T>(ptr<loc<T>>, count<Int>)<loc<T>> ->
is_null<T>(ptr<loc<T>>)<Bool> ->
aligned<T>(ptr<loc<T>>)<Bool> ->
mem_copy(dst<loc<UInt8>>, src<loc<UInt8>>, n<UInt>) ->
mem_set(ptr<loc<UInt8>>, value<UInt8>, n<UInt>) ->
// Atomic & volatile operations
```

---

## 10. Usage Guidelines

1. **Bindings**: choose explicit (`name<Type> = expr`) or inferred (`name = expr`).
2. **Functions**: `name(params)<ReturnType> ->`; the arrow is mandatory and braces are optional for a single expression.
3. **Ownership**: use `my<T>`, `our<T>`, `their<T>`, with canonical `borrow(expr)` / `view(expr)` borrowing.
4. **Intrinsics**: memory ops only in `freedom`, metadata always safe.
5. **Print**: use generic `println(value)` for any type; prefer `to_string()` for explicit conversion.
6. **Serialization**: use auto-serialization for `main()` returns; mode intrinsics for customization.
7. **Stability**: Sections 4–8 are stable; Section 9 is experimental.
