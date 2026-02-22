# Vyn Language — Road to 1.0

> **What is Vyn?** A statically typed, systems programming language with an LLVM backend,
> ownership semantics expressed as readable keywords (`my`, `our`, `their`, `mild`),
> programmer-first memory control via `freedom` blocks, a struct + aspect model for
> polymorphism, and a uniquely clean name-first function syntax—no `fn` keyword noise.
> Vyn is not Rust. It is not C++. It is its own thing.

---

## Overall Completion Estimate

| Domain | Done | Remaining |
|--------|------|-----------|
| Core parsing & lexer | ~95% | Minor edge cases |
| LLVM backend (JIT + AOT + native) | ~85% | LTO, advanced passes |
| Control flow | ~92% | Pattern guards, exhaustiveness, labeled break |
| Type system (primitives + generics) | ~75% | Higher-kinded types |
| Struct system | ~85% | repr(C) for FFI |
| Ownership types (syntax + parsing) | ~80% | Semantic enforcement |
| Ownership types (runtime enforcement) | ~30% | Full lifecycle checking |
| `mild<T>` weak references | ~20% | Control block runtime |
| Aspect/bind system | ~65% | Associated types, dyn dispatch |
| Generic monomorphization | ~70% | Bounds-checked instantiation |
| Async/await | ~80% | Real scheduler/executor |
| Error propagation (`fail`/`trap`) | ~40% | Phases 2-5 |
| Lambda/closure codegen | ~50% | Full closure struct, captured-var codegen |
| Module system (`import`/`smuggle`/`bundle`) | ~20% | Import/smuggle parsing done; resolution, bundle, share pending |
| FFI (`extern "C"`) | ~15% | extern modifier + ExternStatement codegen work; block syntax and C-type mapping pending |
| Standard library | ~45% | Vec, String, I/O, Math done; HashMap, File I/O needed |
| Introspection (`typeof`/`typename`) | ~75% | Downcasting, type assertions |
| Auto-serialization | ~80% | Edge cases remain |
| Pattern matching | ~60% | Destructuring, guards, enum variants |
| Package manager / `vyn.toml` | ~0% | Not started |
| Language server (LSP) | ~0% | Not started |
| REPL | ~0% | Not started |
| Self-hosting compiler | ~0% | Long-term goal |

**Overall: approximately 60-65% complete toward a production 1.0 release.**

### Recently Completed
- [x] **`defer` statement** — LIFO scope-exit deferred execution
- [x] **Math library** — `abs`, `min`, `max`, `sqrt`, `sin`, `cos`, `tan`, `exp`, `log`, `log2`, `log10`, `pow`, `floor`, `ceil`, `round`
- [x] **I/O intrinsics** — `print()` (no newline), `println_int()`, `print_int()`, `println_bool()`, `print_bool()`
- [x] **String methods** — `.len()`, `.contains()`, `.starts_with()`, `.ends_with()`, `.to_upper()`, `.to_lower()`, `.substring()`, `.char_at()`, `String::from_bytes()`
- [x] **Type inference from initializer** — Variables without annotation infer type from RHS
- [x] **Vec `for` loop type inference** — Compiler-generated loop variables no longer require explicit types
- [x] **`Vec::contains()`** — Fixed: was returning hardcoded `false`; now emits correct LLVM loop with element comparison
- [x] **Lambda indirect calls** — Lambdas stored in local variables can now be invoked; `localLambdaTypes` map tracks inferred function types for correct indirect call codegen
- [x] **`println()`/`print()` with multiple arguments** — Space-separated output; all args formatted into a single call
- [x] **Semantic type recognition** — `Int16`, `Int32`, `Int64`, `UInt8`–`UInt64`, `Float32`, `Float64`, `Char`, `Rune` now fully recognized in semantic analysis (were silently rejected)
- [x] **Relaxed struct field syntax** — C-style `Type fieldName` accepted alongside canonical `fieldName<Type>`; helps parse legacy/interop fixtures
- [x] **Test harness** — `--parse-only` flag forwarded to binary for `@parse-only: true` tests; `n/a` annotation values treated as "skip this check"; test count now **657 tests, 315 passing (47.9%)**
- [x] **Vec parameter deep copy** — Vec parameters receive an independent copy of the data on function entry, eliminating double-free bugs (e.g. recursive quicksort base-case return)
- [x] **Vec mutation through borrowed struct fields** — `s.items.push(val)` where `s<their<T>>` now correctly mutates in-place; member-expression Vec calls now get a field *pointer* (not a loaded copy)
- [x] **Semantic use-after-free fix** — `handleVecMethodCallOnMember` no longer stores raw pointers from temporary `VecType` objects into `expressionTypes`; all return types are cloned into `node->type` first
- [x] **`test/new_features` 100% pass** — All new-features tests pass (quicksort, stack, insertion sort, etc.)

---

## What Is Done Today

### Core Language
- [x] **Lexer/tokenizer** — Complete with all documented token types; `freedom` keyword works
- [x] **Name-first function syntax** — `name(params)<ReturnType> -> body`, no `fn` noise
- [x] **Multi-value returns** — `main()<Int,String> -> return 42, "hello"`
- [x] **Variable declarations** — Unified `name<Type> = value` with type inference
- [x] **Struct declarations** — `struct Point { x<Int>, y<Int> }` with generic params `struct Box<T>`
- [x] **Struct construction** — `Point{ x: 1, y: 2 }`
- [x] **All primitive types** — `Int`, `Int8/16/32/64`, `UInt8/16/32/64`, `Float32/64`, `Bool`, `Char`, `Rune`, `String`
- [x] **Type inference** — Local variable types inferred from initializer
- [x] **`const` bindings** — Immutable bindings via `const`

### Control Flow
- [x] **`if`/`else`** — With expressions and blocks
- [x] **`while` loops** — Standard while loops
- [x] **C-style `for` loops** — `for (i = 0; i < n; i = i + 1)`
- [x] **Range-based `for` loops** — `for (i in 0..10)` (inclusive)
- [x] **`for (item in vec)` iteration** — Vec<T> iteration with break/continue
- [x] **`break`/`continue`** — In all loop constructs
- [x] **`match` statements** — `match (expr) { pattern -> result, ? -> default }`
  - `->` arrow (consistent with function syntax, not `=>` which is Rust)
  - `?` wildcard (not `_` — more visible and intentional)
  - Comparison patterns (`>= 90`, `< 0`, etc.)
  - Literal patterns (int, float, string)
- [x] **`select` expressions** — Pattern matching that yields a value, with `pass` for
  explicit multi-statement returns (Vyn-original concept, no equivalent in other languages)

### Type System
- [x] **Generic types** — `<T>`, `<K, V>` with proper scoping and substitution
- [x] **Generic structs** — `struct Box<T> { value<T> }`
- [x] **Ownership type syntax** — `my<T>`, `our<T>`, `their<T>`, `mild<T>` parse correctly
- [x] **`Vec<T>`** — Dynamic array with `new()`, `push()`, `pop()`, `len()`, `get()`
- [x] **Fixed arrays** — `[T; N]` with indexing
- [x] **Tuples** — `Tuple<T,U,V>` and `(T,U,V)` syntax
- [x] **`Future<T>`** — Async return types with type checking

### Expressions & Operations
- [x] **Binary operations** — `+`, `-`, `*`, `/`, `==`, `!=`, `<`, `<=`, `>`, `>=`, `&&`, `||`
- [x] **Unary operations** — `!`, `-`
- [x] **Member access** — `obj.field`, `arr[index]`
- [x] **String concatenation** — `str1 + str2` with mixed-type auto-`toString`
- [x] **`toString` intrinsics** — All primitive types
- [x] **`typeof(expr)`** — Runtime type hash (8-byte i64) — uniquely Vyn introspection
- [x] **`typename(expr)`** — Type name as `String`

### Memory & Ownership
- [x] **`freedom` blocks** — `freedom { ... }` for programmer-controlled sections (not `unsafe`)
- [x] **`loc<T>` raw pointers** — Scoped to `freedom` blocks only
- [x] **`view(expr)`/`borrow(expr)`** — Parsed; semantic validation in progress
- [x] **`soft(expr)`** — Creates `mild<T>` from `our<T>` (parsed)

### Aspect/Bind System (Vyn's Polymorphism)
- [x] **`aspect` declarations** — Method signatures, optional default implementations
- [x] **`bind Aspect -> Type { ... }`** — Unbounded bind for concrete types
- [x] **`bind<T> Aspect -> Type<T> { ... }`** — Generic unbounded bind
- [x] **`bind<T<Aspect1, Aspect2>> Aspect -> Type<T> { ... }`** — Bounded bind
- [x] **Aspect bounds in generic functions** — `fn<T<Display>>(item<T>)<Void> ->`
- [x] **Semantic validation** — Bounds checked against aspect registry
- [x] **Monomorphization Phase 5** — Generic function specialization by concrete type

### Async/Await
- [x] **`async` functions** — `async name()<Future<T>> -> { ... }`
- [x] **`await` expressions** — `value<T> = await future_val`
- [x] **`Future<T>` type system** — Proper type checking throughout
- [x] **State machine codegen** — Async functions lowered to LLVM state machines
- [x] **DWARF debug metadata** — Suspension points, continuation markers

### Error Handling
- [x] **`fail` statement** — `fail<ErrorType>(value)` raises typed errors
- [x] **`trap` handler Phase 1** — `{ ... } trap (e<ErrorType>) -> { ... }`
- [x] **Error type system** — Type-tagged error objects with type hashes
- [x] **Failable function detection** — Semantic analysis marks `canFail` functions

### LLVM Backend
- [x] **JIT execution** — LLVM ORC JIT (migrated from MCJIT in v0.4.0)
- [x] **AOT object file emission** — `--compile` / `-c` flag
- [x] **Native executable generation** — `--build` / `-b` flag (v0.4.4)
- [x] **Optimization levels** — `-O0` through `-O3`
- [x] **Cross-compilation** — 20+ target architectures via LLVM
- [x] **Static linking** — `--static` flag
- [x] **DWARF debug info** — Source-level debugging in compiled output

### Auto-Serialization
- [x] **`main()` return serialization** — Complex types auto-output as JSON
- [x] **`lit()`, `notype()`, `bare()`, `deserial()` intrinsics** — Serialization control
- [x] **JSON construction intrinsics** — `__vyn_serialize_to_json()`, struct metadata
- [x] **JSON deserialization** — `T::from_string(json)` round-trip (v0.4.4)

### Infrastructure
- [x] **CMake build system** — LLVM integration
- [x] **Test harness** — 657+ tests, parallel execution, HTML/JSON reports; `--parse-only` mode; 315 passing (47.9%)
- [x] **`println()`** — Works with all data types including Vec<T>

---

## In Progress

### Error Propagation — Phases 2-5 (HIGH PRIORITY)
- [x] Phase 1: Semantic detection of failable functions (`canFail`)
- [ ] Phase 2: Dual return value codegen `{ T, ptr }` for failable functions
- [ ] Phase 3: `fail` statement returns error to caller when no trap in scope
- [ ] Phase 4: Call site instrumentation — auto-check `{ value, error }` tuple
- [ ] Phase 5: Top-level untrapped error handler (`__vyn_runtime_untrapped_error`)

### Aspect System — Completion (HIGH PRIORITY)
- [x] Phases 1-4: Declarations, method calls, generic impls, type param substitution
- [ ] **Associated types** — `aspect Iterator { type Item }` (fundamental for stdlib)
- [ ] **Aspect objects / dynamic dispatch** — `dyn Aspect` for runtime polymorphism
- [ ] **Aspect inheritance** — `aspect Comparable : Equatable`
- [ ] **Monomorphization with bounds validation** — Reject instantiation where bounds fail
- [ ] **Bind selection precedence** — Bounded bind takes precedence over unbounded

### Pattern Matching — Completion (MEDIUM PRIORITY)
- [x] Literal patterns, wildcard `?`, comparison operators
- [ ] **Struct destructuring** — `Point { x, y } ->` in match arms
- [ ] **Enum/sum type variant patterns** — `Some(value) ->`, `None ->`
- [ ] **Range patterns** — `1..10 ->` in match arms
- [ ] **Guard clauses** — `pattern if condition ->`
- [ ] **Exhaustiveness checking** — Compiler rejects non-exhaustive match
- [ ] **`match` as expression** — Return a value from match directly

### Lambda / Closures (MEDIUM PRIORITY)
- [x] Parsing — `|x, y| -> x + y` and `|x<Int>| -> { ... }`
- [x] Semantic analysis — capture detection, type inference
- [x] **Indirect call codegen** — Lambda stored in a local variable can be called; `localLambdaTypes` map tracks inferred function types; body return value coerced to declared return type
- [ ] **Full LLVM closure struct codegen** — Generate closure structs with capture fields, function pointers
- [ ] **Move capture** — Transfer ownership of `my<T>` into closure
- [ ] **Mutable capture** — Captured variables in mutable context
- [ ] **`our<T>` capture** — Atomic ref-count increment for shared captures

---

## Planned — Needed for 1.0

### 1. Module System (HIGH PRIORITY)
Vyn's `import`/`smuggle`/`bundle`/`share` system is a unique approach to module visibility.
See `doc/bundles_and_sharing.md` and `doc/MODULE_FFI_BINARY_ROADMAP.md`.

- [x] **Phase 1.1 — Import Parsing** — `import <path>`, `import <path> as <alias>`, `import <path> from "<locator>"`, `smuggle <path> as <alias>`, `ImportKind` (TrustedImport / Smuggle) captured in AST
- [ ] **Phase 1.1 — Module Registry** — `ModuleRegistry` class; `loadModule(path)`, `resolveImport()` (currently no-ops at codegen)
  - Circular dependency detection
- [ ] **Phase 1.2 — `bundle(...)` Declaration Parsing**
  - `BundleDeclaration` AST node
  - `bundle(sort.Core, sort.Common)` at file top
- [ ] **Phase 1.3 — `share(...)` Directive Parsing**
  - `share(all)` and `share(bundle1, bundle2)` on declarations
  - Visibility metadata stored in AST
- [ ] **Phase 1.4 — Visibility Checking**
  - Bundle overlap algorithm during import resolution
  - `smuggle` keyword — bypasses all visibility checks
  - Clear error messages: "Symbol 'foo' not visible from your bundle"
- [ ] **Phase 1.5 — Module Path Resolution**
  - `VYN_MODULE_PATH` environment variable
  - `--module-path` CLI flag
  - Standard library auto-discovery
- [ ] **Phase 1.6 — Standard Library as Modules**
  - `stdlib/core/`, `stdlib/io/`, `stdlib/math/`, `stdlib/collections/`
  - Auto-import of `core::*` (opt-out with directive)

### 2. FFI — C Interop (HIGH PRIORITY)
- [x] **`extern` function modifier** — Individual extern function declarations compile to LLVM `ExternalLinkage` via `ExternStatement` codegen; syntax: `extern funcName(params)<ReturnType>`
- [ ] **`extern "C" { }` block syntax** — Multi-declaration block (AST node exists; parser not yet wired)
- [ ] **C type mapping** — `Int` to `int64_t`, `Int32` to `int32_t`, `*i8` to `char*`
- [ ] **`#[repr(C)]` on structs** — Force C-compatible memory layout
- [ ] **Variadic C functions** — `printf(format: *i8, ...) -> Int`
- [ ] **`vyn bindgen`** — Tool to generate Vyn bindings from C headers (v0.6+)

### 3. Ownership Types — Runtime Enforcement (HIGH PRIORITY)
- [ ] **`my<T>` move semantics** — Enforce single-owner at compile time; error on copy
- [ ] **`our<T>` reference counting** — Atomic strong_count; drop when 0
- [ ] **`their<T>` borrow checker** — Validate lifetime of borrows; no dangling refs
- [ ] **`mild<T>` control block** — weak_count + "released" flag; `grab()` and `released()`
- [ ] **`view(expr)` semantic** — Creates immutable `their<T const>`; enforced
- [ ] **`borrow(expr)` semantic** — Creates mutable `their<T>`; enforced
- [ ] **`soft(expr)` semantic** — Creates `mild<T>` from `our<T>`; enforced

### 4. Standard Library Expansion (HIGH PRIORITY)
- [ ] **`Option<T>`** — `Some(value)` / `None` for nullable values
- [ ] **`Result<T, E>`** — `Ok(value)` / `Err(error)` for fallible operations
- [ ] **Core aspects** — `Display`, `Debug`, `Clone`, `Equatable`, `Comparable`, `Hashable`
- [x] **String methods** — `.len()`, `.contains()`, `.starts_with()`, `.ends_with()`, `.to_upper()`, `.to_lower()`, `.substring()`, `.char_at()`, `String::from_bytes()`
- [ ] **String methods (remaining)** — `.trim()`, `.split()`, `.replace()`, `.format()`
- [ ] **String formatting** — Format strings or `fmt()` intrinsic
- [ ] **`HashMap<K, V>`** — Hash map with `Hashable + Equatable` bounds
- [ ] **`HashSet<T>`** — Hash set
- [ ] **`BTreeMap<K, V>`** — Ordered map with `Comparable` bounds
- [ ] **File I/O** — `File::open()`, `File::read()`, `File::write()`, `File::close()`
- [x] **Math library** — `sqrt`, `sin`, `cos`, `tan`, `exp`, `log`, `log2`, `log10`, `pow`, `floor`, `ceil`, `round`, `abs`, `min`, `max`
- [x] **I/O intrinsics** — `print()` (no newline), `println_int()`, `print_int()`, `println_bool()`, `print_bool()`
- [ ] **Iterator aspect** — `next(self)<Option<Item>>` protocol for `for` loop integration
- [ ] **`Vec<T>` expansion** — `.map()`, `.filter()`, `.reduce()`, `.find()`, `.sort()`; `.contains()` is now correctly implemented

### 5. Sum Types / Enums (MEDIUM PRIORITY)
Vyn needs a way to express sum types. Essential for `Option<T>`, `Result<T,E>`, and
expressive APIs. **Vyn-natural approach:** enums should integrate with the aspect system
and pattern matching, not be a separate OOP mechanism.

- [ ] **Enum declaration syntax** — `enum Direction { North, South, East, West }`
- [ ] **Enum variants with data** — `enum Shape { Circle(Float), Rect(Float, Float) }`
- [ ] **Pattern matching on enums** — In `match`, `select`, and destructuring
- [ ] **Enum methods via `bind`** — `bind Drawable -> Shape { ... }` (natural fit!)
- [ ] **`Option<T>` as built-in enum** — `Some(T)` / `None`
- [ ] **`Result<T, E>` as built-in enum** — `Ok(T)` / `Err(E)`

### 6. Introspection System — Completion (MEDIUM PRIORITY)
- [x] `typeof(expr)` — Returns type hash as i64
- [x] `typename(expr)` — Returns type name as String
- [ ] **`as` downcasting operator** — `value as TargetType` (Phase 2)
- [ ] **`typeof` in wildcard trap** — `trap (e<?>) -> { if typeof(e) == typeof<ParseError>() }`
- [ ] **Type registry at startup** — `__vyn_module_init()` registers all types
- [ ] **`Type` as first-class type** — `t<Type> = typeof(42)`, equality comparison

### 7. Select Expressions — Polish (MEDIUM PRIORITY)
The `select` expression is a uniquely Vyn concept: pattern matching that produces a value,
with `pass` for multi-statement case bodies. Needs polishing:

- [ ] **`select` exhaustiveness** — Warn when no `?` wildcard and possible no-match
- [ ] **Nested `select`** — `select` inside a `select` arm
- [ ] **`select` with enum variants** — Full destructuring in arms
- [ ] **`select` as statement** — Allow `select` without a binding target (side-effects only)

### 8. Wildcard Trap Handler (MEDIUM PRIORITY)
- [ ] **`trap (e<?>)` syntax** — Catch any error type
- [ ] **`typeof(e)` in wildcard handler** — Runtime type discrimination
- [ ] **Multi-type trap** — `trap (e<ParseError | IOError>) -> { ... }` (Vyn-native syntax)

### 9. Advanced Control Flow (LOWER PRIORITY)
- [x] **`defer` statement** — `defer cleanup()` runs on scope exit (LIFO order, function-level)
- [ ] **`ensure` statement** — `ensure condition else fail<Error>(...)` (post-condition)
- [ ] **Labeled `break`/`continue`** — Break from outer loops by label

### 10. Async System — Completion (LOWER PRIORITY)
- [ ] **Real event loop / executor** — Single-threaded and multi-threaded runtimes
- [ ] **`spawn` for concurrent tasks** — `task<Future<T>> = spawn compute()`
- [ ] **Typed channels** — `chan<T>` for message passing between tasks (planned)
- [ ] **Actors** — Lightweight isolated concurrency units (planned)
- [ ] **`select` over channels** — Wait on multiple channels (Vyn-natural extension)
- [ ] **Async lambdas** — `async |x| -> await process(x)`
- [ ] **`async for`** — Iterate over async streams

---

## Developer Experience — Needed for 1.0

### Package Manager
- [ ] **`vyn.toml`** — Project manifest with `[package]`, `[dependencies]`, `[[bin]]`
- [ ] **`vyn build`** — Build multi-file projects from manifest
- [ ] **Dependency resolution** — Version constraints and lock file (`vyn.lock`)
- [ ] **Package registry** — Central registry for published packages
- [ ] **`vyn new`** — Scaffold a new Vyn project

### Language Server Protocol (LSP)
- [ ] **Go-to-definition** — Jump to symbol definitions across files
- [ ] **Hover documentation** — Show type signatures and doc comments
- [ ] **Completion** — Aspect method names, struct fields, imports
- [ ] **Diagnostics** — Real-time error reporting in editors
- [ ] **`vyn lsp`** — Launch LSP server mode

### REPL
- [ ] **Interactive mode** — `vyn repl` launches a read-eval-print loop
- [ ] **JIT-backed** — Reuse existing ORC JIT infrastructure
- [ ] **History + multiline** — Standard readline-style editing
- [ ] **`:type` command** — Print the type of an expression

### Documentation Tools
- [ ] **Doc comments** — `/// comment` on declarations
- [ ] **`vyn doc`** — Generate HTML documentation from source
- [ ] **Online reference** — Language reference manual (derived from existing docs)

### Testing & Tooling
- [ ] **`vyn test`** — Run test files alongside source (`*.test.vyn`)
- [ ] **Code formatter** — `vyn fmt` for canonical formatting
- [ ] **Linter** — `vyn check` for warnings beyond errors
- [ ] **Debugger integration** — `gdb`/`lldb` with Vyn source stepping (DWARF done, validate end-to-end)

---

## Contradictions and Undecided Items

These items have conflicting designs or no clear decision. They MUST be resolved before 1.0.

### [UNDECIDED] Class System vs. Struct + Aspect

`doc/WHY_TRAITS_NOT_CLASSES.md` argues emphatically that Vyn should NOT have classes.
`doc/TRAIT_SYSTEM_DESIGN.md` Phase 5 plans an optional class system. The README uses
"aspects + structs instead of classes."

**Proposal (Vyn-native):** Keep the struct + aspect philosophy as the *primary* model.
Classes are an anti-pattern in Vyn's design. If inheritance-like patterns are truly needed,
they should be achieved via aspect composition + `bind`, not via a new `class` keyword.
*Recommendation: Remove the class system from the roadmap. Aspects + structs are sufficient
and more composable. Document this decision clearly so contributors do not re-propose it.*

### [UNDECIDED] `trait`/`impl` vs. `aspect`/`bind` Terminology

The codebase uses both: `doc/TRAIT_SYSTEM_DESIGN.md` uses `trait`/`impl`, while the
current implementation uses `aspect`/`bind`. The README favors `aspect`/`bind`.

**Proposal (Vyn-native):** Commit to `aspect`/`bind`. It is more expressive: an "aspect"
captures that it adds a dimension of behavior, while "bind" clearly expresses that you are
attaching that aspect to a type. `trait`/`impl` is Rust vocabulary. Vyn is not Rust.
`impl` may remain for backwards compatibility, but `bind` is the idiomatic Vyn path.
Remove `trait` keyword references from all docs that are not explicitly historical.

### [UNDECIDED] `fn` Keyword Backward Compatibility

The language dropped `fn` in favor of name-first function syntax. The README says "legacy
`fn<Type>` syntax remains supported." Keeping two syntaxes causes confusion.

**Proposal:** Deprecate `fn` syntax in v0.5, remove in v1.0. The name-first syntax
`name(params)<ReturnType> ->` is cleaner and uniquely Vyn. One syntax is better than two.

### [UNDECIDED] `try`/`catch`/`finally` vs. `fail`/`trap`

`doc/AST_Roadmap.md` mentions `TryStatement` and `ThrowStatement` in the AST, which
suggests C++/Java-style exception handling. But Vyn's system is `fail`/`trap`, which is
different in philosophy: zero-cost success path, typed errors, explicit propagation.

**Proposal (Vyn-native):** Remove `try`/`catch`/`finally` AST nodes entirely. They are
vestigial C++ vocabulary. `fail`/`trap` is Vyn's answer — more explicit, more controlled,
and allows error propagation without hidden costs. There is no `throw` in Vyn.

### [UNDECIDED] Generic Bound Syntax Inconsistency

Two syntaxes appear across docs:
- `<T: Aspect>` — Rust-style, used in some `doc/TRAIT_SYSTEM_DESIGN.md` examples
- `<T<Aspect>>` — Vyn-style, used in `aspect`/`bind` docs and the implementation

**Proposal:** Commit to `<T<Aspect>>` only. This is consistent with Vyn's unified
`name<Type>` syntax. `T: Aspect` is Rust syntax. Vyn is not Rust. Remove all `<T: Trait>`
examples from documentation.

### [UNDECIDED] Iterator Protocol: Aspect or Built-in?

`for (item in collection)` works for `Vec<T>` but there is no `Iterator` aspect that
user types can implement to make themselves iterable.

**Proposal (Vyn-native):** Define an `Iterator` aspect in the standard library using an
associated type `Item` (which itself requires associated types to be implemented first):
```vyn
aspect Iterator {
    type Item                                    # associated type — what the iterator yields
    next(self<their<Self>>)<Option<Self::Item>>  # returns next value or None
}
```
Types that bind `Iterator` become usable in `for` loops. The compiler desugars
`for (item in col)` to repeated `Iterator::next()` calls. This is elegant and composable.
Note: this depends on associated types being implemented in the aspect system first.

### [UNDECIDED] Channels / Actors — 1.0 or Later?

The README mentions actors and channels as planned. They have no design document.

**Proposal:** Defer to post-1.0. A solid 1.0 with `async`/`await` + structured concurrency
is more valuable than an under-designed actor model. Design channels and actors thoroughly
(write a doc first!) before implementing them.

---

## Vyn-Native Ideas Worth Exploring

These are not in any current design document but feel natural given Vyn's identity and
should be prototyped or at least documented before 1.0:

### `pipe` Operator (`|>`)
Functional pipelines without deep nesting:
```vyn
result<String> = data
    |> filter(|x| -> x > 0)
    |> map(|x| -> x * 2)
    |> to_string()
```
The `|>` pipe operator threads the left-hand value as the first argument to the right-hand
function. Fits Vyn's clean aesthetic; makes functional chains readable without a method
chain API requirement.

### `ensure` Contracts
Unlike bare `assert` (C-style), `ensure` integrates with the `fail`/`trap` system:
```vyn
divide(a<Int>, b<Int>)<Int> -> {
    ensure b != 0 else fail<DivisionError>(DivisionError { dividend: a })
    return a / b
}
```
More expressive than `if (condition) { fail ... }` and reads like a contract. Does not
clash with the error handling design — it IS the error handling design.

### `with` Scope Blocks (Resource Management)
A managed scope that calls cleanup when exiting — better than bare `defer` for resources:
```vyn
with file<File> = File::open("data.txt") {
    content<String> = file.read_all()
    println(content)
}  # file.drop() called automatically
```
Implemented by the compiler calling a `Drop` aspect method on scope exit. Does not need GC.
Aligns with `mild<T>` and `our<T>` lifecycle semantics.

### Named Arguments at Call Sites
Vyn's `name<Type>` syntax already names parameters. Callers should be allowed to use names:
```vyn
result<Int> = add(x: 10, y: 20)   # Named args, order-independent
```
Improves readability for functions with many parameters without requiring overloaded forms.

### `select` Over Error Results
Extend `select` to dispatch on `Result<T, E>`:
```vyn
result<Int> = select(risky_operation()) -> {
    ok(value)        -> value * 2,
    err(e<ParseError>) -> -1,
    err(e<IOError>)    -> -2,
    ?                  -> 0
};
```
Unifies error handling with pattern matching in a uniquely Vyn way. No try-catch pyramid.

---

## 1.0 Release Criteria

For Vyn to be considered production-ready at 1.0, **all of the following must be true**:

### Must-Have for 1.0
- [ ] Module system fully working (`import`, `smuggle`, `bundle`, `share`)
- [ ] Lambda/closure codegen complete
- [ ] Ownership types runtime-enforced (borrow checking, move semantics)
- [ ] `mild<T>` control block implemented with `grab()` and `released()`
- [ ] Error propagation (Phases 2-5) complete
- [ ] `Option<T>` and `Result<T, E>` in stdlib
- [ ] Core aspects (`Display`, `Debug`, `Clone`, `Equatable`, `Comparable`, `Hashable`)
- [ ] Iterator aspect with `for` loop desugaring
- [ ] Enum/sum types with pattern matching
- [ ] String methods complete
- [ ] `HashMap<K, V>` and basic collections
- [ ] FFI (`extern "C"`) working
- [ ] `vyn.toml` and `vyn build` project system
- [ ] Wildcard trap handler (`trap (e<?>)`) with `typeof` discrimination
- [ ] All open contradictions resolved (see section above)

### Should-Have for 1.0
- [ ] REPL (`vyn repl`)
- [ ] Language server (LSP) — at least basic completion and diagnostics
- [ ] `vyn fmt` code formatter
- [ ] `vyn doc` documentation generator
- [ ] Comprehensive language reference manual
- [ ] Test suite covering all 1.0 features
- [ ] `vyn test` integrated test runner
- [ ] Debugger integration validated end-to-end with `gdb`/`lldb`

### Post-1.0 Roadmap
- [ ] Channels + actors (design doc first!)
- [ ] Networking + Sockets (see section below)
- [ ] Self-hosting compiler (Vyn written in Vyn)
- [ ] Macros / metaprogramming
- [ ] Package registry
- [ ] `vyn bindgen` for C header automation
- [ ] Higher-kinded types
- [ ] Compile-time function evaluation (CTFE)
- [ ] `pipe` operator (`|>`)
- [ ] `with` scope blocks

---

## Networking and Sockets

Networking is a post-1.0 feature that **depends entirely on FFI being complete first**.
Raw sockets are POSIX system calls (`socket`, `connect`, `bind`, `recv`, `send`), so Vyn
networking is FFI + a thin standard-library wrapper — not a language feature per se.

### Design Approach (Vyn-native)

Once `extern "C"` FFI lands (v0.5), networking follows naturally:

```vyn
// stdlib/net/tcp.vyn — thin wrapper over POSIX sockets
extern "C" {
    socket(domain<Int>, type<Int>, protocol<Int>)<Int>
    connect(sockfd<Int>, addr<loc<SockAddr>>, addrlen<Int>)<Int>
    send(sockfd<Int>, buf<loc<Void>>, len<Int>, flags<Int>)<Int>
    recv(sockfd<Int>, buf<loc<Void>>, len<Int>, flags<Int>)<Int>
    close(fd<Int>)<Int>
}

struct TcpStream {
    fd<Int>
}

// Higher-level async API (requires real event loop in async runtime)
async tcp_connect(host<String>, port<Int>)<TcpStream> -> {
    // ... resolve address, call connect(), wrap in TcpStream
}
```

### Networking Roadmap

- [ ] **v0.5 — FFI foundation** (`extern "C"` blocks, C type mapping)
- [ ] **v0.5 — Raw socket FFI bindings** — `stdlib/net/raw.vyn` wrapping POSIX socket API
- [ ] **v0.6 — `TcpStream` / `UdpSocket`** — Safe Vyn wrappers with `fail`/`trap` error handling
- [ ] **v0.6 — `TcpListener`** — Server-side accept loop integrated with async runtime
- [ ] **v0.6 — Async I/O** — Non-blocking socket I/O using the async executor (requires real event loop)
- [ ] **v0.7 — HTTP/1.1 client** — Built on `TcpStream`, pure Vyn implementation
- [ ] **Post-1.0 — TLS** — Via `extern "C"` bindings to OpenSSL or mbedTLS
- [ ] **Post-1.0 — UDP multicast, raw packets** — Advanced socket options

### Key Design Decisions

**Q: Is networking a language feature or a library?**
Networking is *stdlib*, not a language feature. The only language feature required is
`extern "C"` FFI, which is planned for v0.5. Everything else is library code written in Vyn.

**Q: How do errors surface?**
Socket errors surface as Vyn `fail`/`trap`: failable functions return `{value, error}` pairs.
There is no exception for network I/O — the `fail`/`trap` system handles it uniformly.

**Q: What about async I/O?**
Async socket I/O requires a real event loop in the Vyn async runtime (currently a stub).
Non-blocking I/O (epoll/kqueue/IOCP) integration is planned for v0.6 alongside `TcpStream`.

## Consistency Work

1. Treat i32/i64 as internal only (surface types are Int/Float/Bool/String/UInt).
2. Standardize tests/examples to use auto-stringifying `println` and Java-like string concatenation.
3. Reconcile docs vs runtime regarding `println_int`/`println_bool` intrinsics.

### Action Items

- Fix or move non-Vyn *.vyn fixtures (e.g. extracted tests containing C++ snippets).
- Pick a single canonical test runner.

---

*Last Updated: February 2026*
*Current Version: Vyn v0.5.0 (freedom-1.0 series)*
*Overall Status: ~60-65% complete toward 1.0 — 657 tests, 315 passing (47.9%)*
