# Vyb Language Reference

A **lookup-oriented quick reference** for the Vyb language: keywords,
operators, types, and syntax forms, each with the canonical spelling and a
link to the deeper prose in the [Programmer's Guide](PROGRAMMERS_GUIDE.md).

This page is **derived from** the existing authoritative docs — the Programmer's
Guide ([tool tour](PROGRAMMERS_GUIDE.md#3-language-tour)) and
[`doc/Canonical_Reference_Syntax.md`](../../doc/Canonical_Reference_Syntax.md) —
and mirrors their canonical forms. It is deliberately terse: pick the construct,
read the form, follow the link for the full discussion and examples.
Use it as a cheat-sheet *alongside* the guide, not instead of it.

Module/standard-library symbols live in the generated [module reference](index.md)
and cross-indexes ([functions](functions.md), [types](types.md),
[aspects & binds](aspects.md)).

---

## 1. Program shape

A file is a sequence of top-level declarations. Every executable has `main`:

```vyb
main()<Int> -> { return 0 }          # a side-effecting program can omit <Int> -> Void
```

Calls and type references resolve independent of declaration order (functions,
structs, enums, aliases and `share(all)` module helpers may be used before they
are written). See [guide §3.4](#).

```vyb
main()<Int> -> { return helper() }   # helper is declared below — fine
helper()<Int> -> { return 42 }
```

## 2. Declarations

| Construct | Form | Guide |
|---|---|---|
| function | `name(<params>)<Ret> -> { … }` | [§3.4](PROGRAMMERS_GUIDE.md#34-functions-and-parameters) |
| Void procedure | `name(<params>) -> { … }` (return type omitted) | [§3.4](PROGRAMMERS_GUIDE.md#34-functions-and-parameters) |
| closure / lambda | `\|x\| -> expr` · `\|\| -> { … }` | [§3.5](PROGRAMMERS_GUIDE.md#35-closures-and-lambdas) |
| `fn` type | `fn(Args…) -> Ret` | [§3.4](PROGRAMMERS_GUIDE.md#34-functions-and-parameters) |
| variable (typed) | `name<Type> = value` | [§3.3](PROGRAMMERS_GUIDE.md#33-variables-inference-mutability) |
| variable (inferred) | `name = value` (`auto` optional) | [§3.3](PROGRAMMERS_GUIDE.md#33-variables-inference-mutability) |
| immutable | `name<Type const> = …` or `const<Type> name = …` | [§3.3](PROGRAMMERS_GUIDE.md#33-variables-inference-mutability) |
| tuple destructure | `a, b, c = t` | [§3.3](PROGRAMMERS_GUIDE.md#33-variables-inference-mutability) |
| struct | `struct Name { f<Type> … }` | [§3.9](PROGRAMMERS_GUIDE.md#39-structs) |
| enum | `enum Name { MEMBER … }` | [§3.10](PROGRAMMERS_GUIDE.md#310-enums) |
| constant enum | `enum Name { A = 1, B = 2 }` (bitwise-`|` combinable) | [§3.10](PROGRAMMERS_GUIDE.md#310-enums) |
| type alias | `type Name = …` | aliases may carry generics |
| generic function | `fn name<T>(…) …` | [§3.14](PROGRAMMERS_GUIDE.md#314-generics-and-monomorphization) |
| generic type | `struct Pair<K, V>` | [§3.14](PROGRAMMERS_GUIDE.md#314-generics-and-monomorphization) |
| aspect (contract) | `aspect X { method(self)… }` | [§3.15](PROGRAMMERS_GUIDE.md#315-aspects-and-binds-polymorphism) |
| bind (impl) | `bind Aspect -> Type { … }` | [§3.15](PROGRAMMERS_GUIDE.md#315-aspects-and-binds-polymorphism) |
| import | `import module::{a, b}` | [§3.19](PROGRAMMERS_GUIDE.md#319-modules-and-imports) |
| export | `share(all)` at top of a module | [§3.19](PROGRAMMERS_GUIDE.md#319-modules-and-imports) |

> A callable that returns a value **must** declare `<Ret>`; Vyb never infers a
> value return type from the body. Omitting it means `Void`. An empty `<>` is
> invalid — omit the bracket instead. See [§3.4](PROGRAMMERS_GUIDE.md#34-functions-and-parameters).

## 3. Types

### Primitives

| Type | Meaning | Guide |
|---|---|---|
| `Int` / `Int64` | signed 64-bit (default) | [§3.2](PROGRAMMERS_GUIDE.md#32-primitives-and-literals) |
| `Int32`/`Int16`/`Int8` | narrower signed ints | [§3.2](PROGRAMMERS_GUIDE.md#32-primitives-and-literals) |
| `UInt` / `UInt64` | unsigned 64-bit | [§3.2](PROGRAMMERS_GUIDE.md#32-primitives-and-literals) |
| `UInt32`/`UInt16`/`UInt8` | narrower unsigned ints | [§3.2](PROGRAMMERS_GUIDE.md#32-primitives-and-literals) |
| `Float` / `Float64` | double (default) | [§3.2](PROGRAMMERS_GUIDE.md#32-primitives-and-literals) |
| `Float32` | single | [§3.2](PROGRAMMERS_GUIDE.md#32-primitives-and-literals) |
| `Char` | a UTF-8 code unit | [§3.2](PROGRAMMERS_GUIDE.md#32-primitives-and-literals) |
| `Rune` | a Unicode code point | [§3.2](PROGRAMMERS_GUIDE.md#32-primitives-and-literals) |
| `Bytes` | fat-pointer block of raw bytes | [§3.2](PROGRAMMERS_GUIDE.md#32-primitives-and-literals) |
| `Bool` | `true` / `false` | [§3.2](PROGRAMMERS_GUIDE.md#32-primitives-and-literals) |
| `String` | immutable, fat-pointer, bounds-checked | [§3.17](PROGRAMMERS_GUIDE.md#317-strings) |
| `Void` | the absence of a value | [§3.2](PROGRAMMERS_GUIDE.md#32-primitives-and-literals) |
| `T?` | native optional: present `T?(v)` / absent `T?()` | [§3.2](PROGRAMMERS_GUIDE.md#32-primitives-and-literals) |

Numeric type names accept Vyb (`Int32`), C (`int32`) and LLVM (`i32`) spellings.
Integer literals use `0x`/`0b`/`0`-prefixed bases and flow into any sized type.

### Optional `T?`

No `null` in the data model — absence is typed `T?` and every read must say what
happens when absent:

```vyb
a<Int?> = Int?(5)          # present
b<Int?> = Int?()           # absent
val<Int> = a else 42       # payload when present, else fallback (lazy, right-assoc)
```

`else` is the absence-handling operator (`a else b else c` → first present wins).
Absence *is* failure for fallible stdlib ops (`open_read -> File?`, receives, etc.).
See [§3.2](PROGRAMMERS_GUIDE.md#32-primitives-and-literals) and [§3.16](PROGRAMMERS_GUIDE.md#316-error-handling-trap--fail--ensure).

### Ownership types

| Type | Meaning | Guide |
|---|---|---|
| `my<T>` | unique ownership (like `Box`); moves, use-after-move rejected | [§3.13](PROGRAMMERS_GUIDE.md#313-ownership-and-accessors) |
| `our<T>` | shared, refcounted ownership; freed on last drop | [§3.13](PROGRAMMERS_GUIDE.md#313-ownership-and-accessors) |
| `their<T>` | borrowed, non-owning reference; cannot escape scope | [§3.13](PROGRAMMERS_GUIDE.md#313-ownership-and-accessors) |
| `their<T const>` | read-only borrow (`view(x)`) | [§3.4.1](PROGRAMMERS_GUIDE.md#341-parameter-passing-value-borrow-and-ownership) |
| `mild<T>` | weak reference (`soft(x)`); won't prevent cleanup | [§3.13](PROGRAMMERS_GUIDE.md#313-ownership-and-accessors) |

Construction/borrow **are function calls** (canonical, verified 2026-09-10):
`my(expr)`, `our(expr)`, `view(expr)` (read-only `their`), `borrow(expr)` (mutable
`their`), `soft(expr)` (→ `mild`). Prefix forms (`borrow expr`) are non-canonical.
See `doc/Canonical_Reference_Syntax.md`.

### Collections

| Type | Construct | Guide |
|---|---|---|
| `Vec<T>` | dynamic array | [§4.5](PROGRAMMERS_GUIDE.md#45-collections--vec-map-set-btree) |
| `Map<K, V>` / `HashMap<K, V>` | hash map (`K<Hashable, Equatable>`) | [§4.5](PROGRAMMERS_GUIDE.md#45-collections--vec-map-set-btree) |
| `Set` / `HashSet` | hash set | [§4.5](PROGRAMMERS_GUIDE.md#45-collections--vec-map-set-btree) |
| `BTreeMap` / `BTreeSet` | ordered | [§4.5](PROGRAMMERS_GUIDE.md#45-collections--vec-map-set-btree) |
| `Tuple<A, B, …>` | heterogeneous value sequence, `t[i]`/`.len()` | [§3.8](PROGRAMMERS_GUIDE.md#38-tuples-and-variadic-tuples) |
| `fn(…) -> …` | function/closure type | [§3.4](PROGRAMMERS_GUIDE.md#34-functions-and-parameters) |

`Vec` methods: `push`, `pop`, `get(i)`, `first`/`last`, `len`, `iter`, `map`,
`filter`, `reduce`; `for (x in v)` iterates via `.next()`, and a custom
`bind Iterator` type iterates the same way. See [§3.18](PROGRAMMERS_GUIDE.md#318-collections)
and [§3.6](PROGRAMMERS_GUIDE.md#36-control-flow).

## 4. Operators

Grouped by category (see [§3.11](PROGRAMMERS_GUIDE.md#311-operators) for the
canonical set; bitwise requires matching widths):

```vyb
# Arithmetic / comparison
+ - * / %    == != < > <= >=

# Bitwise (Int/UInt widths) + compound-assign
|  &  ^  ~  <<  >>     |=  &=  ^=  <<=  >>=

# String / generic composition
+            # String concatenation (auto-converts numerics)
as           # casts (see §5)
else         # optional absence fallback
```

Chaining/associativity details for the optional `else` and `+`-concatenation are
documented in [§3.2](PROGRAMMERS_GUIDE.md#32-primitives-and-literals) and
[§3.17](PROGRAMMERS_GUIDE.md#317-strings).

## 5. Casts: `as`

```vyb
s<Int8> = -6;      p<Int> = s as Int       # signed widen -> sign-extends
u<UInt8> = 200;    q<Int> = u as Int       # unsigned widen -> zero-extends
w<Int> = 300;      r<Int8> = w as Int8     # narrowing -> truncates (300 & 0xFF)
```

Widening sign/zero-extends; narrowing truncates; the compiler won't silently
narrow a computed expression — cast explicitly. See [§3.12](PROGRAMMERS_GUIDE.md#312-casts-with-as).

## 6. Control flow

```vyb
if (cond) { … } else { … }
while (cond) { … break … continue … }
for (x in vec) { … }                 # any .next() iterable
for (i in 0..10) { … }               # inclusive range
for (x in v.iter(), 2) { … }         # step stride

# statement-first dispatch (arms can return/break out of the fn/loop)
match (code) { 200 -> { return "ok" }, ? -> { return "?" } }

# expression-first, yields a value (pass carries arm value)
select(n) -> { 1 -> 10, ? -> { v<Int> = n * 100; pass v } }

# comparison, set, range and destructuring patterns; compiler rejects
# unreachable/overlapping arms and a '?' that isn't last
select(income) -> { < 10000 -> 0.0, {1,3,5} -> "odd", ? -> 0.0 }
```

See [§3.6](PROGRAMMERS_GUIDE.md#36-control-flow) (loops) and
[§3.7](PROGRAMMERS_GUIDE.md#37-pattern-matching-match-and-select)
(match vs select, patterns, guards, `pass`).

## 7. Error handling: `fail` / `trap` / `ensure` / `refail`

Fallible calls return `(value, error)` lowered to `{ ret, error_ptr }`; `T?`
absence is the failed-call convention for the stdlib.

```vyb
div(a<Int>, b<Int>)<Int> -> { if (b == 0) { fail "division by zero" } return a / b }
r<Int> = trap div(10, 0) { 42 }      # handler on error
do_work() ensure -> { cleanup() }    # always runs, success or failure
trap f() { … }                       # single type, match arms, wildcard, or union
refail                              # re-raise the caught error untouched
```

Untrapped errors propagate up the call chain. See
[§3.16](PROGRAMMERS_GUIDE.md#316-error-handling-trap--fail--ensure).

## 8. Generics, aspects, binds

```vyb
fn first<T>(v<Vec<T>>)<T> -> { return v.get(0) }      # monomorphized per call
cmp_lt<T<Comparable>>(a<T>, b<T>)<Bool> -> { … }      # aspect-bound param

aspect Drawable { draw(self)<String> -> { } area(self)<Float> -> { } }
struct Circle { r<Float> }
bind Drawable -> Circle { area(self)<Float> -> { return 3.14159 * self.r * self.r } }
```

`<T<Aspect>>` is the modern bound spelling (replaces `T: Aspect`). An aspect can
refine another (`aspect Comparable : Equatable`); generic binds map onto every
instantiation (`Iterator -> VecIter<T>`). Bound to primitives: `Display`,
`Debug`, `Clone`, `Equatable`, `Comparable`, `Hashable`, `StringOps`,
`Iterator`. See [§3.14](PROGRAMMERS_GUIDE.md#314-generics-and-monomorphization)
and [§3.15](PROGRAMMERS_GUIDE.md#315-aspects-and-binds-polymorphism).

## 9. Strings

Immutable fat-pointer, bounds-checked; core family: `len`, `substring`,
`get(i)`, `char_at(i)`, `starts_with`/`ends_with`/`contains`, `index_of`,
`to_upper`/`to_lower`, `trim`/`split`, `format` (`"{}"`), `to_int`/`to_float`,
`String::from_bytes` for FFI. Escapes: `\n \r \t \\ \" \' \0 \xHH` (unknown
escapes kept verbatim). See [§3.17](PROGRAMMERS_GUIDE.md#317-strings).

## 10. Canonical decisions (pinned 2026-09-10)

Cross-checked against the current compiler; see
[`doc/Canonical_Reference_Syntax.md`](../../doc/Canonical_Reference_Syntax.md):

- **Struct construction — both forms valid; named is canonical.**
  `Point { x = 1, y = 2 }` (named) and `Point(1, 2)` (positional) are
  equivalent; prefer named.
- **String indexing — `s[i]` is canonical** (yields `Char`); `.char_at(i)`
  is non-canonical.
- **`for (item in vec)` copies by value** — no write-back. To mutate during a
  loop, index: `for (i in 0..v.len-1) { v[i] = … }`. (Mutable `for (ref … )`
  is a staged follow-on.)
- **Borrowing — `borrow(expr)` / `view(expr)` are canonical** function-call
  forms; the `borrow expr` / `view expr` prefixes are legacy.

---

*This quick reference is maintained alongside the authoritative docs. If a form
differs from the compiler or a deeper page, the [Programmer's Guide](PROGRAMMERS_GUIDE.md)
and the compiler (the suite) win.*
