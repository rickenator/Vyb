// Vyn Runtime Library - Type Conversion Functions
// Comprehensive runtime support for Vyn type conversions

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#if defined(__GNUC__) || defined(__clang__)
#define VYN_WEAK __attribute__((weak))
#else
#define VYN_WEAK
#endif

// ============================================================================
// CORE RUNTIME SHIMS USED BY NATIVE BUILDS
// ============================================================================

VYN_WEAK void __vyn_println(const char* str) {
    fputs(str ? str : "", stdout);
    fputc('\n', stdout);
}

VYN_WEAK void __vyn_print(const char* str) {
    fputs(str ? str : "", stdout);
}

VYN_WEAK void __vyn_println_int(int64_t value) {
    printf("%lld\n", (long long)value);
}

VYN_WEAK void __vyn_print_int(int64_t value) {
    printf("%lld", (long long)value);
}

VYN_WEAK void __vyn_println_bool(int64_t value) {
    puts(value ? "true" : "false");
}

VYN_WEAK void __vyn_print_bool(int64_t value) {
    fputs(value ? "true" : "false", stdout);
}

VYN_WEAK void __vyn_runtime_push_call_frame(const char* function_name, const char* file_path, uint32_t line, uint32_t column) {
    (void)function_name;
    (void)file_path;
    (void)line;
    (void)column;
}

VYN_WEAK void __vyn_runtime_pop_call_frame(void) {}

// ============================================================================
// PRIMITIVE TYPE CONVERSIONS: to_string()
// ============================================================================

// Int to String conversion
char* __vyn_int_to_string(int64_t value) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%ld", (long)value);
    return strdup(buffer);
}

// Float to String conversion
char* __vyn_float_to_string(double value) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%g", value);
    return strdup(buffer);
}

// Bool to String conversion
char* __vyn_bool_to_string(bool value) {
    return strdup(value ? "true" : "false");
}

// String to String (identity, but creates a copy)
char* __vyn_string_to_string(const char* str) {
    return strdup(str ? str : "");
}

// ============================================================================
// PRIMITIVE TYPE CONVERSIONS: from_string()
// ============================================================================

// String to Int conversion
int64_t __vyn_int_from_string(const char* str, bool* success) {
    if (!str || !*str) {
        *success = false;
        return 0;
    }
    
    char* endptr;
    errno = 0;
    long long value = strtoll(str, &endptr, 10);
    
    if (errno != 0 || *endptr != '\0') {
        *success = false;
        return 0;
    }
    
    *success = true;
    return (int64_t)value;
}

// String to Float conversion
double __vyn_float_from_string(const char* str, bool* success) {
    if (!str || !*str) {
        *success = false;
        return 0.0;
    }
    
    char* endptr;
    errno = 0;
    double value = strtod(str, &endptr);
    
    if (errno != 0 || *endptr != '\0') {
        *success = false;
        return 0.0;
    }
    
    *success = true;
    return value;
}

// String to Bool conversion
bool __vyn_bool_from_string(const char* str, bool* success) {
    if (!str) {
        *success = false;
        return false;
    }
    
    if (strcmp(str, "true") == 0 || strcmp(str, "1") == 0) {
        *success = true;
        return true;
    }
    
    if (strcmp(str, "false") == 0 || strcmp(str, "0") == 0) {
        *success = true;
        return false;
    }
    
    *success = false;
    return false;
}

// String to String (identity with validation)
char* __vyn_string_from_string(const char* str, bool* success) {
    if (!str) {
        *success = false;
        return strdup("");
    }
    
    *success = true;
    return strdup(str);
}

// ============================================================================
// COMPLEX TYPE CONVERSIONS: JSON serialization using type metadata
// ============================================================================

#include "vyn_type_metadata.h"

// Generic JSON serialization using type metadata
char* __vyn_complex_to_json(void* instance, const char* type_name) {
    VynTypeMetadata* metadata = __vyn_lookup_type(type_name);
    if (!metadata) {
        fprintf(stderr, "Error: Type '%s' not found in registry\n", type_name);
        return strdup("{}");
    }
    return __vyn_complex_to_json_with_metadata(instance, metadata);
}

// Generic JSON deserialization using type metadata
void* __vyn_complex_from_json(const char* json_str, const char* type_name) {
    VynTypeMetadata* metadata = __vyn_lookup_type(type_name);
    if (!metadata) {
        fprintf(stderr, "Error: Type '%s' not found in registry\n", type_name);
        return NULL;
    }
    return __vyn_complex_from_json_with_metadata(json_str, metadata);
}
