# TypeTable migration — precise plan (turnkey for the dedicated session)

Status: **increments #1 landed** (2026-09-10, commit `2944d69`). What remains is
the big, atomic flip. This document is the complete migration checklist/plan so
a fresh session can execute it top-to-bottom with the highest chance of landing
green. It was written from a full site survey on 2026-09-10; line numbers are
anchors that may drift a little since — re-grep before trusting any one of them.

## Goal

Replace the raw `Node* -> TypeNode*` mirror (`expressionTypes`) and the
`retainType`/`_ownedTypes` raw-ownership registry with an **owning TypeTable**
keyed by the stable `Node::typeId()` (landed in increment #1). After the
migration the AST's types are uniformly `std::shared_ptr<TypeNode>` owned by
either the node or the TypeTable, the raw-mirror/UAF class disappears, and the
AST can be made immutable in a later pass.

## Current state (all in `src/vre/semantic.cpp` / `include/vyb/semantic.hpp`)

- `expressionTypes`: `std::unordered_map<ast::Node*, ast::TypeNode*>` — 241
  `expressionTypes[` sites: 103 assignments via `retainType(...)`, ~133 other
  assignments (incl. ~40 `= nullptr` markers, plus `= node->type.get()` /
  `= rawptr`), ~60 readers via `expressionTypes.find(X)` / `[X]`.
- `retainType(raw)` (semantic.hpp:543): takes ownership of `raw` into
  `_ownedTypes` (`std::vector<std::unique_ptr<ast::TypeNode>>`, :542) and returns
  the raw pointer. Its return value feeds THREE consumers:
  1. `expressionTypes[node]` (103 sites),
  2. `SymbolInfo.type` — a **raw** `ast::TypeNode*` field (semantic.hpp:112),
  3. raw `resultType` locals (e.g. operator-type block ~2110) that are later
     re-retained into `expressionTypes[node] = retainType(resultType)`.
- `node->type` is `std::shared_ptr<TypeNode>` (ast.hpp:409). At many sites it is
  a **separate clone** of the object `expressionTypes` holds:
  `expressionTypes[node] = retainType(intType); node->type = std::shared_ptr(intType->clone());`
- Readers: ~60 `auto it = expressionTypes.find(X); ... it->second` sites across
  ~15 node-kind handlers + fold/return-type logic.

## Target signatures

```cpp
// semantic.hpp
std::unordered_map<unsigned /*node:typeId()*/, std::shared_ptr<ast::TypeNode>> typeTable_;  // already landed
std::shared_ptr<ast::TypeNode> typeOf(const ast::Node*) const;      // landed
void setType(const ast::Node*, std::shared_ptr<ast::TypeNode>);     // landed

// changed in this flip
std::shared_ptr<ast::TypeNode> retainType(std::shared_ptr<ast::TypeNode>);  // or keep raw->shared bridge
struct SymbolInfo { ... std::shared_ptr<ast::TypeNode> type; };             // raw -> shared
// resultType locals become std::shared_ptr<ast::TypeNode>
```

The migration is **atomic over these surfaces** — the map's value type,
`retainType`'s return, `SymbolInfo.type`, and the raw `resultType` locals must
change together. There is **no intermediate green state** until they all compile.

## Execution steps (fresh session, run `git status` first; tree should be clean)

### Step 0 — baseline
Build `build/vyb`; confirm it builds. (Full suite 1141 is the gate at the end.)

### Step 1 — `SymbolInfo.type` raw -> shared_ptr (semantic.hpp:112)
This is the piece that unblocks making `retainType` return `shared_ptr`.
- Change the field to `std::shared_ptr<ast::TypeNode> type = nullptr;`.
- Audit every `SymbolInfo` construction passing `retainType(...)` as the last
  field (lines ~1287/1299/1308 struct+enum+alias forward decls; ~1657 param;
  ~1925 variable; ~1935/1993 type) — these currently store a raw registry-owned
  pointer. With a shared field they hold the shared_ptr directly.
- Audit every read of a `SymbolInfo::type` that treats it as raw
  (`sym->type`, `.type`) — most already work through `shared_ptr` auto-deref or
  `.get()`; fix the handful that assign to a `ast::TypeNode*` local.
- Pattern to reuse: `theSymbolInfo.type->clone()`, `.get()`, `!= nullptr`.

### Step 2 — `retainType` returns `std::shared_ptr<ast::TypeNode>`
- `_ownedTypes` becomes `std::vector<std::shared_ptr<ast::TypeNode>>`; the
  registry still exists to keep synthesized types alive for the lifetime of the
  analyzer, but entries are now shared.
- Make `retainType` accept/take `std::shared_ptr<ast::TypeNode>` and store it
  into `_ownedTypes`, returning the shared_ptr.
- `resultType` locals that were `ast::TypeNode*` and assigned
  `resultType = retainType(new ast::TypeName(...))` (operator block ~2110-2300,
  call-return ~2702-3260, deref logic) become `std::shared_ptr<ast::TypeNode>`.
  Where `resultType` is later handed to `expressionTypes[node] = retainType(resultType)`,
  prefer `setType(node, resultType)` (shared, no re-retain) — this is where the
  current re-retain of an already-owned pointer is risky.
- A `retainType`-returning-shared_ptr feeding `expressionTypes` auto-converts
  once Step 3 changes the map.

### Step 3 — `expressionTypes` -> owning shared_ptr map (then node-id key)
- Change the map to
  `std::unordered_map<unsigned, std::shared_ptr<ast::TypeNode>>` keyed by
  `node->typeId()` and route through `setType(node, ...)` / `typeOf(node)`.
- Convert writers:
  - `expressionTypes[node] = retainType(X)` (103) -> `setType(node, retainType(X))`.
  - `expressionTypes[node] = nullptr` (~40) -> `setType(node, nullptr)` (shared
    null is fine) or erase.
  - `expressionTypes[node] = <shared_ptr local>` (e.g. `node->type`,
    `emptyTupleType`) -> `setType(node, <sharedptr>)` (share the same object —
    no clone).
  - `expressionTypes[node] = <raw TypeNode*>` (the ~15 odd writers:
    `pal` 5256, `savedDerefType` 5427, `actualReturnType` 4117/4762,
    `currentImplType` 8491, `addressValueType` etc.) -> wrap as
    `setType(node, std::shared_ptr<TypeNode>(raw->clone()))` only if the raw is
    privately owned; if the raw aliases `node->type`, share `node->type`
    instead. Decide per site; these are the subtle ones.
  - The channel/literal/Vec blocks (lines ~85/98/105/118/125/136, 1169-1193,
    2861-3260) mostly use `retainType(new ...)` then set `node->type =
    shared(new...->clone())` — collapse to ONE shared object:
    `auto t = std::make_shared<...>(...); setType(node, t); node->type = t;`
    (removes the double allocation and the raw mirror).
- Convert readers (~60): after the map holds shared_ptr,
  - `auto it = expressionTypes.find(X); ast::TypeNode* t = it->second;` -> `t = it->second.get()`.
  - `it->second->toString()/->clone()` works unchanged on shared_ptr.
  - `bodyTy = it->second;` (6076), `yielded = it->second` (6174/6186/6596/6604/7111),
    `leftType = it->second` (2031/5448/5552), `argType = it->second`
    (2625/2679/3617), `operandTy` (10822), `receiverType` (9585) — add `.get()`.
  - `expressionTypes[node]->clone()` (5913/8686) — `typeOf(node)->clone()`.

### Step 4 — drop the raw mirror + old registry
Once every writer/reader routes through `setType`/`typeOf`:
- Delete the `expressionTypes` member, `_ownedTypes`, and any raw pointer that
  only existed to alias them.
- `retainType` may collapse: if only the TypeTable + `node->type` own types, the
  `_ownedTypes` registry (which was there because raw pointers had no owner) is
  removed and `retainType` can be deleted, replaced by `setType`/`make_shared`.

### Step 5 — make the AST immutable (stretch, later increment)
With types owned by the TypeTable and `node->type`, make `Node` fields
const/read-only-after-parse as a follow-on. Not required for green.

## Verification (must all pass before commit)
1. `cmake --build build --target vyb` clean.
2. Full suite: `python3 test/run_tests.py --vyb build/vyb --test-dir test --execute-jit`
   -> **1141/1141** (currently). No new failures, no new sanitizer/UAF surfaced.
3. The three CLI smokes, since they exercise the compiler end-to-end:
   `test/lsp_smoke.py`, `test/repl_smoke.py`, `test/gitdep_smoke.py`.
4. `tools/refman.py --check` unaffected (no doc regen unless stdlib touched —
   it isn't).
5. A UAF/double-free watch: rebuild once with `-fsanitize=address,undefined` if
   the project's sanitize build is configured (the `_ownedTypes` re-retain path
   at resultType sites is the prime double-free suspect) — at minimum run
   `test/ownership/*` and `test/units/test_vec_last_peek.vyb`.

## Concrete site anchor list (survey 2026-09-10; re-grep to refresh)
Read-side `find` sites (add `.get()` on the value when assigned to `TypeNode*`):
871,1028,1833,1851,1919,2010,2028,2031,2624,2678,2725,2765,2872,3616,3750,3783,
4466,4828,4842,4874,4903,5005,5368,5371,5445,5551,5601,5636,5683,5736,5769,5803,
5886,6019,6075,6113,6173,6186,6210,6595,6603,6664,6725,6754,6813,7007,7091,7110,
8181,8381,8408,8443,8730,8760,8783,9256,9584,10080,10821,10879.
Raw/owning writers to review for clone-vs-share semantics:
4117,4762,5256,5427,5673,8491,8723,8728 (mark-as-error `=nullptr` set), and the
deref/error blocks 6664-6816, 8181-8444 (optionals/unwraps).
SymbolInfo/resultType retainType consumers: 1287,1299,1308,1657,1681,1925,1935,
1993,2110,2117.

## Rollback
The flip has no intermediate green, so if it can't reach a green all-suite state
in the fresh session, revert cleanly: `git checkout -- src/vre/semantic.cpp
include/vyb/semantic.hpp` (nothing else in the repo is touched by the flip) and
keep increment #1 (`2944d69`) as the milestone. Do NOT commit a red tree.
