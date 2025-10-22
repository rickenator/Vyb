// Vyn Runtime Library
// Minimal runtime support for Vyn executables

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Call frame tracking (minimal implementation for now)
void __vyn_runtime_push_call_frame(const char* func_name, const char* file, int32_t line, int32_t col) {
    // For now, do nothing - this is for debugging/stack traces
    (void)func_name;
    (void)file;
    (void)line;
    (void)col;
}

void __vyn_runtime_pop_call_frame(void) {
    // For now, do nothing
}

// Print function
void __vyn_println(const char* str) {
    printf("%s\n", str);
}

// Serialization stub (for future use)
char* __vyn_serialize_to_json(void* data) {
    (void)data;
    return strdup("{}");
}

// String conversion stub
char* __vyn_convert_lit_string(const char* str) {
    return strdup(str);
}
