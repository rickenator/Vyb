# Vyn Error Handling: Failure & Trap System

**Status:** Design Specification (v0.4.2)  
**Last Updated:** October 20, 2025  
**Philosophy:** Fresh thinking on error handling - no tired try/catch patterns

---

## Core Philosophy

Error handling should be:
- **Explicit but not verbose:** Errors visible at call sites without boilerplate
- **Type-safe:** Compile-time checking of error types and compatibility
- **Composable:** Easy to build error hierarchies and handlers
- **Unique to Vyn:** Fresh approach that fits Vyn's design philosophy

**Key Insight:** Blocks can fail, and failures can be trapped. No exceptions, no try/catch - just **failure** and **trap**.

---

## Basic Syntax

### Simple Failure

```vyn
divide(a<Int>, b<Int>)<Int> -> {
    if (b == 0) {
        fail DivisionByZero { dividend = a }
    }
    return a / b
}
```

### Trapping Failures

```vyn
safe_divide(a<Int>, b<Int>)<Int> -> {
    result<Int> = {
        divide(a, b)
    } trap (e<DivisionByZero>) -> {
        println("Cannot divide by zero!")
        return -1
    }
    return result
}
```

### Multiple Traps (Type-Based Dispatch)

```vyn
process_file(path<String>)<String> -> {
    content<String> = {
        file<File> = open_file(path)
        data<String> = file.read()
        data
    } trap (e<FileNotFound>) -> {
        return "File not found: " + path
    } trap (e<PermissionDenied>) -> {
        return "Permission denied: " + path
    } trap (e<IOError>) -> {
        return "IO error: " + e.message
    }
    
    return content
}
```

---

## Trap Clause Rules

### Rule 1: Type Compatibility
When a block's value is consumed, all trap clauses must evaluate to the same type.

```vyn
# ✅ VALID - All traps return Int
compute()<Int> -> {
    value<Int> = {
        risky_operation()
    } trap (e<Error1>) -> {
        return 0
    } trap (e<Error2>) -> {
        return -1
    }
    return value
}

# ❌ INVALID - Trap returns String, block returns Int
bad_compute()<Int> -> {
    value<Int> = {
        risky_operation()
    } trap (e<Error1>) -> {
        return "error"  # Type mismatch!
    }
    return value
}
```

### Rule 2: First Type-Compatible Trap Wins
Traps are checked in order. First matching type handles the failure.

```vyn
{
    operation()
} trap (e<SpecificError>) -> {
    # Catches SpecificError
} trap (e<GeneralError>) -> {
    # Catches GeneralError and its subtypes (except SpecificError)
} trap (e<Error>) -> {
    # Catches all other errors
}
```

### Rule 3: Untrapped Failures Crash After Cleanup
If no trap matches, the failure crashes the current task after scope cleanup.

```vyn
dangerous_operation()<Int> -> {
    resource<Resource> = acquire_resource()
    # If this fails and no trap, resource is cleaned up before crash
    result<Int> = resource.process()
    return result
}
```

---

## Propagation: `rethrow`

The `rethrow` keyword propagates the error upward without handling it.

```vyn
wrapper_function()<Int> -> {
    {
        might_fail()
    } trap (e<NonCriticalError>) -> {
        println("Non-critical error, ignoring")
        return 0
    } trap (e<CriticalError>) -> {
        println("Critical error, propagating!")
        rethrow  # Send error to caller
    }
}
```

### Rethrow with Transformation

```vyn
{
    low_level_operation()
} trap (e<LowLevelError>) -> {
    # Transform error and propagate
    fail HighLevelError { 
        message = "Operation failed: " + e.details,
        cause = e
    }
}
```

---

## Cleanup: `ensure`

The `ensure` clause runs after either success or failure, similar to `finally` but cleaner.

```vyn
process_with_cleanup(path<String>)<String> -> {
    file<File> = open_file(path)
    
    result<String> = {
        file.read()
    } trap (e<IOError>) -> {
        return ""
    } ensure -> {
        file.close()  # Always runs, success or failure
    }
    
    return result
}
```

### Ensure Execution Order

```vyn
{
    resource<Resource> = acquire()
    {
        resource.use()
    } ensure -> {
        println("Inner cleanup")
    }
} trap (e<Error>) -> {
    println("Error handler")
} ensure -> {
    println("Outer cleanup")
}

# Execution on failure:
# 1. Inner cleanup
# 2. Error handler (if trap matches)
# 3. Outer cleanup
```

---

## The Error Type Hierarchy

### Base Error Type

```vyn
# Built-in base error type
struct Error {
    message<String>,
    location<SourceLocation>,
    timestamp<Int64>
}

aspect Errorable {
    message(self<Self>)<String> -> { }
    details(self<Self>)<String> -> { }
}

bind Errorable -> Error {
    message(self<Self>)<String> -> {
        return self.message
    }
    
    details(self<Self>)<String> -> {
        return self.message + " at " + self.location.to_string()
    }
}
```

### Creating Custom Errors

#### Option 1: Struct Extension (Composition)

```vyn
struct DivisionByZero {
    base<Error>,
    dividend<Int>
}

bind Errorable -> DivisionByZero {
    message(self<Self>)<String> -> {
        return "Division by zero: dividend=" + self.dividend.to_string()
    }
    
    details(self<Self>)<String> -> {
        return self.base.details() + " [dividend=" + self.dividend.to_string() + "]"
    }
}

# Usage
if (divisor == 0) {
    fail DivisionByZero {
        base = Error { 
            message = "Cannot divide by zero",
            location = here(),
            timestamp = now()
        },
        dividend = dividend
    }
}
```

#### Option 2: Aspect-Based (Duck Typing)

```vyn
struct MyError {
    msg<String>,
    code<Int>
}

bind Errorable -> MyError {
    message(self<Self>)<String> -> {
        return "[" + self.code.to_string() + "] " + self.msg
    }
    
    details(self<Self>)<String> -> {
        return self.message() + " (custom error)"
    }
}

# Any type implementing Errorable can be failed
fail MyError { msg = "Something broke", code = 42 }
```

#### Option 3: Error Macro (Syntactic Sugar)

```vyn
# Macro-generated error type
error FileError : Error {
    path<String>,
    operation<String>
}

# Expands to struct + Errorable binding automatically

# Usage
fail FileError {
    message = "File operation failed",
    location = here(),
    timestamp = now(),
    path = "/tmp/file.txt",
    operation = "read"
}
```

---

## Error Hierarchy Patterns

### Inheritance-Style (Via Composition)

```vyn
struct Error {
    message<String>,
    location<SourceLocation>
}

struct IOError {
    base<Error>,
    path<String>
}

struct FileNotFound {
    base<IOError>,
    attempted_paths<Vec<String>>
}

# Trap hierarchy - most specific first
{
    open_file(path)
} trap (e<FileNotFound>) -> {
    println("Not found, tried: " + e.attempted_paths.len().to_string() + " paths")
} trap (e<IOError>) -> {
    println("IO error at: " + e.path)
} trap (e<Error>) -> {
    println("General error: " + e.message)
}
```

### Enum-Style Error Variants

```vyn
enum ParseError {
    UnexpectedToken { expected<String>, got<String> },
    UnexpectedEOF { position<Int> },
    InvalidSyntax { message<String>, line<Int>, column<Int> }
}

bind Errorable -> ParseError {
    message(self<Self>)<String> -> {
        match (self) {
            UnexpectedToken -> return "Expected " + self.expected + ", got " + self.got,
            UnexpectedEOF -> return "Unexpected EOF at position " + self.position.to_string(),
            InvalidSyntax -> return self.message
        }
    }
}

# Usage
fail ParseError::UnexpectedToken { expected = "identifier", got = "number" }
```

---

## Advanced Patterns

### Panic vs Fail

```vyn
# fail - trappable, can be handled
fail UserError { message = "Invalid input" }

# panic - untrappable, always crashes (use sparingly)
panic("Invariant violated: this should never happen!")
```

### Result<T, E> Pattern (Optional)

```vyn
enum Result<T, E> {
    Ok { value<T> },
    Err { error<E> }
}

# Functional error handling without trap
parse_int(s<String>)<Result<Int, ParseError>> -> {
    if (is_valid_int(s)) {
        return Result::Ok { value = convert_to_int(s) }
    } else {
        return Result::Err { error = ParseError { message = "Invalid integer" } }
    }
}

# Usage with match
result<Result<Int, ParseError>> = parse_int("42")
match (result) {
    Ok -> println("Parsed: " + result.value.to_string()),
    Err -> println("Error: " + result.error.message)
}
```

### Freedom Blocks and Errors

```vyn
# Freedom blocks can fail too
process_pointer(ptr<loc<Int>>)<Int> -> {
    value<Int> = {
        freedom {
            if (ptr == null()) {
                fail NullPointerError { address = 0 }
            }
            at(ptr)
        }
    } trap (e<NullPointerError>) -> {
        println("Null pointer detected!")
        return 0
    }
    return value
}
```

### Async Errors

```vyn
async fetch_data(url<String>)<Future<String>> -> {
    response<String> = {
        await http_get(url)
    } trap (e<NetworkError>) -> {
        println("Network error, retrying...")
        await http_get(url)  # Retry once
    } trap (e<Timeout>) -> {
        return "Request timed out"
    }
    
    return response
}
```

---

## Compiler Implementation Notes

### Type Checking Rules

1. **Trap Type Compatibility:** Each trap parameter type must be compatible with failure types in the block
2. **Return Type Unification:** All trap bodies must return same type as block expression
3. **Ensure Type Validation:** Ensure blocks must return `Void` or be expression statements
4. **Rethrow Context:** `rethrow` only valid inside trap clauses
5. **Fail Type Requirement:** `fail` argument must implement `Errorable` aspect

### Codegen Strategy

1. **Block Wrapping:** Wrap blocks with trap clauses in hidden try-like mechanism (LLVM exception handling)
2. **Type Dispatch:** Generate type checks for each trap clause (most specific first)
3. **Ensure Execution:** Use RAII-like patterns to guarantee ensure execution
4. **Rethrow:** Generate throw of current error being handled
5. **Cleanup:** Integrate with ownership system for proper resource cleanup

### Error Value Representation

```cpp
// LLVM IR representation
struct VynError {
    void* vtable;        // Type information for dispatch
    void* data;          // Error-specific data
    SourceLocation loc;  // Where error occurred
    int64_t timestamp;   // When error occurred
}
```

---

## Syntax Summary

### Keywords
- `fail` - Trigger a failure with an error value
- `trap` - Catch and handle specific error types
- `rethrow` - Propagate error to caller
- `ensure` - Cleanup code that always runs
- `panic` - Unrecoverable error (crashes immediately)

### Grammar Extensions

```ebnf
block_with_traps ::= block { trap_clause }* [ ensure_clause ]

trap_clause ::= 'trap' '(' IDENTIFIER '<' Type '>' ')' '->' block

ensure_clause ::= 'ensure' '->' block

fail_statement ::= 'fail' expression [';']

rethrow_statement ::= 'rethrow' [';']

panic_statement ::= 'panic' '(' STRING_LITERAL ')' [';']
```

---

## Examples

### Example 1: File Processing

```vyn
read_config(path<String>)<Config> -> {
    content<String> = {
        file<File> = open_file(path)
        {
            file.read_all()
        } ensure -> {
            file.close()
        }
    } trap (e<FileNotFound>) -> {
        println("Config not found, using defaults")
        return default_config_string()
    } trap (e<PermissionDenied>) -> {
        fail ConfigError { 
            message = "Cannot read config: permission denied",
            path = path
        }
    }
    
    config<Config> = parse_config(content)
    return config
}
```

### Example 2: Transaction Processing

```vyn
process_transaction(txn<Transaction>)<Result> -> {
    {
        validate_transaction(txn)
        apply_to_database(txn)
        notify_user(txn.user_id)
    } trap (e<ValidationError>) -> {
        println("Transaction invalid: " + e.message())
        return Result { success = false, reason = "validation" }
    } trap (e<DatabaseError>) -> {
        println("Database error: " + e.message())
        rollback_transaction(txn)
        rethrow  # Critical - propagate to caller
    } trap (e<NotificationError>) -> {
        println("Failed to notify user, but transaction succeeded")
        # Don't fail the transaction
    } ensure -> {
        log_transaction_attempt(txn)
    }
    
    return Result { success = true, reason = "" }
}
```

### Example 3: Parser with Error Recovery

```vyn
parse_expression(tokens<Vec<Token>>)<Expr> -> {
    expr<Expr> = {
        parse_term(tokens)
    } trap (e<UnexpectedToken>) -> {
        println("Unexpected token, attempting recovery...")
        skip_until_semicolon(tokens)
        fail ParseError { message = "Expression parsing failed" }
    } trap (e<UnexpectedEOF>) -> {
        fail ParseError { message = "Unexpected end of input" }
    }
    
    return expr
}
```

---

## Design Decisions

### Why Not Try/Catch?
- Try/catch is tired and verbose
- Unclear what code might throw
- No compile-time checking of exception types
- Separates error handling from the code that might fail

### Why Trap?
- **Explicit:** Trap clauses attached to blocks make error handling visible
- **Type-safe:** Compile-time checking of error types
- **Composable:** Easy to chain and nest
- **Fresh:** Unique to Vyn, not borrowed from other languages

### Why Ensure Instead of Finally?
- Shorter, clearer keyword
- Emphasizes "ensuring" cleanup happens
- No confusion with "finalize" or "finalizer" patterns

### Why Fail Instead of Throw?
- More descriptive of what's happening
- Avoids confusion with C++ exceptions
- Pairs nicely with "trap" conceptually

---

## Open Questions

1. **Error Type Hierarchy:** Composition vs inheritance vs aspects?
   - **Current preference:** Aspects (most flexible, fits Vyn philosophy)

2. **Automatic Error Propagation:** Should `?` operator auto-propagate?
   ```vyn
   value<Int> = risky_operation()?  # Auto-propagates failure
   ```
   - **Decision needed:** Explicit vs implicit propagation

3. **Multiple Error Types:** Can a block fail with multiple unrelated types?
   ```vyn
   fail NetworkError | FileError  # Union type?
   ```
   - **Decision needed:** Union types or require common base

4. **Stack Traces:** Should errors automatically capture stack traces?
   - **Tradeoff:** Performance vs debuggability

5. **Error Annotations:** Should functions declare what they can fail with?
   ```vyn
   risky_function()<Int> fails<NetworkError, TimeoutError> -> { ... }
   ```
   - **Decision needed:** Checked vs unchecked failures

---

## Implementation Roadmap

### Phase 1: Core Mechanism (v0.4.3)
- [ ] `fail` statement parsing and semantic analysis
- [ ] `trap` clause parsing and type checking
- [ ] Basic Error type and Errorable aspect
- [ ] Simple trap-based error handling (single trap)

### Phase 2: Advanced Features (v0.4.4)
- [ ] Multiple trap clauses with type dispatch
- [ ] `rethrow` statement
- [ ] `ensure` clause
- [ ] Integration with ownership system for cleanup

### Phase 3: Error Hierarchy (v0.4.5)
- [ ] Custom error types
- [ ] Error composition patterns
- [ ] Standard library errors (IOError, ParseError, etc.)

### Phase 4: Advanced Patterns (v0.5.0)
- [ ] Result<T, E> type in standard library
- [ ] `?` operator for auto-propagation
- [ ] Error annotations on function signatures
- [ ] Stack trace capture

---

## Testing Strategy

### Test Categories

1. **Basic Failure:** Simple fail/trap scenarios
2. **Type Dispatch:** Multiple traps with type hierarchy
3. **Propagation:** Rethrow and nested traps
4. **Cleanup:** Ensure clause execution order
5. **Integration:** Errors with ownership, async, generics
6. **Edge Cases:** Empty traps, unreachable traps, type mismatches

---

*"Errors are not exceptional - they're part of the program. Handle them with FREEDOM and clarity."*
