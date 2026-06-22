# Memory & Ownership Tests

This directory contains comprehensive tests for VyB's ownership and memory management system, including the unique ownership types (`my<T>`, `our<T>`, `their<T>`), borrowing operations (`borrow()`, `view()`), and low-level memory operations (`loc()`, `at()`).

## Test Files

### ownership_test.vyb
**Purpose**: Comprehensive test suite covering all ownership and borrowing semantics

**What it tests**:
1. **my<T> unique ownership** - Single-owner semantics and move operations
2. **our<T> shared ownership** - Reference-counted shared ownership
3. **borrow() mutable borrowing** - Creating mutable `their<T>` references
4. **view() immutable borrowing** - Creating read-only `their<T const>` references
5. **loc() and at() pointers** - Low-level pointer operations
6. **Function parameter borrowing** - Passing borrows and views to functions
7. **Complex ownership chains** - Multiple ownership types interacting
8. **String ownership** - Ownership with String types
9. **Sequential borrows** - Multiple borrow operations in sequence
10. **Ownership transfers** - Moving ownership between variables

**Expected output**:
```
╔═══════════════════════════════════════════════════════════╗
║    Comprehensive Ownership & Memory Semantics Test       ║
╚═══════════════════════════════════════════════════════════╝

=== Test 1: my<T> unique ownership ===
Created my<Int> with value: 42
Moved ownership successfully
✓ Test 1 passed

=== Test 2: our<T> shared ownership ===
Created our<String>: shared data
Created multiple references to shared data
✓ Test 2 passed

[... 8 more tests ...]

╔═══════════════════════════════════════════════════════════╗
║                    Test Summary                          ║
╠═══════════════════════════════════════════════════════════╣
║  Passed: 10/10                                           ║
╚═══════════════════════════════════════════════════════════╝
```

**Run with**:
```bash
build/vyb test/memory/ownership_test.vyb
```

---

### borrow_test.vyb
**Purpose**: Tests basic borrow functionality with structs

**What it tests**:
- Creating and modifying structs
- Using `borrow()` to pass mutable references to functions
- Verifying modifications through borrows persist

**Expected output**:
```
Before: 0
After: 42
```

**Run with**:
```bash
build/vyb test/memory/borrow_test.vyb
```

---

### simple_memory_test.vyb
**Purpose**: Basic memory operations test

**What it tests**:
- Pointer creation with `loc()`
- Pointer dereferencing with `at()`
- Creating borrows with `borrow()`
- Creating views with `view()`
- All inside `freedom {}` blocks

**Expected behavior**:
- Returns 100 (modified value through pointer)
- Demonstrates low-level memory manipulation

**Run with**:
```bash
build/vyb test/memory/simple_memory_test.vyb
```

---

## Ownership System Quick Reference

### Ownership Types

| Type | Description | Mutability | Lifetime |
|------|-------------|------------|----------|
| `my<T>` | Unique ownership | Mutable | Until moved/dropped |
| `our<T>` | Shared ownership | Immutable | Reference counted |
| `their<T>` | Mutable borrow | Mutable | Until borrow ends |
| `their<T const>` | Immutable borrow | Immutable | Until view ends |

### Ownership Operations

| Operation | Syntax | Returns | Description |
|-----------|--------|---------|-------------|
| Unique constructor | `my(expr)` | `my<T>` | Creates unique ownership |
| Shared constructor | `our(expr)` | `our<T>` | Creates shared ownership |
| Mutable borrow | `borrow(expr)` | `their<T>` | Borrows mutably (safe, checked) |
| Immutable borrow | `view(expr)` | `their<T const>` | Borrows immutably (safe, checked) |
| Pointer creation | `loc(expr)` | `loc<T>` | Gets memory location (freedom) |
| Pointer dereference | `at(ptr)` | `T` | Accesses value at pointer (freedom) |

### Safety Rules

1. **`borrow()` and `view()` are safe checked borrows** - No `freedom {}` required
2. **`loc()` and `at()` require `freedom {}`** - Direct memory access is inherently freedom
3. **`my<T>` ownership is unique** - Only one owner at a time (moves invalidate original)
4. **`our<T>` is reference counted** - Multiple owners share immutable data
5. **`their<T>` is temporary** - Borrows must not outlive their owner

## Test Execution

Run all memory tests:
```bash
# Comprehensive ownership test
build/vyb test/memory/ownership_test.vyb

# Basic borrow test
build/vyb test/memory/borrow_test.vyb

# Simple memory operations
build/vyb test/memory/simple_memory_test.vyb
```

## Expected Results

All tests should:
- ✅ Compile without errors
- ✅ Execute with expected output
- ✅ Return correct values
- ✅ Demonstrate proper ownership semantics

## Common Issues

### "Prefix 'borrow expr' syntax is no longer supported"
**Solution**: Use function-call syntax:
```vyb
borrowed<their<Int>> = borrow(data)
viewed<their<Int const>> = view(data)
```

### "borrow() argument must be an owned type my<T> or our<T>"
**Solution**: Ensure you're borrowing from `my<T>` or `our<T>`, not plain `T`:
```vyb
data<my<Int>> = my(42)  // Correct
borrowed<their<Int>> = borrow(data)  // Works
```

### Pointer operations fail
**Solution**: All pointer operations require `freedom {}`:
```vyb
freedom {
    ptr<loc<Int>> = loc(x)
    value<Int> = at(ptr)
}
```

## Related Documentation

- `doc/RUNTIME.md` - Runtime ownership semantics
- `doc/Canonical_Reference_Syntax.md` - Borrowing syntax reference
- `doc/Reference_Syntax_Unification.md` - Syntax design rationale
- `README.md` - Language overview with ownership examples

## Future Tests

Planned additions:
- [ ] Nested struct borrowing
- [ ] Ownership with collections (Vec, Map)
- [ ] Async ownership transfer
- [ ] Cyclic reference detection
- [ ] Borrow checker validation tests
