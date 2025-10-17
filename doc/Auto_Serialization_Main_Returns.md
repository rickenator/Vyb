**Multi-Value Returns with Typeful JSON Serialization**

**Status:** ✅ Core functionality IMPLEMENTED in v0.3.7
- Smart type detection working
- Multi-value returns functional: `fn<Int,String> main() -> return 42, "hello"`
- Auto-serialization prevents segfaults
- Basic structured output implemented
- Full JSON formatting and advanced features in progress

## Goal

Enable any function in Vyn to return one or more values of arbitrary types, with the Vyn runtime auto‑serializing the result into a concise, type‑annotated JSON representation and allowing a matching auto‑deserializer to produce faithful Vyn structs.

## Why

* **Zero boilerplate** for round‑trip between VynR and storage or IPC.
* **Type fidelity**: preserve declared types (e.g. `Int8`, `Int32`) without implicit demotion.
* **Interoperability**: leverage JSON as lingua franca for external tools and runtimes.

## Syntax

Auto‑JSONification applies only to `main()` returns. Other functions can use an explicit emitter (`toJSON()`).

Declare `main`’s return types as generics on the function, then `return` comma‑separated variables:

```vyn
fn<Point,Rectangle> main() ->
  var<Point> p, var<Rectangle> r = createShapes()
  // modify
  p.x *= 2
  r.height += 25
  return p, r    // auto‑serializes p,r to JSON on exit
```

For non‑`main` functions, call each struct’s built‑in `toJSON()` helper explicitly:

```vyn
fn<String> compute() ->
  var<Point> p = makePoint()
  return p.toJSON()  // returns JSON string
```

* **`fn<T1,T2,…>`**: generic return‑type list for `main`.
* **`return`** in `main` triggers auto‑JSONification; elsewhere it returns native Vyn values or JSON if using `toJSON()`.

## Auto-Deserialization

VynR provides a built‑in intrinsic:

```vyn
// this section needs more thinking. 
// I don't have a clearly defined Tuple for multi-type members, or even like-types

auto tuple = deserial(jsonString)
var<Tuple<Person,Company>> x = tuple // does type check validation in runtime
// is that then var<Int> id = x.Person.id or var<Company> x.Company for use later?
```

* `deserial(str)` returns an anonymous tuple whose elements’ types and field names match exactly what was serialized.
* No loss of precision: if `id: Int8`, it remains `Int8` inside the tuple.

## JSON Output

### Multi-value Structs

Returns of multiple structs serialize to a JSON array of objects keyed by type name:

```json
[
  { "Person": { "id<Int32>": 123, "name<String>": "Alice", "salary<Float>": 88000.0 } },
  { "Company": { "name<String>": "Acme", "employeeCount<Int8>": 150 } } 
]
```

### Single-value Struct

A lone struct return emits a single object (no array):

```json
{ "User": { "id<Int>": 42, "active<String>": true } }
```

### Literal-only Values

Wrap literals in the intrinsic `lit()` to emit raw JSON primitives without wrapping:

```vyn
fn<Int,Float,String> main() ->
  return lit(42, 3.14, "hello")    // emits [42, 3.14, "hello"]

fn<Int> main() ->
  return lit(0)                    // emits 0
```

* Without `lit()`, `return 0` would emit `{ "Int": 0 }` — i.e., with type annotation.
* `lit()` is restricted to primitive values: Int, Float, String, Bool.
* Cannot include structs or complex types.
* Cannot be applied to values that are already named or typed structs.
* Using `lit()` with a struct should emit a compile-time error:

```vyn
return lit(Person(name="Alice", id=1))  // ❌
```

Expected diagnostic:

```text
// @expect-error: lit() only accepts primitive literals (Int, Float, String, Bool)
```

## Type Inference and Annotation

* **Declared field types**: serialization always uses each struct’s field definitions; no implicit numeric demotion.
* **Ad-hoc JSON annotation**: in contexts outside `return`, annotate keys to guide parsing:

  ```json
  { "id<Int8>": "123", "flag<Bool>": "true" }
  ```

  The parser reads `<Type>` suffix and casts the literal accordingly, erroring on mismatches.

## Stripping Type Metadata (`notype()`)

The `notype()` intrinsic removes `<Type>` suffixes from field names in struct return values. It is only valid when applied to structs or collections of structs. It must not be used on primitive values or anonymous tuples.

If `notype()` is used with raw primitives (e.g., `notype(42)`), the compiler should emit a diagnostic error:

```vyn
// Invalid usage: notype() requires struct input
fn<String, Int, String> main() -> {
    return "hello", notype(42), "world";
}
```

Expected behavior:

```text
// @expect: fail
// @expect-error: notype() requires struct input, but got primitive Int
```

For correct usage, wrap one or more structs:

```vyn
fn<Person, Company> main():
  var<Person> u, var<Company> c = createEntities()
  // emit JSON without <Type> suffix on fields
  return notype(u), c
```

Emits:

```json
[
  { "Person": { "id": 123, "name": "Alice", "salary": 88000.0 } },
  { "Company": { "name<String>": "Acme", "employeeCount<Int32>": 150 } }
]
```

`notype()` bypasses the default `<Type>` suffix emission but preserves the root object key as the type name.

Use `lit()` for emitting raw primitives or anonymous tuples.

By default, returned JSON field names include `<Type>` suffixes. To emit clean JSON without embedded type metadata, wrap return values in the `notype()` intrinsic within `main`:

```vyn
fn<Person, Company> main():
  var<Person> u, var<Company> c = createEntities()
  // emit JSON without <Type> suffix on fields
  return notype(u), c
```

Emits:

```json
[
  { "Person": { "id": 123, "name": "Alice", "salary": 88000.0 } },
  { "Company": { "name<String>": "Acme", "employeeCount<Int32>": 150 } }
]
```

`notype()` bypasses the default `<Type>` suffix emission but preserves the root object key as the type name.

## Minimal Output (`bare()` — Experimental)

Use `bare()` to emit *only the raw field values* of a struct as a JSON array — removing all type and field metadata. Useful for returning unnamed tuples.

```vyn
var<Person> p = Person(id=123, name="Rick", salary=88000.0)
return bare(p)
```

Emits:

```json
[123, "Rick", 88000.0]
```

**Constraints:**

* Must be used on structs only.
* Cannot be applied to primitives or anonymous literals.
* Cannot be applied more than once per return expression unless explicitly supported in future.

Invalid usage:

```vyn
return bare(42)  // ❌
```

Expected diagnostic:

```text
// @expect-error: bare() requires a struct input, not a primitive
```

* Assumes consumers know field order.
* Cannot reconstruct type or fields without external schema.
* Recommended only for `main()` or data pipelines where structural metadata is implied.

## Return Intrinsics Summary

| Form                         | Output JSON                            | Notes                            |
| ---------------------------- | -------------------------------------- | -------------------------------- |
| `return lit(42, "foo")`      | `[42, "foo"]`                          | Raw literal tuple                |
| `return notype(Person(...))` | `{ "Person": { "field": val, ... } }`  | No `<Type>` suffixes             |
| `return bare(Person(...))`   | `[val1, val2, val3]`                   | Anonymous field values           |
| `return Person(...)`         | `{ "Person": { "field<Type>": val } }` | Full metadata (default behavior) |

## Deserialization API

* **`deserial(str)`**: returns an anonymous tuple *or* a named struct, depending on the shape of the JSON input.

  * If the top-level JSON is an object with a single key (e.g. `{ "Person": {...} }`), it is assumed to be a single struct.
  * If the top-level JSON is an array (e.g. `[ { "Person": {...} }, { "Company": {...} } ]`), it returns an anonymous tuple whose elements match the declared types.
  * If the top-level JSON is a raw literal or literal array (e.g. `42` or `["foo", 42]`), it returns a literal or tuple of literals (matching `lit()` output).

This preserves symmetry with `main()` return rules and guarantees a 1:1 mapping for reconstitution.

* **`vyn::fromJson<T>(json, index?)`**: extract element(s) by declared type from array or object. This is not really well defined yet as we don't have a standard vyn:: defined yet, nor ? syntax for maybe type (yet?).

## Runtime Behavior

* **Reflection metadata**: drives single-pass JSON emitter.
* **JSON-return toggle**: compiler flag `--json-return` enables/disables automatic serialization on `return`.
* **Performance**: single-value returns skip array wrapper; multi-value adds minimal loop overhead.

## Next Steps

1. Define reflection metadata format in vyn.
2. Implement JSON emitter after AST lowering.
3. Extend VynR with `deserial`, `fromJson`, and error-reporting.
4. Test nested structs, literal returns, array-of-primitives, versioning, and error cases.

---

*Concise returns, faithful types, zero ceremony.*
*references to VynL and VynR are Vyn Language and Vyn Runtime respectively*
