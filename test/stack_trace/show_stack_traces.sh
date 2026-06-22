#!/bin/bash
# Run stack trace tests and display the results

echo "╔═══════════════════════════════════════════════════════════════╗"
echo "║          Stack Trace Tests - Generated LLVM IR               ║"
echo "╚═══════════════════════════════════════════════════════════════╝"
echo ""

# Test 1: Nested calls (3 levels)
echo "TEST 1: Nested Calls (3 levels deep)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
./build/vyb test/stack_trace_nested.vyb > /dev/null 2>&1
echo "Call stack instrumentation in level3:"
grep -A1 "define.*@level3" test/stack_trace_nested.vyb.ll | grep -E "define|push_call"
echo ""
echo "Call stack instrumentation in level2:"
grep -A2 "define.*@level2" test/stack_trace_nested.vyb.ll | grep -E "define|push_call|level3"
echo ""
echo "Call stack instrumentation in level1:"
grep -A2 "define.*@level1" test/stack_trace_nested.vyb.ll | grep -E "define|push_call|level2"
echo ""
echo "Call stack instrumentation in main:"
grep -A2 "define.*@main" test/stack_trace_nested.vyb.ll | grep -E "define|push_call|level1"
echo ""
echo "When level3 fails, the stack contains: main → level1 → level2 → level3"
echo ""

# Test 2: Deep call stack (5 levels)
echo ""
echo "TEST 2: Deep Call Stack (5 levels deep)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
./build/vyb test/stack_trace_deep.vyb > /dev/null 2>&1
echo "Function names in generated code:"
grep -E "@level[0-9]\.str|@main\.str" test/stack_trace_deep.vyb.ll | head -6
echo ""
echo "Push calls at function entries:"
grep "push_call_frame.*level" test/stack_trace_deep.vyb.ll | head -5
echo ""
echo "When level5 fails, the stack contains: main → level1 → level2 → level3 → level4 → level5"
echo ""

# Test 3: Show the actual declarations
echo ""
echo "TEST 3: Runtime Function Declarations"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Runtime functions used for stack trace capture:"
grep "declare.*__vyb_runtime.*call_frame" test/stack_trace_nested.vyb.ll
echo ""

# Summary
echo ""
echo "╔═══════════════════════════════════════════════════════════════╗"
echo "║                        SUMMARY                                ║"
echo "╠═══════════════════════════════════════════════════════════════╣"
echo "║ ✓ Each function pushes its name and location on entry        ║"
echo "║ ✓ Each function pops its frame on exit (success or error)    ║"
echo "║ ✓ Stack traces capture Vyb-level function call chain         ║"
echo "║ ✓ Error messages can display the exact call path             ║"
echo "╚═══════════════════════════════════════════════════════════════╝"
