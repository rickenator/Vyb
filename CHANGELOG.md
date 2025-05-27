# Vyn Programming Language - Changelog

All notable changes to the Vyn programming language will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [0.3.5] - 2025-05-26

### Added
- **Comprehensive Auto-Serialization Capabilities**: Added full support for automatic serialization of structured data types when returned from `main()` functions
  - New serialization mode intrinsics: `lit()`, `notype()`, `bare()`, `deserial()`
  - JSON construction intrinsics: `__vyn_serialize_to_json()`, `__vyn_serialize_struct_with_names()`
  - Array and object construction functions for manual JSON building
  - Automatic activation for structured return values from `main()`
  - Comprehensive documentation in `doc/Intrinsics.md` Section 7

### Improved
- **Enhanced Parser Error Handling**: Improved error messages and handling for common syntax mistakes
- **Documentation Updates**: 
  - Updated all version references from 0.3.4 to 0.3.5
  - Enhanced feature descriptions in README.md
  - Comprehensive auto-serialization documentation added to intrinsics guide
  - Updated installation guide to reference v0.3.5

### Fixed
- Parser error handling for malformed syntax constructs
- Documentation consistency across all files

### Documentation
- Updated `README.md` with enhanced feature descriptions and v0.3.5 installation guide
- Updated `doc/AST_Declarations.md` version reference to v0.3.5
- Comprehensive auto-serialization documentation added to `doc/Intrinsics.md`
- Updated project version in `CMakeLists.txt`

### Supporting Tests
The following test files validate the v0.3.5 functionality:

#### Auto-Serialization Tests
- **`test/test_auto_serialize_basic.vyn`**: Basic auto-serialization without intrinsics (multi-value return)
- **`test/test_lit_intrinsic_simple.vyn`**: Simple `lit()` intrinsic for raw JSON literal output
- **`test/test_lit_intrinsic_multiple.vyn`**: Multiple `lit()` intrinsics generating JSON array output
- **`test/test_notype_intrinsic.vyn`**: Error handling test for `notype()` with primitives (should fail)
- **`test/test_notype_struct.vyn`**: Proper `notype()` usage with structs for metadata suppression
- **`test/test_lit_primitives.vyn`**: Additional primitive type serialization tests

#### Multi-Value Return & Function Tests
- **`test/test_multi_value_return.vyn`**: Multi-value function returns with auto-serialization
- **`test/test_multi_value_parser.vyn`**: Parser validation for multi-value syntax
- **`test/simple_fn_test.vyn`**: Simple function declaration and execution
- **`test/direct_return.vyn`**: Direct return value handling

#### Parser Error Handling Tests
- **`test/test_function_syntax_error_handling.vyn`**: Enhanced error messages for common function syntax mistakes
- **`test/units/parser/test*.vyn`**: Comprehensive parser validation suite (58 test files)
- **`test/units/extracted/test*.vyn`**: Extracted test cases for edge cases (100+ test files)

#### Struct & Type System Tests
- **`test/test_struct_syntax.vyn`**: Advanced struct declarations with auto-serialization
- **`test/test_struct_syntax_simplified.vyn`**: Simplified struct syntax validation
- **`test/test_type_alias.vyn`**: Type alias functionality
- **`test/test_type_alias_simple.vyn`**: Simple type alias cases

#### Integration & Semantic Tests
- **`test/test_semantic_integration.vyn`**: Full semantic analysis integration
- **`test/debug_test.vyn`**: Debug output and analysis validation
- **`test/println_test.vyn`**: Basic output functionality

#### Relaxed Syntax Tests
- **`test/test_relaxed*.vyn`**: Relaxed syntax parsing for improved developer experience
- **`test/units/test_relaxed*.vyn`**: Additional relaxed syntax validation

**Test Statistics:**
- **Core Feature Tests**: 15+ dedicated auto-serialization and multi-value tests
- **Parser Tests**: 58 comprehensive parser validation tests
- **Extracted Tests**: 100+ edge case and regression tests
- **Integration Tests**: 10+ semantic and integration validation tests
- **Total Test Coverage**: 180+ test files ensuring robust v0.3.5 functionality

All tests include expected output validation and error condition testing where appropriate.

---

## [0.3.4] - Previous Release

Previous version with support for:
- Advanced constructs like asynchronous programming
- Generic templates and operator overloading
- Class declarations within templates
- Comprehensive test suite validation

---

*For detailed documentation on auto-serialization capabilities and configuration, see `doc/Auto_Serialization_Main_Returns.md`.*
