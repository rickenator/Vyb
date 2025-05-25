#include <iostream> // For std::cout, std::endl
#include <string>
#include <vector>
#include <cstring> // For strlen, strcpy, strcat
#include <cstdlib> // For malloc

namespace vyn {
namespace intrinsics {

/**
 * RUNTIME INTRINSICS
 * 
 * This file contains implementations of runtime intrinsics for the Vyn language.
 * These intrinsics are functions that are directly recognized and called by the runtime.
 *
 * NOTE: Memory operations such as loc(), at(), addr(), and from<loc<T>>() are NOT
 * implemented here. They are compiler intrinsics that are directly translated to LLVM
 * IR during code generation in src/vre/llvm/cgen_expr.cpp. These operations do not
 * have runtime function implementations.
 *
 * MEMORY OPERATION IMPLEMENTATION LOCATIONS:
 * - AST Nodes: include/vyn/parser/ast.hpp (LocationExpression, PointerDerefExpression, etc.)
 * - Semantic Analysis: src/vre/semantic.cpp (type checking & unsafe block verification)
 * - Code Generation: src/vre/llvm/cgen_expr.cpp (LLVM IR generation)
 */

// Console output intrinsic - this is an actual runtime function
void println(const std::string& output) {
    std::cout << output << std::endl;
}

// Println intrinsic for Vyn - handles string output and auto-serialization
extern "C" void __vyn_println(const char* str) {
    if (!str) {
        std::cout << "null" << std::endl;
    } else {
        std::cout << str << std::endl;
    }
}

// Serialization helper for auto-serializing complex types to JSON
extern "C" char* __vyn_serialize_to_json(void* obj, const char* type_name) {
    // This is a placeholder for actual JSON serialization
    // In a full implementation, this would use reflection or type information
    // to convert the object to a JSON representation
    
    if (!obj) {
        const char* nullJson = "null";
        char* result = (char*)malloc(strlen(nullJson) + 1);
        if (result) strcpy(result, nullJson);
        return result;
    }
    
    // For demonstration purposes, create a simple JSON representation
    // In a real implementation, this would use the type information to build
    // a proper JSON structure based on the object's fields
    std::string jsonStr = "{ \"type\": \"";
    jsonStr += type_name ? type_name : "unknown";
    jsonStr += "\", \"address\": \"";
    
    // Convert address to hex string
    char addrBuf[32];
    snprintf(addrBuf, sizeof(addrBuf), "%p", obj);
    jsonStr += addrBuf;
    jsonStr += "\" }";
    
    // Return a heap-allocated copy of the string
    char* result = (char*)malloc(jsonStr.length() + 1);
    if (result) strcpy(result, jsonStr.c_str());
    return result;
}

// String concatenation intrinsic
extern "C" char* __vyn_string_concat(const char* left, const char* right) {
    if (!left) left = "";
    if (!right) right = "";
    
    // Calculate the combined length
    size_t leftLen = strlen(left);
    size_t rightLen = strlen(right);
    size_t totalLen = leftLen + rightLen + 1;  // +1 for null terminator
    
    // Allocate memory for the new string
    char* result = (char*)malloc(totalLen);
    if (!result) {
        // Handle allocation failure
        std::cerr << "Memory allocation failed for string concatenation" << std::endl;
        return nullptr;
    }
    
    // Copy the strings and add null terminator
    strcpy(result, left);
    strcat(result, right);
    
    return result;
}

// Future runtime intrinsics may be added here

} // namespace intrinsics
} // namespace vyn
