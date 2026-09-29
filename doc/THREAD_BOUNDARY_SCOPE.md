# Scope — #365: `viewable` handoff under a retained owner (b), and explicit resolution at codegen (c)

Decided with Rick on 2026-09-29: take **(b)** — allow a read-only borrow across a
thread boundary when the closure also captures the owner, so the payload stays
alive — and **(c)** — make the resolution explicit by running the capability
check where monomorphization has already happened.

## Why the simple reading of `viewable` fails

A borrow (`their<T>` / `loc<T>`) addresses the **spawner's frame**. The borrow
model is lexical (`doc/OWNERSHIP_MILD.md`, #149): nothing proves the frame
outlives a detached thread's read. Permitting a read-only `view(x)` across a
spawn site flipped `test/ownership/thread_send_their.vyb` (the #149 pin) from
rejected to accepted — a reopened dangling read. Measured: `Ran 1209 / Passed
1203 / Failed 6` with that cut; reverted, back to `Passed 1204 / Failed 5` (the
5 CUDA VRAM-only reds).

## (b) — BLOCKED: the closure-capture lowering mis-reads a captured `their<T>`

The rule below was implemented, and the gate admitted the case — but the accepted
program is **wrong at runtime**, so the rule is scoped and *not landed*:

```
main:  owner.n=7
thread: ro.n=99157966951504     <- the address, not the field
thread: hold.n=7                <- the retained owner reads correctly
joined: 99157966951511
```

`hold.n` (a captured `our<Reading>`) is right, `ro.n` (a captured
`their<Reading>` borrow) yields the pointer value. So capturing a borrow into a
closure environment and reading through it does not dereference — and that, not
just the frame-lifetime argument, is why the spawn-site rule refuses every
borrow today. Landing (b) therefore has a **step 0**: fix the closure-capture
lowering for `their<T>` so a field read through a captured borrow derefs.

Evidence for that work:

* closure prologue: `src/vre/llvm/cgen_expr.cpp` (the closure prologue, lines 9700-9727) — per capture it loads
  the env field and stores it into `closure.cap.<name>`, then maps
  `valueTypeMap[capAlloca]` to the captured variable's AST type so field access
  resolves on ownership-wrapped captures (the comment at `:9718` names
  `their/my/our` explicitly);
* the member-access path that consumes that map is where the deref for a
  `their<T>` slot has to happen (a captured borrow slot holds a pointer, while a
  captured value slot holds the value or an owner pointer);
* current state is pinned by
  `test/threads/test_thread_boundary_view_retained_owner_rejected.vyb`, which
  flips to *accepted* once step 0 lands.

## (b) — the rule to implement (once step 0 is in)

At a thread-crossing site (`thread_spawn`, `task_spawn`, `async_spawn`,
`agent_start`), accept a capture `b` that is *not* handoff-capable iff **all**
of these hold:

1. `b`'s type is a borrow — `their<X>` or `loc<X>` — i.e. exactly the case
   `viewable` covers and `handoffCapable` refuses.
2. `b` is **not written** by the closure. The analyzer already decides this:
   `LambdaCaptureCtx.written` feeds `mutableCapturedVariables`, and writes
   through a field now count too (`v.field = ...` marks `v`, via
   `borrowedRootName`).
3. The closure **also captures the owner** `b` borrows from. The borrow root is
   already recorded: `recordTheirBorrowInto(varName, root)` fills
   `theirVarRoot_` (`include/vyb/semantic.hpp:698`,
   `src/vre/semantic_ownership.cpp:91`), so `theirVarRoot_[b]` names the owner.
4. That owner's declared type is ownership-carrying — `our<X>`, `mild<X>` or
   `my<X>`. This is what makes it safe: capturing an `our<X>` retains a
   reference in the closure env, and capturing a `my<X>` moves the heap object
   into it, so the closure itself keeps the payload alive for its whole run. A
   plain by-value owner does **not** qualify: the closure gets a *copy*, which
   does not keep the original variable's object alive.
5. The payload `X` is itself handoff-capable (no `my<T>`/borrow/raw-pointer
   inside), so concurrent reads of the payload are race-free by construction.

Anything else keeps the current verdict ("it cannot cross a thread boundary").

Diagnostic wording stays provisional (the drop-semantics row is still open), and
the message for the accepted case is silence — the gate only speaks to refuse.

## (c) — explicit resolution where the substitutions exist

The semantic pass has **no** monomorphization data: `include/vyb/semantic.hpp`
carries no substitution map (checked: no `substitutions`, `typeParamNames`,
`concreteTypeArgs`, `genericBindings` members), so a capture typed `T` inside a
generic function cannot be resolved and the gate stays permissive. That
permissive default is the current fallback — verified, not assumed.

Monomorphization lives in codegen: `currentTypeSubstitutions` is already used to
resolve bare type parameters at `src/vre/llvm/cgen_decl.cpp:361-366` and
`src/vre/llvm/cgen_expr.cpp:1237-1244`. The explicit fix is to run the
capability check **there**, at the spawn call sites, against the substituted
concrete types.

Open design question to settle before implementing (c): codegen has no
struct-field/enum-payload registry of its own (`structFieldTypes` /
`enumVariantPayloadTypes` exist only on `SemanticAnalyzer`), so the check at
codegen needs one of:

* a: give codegen a structural view of the type graph (share the semantic
  registries, or thread them through);
* b: run the predicate in a small helper that takes the registries as parameters,
  so both passes use one implementation;
* c: re-run the semantic capability query post-monomorphization through an
  interface the driver owns.

Preference: (b) — one predicate, two callers, no duplicated structural logic.

## Step plan (checkpointed, suite green at every step)

1. **(b) in the semantic pass** — extend `checkThreadBoundaryCaptures` with the
   five conditions above. Fixtures:
   * accepted: closure captures `our<Reading>` and a read-only `view` of it;
   * rejected: read-only `view` with the owner **not** captured (the #149 case);
   * rejected: a `view` the closure writes through.
   Build, run the suite; only the known CUDA reds may fail.
2. **(c) shared predicate** — lift `handoffCapable` into a form that takes the
   two registries as parameters; keep the member function as a thin wrapper.
   Re-run the suite (pure refactor, no behaviour change).
3. **(c) codegen call site** — at the spawn intrinsics in
   `src/vre/llvm/cgen_expr.cpp`, resolve each capture's type through
   `currentTypeSubstitutions`, run the shared predicate, and report a codegen
   error with the same wording. Fixture: a generic function that spawns with a
   capture typed `T`, instantiated at a non-handoff-capable type — must be
   rejected; instantiated at a handoff-capable type — must compile.
4. **Docs** — `TODO.md` + `doc/FEATURE_STATUS.md` rows; `docs/refman/PROGRAMMERS_GUIDE.md`
   §5 thread-boundary subsection (still outstanding from the issue body); suite
   count bump.
5. **Issue hygiene** — post the landed scope on #365; close only when (b) and (c)
   are both merged *and* the wording question is settled by the drop-semantics row.

## Risks / notes

* Enabling (b) *widens* what compiles (fewer errors), so the risk is a missed
  unsoundness rather than fixture churn. The five conditions are the guard; the
  key one is 4 (ownership-carrying owner), which is what distinguishes "keeps it
  alive" from "holds a copy".
* `loc<X>` borrows (FFI memory) satisfy 1 but their root is not recorded by
  `recordTheirBorrowInto` (that covers `borrow`/`view` of Vyb variables), so in
  practice (b) applies to `their<X>`; `loc<X>` stays refused until it has a
  recorded owner. State this in the row rather than implying coverage.
* Codegen-time failures are later in the pipeline than semantic ones, so (c)
  should report with the same `siteName: …` shape and be fixture-tested both
  ways (reject and accept).
