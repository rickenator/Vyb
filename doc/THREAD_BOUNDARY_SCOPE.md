# Scope — #365: `viewable` handoff under a retained owner (b), and explicit resolution at codegen (c)

Decided with Rick on 2026-09-29: take **(b)** — allow a read-only borrow across a
thread boundary when the closure also captures the owner, so the payload stays
alive — and **(c)** — make the resolution explicit by running the capability
check where monomorphization has already happened.

Status: **step 0 and (b) are landed** (branch `feat/365-viewable-and-write-detection`,
PR #368). (c) is not started; the design choice below is settled.

## Why the simple reading of `viewable` fails

A borrow (`their<T>` / `loc<T>`) addresses the **spawner's frame**. The borrow
model is lexical (`doc/OWNERSHIP_MILD.md`, #149): nothing proves the frame
outlives a detached thread's read. Permitting a read-only `view(x)` across a
spawn site flipped `test/ownership/thread_send_their.vyb` (the #149 pin) from
rejected to accepted — a reopened dangling read. Measured: `Ran 1209 / Passed
1203 / Failed 6` with that cut; reverted, back to `Passed 1204 / Failed 5` (the
5 CUDA VRAM-only reds).

## Step 0 — a borrow addressed the variable slot, not the object (LANDED)

The first probe of rule (b) printed a borrow field read as a pointer:

```
main:  owner.n=7
thread: ro.n=99157966951504     <- the address, not the field
thread: hold.n=7                <- the retained owner reads correctly
```

The first diagnosis — "the closure-capture lowering mis-reads a captured
`their<T>`" — was **wrong**. The lambda IR was already correct (`load %closure.cap.b`,
then `getelementptr %Reading, …, 0, 0`, then `load i64`). Isolating the same read
with no threads and no closures found the real fault:

```
owner.n  = 7          stack owner : 3
borrow.n = 110959278044368        borrow(stack) : 3
```

`borrow(x)` / `view(x)` returned the address of the operand's **variable slot**,
not the object. The IR showed it outright: `hb<their<Reading>> = borrow(mut_h)`
emitted `store ptr %mut_h, ptr %hb` — the slot address. Correct for a plain stack
struct (the object *is* the slot), wrong for every pointer-backed owner, where the
slot holds the pointer (`my`/`their`/`ptr`) or a control-block pointer whose
payload pointer is field 3 (`our`/`mild`). Member access already unwraps those
correctly; the borrow path did not.

Fixed by `LLVMCodegen::borrowTargetPointer` (`src/vre/llvm/cgen_expr.cpp`), used by
both the `BorrowExpression` visitor (the live path) and the call-form
`borrow`/`view` handler. After the fix:

```
my owner   : 11        stack owner : 3        owner.n = 7
borrow(my) : 11        borrow(stack): 3       b.n     = 7
our owner  : 7
borrow(our): 7
```

Pinned by `test/ownership/borrow_pointer_owner_field_read.vyb`,
`test/ownership/view_pointer_owner_field_read.vyb` and
`test/ownership/borrow_pointer_owner_write.vyb` (a write through the borrow must
mutate the owner's object, not clobber the owner's pointer slot).

## Rule (b) as landed

At a thread-crossing site (`thread_spawn`, `task_spawn`, `async_spawn`,
`agent_start`), accept a capture `b` that is *not* handoff-capable iff **all** of
these hold (`SemanticAnalyzer::mayCrossBoundaryWithRetainedOwner`):

1. `b`'s type is a borrow — `their<X>` / `view<X>` / `borrow<X>` — i.e. exactly the
   case `viewable` covers and `handoffCapable` refuses.
2. `b` is **not written** by the closure (a write makes it a mutable capture, which
   is reported separately; writes through a field count too).
3. The closure **also captures the owner** `b` borrows from. The borrow root is
   recorded by `recordTheirBorrowInto` in `theirVarRoot_`
   (`src/vre/semantic_ownership.cpp`), filled at the `borrow`/`view` binding.
4. That owner is held through **strong** ownership — `our<X>` or `my<X>`. Capturing
   an `our<X>` retains a reference in the closure environment and capturing a
   `my<X>` moves the heap object into it, so the closure itself keeps the payload
   alive for its whole run. A plain by-value owner hands the closure a *copy* and
   does not keep the original alive; **`mild<X>` is weak**, so it does not qualify
   either (this is tighter than the first write-up of the rule, which listed
   `mild` — a weak-only capture can outlive the last strong reference).
5. The payload `X` is itself handoff-capable (no `my<T>`/borrow/raw-pointer inside),
   so concurrent reads of the payload are race-free by construction.

Anything else keeps the verdict ("it cannot cross a thread boundary"). The gate only
speaks to refuse, so the accepted case is silent.

Fixtures: `test/threads/test_thread_boundary_view_retained_owner_accepted.vyb`
(spawns a real thread, reads through both the borrow and the retained owner, and
checks the joined value — 14), `..._view_owner_not_captured_rejected.vyb` (condition
3) and `..._view_written_rejected.vyb` (condition 2).

## (c) — explicit resolution where the substitutions exist

The semantic pass has **no** monomorphization data: `include/vyb/semantic.hpp`
carries no substitution map (checked: no `substitutions`, `typeParamNames`,
`concreteTypeArgs`, `genericBindings` members), so a capture typed `T` inside a
generic function cannot be resolved and the gate stays permissive. That permissive
default is the current fallback — verified, not assumed.

Monomorphization lives in codegen: `currentTypeSubstitutions` is already used to
resolve bare type parameters at `src/vre/llvm/cgen_decl.cpp:361-366` and
`src/vre/llvm/cgen_expr.cpp:1237-1244`. The explicit fix is to run the capability
check **there**, at the spawn call sites, against the substituted concrete types.

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

## Step plan

1. ~~**(b) in the semantic pass**~~ — **done**: `mayCrossBoundaryWithRetainedOwner`
   with the five conditions above, plus the three fixtures.
2. ~~**step 0 — borrow addressing**~~ — **done**: `borrowTargetPointer`, three
   ownership fixtures. Suite: `Ran 1214 / Passed 1214 / Failed 0` (the CUDA reds
   pass with the GPU card free).
3. **(c) shared predicate** — lift `handoffCapable` into a form that takes the two
   registries as parameters; keep the member function as a thin wrapper. Pure
   refactor, no behaviour change.
4. **(c) codegen call site** — at the spawn intrinsics in `src/vre/llvm/cgen_expr.cpp`,
   resolve each capture's type through `currentTypeSubstitutions`, run the shared
   predicate, and report a codegen error with the same wording. Fixture: a generic
   function that spawns with a capture typed `T`, instantiated at a
   non-handoff-capable type — must be rejected; instantiated at a handoff-capable
   type — must compile.
5. **Docs** — `docs/refman/PROGRAMMERS_GUIDE.md` §5 thread-boundary subsection (still
   outstanding from the issue body); suite count bump.
6. **Issue hygiene** — post the landed scope on #365; close only when (c) is merged
   *and* the wording question is settled by the drop-semantics row.

## Risks / notes

* Enabling (b) *widens* what compiles (fewer errors), so the risk is a missed
  unsoundness rather than fixture churn. Condition 4 is the guard that separates
  "keeps it alive" from "holds a copy" or "holds only a weak ref".
* `loc<X>` borrows (FFI memory) satisfy condition 1 but their root is not recorded
  by `recordTheirBorrowInto`, so in practice (b) applies to `their<X>`; `loc<X>`
  stays refused until it has a recorded owner. Stated in the rows rather than
  implied as covered.
* Codegen-time failures land later in the pipeline than semantic ones, so (c) should
  report with the same `siteName: …` shape and be fixture-tested both ways.
* Lesson recorded for the future: a diagnosis from a *threaded* probe concluded the
  closure capture lowering was at fault; a thread-free, closure-free probe found the
  real defect in `borrow()`/`view()`. Isolate the smallest failing construct before
  writing a scope around an inferred cause.
