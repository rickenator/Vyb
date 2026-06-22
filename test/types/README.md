# Primitive Type Tests

This directory contains comprehensive tests for all VyB primitive types.

## Test Files

### Integer Types
- **`sized_integers_test.vyb`** - Tests signed integers: Int8, Int16, Int32, Int64 (Int)
- **`unsigned_integers_test.vyb`** - Tests unsigned integers: UInt8, UInt16, UInt32, UInt64

### Floating Point Types
- **`float_types_test.vyb`** - Tests Float32 and Float64 (Float)

### Character Types
- **`char_types_test.vyb`** - Tests Char (UTF-8 code unit) and Rune (Unicode code point)

### Binary Data
- **`bytes_type_test.vyb`** - Tests Bytes type for raw binary data

### Comprehensive
- **`all_primitives_test.vyb`** - Tests all primitive types together

## Type System Reference

### Signed Integers
| Type | LLVM | Size | Range |
|------|------|------|-------|
| Int8 | i8 | 8-bit | -128 to 127 |
| Int16 | i16 | 16-bit | -32,768 to 32,767 |
| Int32 | i32 | 32-bit | -2,147,483,648 to 2,147,483,647 |
| Int (Int64) | i64 | 64-bit | -9,223,372,036,854,775,808 to 9,223,372,036,854,775,807 |

### Unsigned Integers
| Type | LLVM | Size | Range |
|------|------|------|-------|
| UInt8 | i8 | 8-bit | 0 to 255 |
| UInt16 | i16 | 16-bit | 0 to 65,535 |
| UInt32 | i32 | 32-bit | 0 to 4,294,967,295 |
| UInt64 | i64 | 64-bit | 0 to 18,446,744,073,709,551,615 |

*Note: At LLVM IR level, signed and unsigned integers use the same types. The distinction is semantic and affects operations like comparisons and arithmetic.*

### Floating Point
| Type | LLVM | Size | Precision |
|------|------|------|-----------|
| Float32 | float | 32-bit | IEEE 754 single precision (~7 decimal digits) |
| Float (Float64) | double | 64-bit | IEEE 754 double precision (~15 decimal digits) |

### Character Types
| Type | LLVM | Size | Purpose |
|------|------|------|---------|
| Char | i8 | 8-bit | Single UTF-8 code unit (byte) |
| Rune | i32 | 32-bit | Full Unicode code point (supports all Unicode) |

### Binary Data
| Type | Structure | Purpose |
|------|-----------|---------|
| Bytes | `{ ptr: *u8, len: i64 }` | Raw binary data with length (similar to String but for bytes) |

## Running Tests

```bash
# Run all type tests
for test in test/types/*.vyb; do
    echo "Testing: $test"
    build/vyb "$test"
done

# Run specific test
build/vyb test/types/sized_integers_test.vyb  # Should exit with code 120
build/vyb test/types/char_types_test.vyb      # Should exit with code 313
```

## Expected Results

- **sized_integers_test.vyb**: Exit code 120 (8 + 16 + 32 + 64)
- **unsigned_integers_test.vyb**: Exit code 120 (8 + 16 + 32 + 64)
- **float_types_test.vyb**: Exit code 96 (32 + 64)
- **char_types_test.vyb**: Exit code 313 (309 + 4)
- **bytes_type_test.vyb**: Exit code 42
- **all_primitives_test.vyb**: Exit code 120 (8 + 16 + 32 + 64)

## Implementation Notes

### Type Aliases
All types support multiple naming conventions:
- VyB style: `Int32`, `Float64`, `UInt8`
- C style: `int32`, `float64`, `uint8`
- LLVM style: `i32`, `f64`, `u8`

### LLVM Representation
- Integers map to corresponding LLVM integer types (`i8`, `i16`, `i32`, `i64`)
- Floats map to LLVM floating point types (`float`, `double`)
- Char maps to `i8` (single byte)
- Rune maps to `i32` (4 bytes, enough for any Unicode code point)
- Bytes is a fat pointer struct like String: `{ *i8, i64 }`

### Future Enhancements
- Byte array literals: `data<Bytes> = [0xDE, 0xAD, 0xBE, 0xEF]`
- Character literals: `ch<Char> = 'A'`, `rune<Rune> = '💡'`
- Explicit signedness in literals: `42u32`, `255u8`
- Type suffix notation: `3.14f32` for Float32
- Min/max constants: `Int32::MAX`, `UInt8::MIN`

## Type Safety

All types are statically checked at compile time:
- Type mismatches are caught during semantic analysis
- Implicit conversions require explicit casts (planned)
- Overflow behavior is defined (wrapping for unsigned, undefined for signed)

The type system ensures memory safety and prevents common bugs like integer overflow vulnerabilities.
