# VyB Async Programming and Debug System

VyB v0.4.0 introduces comprehensive async programming support with advanced debugging capabilities, making it one of the most debuggable async systems in modern programming languages.

## Overview

The VyB async system provides:
- **Native async/await syntax** with proper parsing and semantic analysis
- **Future<T> type system** for asynchronous operations
- **LLVM debug integration** with comprehensive metadata generation
- **State machine debugging** with suspension point tracking
- **Debug variable information** for async function parameters and locals

## Async Function Syntax

### Basic Async Functions

```vyb
// Simple async function returning a Future<Int>
async fetch_data()<Future<Int>> -> {
    result<Int> = 10
    println("Fetching data...")
    return result
}
```

### Async Functions with Await

```vyb
// Async function that awaits other async operations
async process_data()<Future<Int>> -> {
    data1<Int> = await fetch_data()    // Suspension point 1
    data2<Int> = await fetch_data()    // Suspension point 2

    combined<Int> = data1 + data2
    return combined
}
```

### Main Function Integration

```vyb
main()<Int> -> {
    println("=== Debug Async Test ===")
    result<Future<Int>> = process_data()
    return 0
}
```

## Debug Infrastructure

### LLVM Debug Integration

VyB's async debugging system integrates deeply with LLVM's debug infrastructure:

- **DIBuilder Integration**: Uses LLVM's DIBuilder for metadata generation
- **DICompileUnit**: Creates compile unit information for debug files
- **DISubprogram**: Generates function-level debug information
- **DILocalVariable**: Tracks local variables with type information
- **Debug Locations**: Precise source location tracking for all operations

### Async State Machine Debugging

The async debugging system provides comprehensive state machine introspection:

#### AsyncState Structure

```cpp
struct AsyncState {
    llvm::Value* stateVar;                           // Current state variable
    llvm::Value* futureVar;                          // Future result variable
    llvm::DILocalVariable* stateDebugVar;            // Debug info for state
    llvm::DILocalVariable* futureDebugVar;           // Debug info for future
    std::vector<llvm::DebugLoc> suspensionPointLocations;  // Suspension points
    std::map<int, std::string> stateDescriptions;    // Human-readable state names
    // ... other fields
};
```

#### Debug Method Implementations

**Initialization**:
```cpp
void initializeAsyncStateDebugInfo(const std::string& functionName, unsigned line)
```
- Creates debug variables for async state tracking
- Initializes suspension point location storage
- Sets up state descriptions for human-readable debugging

**Suspension Point Creation**:
```cpp
llvm::DebugLoc createSuspensionPointDebugInfo(unsigned line, unsigned column, int stateId)
```
- Creates debug location for each await expression
- Tracks suspension points for debugger integration
- Associates state IDs with source locations

**State Transition Debugging**:
```cpp
void insertAsyncStateTransitionDebugInfo(int fromState, int toState, llvm::DebugLoc loc)
```
- Logs state transitions during async execution
- Provides debugging context for state machine progression
- Enables step-through debugging of async operations

**Continuation Debugging**:
```cpp
void insertContinuationDebugMarker(int stateId, llvm::DebugLoc resumeLocation)
```
- Marks continuation points after suspension
- Enables debugging of async resume operations
- Tracks control flow restoration after await

### Debug Output Example

When running async code with debug information:

```
DEBUG: Initialized async state debug info for function 'fetch_data' at line 4
DEBUG: Created suspension point 1 (await_expression_1) at line 12 column 18
DEBUG: Async state infrastructure not initialized, skipping state storage
DEBUG: Continuation point for state 1 (await_expression_1) at line 12 column 18
DEBUG: Resuming from suspension point at line 12 column 18
```

## Technical Implementation

### Parser Integration

The async system integrates with VyB's parser:

- **Async keyword recognition**: `async` functions are properly parsed
- **Await expression parsing**: `await` expressions create proper AST nodes
- **Future type parsing**: `Future<T>` types are recognized and validated

### Semantic Analysis

Comprehensive semantic checking:

- **Async function validation**: Ensures async functions return Future<T>
- **Await expression type checking**: Validates await operand types
- **Control flow analysis**: Tracks async control flow patterns

### Code Generation

LLVM codegen with debug integration:

- **Async function generation**: Creates proper async function signatures
- **Await expression codegen**: Generates suspension and continuation code
- **Debug metadata emission**: Embeds comprehensive debug information

## Usage Examples

### Simple Async Operation

```vyb
// @test: Simple Async Function
// @description: Tests basic async function execution with debugging
// @category: async, debug
// @expect: pass

async simple_async()<Future<Int>> -> {
    value<Int> = 42
    return value
}

main()<Int> -> {
    result<Future<Int>> = simple_async()
    return 0
}
```

### Complex Async with Multiple Awaits

```vyb
// @test: Complex Async Debugging
// @description: Tests async state machine debugging with multiple suspension points
// @category: async, debug
// @expect: pass

async fetch_user_data(userId<Int>)<Future<String>> -> {
    // Suspension point 1: Fetch user info
    userInfo<String> = await fetch_user_info(userId)

    // Suspension point 2: Fetch user preferences
    preferences<String> = await fetch_user_preferences(userId)

    // Combine results
    combined<String> = userInfo + ":" + preferences
    return combined
}

async fetch_user_info(userId<Int>)<Future<String>> -> {
    return "user_" + String::from_int(userId)
}

async fetch_user_preferences(userId<Int>)<Future<String>> -> {
    return "prefs_" + String::from_int(userId)
}

main()<Int> -> {
    result<Future<String>> = fetch_user_data(123)
    return 0
}
```

## Debugging Workflow

### 1. Enable Debug Information

Compile with debug information enabled:
```bash
build/vyb --emit-llvm --debug-info test/async_test.vyb
```

### 2. Analyze Debug Output

The compiler emits detailed debug information:
- Suspension point creation
- State transition logging
- Continuation point tracking
- Variable scope information

### 3. Debug Execution

Use standard debugging tools with full async support:
- **GDB**: Step through async state machines
- **LLDB**: Inspect suspension points and continuations
- **Valgrind**: Memory analysis for async operations

## Advanced Features

### State Machine Introspection

The debug system provides deep insights into async state machines:

- **State enumeration**: Each suspension point gets a unique state ID
- **State descriptions**: Human-readable names for debugging
- **Transition tracking**: Logs all state changes with source locations
- **Variable scoping**: Tracks variable lifetime across suspensions

### Performance Debugging

Async performance analysis capabilities:

- **Suspension timing**: Track time spent in each async state
- **Memory usage**: Monitor async frame allocation and cleanup
- **Continuation overhead**: Measure async/await performance impact

### Integration with Test Harness

The async debugging system integrates with VyB's test harness:

```bash
# Run async tests with debugging
./test_harness.py --category async --verbose

# Generate debug reports
./test_harness.py --category async --html-report async_debug.html
```

## Future Enhancements

Planned improvements to the async debugging system:

### Enhanced Debugger Integration
- Visual async state machine representation
- Interactive suspension point exploration
- Async call stack visualization

### Performance Profiling
- Async operation timing analysis
- Memory allocation tracking for async frames
- Hot path identification in async code

### Advanced State Inspection
- Async variable value tracking across suspensions
- State machine graph visualization
- Deadlock detection for complex async patterns

## Best Practices

### Writing Debuggable Async Code

1. **Use descriptive variable names** in async functions
2. **Add debug annotations** to complex async operations
3. **Structure async operations** for clear state transitions
4. **Test async functions** individually before composition

### Debugging Async Issues

1. **Enable verbose debug output** during development
2. **Use the test harness** for systematic async testing
3. **Analyze state transitions** in failing async operations
4. **Verify suspension points** match expected control flow

## Conclusion

VyB's async debugging system represents a significant advancement in async programming language tooling. With comprehensive LLVM debug integration, state machine introspection, and modern test harness support, VyB provides developers with unprecedented visibility into async program execution.

The combination of clean async/await syntax, robust Future<T> types, and comprehensive debugging makes VyB an excellent choice for async systems programming where debugging and maintainability are critical.