cmake_minimum_required(VERSION 3.15)
project(VynParser VERSION 0.5.0)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Catch2 REQUIRED)

# Find LLVM
find_package(LLVM 18.1.3 REQUIRED CONFIG)
message(STATUS "LLVM package version: ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM include dirs: ${LLVM_INCLUDE_DIRS}")
message(STATUS "LLVM library dirs: ${LLVM_LIBRARY_DIRS}")
message(STATUS "LLVM available libraries: ${LLVM_AVAILABLE_LIBS}")
message(STATUS "LLVM available components: ${LLVM_AVAILABLE_COMPONENTS}")

# Check components using LLVM_AVAILABLE_LIBS
foreach(comp Core Support IRReader AsmParser BitReader BitWriter ipo TransformUtils ExecutionEngine MCJIT Object X86CodeGen RuntimeDyld X86AsmParser)
  if(NOT "LLVM${comp}" IN_LIST LLVM_AVAILABLE_LIBS)
    message(WARNING "LLVM component ${comp} not found in LLVM_AVAILABLE_LIBS")
  endif()
endforeach()

message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Make sure AddLLVM.cmake is on the module path
list(APPEND CMAKE_MODULE_PATH "${LLVM_DIR}")
include(AddLLVM)

# Build a list of actual .a/.so names for the components
llvm_map_components_to_libnames(LLVM_LIBS
  Core
  Support
  IRReader
  AsmParser
  BitReader
  BitWriter
  IPO
  TransformUtils
  ExecutionEngine  # Legacy JIT support
  MCJIT            # Older JIT API
  OrcJIT           # Modern JIT API
  JITLink          # Helper for OrcJIT
  Interpreter      # Add Interpreter support
  Object
  X86CodeGen       # X86 backend (was X86)
  RuntimeDyld
  X86AsmParser
  # Target components for object file emission (all architectures)
  AllTargetsInfos
  AllTargetsDescs
  AllTargetsCodeGens
  AllTargetsAsmParsers
)
message(STATUS "LLVM libraries: ${LLVM_LIBS}")

# Parser sources
set(PARSER_SOURCES
    src/parser/ast.cpp
    src/parser/base_parser.cpp
    src/parser/declaration_parser.cpp
    src/parser/expression_parser.cpp
    src/parser/lexer.cpp
    src/parser/module_parser.cpp
    src/parser/parser.cpp
    src/parser/statement_parser.cpp
    src/parser/token.cpp
    src/parser/type_parser.cpp
)

# Parser headers
set(PARSER_HEADERS
    include/vyn/parser/ast.hpp
    include/vyn/parser/lexer.hpp
    include/vyn/parser/parser.hpp
    include/vyn/parser/source_location.hpp
    include/vyn/parser/token.hpp
)

# VRE sources
set(VRE_SOURCES
    src/vre/value.cpp
    src/vre/semantic.cpp
    src/vre/intrinsics.cpp
    src/vre/llvm/cgen_main.cpp
    src/vre/llvm/cgen_types.cpp
    src/vre/llvm/cgen_util.cpp
    src/vre/llvm/cgen_expr.cpp
    src/vre/llvm/cgen_decl.cpp
    src/vre/llvm/cgen_monomorph.cpp
    src/vre/llvm/cgen_metadata.cpp
    src/vre/llvm/cgen_function_mono.cpp
    src/vre/llvm/cgen_trait_mono.cpp
    src/vre/llvm/cgen_stmt.cpp
    src/vre/llvm/cgen_helpers.cpp
    src/vre/llvm/cgen_string.cpp
    src/vre/llvm/cgen_println.cpp
    src/vre/llvm/cgen_vec.cpp
    src/vre/llvm/cgen_string_type.cpp
    src/vre/llvm/cgen_function.cpp
    src/vre/llvm/cgen_semantics.cpp
    src/vre/llvm/cgen_ownership.cpp
    src/vre/llvm/cgen_async.cpp
    src/vre/llvm/cgen_async_impl.cpp
    src/vre/llvm/cgen_stack_trace.cpp
)

# VRE headers
set(VRE_HEADERS
    include/vyn/vre/value.hpp
    include/vyn/semantic.hpp
    include/vyn/vre/memory.hpp
    include/vyn/vre/runtime_types.hpp
    include/vyn/vre/llvm/codegen.hpp
)

# Runtime sources
set(RUNTIME_SOURCES
    src/runtime/async_runtime.cpp
    src/runtime/error_handling.cpp
    runtime/vyn_runtime.c
    runtime/vyn_type_metadata.c
)

# Runtime headers
set(RUNTIME_HEADERS
    include/vyn/runtime/async_runtime.hpp
    include/vyn/runtime/error_handling.hpp
)

# Main and tests
set(MAIN_SOURCES src/main.cpp src/tests.cpp)

add_executable(vyn
    ${PARSER_SOURCES}
    ${PARSER_HEADERS}
    ${VRE_SOURCES}
    ${VRE_HEADERS}
    ${RUNTIME_SOURCES}
    ${RUNTIME_HEADERS}
    ${MAIN_SOURCES}
)

target_include_directories(vyn PRIVATE
    include # Project's own include directory
    SYSTEM ${LLVM_INCLUDE_DIRS} # Treat LLVM includes as system headers
)

# Add LLVM definitions
target_compile_definitions(vyn PRIVATE ${LLVM_DEFINITIONS})

# Link libraries
target_link_libraries(vyn PRIVATE
    Catch2::Catch2WithMain
    ${LLVM_LIBS} # Link against the mapped LLVM libraries
)

# Platform-specific libraries for error handling runtime
if(UNIX AND NOT APPLE)
    # Linux: backtrace() requires -ldl, pthread for thread safety
    target_link_libraries(vyn PRIVATE dl pthread)
elseif(APPLE)
    # macOS: backtrace() is in system libraries, only need pthread
    target_link_libraries(vyn PRIVATE pthread)
elseif(WIN32)
    # Windows: CaptureStackBackTrace requires dbghelp.lib
    target_link_libraries(vyn PRIVATE dbghelp)
endif()

# Add debug flags for tests.cpp
set_source_files_properties(src/tests.cpp PROPERTIES COMPILE_FLAGS "-DDEBUG_TESTS")

# Parser verbose tracing is off by default. Define VERBOSE (e.g. -DVERBOSE in cmake)
# or use --debug-codegen at runtime to re-enable detailed diagnostics.

# Add a custom target for running tests
find_package(Python3 REQUIRED)
add_custom_target(run-tests
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/test/run_tests.py --vyn $<TARGET_FILE:vyn> --test-dir ${CMAKE_SOURCE_DIR}/test
    DEPENDS vyn
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running Vyn test suite"
)