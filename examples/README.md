# Vyn Examples

Runnable Vyn programs demonstrating language features and common algorithms.
All examples use current Vyn v0.4.x syntax and are compiled/run with:

```bash
./build/vyn examples/<file>.vyn
```

---

## Algorithm Examples

### `sort.vyn` — Insertion Sort
Functional-style insertion sort for `Vec<Int>`.  Uses only the Vec operations
available in v0.4.x (`push`, `get`, `len`).  Good starting point for
understanding how to work with Vecs without a `set()` operation.

### `quicksort.vyn` — Quicksort
Recursive quicksort that partitions a `Vec<Int>` into three groups (less-than,
equal, greater-than the pivot) and concatenates the sorted halves.  Demonstrates
recursive functions and Vec construction.

---

## Data Structure Examples

### `stack.vyn` — Stack (LIFO)
A stack built on top of `Vec<Int>` using a `Stack` struct and free functions
(`stack_push`, `stack_pop`, `stack_peek`, `stack_empty`).  Illustrates the
idiomatic Vyn pattern of struct + free functions with `their<T>` borrowed
references before aspect/bind method dispatch is needed.

### `binary_tree_clean.vyn` — Binary Search Tree
A flat-storage BST using `Vec<TreeNode>` for node storage (avoids self-referential
pointer cycles).  Demonstrates struct composition, Vec iteration, and string results.

---

## Other Examples

### `main.vyn` — Language Feature Showcase
Shows arithmetic, structs, `Vec<T>`, `match`, `defer`, string operations, and
Fibonacci recursion in a single runnable file.  Good first example to read.

### `vec_filter.vyn` — Vec Filtering
Filter even numbers from a `Vec<Int>` by building a new Vec.

### `vec_max.vyn` — Max Value in Vec
Find the maximum value in a `Vec<Int>` using a `for (item in vec)` loop.

### `vec_point_distance.vyn` — Struct Vec Iteration
Iterate over a `Vec<Point>` and compute squared distances from the origin.

### `memory_semantics.vyn` — Ownership Keywords
Shows `my`, `our`, `their`, `mild` ownership syntax.  Some runtime enforcement
is still in progress; see `doc/OWNERSHIP_MILD.md` for current status.

### `mild_references.vyn` — Weak References
Demonstrates `mild<T>` (soft/weak) reference syntax using `soft()`, `.released()`.
Control block runtime enforcement is planned for v0.5.

### `ffi_puts.vyn` — C FFI Through Freedom
Declares libc `puts` in an `extern "C"` block and calls it inside `freedom { }`.
This is the intended boundary for direct FFI calls.

---

## Coming Soon (require features in progress)

| Example | Blocking feature |
|---------|-----------------|
| Generic sort `<T<Comparable>>` | Generic aspect bounds (v0.5) |
| `HashMap` word count | `HashMap<K,V>` stdlib (v0.5) |
| File I/O | `File::open/read/write` stdlib (v0.5) |
| FFI — variadic calls / `repr(C)` structs | Expanded FFI (v0.5) |
| Multi-file project | Module system `import`/`smuggle` (v0.5) |
| TCP server | Sockets via FFI (post-1.0) |
