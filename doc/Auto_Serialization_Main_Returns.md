**Multi-Value Returns with Typeful JSON Serialization**

**Status:** Core functionality IMPLEMENTED and COHERENT (v0.5.3+). The intrinsics
proposed further down — the multi-argument `lit(...)`, `notype()`, `bare()` and
`<Type>`-suffixed field names — are DESIGN ONLY and are not implemented; see
"Implementation status" at the end.

- Auto-serialization implemented for non-Void `main()` return types
- Coherent rules: Int, Bool, Float, String, single structs and multi-value tuples
  JSONify to stdout; the exit code is always 0
- `exit(n)` built-in available to set the process exit code explicitly
- Multi-value returns produce JSON arrays: `[42, "hello"]`
- Single Int returns print the numeric value (e.g. `42`)
- Single Bool returns print `true` or `false`
- Single Float returns print the numeric value
- Single String returns print the JSON-encoded string `"hello"`
- Single struct returns print a keyed JSON object of its fields, e.g.
  `{"system": "x86_64-linux", "hostname": "hera", "packages": ["bash"]}` — no
  type wrapper and no `<Type>` field suffixes
- Single `Vec<T>` returns print a JSON array of the elements, e.g.
  `["alpha", "beta"]`; nested `Vec<Vec<T>>` returns nest the arrays, and struct
  or `Vec` elements inside a multi-value return serialize the same way (#375)

## Coherence Rules for main() Return Values

The rules below define how `main()` return values are handled. Every non-Void
return type produces JSON output to stdout and the process exits with code 0.
Use `exit(n)` to set a custom exit code.

| Return type     | Output                            | Exit code | Notes                        |
| --------------- | --------------------------------- | --------- | ---------------------------- |
| `main()<Void>`  | *(none)*                          | 0         | No output                    |
| `main()<Int>`   | number (e.g. `42`)               | 0         | JSON number                  |
| `main()<Bool>`  | `true` or `false`                 | 0         | JSON boolean                 |
| `main()<Float>` | number (e.g. `3.14`)             | 0         | JSON number                  |
| `main()<String>`| `"hello world"` (quoted)          | 0         | JSON string                  |
| `main()<T>` (struct) | `{"field": value, ...}`      | 0         | Keyed JSON object            |
| `main()<T1,T2>` | `[val1, val2]` JSON array         | 0         | Multi-value tuple; struct/Vec elements serialize too |
| `main()<Vec<T>>` | `["alpha", "beta"]`              | 0         | JSON array of the elements (#375) |
| `main()<Struct, Struct>` | `[{"id": 1, "name": "Alice"}, {...}]` | 0 | Struct elements keyed objects (#375) |
| `exit(n)`       | *(none)*                          | n         | Built-in; works anywhere     |

### Key rule

- **All non-Void return types** are auto-JSONified: the value is printed to stdout as JSON
  and the process exits with code 0.
- **`exit(n)`** terminates the process immediately with exit code `n`. No output is produced.
  Use this to explicitly control the process exit code from any function.
- **A `Vec<T>` return** prints a JSON array of its elements (`["alpha", "beta"]`);
  nested `Vec<Vec<T>>` returns nest the arrays, and a struct or `Vec` element of
  a multi-value return serializes the same way (#375).

### Migration note

Prior to v0.5.3, `main()<Int>` used the Unix exit-code convention (the integer return value
became the process exit code). Programs that relied on that behavior should be updated to
call `exit(n)` explicitly:

```vyb
// Old behavior (v0.5.2 and earlier):
main()<Int> -> { return 42 }  // exit code 42, no stdout output

// New behavior (v0.5.3+): returns 42 as JSON to stdout, exits 0
main()<Int> -> { return 42 }  // stdout: 42, exit code: 0

// To get the old behavior, use exit(n):
main() -> { exit(42) }         // no stdout output, exit code: 42
```

## Goal

Enable any function in Vyb to return one or more values of arbitrary types, with the Vyb runtime auto‑serializing the result into a concise, type‑annotated JSON representation and allowing a matching auto‑deserializer to produce faithful Vyb structs.

## Why

* **Zero boilerplate** for round‑trip between VybR and storage or IPC.
* **Type fidelity**: preserve declared types (e.g. `Int8`, `Int32`) without implicit demotion.
* **Interoperability**: leverage JSON as lingua franca for external tools and runtimes.

## Syntax

Auto‑JSONification applies only to `main()` returns. Other functions can use an explicit emitter (`toJSON()`).

Declare `main`'s return types as generics on the function, then `return` comma‑separated values:

```vyb
main()<Point,Rectangle> ->
  p<Point>, r<Rectangle> = createShapes()
  // modify
  p.x *= 2
  r.height += 25
  return p, r    // auto‑serializes p,r to JSON on exit
```

For non‑`main` functions, call each struct’s built‑in `toJSON()` helper explicitly:

```vyb
compute()<String> ->
  p<Point> = makePoint()
  return p.toJSON()  // returns JSON string
```

* **`main()<T1,T2,…>`**: generic return‑type list for `main`.
* **`return`** in `main` triggers auto‑JSONification; elsewhere it returns native Vyb values or JSON if using `toJSON()`.

## Auto-Deserialization

VybR provides a built‑in intrinsic:

```vyb
// this section needs more thinking.
// I don't have a clearly defined Tuple for multi-type members, or even like-types

auto tuple = deserial(jsonString)
x<Tuple<Person,Company>> = tuple // does type check validation in runtime
// is that then id<Int> = x.Person.id or company<Company> = x.Company for use later?
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

```vyb
main()<Int,Float,String> ->
  return lit(42, 3.14, "hello")    // emits [42, 3.14, "hello"]

main()<Int> ->
  return lit(0)                    // emits 0
```

* Without `lit()`, `return 0` would emit `{ "Int": 0 }` — i.e., with type annotation.
* `lit()` is restricted to primitive values: Int, Float, String, Bool.
* Cannot include structs or complex types.
* Cannot be applied to values that are already named or typed structs.
* Using `lit()` with a struct should emit a compile-time error:

```vyb
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

```vyb
// Invalid usage: notype() requires struct input
main()<String, Int, String> -> {
    return "hello", notype(42), "world";
}
```

Expected behavior:

```text
// @expect: fail
// @expect-error: notype() requires struct input, but got primitive Int
```

For correct usage, wrap one or more structs:

```vyb
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

```vyb
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

```vyb
p<Person> = Person(id=123, name="Example", salary=88000.0)
return bare(p)
```

Emits:

```json
[123, "Example", 88000.0]
```

**Constraints:**

* Must be used on structs only.
* Cannot be applied to primitives or anonymous literals.
* Cannot be applied more than once per return expression unless explicitly supported in future.

Invalid usage:

```vyb
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

* **`vyb::fromJson<T>(json, index?)`**: extract element(s) by declared type from array or object. This is not really well defined yet as we don't have a standard vyb:: defined yet, nor ? syntax for maybe type (yet?).

## Runtime Behavior

* **Reflection metadata**: drives single-pass JSON emitter.
* **JSON-return toggle**: compiler flag `--json-return` enables/disables automatic serialization on `return`.
* **Performance**: single-value returns skip array wrapper; multi-value adds minimal loop overhead.

## Next Steps

1. Define reflection metadata format in vyb.
2. Implement JSON emitter after AST lowering.
3. Extend VybR with `deserial`, `fromJson`, and error-reporting.
4. Test nested structs, literal returns, array-of-primitives, versioning, and error cases.

## Implementation status

Verified against `build/vyb` (v0.7.7). What ships:

| Feature | State |
| --- | --- |
| Int / Bool / Float / String returns | shipped, JSONified |
| Single struct return (keyed object, intact `Vec` fields) | shipped (#182) |
| Multi-value return of primitives, e.g. `[42, "hello"]` | shipped |
| `main()<Vec<T>>` return, e.g. `["alpha", "beta"]` | shipped (#375) |
| Struct or `Vec` element inside a multi-value return | shipped (#375) |
| Nested `main()<Vec<Vec<T>>>` return | shipped (#375) |
| `exit(n)` | shipped (sets the process status) |
| `lit("...")` with a single primitive argument | shipped (raw literal output) |
| `lit(a, b, c)` multi-argument tuple form | **not implemented** — a semantic error |
| `bare(...)` in a multi-value return | runs, but emits the keyed object rather than raw field values (design-only) |
| `notype(...)` in a multi-value return | shipped — elements serialize with plain field names |
| `<Type>`-suffixed field names in struct output | **not implemented** — a single struct prints its plain field names (`{"name": "vyb", "n": 7}`) with no type wrapper |
| `deserial(str)` / `vyb::fromJson<T>` | not implemented: `deserial(x)` is a pass-through placeholder in codegen, and there is no `json` module — use the per-type `T::from_string` JSON reader |
| `--json-return` compiler flag | not present |

The sections above that describe `notype()`, `bare()`, the `<Type>` suffix scheme
and the raw-literal tuple form are design notes for future work, not shipped
behaviour. For the shipped contract, trust `docs/refman/PROGRAMMERS_GUIDE.md`
(Appendix B) and the code.

---

*Concise returns, faithful types, zero ceremony.*
*references to VybL and VybR are Vyb Language and Vyb Runtime respectively*
