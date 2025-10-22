# Stack Trace Tests - Examples and Analysis

## Overview
These tests demonstrate Phase 6.4 (v0.5.1) stack trace capture implementation in Vyn.

## Test Files

### 1. `test/stack_trace_nested.vyn` - Basic 3-Level Call Stack
**Purpose**: Demonstrates simple nested function calls with error at deepest level.

**Call chain**: `main() → level1() → level2() → level3() [fails here]`

**Key features**:
- Each function pushes its frame on entry
- Error occurs in deepest function (level3)
- Stack trace would show: main, level1, level2, level3

### 2. `test/stack_trace_deep.vyn` - Deep 5-Level Call Stack
**Purpose**: Tests deeper call stacks to ensure proper frame management.

**Call chain**: `main() → level1() → level2() → level3() → level4() → level5() [fails here]`

**Key features**:
- Tests stack depth handling (MAX_DEPTH = 256)
- Demonstrates thread-safe stack management
- Each level properly instruments push/pop calls

### 3. `test/stack_trace_multiple_paths.vyn` - Different Error Locations
**Purpose**: Shows errors can occur at different points in the call tree.

**Two paths**:
1. Path A: Error in nested call (`pathA() → willFail()`)
2. Path B: Error in caller after successful nested call

**Key features**:
- Conditional failure based on parameter
- Shows stack traces differ based on where error occurs

### 4. `test/stack_trace_with_data.vyn` - Contextual Information
**Purpose**: Demonstrates stack traces with meaningful function names that provide context.

**Call chain**: `main() → handleRequest() → validateAndProcess() → processValue() [may fail]`

**Key features**:
- Function names describe what's happening
- Multiple failure conditions (value < 0 or value > 100)
- Stack trace helps identify which validation failed

## Generated LLVM IR Structure

Each function has this pattern:

```llvm
define { i64, ptr } @functionName() !dbg !N {
entry:
  ; 1. PUSH call frame with function name and source location
  call void @__vyn_runtime_push_call_frame(ptr @funcname.str, ptr @filepath.str, i32 LINE, i32 COL)
  
  ; 2. Function body executes here
  ; ...
  
  ; 3. POP call frame before any return (success or error)
  call void @__vyn_runtime_pop_call_frame()
  ret { i64, ptr } %result
}
```

## Runtime Functions

**Push frame** (on function entry):
```c
void __vyn_runtime_push_call_frame(
    const char* function_name,
    const char* file_path,
    uint32_t line,
    uint32_t column
);
```

**Pop frame** (on function exit):
```c
void __vyn_runtime_pop_call_frame();
```

**Get stack trace** (when error created):
```c
VynStackTrace* __vyn_runtime_get_current_stack_trace();
```

## Stack Trace Information Captured

Each frame in the stack trace contains:
- **Function name**: Vyn-level name (not mangled C++ name)
- **File path**: Source file where function is defined
- **Line number**: Line where function declaration starts
- **Column number**: Column where function declaration starts

## Example Stack Trace Output

When an error occurs in `level3()` called from `level2()` called from `level1()` called from `main()`:

```
Stack trace (from innermost to outermost):
  at level3 (test/stack_trace_nested.vyn:4:1)
  at level2 (test/stack_trace_nested.vyn:10:1)
  at level1 (test/stack_trace_nested.vyn:15:1)
  at main (test/stack_trace_nested.vyn:20:1)
```

## Verification

Run the demonstration scripts:
```bash
./test/show_stack_traces.sh      # Shows overview of all tests
./test/show_detailed_ir.sh        # Shows detailed LLVM IR structure
```

## Technical Implementation

1. **Global call stack**: Thread-safe vector of frames (max 256 depth)
2. **Codegen integration**: Automatic push/pop at function entry/exit
3. **Error integration**: Stack trace captured when error is created
4. **JIT symbol resolution**: Runtime functions registered with ORC JIT

## Benefits

✅ **Accurate debugging**: Shows Vyn function names, not native addresses  
✅ **Source locations**: Exact file/line/column information  
✅ **Multi-level**: Captures entire call chain  
✅ **Thread-safe**: Mutex-protected global stack  
✅ **Automatic**: No manual instrumentation required  

## Status

**Phase 6.4 (v0.5.1) - Stack Trace Capture: ✅ COMPLETE**
