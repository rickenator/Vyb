#!/bin/bash
# Detailed view of stack trace instrumentation in generated code

echo "╔═══════════════════════════════════════════════════════════════╗"
echo "║     Stack Trace Implementation - Detailed LLVM IR View       ║"
echo "╚═══════════════════════════════════════════════════════════════╝"
echo ""

# Generate the test
./build/vyn test/stack_trace_nested.vyn > /dev/null 2>&1

echo "Example: level3 function (deepest in call stack)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
sed -n '/^define.*@level3/,/^}/p' test/stack_trace_nested.vyn.ll | head -15
echo ""

echo "Example: level2 function (calls level3)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
sed -n '/^define.*@level2/,/^}/p' test/stack_trace_nested.vyn.ll | head -20
echo ""

echo "Key Observations:"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "1. Each function starts with: __vyn_runtime_push_call_frame(name, file, line, col)"
echo "2. Before each return: __vyn_runtime_pop_call_frame()"
echo "3. Function names are stored as string constants (@level3.str, etc.)"
echo "4. File paths and source locations are preserved"
echo ""
echo "Stack trace capture happens automatically:"
echo "  • When an error is created: __vyn_runtime_get_current_stack_trace()"
echo "  • Returns VynStackTrace with Vyn function names (not just addresses)"
echo "  • Each frame shows: function name, file path, line, column"
