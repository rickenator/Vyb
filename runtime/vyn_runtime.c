// Vyn Runtime Library - Type Conversion Functions
// Comprehensive runtime support for Vyn type conversions

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

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
// COMPLEX TYPE CONVERSIONS: JSON serialization (stubs for now)
// ============================================================================

// Generic JSON serialization stub
// In a full implementation, this would use reflection/RTTI to serialize complex types
char* __vyn_complex_to_json(void* data) {
    (void)data;
    return strdup("{}");
}

// Generic JSON deserialization stub
// In a full implementation, this would parse JSON and construct the type
void* __vyn_complex_from_json(const char* json_str, const char* type_name) {
    (void)json_str;
    (void)type_name;
    return NULL;
}

