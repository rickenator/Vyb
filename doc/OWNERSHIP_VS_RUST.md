# Vyb ownership vs Rust: parity, differences, and the tracked gaps

> **Status: current as of v0.7.7.** This is a comparison page under `doc/`
> (`doc/DOCS_POLICY.md`: design notes are historical), written to be *falsifiable*:
> every claim about Vyb below cites an executable path or a tracking row, and every
> claim about Rust is the ordinary language behaviour. The four gaps it records are
> roadmap items in `TODO.md` § 3 and rows in `doc/FEATURE_STATUS.md`, so their status
> cannot drift silently (`tools/docstatus.vyb` fails CI if a shipped item's row still
> reads 📋/🚧). For user-facing behaviour, the authoritative surface remains
> `docs/refman/PROGRAMMERS_GUIDE.md` cross-checked against `build/vyb`.

## Vyb's model today

Four ownership keywords, no lifetime annotations anywhere in the language:

| Keyword | Meaning | Copying |
|---|---|---|
| `my<T>` | Unique ownership | Move (use-after-move rejected) |
| `our<T>` | Shared, reference-counted ownership | Copy bumps a shared refcount |
| `their<T>` | Non-owning borrow | Non-owning; `borrow(x)` mutable, `view(x)` read-only |
| `mild<T>` | Weak reference into an `our<T>` control block | `soft()` / `grab()` / `released()` |

What ships (all exercised under `test/ownership/`):

- **`my<T>` — compile-time single-owner enforcement.** Use-after-move rejection,
  transfer on assignment / init / `my` parameter / move-capture, revive on
  reassignment, and read/copy out to a plain target without moving.
- **`our<T>` — atomic control-block refcounting.** Every binding holds its own strong
  ref: retained on shared copy, released on scope exit and on overwrite. The runtime
  keeps `refs` as an atomic `int64_t` with lock-free retain/release RMWs
  (`runtime/vyb_runtime.c`), and the legacy per-name refcounts in
  `cgen_ownership.cpp` (`incrementRefCount` / `decrementRefCount`) emit LLVM
  `AtomicRMW`.
- **`their<T>` — a lexical-phase borrow checker.** `borrow` / `view` require lvalues;
  overlapping mutable/view borrows are rejected; assigning while borrowed is
  rejected. Negative coverage is explicit:
  `borrow_escape_return`, `borrow_escape_scope`, `borrow_escape_struct_field`,
  `borrow_overlap_rejected`, `borrow_assignment_rejected`, `borrow_temporary_rejected`
  (under `test/ownership/`).
- **`mild<T>` — weak handles as language constructs.** `soft()` increments
  `weak_count`; a failed `grab()` yields `our<T>?`; `released()` reads the release
  flag. See `doc/OWNERSHIP_MILD.md`.
- **Runtime faults that Rust rejects at compile time** are already filed as defects,
  not papered over: #284 (`Vec<Vec<T>>` silent corruption / SIGSEGV / heap
  corruption) and #285 (re-borrowing an existing `their<T>` segfaults).

## Where Rust leads — four gaps, each now tracked

1. **Lifetime inference + NLL.** `borrow` / `view` are lexical-phase: there is no
   lifetime inference across signatures, and a borrow runs to the end of its scope
   rather than its last use, so programs Rust accepts are rejected here. This is
   documented behaviour (#149), not an accident. *Tracked:* the "Lifetime inference
   beyond lexical scope" row + the matching `TODO.md` § 3 item; extending it is a
   post-1.0 direction, with the lexical model as the 1.0 contract.
2. **Static exclusivity at zero cost.** Rust proves `&`-XOR-`&mut` at compile time and
   `Rc` / `Arc` are opt-in library types. In Vyb, shared ownership *is* the refcount:
   an `our<T>` binding carries an Rc-class cost per shared location, and cycle
   breaking is the manual `mild<T>` route. This is by design and stated here
   explicitly rather than left implied.
3. **Drop on every exit path.** Scope exit, overwrite and `return` are covered and
   verified. Destructor / leak behaviour along `fail`/`trap` propagation is
   **unverified** — and it matters because `fail` propagates through callees via the
   dual-return `{T, i8*}` ABI. *Tracked:* the row "Drop semantics on propagation
   paths (`fail`/`trap`)" and its `TODO.md` § 3 item; before this write-up the only
   mention was a trailing clause inside the ownership row, with no item of its own.
4. **Thread-boundary capability (`handoff` / `viewable`).** There is no `Send` / `Sync`
   trait *name*, but the distinction itself is now type-level rather than conventional:
   atomic refcounts have landed (`our<T>` control block, heap-`String` registry,
   `AtomicRMW` retain / release), and a closure handed to `thread_spawn`, `task_spawn`,
   `async_spawn` or `agent_start` is checked structurally — shared ownership qualifies
   iff its payload does, a named struct/enum iff every field / variant payload does, and
   unique owners, borrows and raw pointers never do; a mutable capture is refused
   whatever its type. A capture whose type still names a type parameter is judged at the
   resolved type once codegen has the substitutions, and a shape the derivation cannot
   see through (an FFI struct holding a `ptr<T>`) is admitted only by a reviewed
   `bind Handoff -> T`, which reports a contradiction as a warning rather than silently
   overriding. *Tracked:* the "Thread-boundary capability (`handoff` / `viewable`)"
   row (✅ since v0.7.7) and its `TODO.md` § 3 item; the one residue is diagnostic
   wording, which stays provisional until the drop-semantics-on-propagation row closes.

## Where Vyb leads (or differs deliberately)

- **`mild<T>` / `soft()` / `grab()` / `released()` are language constructs**; Rust's
  `Weak` is a library type with a different lifecycle API.
- **No lifetime annotations at all.** Ownership is keyword-readable (`my` / `our` /
  `their` / `mild`), and the borrow checker's lexical scope is the whole model.
- **Parameter passing is explicit and visible at the call site.** A plain `x<T>`
  parameter is a *value copy*; in-place access requires `borrow(x)` / `view(x)`, so
  "copy or reference" is a syntactic decision a reader can see, rather than an
  inferred one.
- **A mutable-capture closure cannot escape its defining function.** The environment
  holds the captured variable's *stack address*, so returning such a closure is
  rejected at compile time (`test/lambda/test_closure_mutable_return_rejected.vyb`).
  Rust's `FnMut` closures can be returned, so this is a real expressiveness loss —
  recorded as a deliberate 1.0 postponement (#359), not an oversight.

## How the comparison stays honest

The mechanism is the project's, not this page's:

- Every Vyb claim above corresponds to a row in `doc/FEATURE_STATUS.md` that cites an
  executable path, checked by `tools/docstatus.vyb` check 1 (cited paths must exist).
- Every remaining gap has both a row and a `TODO.md` item, and check 4 fails CI when a
  shipped item's row still reads 📋/🚧.
- The suite counts quoted anywhere in the docs are checked by
  `test/suite_count_check.vyb`.

So the next person comparing Vyb to Rust can argue from a tracked list instead of
discovering the gaps by review — which is the point of #358.
