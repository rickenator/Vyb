# Vec<T> Iteration Implementation

## Status: ✅ COMPLETE

Vec<T> iteration is fully implemented and working in Vyb v0.4.1.

## Syntax

```vyb
for (item in vector_expr) {
    // body
}
```

**Note**: Parentheses are MANDATORY for all for loops in Vyb.

## Features

### ✅ Basic Iteration
```vyb
v<Vec<Int>> = Vec::new();
v.push(1);
v.push(2);
v.push(3);

for (x in v) {
    println(x);  // Prints 1, 2, 3
}
```

### ✅ Empty Vec Handling
```vyb
v<Vec<Int>> = Vec::new();
for (x in v) {
    // This block is never executed
}
```

### ✅ Break Statement
```vyb
for (x in v) {
    if (x > 5) {
        break;  // Exits the loop
    }
}
```

### ✅ Continue Statement
```vyb
for (x in v) {
    if (x == 3) {
        continue;  // Skips to next iteration
    }
    sum = sum + x;
}
```

## Implementation Details

Vec iteration desugars to an index-based for loop:

```vyb
for (item in vec) { body }
```

Becomes:

```vyb
for (__run_once = true; __run_once; __run_once = false) {
    var __len = vec.len();
    for (__idx = 0; __idx < __len; __idx = __idx + 1) {
        var item = vec.get(__idx);
        body;
    }
}
```

### Why this structure?
1. **For loop instead of while**: Ensures increment happens even with `continue`
2. **Outer run-once loop**: Matches `ForStatement` return type requirement
3. **Direct Vec usage**: No temporary variable to avoid double-free
4. **Index-based**: Uses Vec's `len()` and `get(idx)` methods

## Limitations

Verified against `build/vyb` at the time of writing (each bullet says how it was
checked):

- **Identifier iterables work for any element type** — `for (x in v)` ✔, including
  struct elements (`for (p in v)` where `v<Vec<Point>>`: field access `p.x` resolves).
  Requires the stdlib iterator modules to be imported (`import collections`,
  `import core::iter`); without them the desugar has no `iter()`/`next()` to bind and
  reports `Unknown method 'next' on type 'Vec<Point>'`.
- **Non-identifier iterables work for scalar element Vecs** —
  `for (x in ints.iter())` ✔ (`test/modules/test_for_iter.vyb`), and re-evaluating the
  producer each loop starts a fresh iterator.
- **Non-identifier iterables are NOT yet supported for struct element Vecs** —
  `for (p in make_vec())` where `make_vec()<Vec<Point>>` fails semantic analysis with
  `Unknown method 'next' on type 'Vec<Point>'` (a deliberate limitation, tracked by
  the "Non-identifier `for` over a struct-element `Vec`" row in
  `doc/FEATURE_STATUS.md`), even though the identifier form over the same Vec works.
- **`Vec::get()` is element-typed** — `p<Point> = v.get(0)` returns a `Point` and
  `p.x` reads correctly, so the older note here ("`Vec.get()` returns Int regardless
  of the element type") no longer holds; typed constructors (`Vec<Point>()`) and
  iteration over struct elements both work.

## Tests

All tests passing:
- `comprehensive_test.vyb`: All Vec iteration features
- `comprehensive_range_test.vyb`: All range-based for loops
- Individual feature tests in `test/vec_for/`

## Range-Based For Loops

Also fully implemented with same syntax:

```vyb
for (i in 0..10) { }        // Inclusive: 0 to 10
for (i in 0..10, 2) { }     // With step: 0, 2, 4, 6, 8, 10
for (i in -5..5) { }        // Negative ranges work
```

All control flow (break/continue) works in range loops too.
