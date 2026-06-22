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

- Currently only supports identifier expressions: `for (x in v)` ✓
- Complex expressions not yet supported: `for (x in get_vec())` ✗
- **Vec<Struct> limitation**: Vec.get() currently returns Int type regardless of actual element type
  - Works correctly for Vec<Int>: `for (x in vec_int)` ✓
  - Type inference issue for Vec<Point> or other struct types
  - Accessing struct fields on iteration variable fails: `p.x` returns error
  - Will be fixed when Vec type system is enhanced
- Will be extended to support any expression in future

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
