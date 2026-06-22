// VyB Type Metadata System
// Runtime type information for JSON serialization/deserialization and aspect dispatch

#ifndef VYB_TYPE_METADATA_H
#define VYB_TYPE_METADATA_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

// Field metadata - describes each field in a struct
typedef struct VyBFieldMetadata {
    const char* name;           // Field name (e.g., "count")
    const char* type_name;      // Type name (e.g., "Int", "String", "Person")
    size_t offset;              // Offset in struct (bytes from start)
    size_t size;                // Size of field in bytes
    bool is_primitive;          // true for Int/Float/Bool/String, false for custom structs
    bool is_vec;                // true if this is a Vec<T>
    const char* vec_element_type; // If is_vec, what's the element type
} VyBFieldMetadata;

// Aspect method metadata - describes a method in an aspect
typedef struct VyBAspectMethod {
    const char* name;           // Method name (e.g., "show")
    void* function_ptr;         // Pointer to the actual implementation
    const char* return_type;    // Return type name
} VyBAspectMethod;

// Aspect binding metadata - describes a bound aspect for a type
typedef struct VyBAspectBinding {
    const char* aspect_name;    // Aspect name (e.g., "Display")
    size_t num_methods;         // Number of methods
    VyBAspectMethod* methods;   // Array of methods
} VyBAspectBinding;

// Type metadata - complete description of a struct type
typedef struct VyBTypeMetadata {
    const char* type_name;      // Type name (e.g., "Counter", "Person")
    size_t struct_size;         // Total size of struct in bytes
    size_t num_fields;          // Number of fields
    VyBFieldMetadata* fields;   // Array of field metadata
    size_t num_aspects;         // Number of bound aspects
    VyBAspectBinding* aspects;  // Array of aspect bindings
} VyBTypeMetadata;

// Type registry - global map from type name to metadata
// This will be populated at program startup by generated registration code
void __vyb_register_type(VyBTypeMetadata* metadata);
VyBTypeMetadata* __vyb_lookup_type(const char* type_name);

// JSON serialization using type metadata
char* __vyb_complex_to_json_with_metadata(void* instance, VyBTypeMetadata* metadata);

// JSON deserialization using type metadata
void* __vyb_complex_from_json_with_metadata(const char* json_str, VyBTypeMetadata* metadata);

#endif // VYB_TYPE_METADATA_H
