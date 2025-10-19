# Match Statement Syntax

## Overview

The `match` statement provides pattern matching in Vyn with clean, intuitive syntax designed for clarity and consistency.

## Syntax

```vyn
match (expression) {
    pattern -> result_expression,
    pattern -> result_expression,
    ?       -> default_expression
}
```

## Key Features

### Parenthesized Expression
The expression to match is enclosed in parentheses for visual clarity and to avoid ambiguity:

```vyn
match (x + y) {
    0 -> println("zero"),
    1 -> println("one")
}
```

### Arrow Operator (`->`)
Pattern matching uses the `->` arrow, consistent with function syntax:

```vyn
match (value) {
    42 -> "the answer",
    0  -> "zero"
}
```

### Wildcard Pattern (`?`)
The wildcard pattern `?` matches anything:

```vyn
match (status) {
    0 -> "init",
    1 -> "running",
    ? -> "unknown"  // Matches all other values
}
```

### No-Match Behavior
If no pattern matches and there's no wildcard, execution continues as a NOP:

```vyn
match (x) {
    1 -> println("one"),
    2 -> println("two")
    // If x is 3, nothing happens - execution continues
}
```

## Pattern Types

### Literal Patterns
Match specific literal values:

```vyn
match (x) {
    0    -> "zero",
    42   -> "forty-two",
    3.14 -> "pi"
}
```

### Identifier Patterns  
Currently match identifiers as values (future: destructuring):

```vyn
match (result) {
    success -> handle_success(),
    error   -> handle_error()
}
```

### Future: Complex Patterns
Planned support for:
- Struct destructuring: `Point { x, y } > ...`
- Enum variants: `Some(value) > ...`
- Range patterns: `1..10 > ...`
- Guard clauses: `x if x > 0 > ...`

## Examples

### Simple Integer Matching
```vyn
describe_number(x<Int>)<String> -> {
    match (x) {
        0  -> "zero",
        1  -> "one",
        42 -> "the answer",
        ?  -> "some number"
    }
}
```

### Status Code Handling
```vyn
process_status(code<Int>)<Void> -> {
    match (code) {
        200 -> println("OK"),
        404 -> println("Not Found"),
        500 -> println("Server Error"),
        ?   -> println("Unknown status")
    }
}
```

### Without Wildcard
```vyn
check_specific(n<Int>)<Void> -> {
    match (n) {
        1 -> println("one"),
        2 -> println("two")
        // For any other value, nothing happens
    }
}
```

## Implementation Status

### ✅ Completed
- Parenthesized match expressions
- `>` arrow syntax
- `?` wildcard pattern
- NOP behavior for no-match without wildcard
- Integer and float pattern matching
- Multiple case branches
- Proper LLVM IR generation

### 🚧 In Progress
- Semantic analysis (currently empty visitor)
- Exhaustiveness checking
- Pattern type checking

### 📋 Planned
- Struct destructuring patterns
- Enum variant patterns
- Range patterns (`1..10`)
- Guard clauses (`pattern if condition`)
- Match as expression (returning values)
- Tuple patterns

## Grammar

```
match_statement ::= 'match' '(' expression ')' '{' match_arm* '}'
match_arm       ::= pattern '->' expression ','?
pattern         ::= literal
                  | identifier  
                  | '?'                    // wildcard
                  | path '{' field_pattern* '}' // future: struct
                  | path '(' pattern* ')'       // future: enum
                  | pattern 'if' expression     // future: guard
```

## Design Rationale

### Why Parentheses?
- **Visual Clarity**: Clear separation between match keyword and expression
- **Consistency**: Aligns with `if (condition)` and other control flow
- **Flexibility**: Allows complex expressions without ambiguity

### Why `->` Instead of `=>`?
- **Consistency**: Matches function declaration syntax `foo() -> { ... }`
- **Familiarity**: Standard arrow operator used throughout Vyn
- **Clarity**: Clearly shows flow from pattern to result
- **Uniformity**: Same arrow for all "points to" semantics

### Why `?` Instead of `_`?
- **Intuitive**: `?` universally means "anything" or "unknown"
- **Readable**: More visible than underscore
- **Consistent**: Aligns with optional types `T?`

### Why NOP on No-Match?
- **Explicit Intent**: Forces wildcard for default behavior
- **Debugging**: Makes missing patterns obvious
- **Flexibility**: Allows match as control flow filter

## Comparison with Other Languages

### Rust
```rust
match x {
    0 => "zero",
    _ => "other"
}
```

### Swift
```swift
switch x {
case 0: return "zero"
default: return "other"
}
```

### Vyn
```vyn
match (x) {
    0 -> "zero",
    ? -> "other"
}
```

**Advantages**:
- Consistent arrow syntax with functions
- Clear expression boundaries with parens
- Intuitive wildcard symbol
- Flexible no-match behavior
