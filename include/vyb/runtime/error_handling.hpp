#pragma once

#include <cstdint>
#include <cstddef>
#include <cstdio>

// Forward declarations for opaque types in generated code
extern "C" {

// String type used by VyB (matches generated struct)
struct VyBString {
    char* data;
    int64_t length;
};

// Source location information
struct VyBSourceLocation {
    const char* file_path;
    uint32_t line;
    uint32_t column;
};

// Stack frame information
struct VyBStackFrame {
    const char* function_name;
    VyBSourceLocation location;
    void* native_address;  // For native stack trace
};

// Stack trace
struct VyBStackTrace {
    VyBStackFrame* frames;
    size_t frame_count;
    size_t capacity;
};

// Error value representation
struct VyBError {
    uint64_t type_hash;          // Stable hash for runtime matching
    const char* type_name;       // e.g., "DivisionByZero"
    void* payload;               // Error-specific payload bytes
    const char* file;            // Source file of fail site
    uint32_t line;               // Source line of fail site
    uint32_t col;                // Source column of fail site
    // Legacy aliases still used by runtime internals
    void* type_id;               // Unique type identifier (legacy)
    void* data;                  // Alias of payload
    size_t data_size;            // Size of payload
    VyBSourceLocation location;  // Alias of file/line/col
    VyBStackTrace* stack_trace;  // Captured stack
    VyBError* cause;             // Causality chain
    uint64_t timestamp;          // When error occurred
    uint64_t thread_id;          // Which thread
    void (*destructor)(void*);   // Cleanup for data
};

// Handler function type for custom untrapped error handling
typedef void (*VyBUntrappedErrorHandler)(VyBError* error, VyBStackTrace* trace);

// ===== Runtime Function Declarations =====

// Panic - immediate termination (noreturn)
void __vyb_runtime_panic(const char* message) __attribute__((noreturn));

// Untrapped error - cleanup then exit (noreturn)
void __vyb_runtime_untrapped_error(VyBError* error) __attribute__((noreturn));

// Error creation
VyBError* __vyb_runtime_create_error(
    const char* type_name,
    void* type_id,
    void* data,
    size_t data_size,
    void (*destructor)(void*),
    VyBSourceLocation location
);

VyBError* __vyb_runtime_create_error_ex(
    const char* type_name,
    void* type_id,
    void* data,
    size_t data_size,
    void (*destructor)(void*),
    const char* file,
    uint32_t line,
    uint32_t column
);

// Error cleanup
void __vyb_runtime_free_error(VyBError* error);
VyBError* __vyb_runtime_clone_error(VyBError* error);
void __vyb_runtime_set_error_cause(VyBError* error, VyBError* cause);

// Stack trace operations
VyBStackTrace* __vyb_runtime_capture_stack_trace(size_t max_frames);
void __vyb_runtime_print_stack_trace(VyBStackTrace* trace, FILE* output);
void __vyb_runtime_free_stack_trace(VyBStackTrace* trace);

// Call stack management for VyB-level stack traces (Phase 6.4)
void __vyb_runtime_push_call_frame(const char* function_name, const char* file_path, uint32_t line, uint32_t column);
void __vyb_runtime_pop_call_frame();
VyBStackTrace* __vyb_runtime_get_current_stack_trace();

// Custom handler registration
void __vyb_runtime_set_untrapped_handler(VyBUntrappedErrorHandler handler);
void __vyb_runtime_clear_untrapped_handler();
VyBUntrappedErrorHandler __vyb_runtime_get_untrapped_handler();

// Type dispatch for trap clauses
bool __vyb_runtime_error_matches_type(
    VyBError* error,
    const char* trap_type_name,
    void* trap_type_id
);
void* __vyb_runtime_cast_error(VyBError* error, const char* target_type_name);

// Defer/ensure support (for future integration with ownership)
void __vyb_runtime_register_defer(void (*cleanup_fn)(void*), void* context);
void __vyb_runtime_execute_defers();
void __vyb_runtime_push_ensure_block(void (*ensure_fn)());
void __vyb_runtime_pop_ensure_block();

} // extern "C"
