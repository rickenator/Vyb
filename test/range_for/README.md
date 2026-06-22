# Range-Based For Loop Tests

This directory contains tests for Vyb's range-based for loop syntax.

## Syntax

Vyb supports range-based for loops with **inclusive** ranges:

### Basic Range (`..`)
```vyb
for i in 0..5 {
    // i takes values: 0, 1, 2, 3, 4, 5 (INCLUSIVE of end)
}
```

### Range with Step
```vyb
for i in 0..10, 2 {
    // i takes values: 0, 2, 4, 6, 8, 10 (step by 2)
}
```

### Future: Iterate over Vec<T>
```vyb
for item in my_vec {
    // Iterate over elements (planned)
}
```

## Test Files

| Test File | Description | Expected Result |
|-----------|-------------|-----------------|
| `simple_range.vyb` | Basic inclusive range `0..4` | Returns 10 (sum of 0+1+2+3+4) |
| `inclusive_range.vyb` | Inclusive range `0..5` | Returns 15 (sum of 0+1+2+3+4+5) |
| `range_with_step.vyb` | Range with step `0..10, 2` | Returns 30 (sum of 0+2+4+6+8+10) |
| `range_step_large.vyb` | Large step `1..100, 10` | Returns 10 (count of iterations) |
| `range_with_break.vyb` | Break statement in range loop | Returns 10 (exits at i=5) |
| `range_with_continue.vyb` | Continue statement in range loop | Returns 40 (skips i=5) |
| `negative_range.vyb` | Range with negative start `-5..4` | Returns -5 (sum of -5 to 4) |

## Implementation Notes

Range-based for loops are implemented as syntactic sugar over traditional for loops:

```vyb
for i in start..end {
    body
}
```

Desugars to:
```vyb
{
    var i = start;
    while i <= end {      // INCLUSIVE (<=)
        body;
        i = i + 1;        // Default step = 1
    }
}
```

With step parameter:
```vyb
for i in start..end, step {
    body
}
```

Desugars to:
```vyb
{
    var i = start;
    while i <= end {
        body;
        i = i + step;     // Custom step value
    }
}
```

## Design Philosophy

**Ranges are INCLUSIVE**: `for i in 1..10` includes both 1 and 10. This is more intuitive than exclusive ranges for most use cases.

## Features

- ✅ Inclusive ranges (`..`) - `start..end` includes both endpoints
- ✅ Custom step values - `start..end, step`
- ✅ Works with `break` and `continue`
- ✅ Supports negative numbers
- ✅ Loop variable is automatically declared
- ✅ End can be any expression that evaluates to Int
- ✅ Step can be any expression that evaluates to Int
- ⏳ Iterating over Vec<T> (future: `for item in vec`)
- ⏳ Reverse iteration (future: negative step or `.rev()`)
- ⏳ Range objects as first-class values (future)

## Type System

The loop variable (`i` in the examples) automatically takes the type `Int` for numeric ranges. The end expression and step expression must also evaluate to `Int`.

**Future**: When iterating over `Vec<T>`, the loop variable will have type `T`.
