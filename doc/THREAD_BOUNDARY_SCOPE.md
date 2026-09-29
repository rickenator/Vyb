# Scope — #365: the thread-boundary capability, landed

Decided with Rick on 2026-09-29: take **(b)** — allow a read-only borrow across a
thread boundary when the closure also captures the owner, so the payload stays
alive — and **(c)** — make the resolution explicit by running the capability check
where monomorphization has already happened.

Status: **step 0, (b) and (c) are landed, and the curated escape hatch is in**
(PR #368 for step 0 + (b); PR #369 for the first cut of (c); the deferral rework,
the registries and the curated binds follow on main). What remains is one refman
subsection and the wording question, which is settled by the drop-semantics row.

## Why the simple reading of `viewable` fails

A borrow (`their<T>` / `loc<T>`) addresses the **spawner's frame**. The borrow
model is lexical (`doc/OWNERSHIP_MILD.md`, #149): nothing proves the frame
outlives a detached thread's read. Permitting a read-only `view(x)` across a
spawn site flipped `test/ownership/thread_send_their.vyb` (the #149 pin) from
rejected to accepted — a reopened dangling read. Measured: `Ran 1209 / Passed
1203 / Failed 6` with that cut; reverted, back to `Passed 1204 / Failed 5` (the
5 CUDA VRAM-only reds). `viewable` is a *relation*, not a licence at a spawn site.

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

## (c) — decide where the evidence is, defer where it is not (LANDED)

The semantic pass runs before monomorphization and holds no substitutions
(verified: `include/vyb/semantic.hpp` carries no substitution map), so a capture
whose declared type still names a type parameter cannot be judged there. Codegen
holds the substitutions, so (c) is the resolution point.

The predicate moved to `vyb::thread_boundary`
(`include/vyb/vre/thread_boundary.hpp`, `src/vre/semantic_thread_boundary.cpp`),
with the struct/enum registries as **parameters**, and it now answers a tri-state:

```cpp
enum class Capability { Capable, NotCapable, Undecidable };
```

That tri-state is what makes the two passes agree instead of second-guessing each
other:

* **The semantic pass decides what it can.** A type naming a type parameter — or
  any name neither registry knows — is `Undecidable` there. The pass records the
  capture against the closure node (`deferredBoundaryCaptures_`) and reports
  nothing. Everything else, *including the rule-(b) admission*, is decided for
  good and codegen never revisits it.
* **Codegen judges exactly the deferred captures.** The semantic pass and codegen
  walk one shared AST, so the closure pointer matches (measured `saw=1` for the
  deferred, rule-(b) and plain cases alike). Codegen resolves each deferred capture
  with `currentTypeSubstitutions`, then asks the **semantic analyzer's own
  predicate** — `boundaryCapable_`, bound in the `LLVMCodegen` constructor next to
  `nodeTypeOf_` — and refuses a non-capable capture as a **hard** codegen error
  (`flagHardCodegenError()`; codegen's `logError` alone only prints, so the IR would
  otherwise still be linked and run).

Because the question goes back to the analyzer, the registries reach codegen too,
which closes the named-struct hole: `T` instantiating to `Holder` is judged by
`Holder`'s fields, so a field holding `my<Int>` is refused — fixture
`test/threads/test_thread_boundary_generic_named_struct_rejected.vyb`.

**Two heuristics were tried first and both were rejected by measurement**, which is
why the deferral list is the design:

* "judge every capture at codegen" refused
  `test/threads/test_thread_boundary_view_retained_owner_accepted.vyb` — the
  rule-(b) fixture merged in PR #368 — because codegen has no borrow-root data.
* "judge captures whose type string changed under substitution" missed the
  named-struct case entirely: the parameter's type was already resolved when the
  capture was recorded, so the string reads `Holder` unchanged (`subst=0`) and the
  program printed `0` and ran.

Other facts learned wiring it, both load-bearing:

* **Type inference unwraps ownership at the call boundary.** Passing a `my<Int>` or
  a `their<Int>` to a parameter declared `x<T>` instantiates `T = Int`, so a bare
  parameter capture can never be the (c) case. What can is a wrapper built around
  the parameter — `our<T>` in the accepted/rejected pair: the semantic pass sees
  `our<T>` with an unresolved payload, while codegen sees `our<my<Int>>`, a shared
  owner *of* a unique owner, and refuses.
* **Capture kind already reaches the semantic pass.** `ast::FunctionExpression`
  carries `mutableCapturedVariables`, filled by the semantic pass itself
  (`src/vre/semantic.cpp`, the capture analysis for `FunctionExpression`), so the
  mutable-address case is decided at semantic time and needs no deferral. The
  issue's "capture mode must reach semantic analysis" item is satisfied by that
  field — it is what the gate reads, not a re-derivation from writes.

Fixtures: `test/threads/test_thread_boundary_generic_capture_rejected.vyb` (the
generic function instantiated at `our<my<Int>>` — refused at codegen, and
deliberately NOT `@semantic-only`, since the verdict only exists after
monomorphization), `..._generic_capture_accepted.vyb` (the same function at
`our<Int>` — accepted, spawns a real thread, checks the joined value), and
`..._generic_named_struct_rejected.vyb` (the named-struct case above).

## Curated escape hatch — `bind Handoff -> T` (LANDED)

`stdlib/core/aspects.vyb` declares two marker aspects, re-exported by
`core::prelude` alongside `Display`/`Clone`/`Equatable`:

```
aspect Handoff  { handoff(self)<Bool>  -> { return true } }
aspect Viewable { viewable(self)<Bool> -> { return true } }
```

No bind is needed for ordinary types — both properties are derived structurally.
The bind exists for what the derivation cannot see through: an FFI struct holding a
`ptr<T>`, an opaque C handle, a `loc<T>` carrier.

```
share(all)
bind Handoff -> FfiHandle {
    handoff(self<FfiHandle>)<Bool> -> { return true }
}
```

`SemanticAnalyzer::handoffCapability` consults the bind **first**, so the explicit
claim wins over the computed verdict; `Handoff` implies `Viewable`, and a
`bind Viewable -> T` is honoured independently. Registration
(`registerTraitImpl`) computes the structural verdict for the bind's target type
and, when the curated claim contradicts it, reports a **warning** — a new
non-fatal diagnostic (`SemanticAnalyzer::addWarning`: printed to stderr, kept in
`warnings`, no effect on the exit code), because overriding is exactly the point.
The payload then carries the programmer's guarantee.

Fixtures: `test/threads/test_thread_boundary_curated_bind_accepted.vyb`
(`@semantic-only`, asserting the compile-time acceptance of a bind for a struct
holding a `my<Int>`) and `..._curated_bind_absent_rejected.vyb` (the same shape and
the same site with no bind — the structural verdict, refused). The pair isolates the
bind as the thing that changes the verdict.

## Step plan

1. ~~**(b) in the semantic pass**~~ — **done**: `mayCrossBoundaryWithRetainedOwner`
   with the five conditions above, plus the three fixtures.
2. ~~**step 0 — borrow addressing**~~ — **done**: `borrowTargetPointer`, three
   ownership fixtures.
3. ~~**(c) shared predicate**~~ — **done**: `vyb::thread_boundary` +
   `Capability`, the member functions thin wrappers.
4. ~~**(c) codegen call site**~~ — **done**: `checkSpawnHandoffWithSubstitutions`
   invoked from the `CallExpression` visitor for the four spawn intrinsics.
5. ~~**(c) deferral + registries**~~ — **done**: `deferredBoundaryCaptures_` on the
   analyzer, `deferredBoundaryCapturesFor(fe)` for codegen, `boundaryCapable_` bound
   from the analyzer in the `LLVMCodegen` constructor.
6. ~~**curated escape hatch**~~ — **done**: the two marker aspects, the
   bind-first lookup, the contradiction warning, two fixtures.
7. **Remaining** — the `docs/refman/PROGRAMMERS_GUIDE.md` thread-boundary
   subsection (from the issue body), and the wording question, which waits on the
   drop-semantics-on-propagation row: until a moved value is reliably reclaimed on
   its new thread, the docs say "accepted for handoff", never "guaranteed safe".

## Risks / notes

* Enabling (b) *widens* what compiles (fewer errors), so the risk is a missed
  unsoundness rather than fixture churn. Condition 4 is the guard that separates
  "keeps it alive" from "holds a copy" or "holds only a weak ref".
* `loc<X>` borrows (FFI memory) satisfy condition 1 but their root is not recorded
  by `recordTheirBorrowInto`, so in practice (b) applies to `their<X>`; `loc<X>`
  stays refused until it has a recorded owner. Stated in the rows rather than
  implied as covered.
* A curated bind is a *claim*, not a proof. The warning is the only check, and it
  is deliberately non-fatal — the escape hatch is for reviewed bindings, and the
  responsibility travels with the bind author.
* Codegen-time failures land later in the pipeline than semantic ones, so (c)
  reports with the same `siteName: …` shape and is fixture-tested both ways.
* Lesson recorded for the future: a diagnosis from a *threaded* probe concluded the
  closure capture lowering was at fault; a thread-free, closure-free probe found the
  real defect in `borrow()`/`view()`. Isolate the smallest failing construct before
  writing a scope around an inferred cause.
* Second lesson, same shape: both codegen-side heuristics for "which captures does
  codegen own" looked right and were wrong. Ask the pass that has the evidence to
  *record what it could not decide*, rather than inferring the boundary from type
  strings.
