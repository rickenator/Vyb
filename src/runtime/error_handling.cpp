#include "vyn/runtime/error_handling.hpp"
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <atomic>
#include <thread>
#include <mutex>
#include <vector>

// Platform-specific includes for stack trace
#ifdef __linux__
#include <execinfo.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <windows.h>
#include <dbghelp.h>
#else
// macOS
#include <execinfo.h>
#include <unistd.h>
#endif

extern "C" {

// ===== Global State =====

static std::atomic<VynUntrappedErrorHandler> g_custom_handler{nullptr};

// ===== Helper Functions =====

static uint64_t get_timestamp_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

static uint64_t get_thread_id() {
    return std::hash<std::thread::id>{}(std::this_thread::get_id());
}

static void format_timestamp(uint64_t timestamp_ns, char* buffer, size_t bufsize) {
    time_t seconds = timestamp_ns / 1000000000ULL;
    long nanoseconds = timestamp_ns % 1000000000ULL;
    struct tm* tm_info = localtime(&seconds);
    snprintf(buffer, bufsize, "%04d-%02d-%02d %02d:%02d:%02d.%03ld",
             tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
             tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec,
             nanoseconds / 1000000);
}

// ===== Stack Trace Implementation =====

VynStackTrace* __vyn_runtime_capture_stack_trace(size_t max_frames) {
    VynStackTrace* trace = new VynStackTrace;
    trace->capacity = max_frames;
    trace->frames = new VynStackFrame[max_frames];
    trace->frame_count = 0;
    
#if defined(__linux__) || defined(__APPLE__)
    // Use backtrace
    void** addresses = new void*[max_frames];
    int count = backtrace(addresses, max_frames);
    
    // For now, just store addresses
    // Full symbol resolution would require DWARF parsing
    for (int i = 0; i < count; i++) {
        trace->frames[i].function_name = "<native>";
        trace->frames[i].location.file_path = "<unknown>";
        trace->frames[i].location.line = 0;
        trace->frames[i].location.column = 0;
        trace->frames[i].native_address = addresses[i];
        trace->frame_count++;
    }
    
    delete[] addresses;
#elif defined(_WIN32)
    // Windows stack trace
    void** addresses = new void*[max_frames];
    USHORT count = CaptureStackBackTrace(0, max_frames, addresses, NULL);
    
    for (USHORT i = 0; i < count; i++) {
        trace->frames[i].function_name = "<native>";
        trace->frames[i].location.file_path = "<unknown>";
        trace->frames[i].location.line = 0;
        trace->frames[i].location.column = 0;
        trace->frames[i].native_address = addresses[i];
        trace->frame_count++;
    }
    
    delete[] addresses;
#else
    // No stack trace support
    trace->frames[0].function_name = "<stack trace not available>";
    trace->frames[0].location.file_path = "";
    trace->frames[0].location.line = 0;
    trace->frames[0].location.column = 0;
    trace->frames[0].native_address = nullptr;
    trace->frame_count = 1;
#endif
    
    return trace;
}

void __vyn_runtime_print_stack_trace(VynStackTrace* trace, FILE* output) {
    fprintf(output, "\nStack Trace:\n");
    if (!trace || trace->frame_count == 0) {
        fprintf(output, "  <no stack trace available>\n");
        return;
    }
    
    for (size_t i = 0; i < trace->frame_count; i++) {
        const VynStackFrame& frame = trace->frames[i];
        if (frame.location.line > 0) {
            fprintf(output, "  at %s (%s:%u:%u)\n",
                    frame.function_name,
                    frame.location.file_path,
                    frame.location.line,
                    frame.location.column);
        } else {
            fprintf(output, "  at %s (address: %p)\n",
                    frame.function_name,
                    frame.native_address);
        }
    }
}

void __vyn_runtime_free_stack_trace(VynStackTrace* trace) {
    if (!trace) return;
    delete[] trace->frames;
    delete trace;
}

// ===== Error Management =====

VynError* __vyn_runtime_create_error(
    const char* type_name,
    void* type_id,
    void* data,
    size_t data_size,
    void (*destructor)(void*),
    VynSourceLocation location
) {
    VynError* error = new VynError;
    error->type_name = type_name;
    error->type_id = type_id;
    error->data_size = data_size;
    error->destructor = destructor;
    error->location = location;
    error->cause = nullptr;
    error->timestamp = get_timestamp_ns();
    error->thread_id = get_thread_id();
    
    // Copy error data
    if (data && data_size > 0) {
        error->data = malloc(data_size);
        memcpy(error->data, data, data_size);
    } else {
        error->data = nullptr;
    }
    
    // Capture stack trace
    error->stack_trace = __vyn_runtime_capture_stack_trace(50);
    
    return error;
}

void __vyn_runtime_free_error(VynError* error) {
    if (!error) return;
    
    // Call custom destructor if provided
    if (error->destructor && error->data) {
        error->destructor(error->data);
    }
    
    // Free data
    if (error->data) {
        free(error->data);
    }
    
    // Free stack trace
    __vyn_runtime_free_stack_trace(error->stack_trace);
    
    // Recursively free cause chain
    if (error->cause) {
        __vyn_runtime_free_error(error->cause);
    }
    
    delete error;
}

VynError* __vyn_runtime_clone_error(VynError* error) {
    if (!error) return nullptr;
    
    VynError* clone = new VynError(*error);
    
    // Deep copy data
    if (error->data && error->data_size > 0) {
        clone->data = malloc(error->data_size);
        memcpy(clone->data, error->data, error->data_size);
    }
    
    // Clone stack trace
    if (error->stack_trace) {
        clone->stack_trace = new VynStackTrace;
        clone->stack_trace->capacity = error->stack_trace->capacity;
        clone->stack_trace->frame_count = error->stack_trace->frame_count;
        clone->stack_trace->frames = new VynStackFrame[error->stack_trace->capacity];
        memcpy(clone->stack_trace->frames, error->stack_trace->frames,
               error->stack_trace->frame_count * sizeof(VynStackFrame));
    }
    
    // Clone cause chain
    if (error->cause) {
        clone->cause = __vyn_runtime_clone_error(error->cause);
    }
    
    return clone;
}

void __vyn_runtime_set_error_cause(VynError* error, VynError* cause) {
    if (!error) return;
    
    // Free existing cause if present
    if (error->cause) {
        __vyn_runtime_free_error(error->cause);
    }
    
    error->cause = cause;
}

// ===== Type Dispatch =====

bool __vyn_runtime_error_matches_type(
    VynError* error,
    const char* trap_type_name,
    void* trap_type_id
) {
    if (!error || !trap_type_name) return false;
    
    // Simple string comparison for now
    // TODO: Add aspect-based matching
    return strcmp(error->type_name, trap_type_name) == 0;
}

void* __vyn_runtime_cast_error(VynError* error, const char* target_type_name) {
    if (!error || !target_type_name) return nullptr;
    
    if (strcmp(error->type_name, target_type_name) == 0) {
        return error->data;
    }
    
    return nullptr;
}

// ===== Custom Handler Management =====

void __vyn_runtime_set_untrapped_handler(VynUntrappedErrorHandler handler) {
    g_custom_handler.store(handler);
}

void __vyn_runtime_clear_untrapped_handler() {
    g_custom_handler.store(nullptr);
}

VynUntrappedErrorHandler __vyn_runtime_get_untrapped_handler() {
    return g_custom_handler.load();
}

// ===== Panic Implementation =====

void __vyn_runtime_panic(const char* message) {
    char timestamp_buf[64];
    format_timestamp(get_timestamp_ns(), timestamp_buf, sizeof(timestamp_buf));
    
    char thread_buf[128];
    snprintf(thread_buf, sizeof(thread_buf), "Thread: %llu", (unsigned long long)get_thread_id());
    
    char time_buf[128];
    snprintf(time_buf, sizeof(time_buf), "Time: %s", timestamp_buf);
    
    fprintf(stderr, "\n");
    fprintf(stderr, "┌─ PANIC ──────────────────────────────────────────────────────┐\n");
    fprintf(stderr, "│ %-61s│\n", thread_buf);
    fprintf(stderr, "│ %-61s│\n", time_buf);
    fprintf(stderr, "└──────────────────────────────────────────────────────────────┘\n");
    fprintf(stderr, "\n");
    
    // Print message (null-terminated C string)
    if (message) {
        fprintf(stderr, "Message: %s\n", message);
    } else {
        fprintf(stderr, "Message: <no message>\n");
    }
    
    fprintf(stderr, "\nABORTING\n");
    fflush(stderr);
    
    abort();
}

// ===== Untrapped Error Handler =====

void __vyn_runtime_untrapped_error(VynError* error) {
    char timestamp_buf[64];
    format_timestamp(get_timestamp_ns(), timestamp_buf, sizeof(timestamp_buf));
    
    fprintf(stderr, "\n");
    fprintf(stderr, "┌─ UNTRAPPED FAILURE ──────────────────────────────────────────┐\n");
    
    char line_buf[128];
    
    // For now, just print generic error - proper VynError support needs more work
    snprintf(line_buf, sizeof(line_buf), "Error: <runtime error>");
    fprintf(stderr, "│ %-61s│\n", line_buf);
    
    snprintf(line_buf, sizeof(line_buf), "Thread: %llu", (unsigned long long)get_thread_id());
    fprintf(stderr, "│ %-61s│\n", line_buf);
    
    snprintf(line_buf, sizeof(line_buf), "Time: %s", timestamp_buf);
    fprintf(stderr, "│ %-61s│\n", line_buf);
    
    fprintf(stderr, "└──────────────────────────────────────────────────────────────┘\n");
    
    // Print stack trace if available
    if (error) {
        if (error->stack_trace) {
            __vyn_runtime_print_stack_trace(error->stack_trace, stderr);
        }
        
        // Call custom handler if registered
        VynUntrappedErrorHandler custom_handler = g_custom_handler.load();
        if (custom_handler) {
            fprintf(stderr, "\nCalling custom error handler...\n");
            custom_handler(error, error->stack_trace);
        }
        
        // Cleanup error
        __vyn_runtime_free_error(error);
    } else {
        fprintf(stderr, "\nNote: Error details not available (error structure not yet implemented)\n");
    }
    
    fprintf(stderr, "\nExit Code: 1\n");
    fflush(stderr);
    
    exit(1);
}

// ===== Defer/Ensure Support (Stubs for now) =====

void __vyn_runtime_register_defer(void (*cleanup_fn)(void*), void* context) {
    // TODO: Implement defer stack
}

void __vyn_runtime_execute_defers() {
    // TODO: Execute defers in LIFO order
}

void __vyn_runtime_push_ensure_block(void (*ensure_fn)()) {
    // TODO: Implement ensure stack
}

void __vyn_runtime_pop_ensure_block() {
    // TODO: Pop ensure stack
}

} // extern "C"
