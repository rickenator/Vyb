#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdio>

// Forward declarations for opaque types in generated code
extern "C" {

// String type used by Vyn (matches generated struct)
struct VynString {
    char* data;
    int64_t length;
};

// Source location information
struct VynSourceLocation {
    const char* file_path;
    uint32_t line;
    uint32_t column;
};

// Stack frame information
struct VynStackFrame {
    const char* function_name;
    VynSourceLocation location;
    void* native_address;  // For native stack trace
};

// Stack trace
struct VynStackTrace {
    VynStackFrame* frames;
    size_t frame_count;
    size_t capacity;
};

// Error value representation
struct VynError {
    const char* type_name;       // e.g., "DivisionByZero"
    void* type_id;               // Unique type identifier
    void* data;                  // Error-specific data
    size_t data_size;            // Size of data
    VynSourceLocation location;  // Where fail occurred
    VynStackTrace* stack_trace;  // Captured stack
    VynError* cause;             // Causality chain
    uint64_t timestamp;          // When error occurred
    uint64_t thread_id;          // Which thread
    void (*destructor)(void*);   // Cleanup for data
};

// Handler function type for custom untrapped error handling
typedef void (*VynUntrappedErrorHandler)(VynError* error, VynStackTrace* trace);

// ===== Runtime Function Declarations =====

// Panic - immediate termination (noreturn)
void __vyn_runtime_panic(const char* message) __attribute__((noreturn));

// Untrapped error - cleanup then exit (noreturn)
void __vyn_runtime_untrapped_error(VynError* error) __attribute__((noreturn));

// Error creation
VynError* __vyn_runtime_create_error(
    const char* type_name,
    void* type_id,
    void* data,
    size_t data_size,
    void (*destructor)(void*),
    VynSourceLocation location
);

// Error cleanup
void __vyn_runtime_free_error(VynError* error);
VynError* __vyn_runtime_clone_error(VynError* error);
void __vyn_runtime_set_error_cause(VynError* error, VynError* cause);

// Stack trace operations
VynStackTrace* __vyn_runtime_capture_stack_trace(size_t max_frames);
void __vyn_runtime_print_stack_trace(VynStackTrace* trace, FILE* output);
void __vyn_runtime_free_stack_trace(VynStackTrace* trace);

// Custom handler registration
void __vyn_runtime_set_untrapped_handler(VynUntrappedErrorHandler handler);
void __vyn_runtime_clear_untrapped_handler();
VynUntrappedErrorHandler __vyn_runtime_get_untrapped_handler();

// Type dispatch for trap clauses
bool __vyn_runtime_error_matches_type(
    VynError* error,
    const char* trap_type_name,
    void* trap_type_id
);
void* __vyn_runtime_cast_error(VynError* error, const char* target_type_name);

// Defer/ensure support (for future integration with ownership)
void __vyn_runtime_register_defer(void (*cleanup_fn)(void*), void* context);
void __vyn_runtime_execute_defers();
void __vyn_runtime_push_ensure_block(void (*ensure_fn)());
void __vyn_runtime_pop_ensure_block();

} // extern "C"
