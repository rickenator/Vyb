#!/bin/bash
# Demonstrate Vyb call stack traces

echo "╔════════════════════════════════════════════════════════════════════════════╗"
echo "║              Vyb Call Stack Traces - Live Demonstration                    ║"
echo "╚════════════════════════════════════════════════════════════════════════════╝"
echo ""

echo "TEST 1: Nested Function Calls (3 levels)"
echo "────────────────────────────────────────────────────────────────────────────"
echo "Program: main() → level1() → level2() → level3() [fail]"
echo ""
./build/vyb test/stack_trace_nested.vyb 2>&1 | grep -A12 "UNTRAPPED FAILURE"
echo ""
echo ""

echo "TEST 2: Deep Call Stack (5 levels)"
echo "────────────────────────────────────────────────────────────────────────────"
echo "Program: main() → level1() → level2() → level3() → level4() → level5() [fail]"
echo ""
./build/vyb test/stack_trace_deep.vyb 2>&1 | grep -A15 "UNTRAPPED FAILURE"
echo ""
echo ""

echo "╔════════════════════════════════════════════════════════════════════════════╗"
echo "║                              KEY FEATURES                                  ║"
echo "╠════════════════════════════════════════════════════════════════════════════╣"
echo "║ ✓ Shows Vyb function names (not native/mangled C++ names)                  ║"
echo "║ ✓ Displays source file, line, and column for each frame                    ║"
echo "║ ✓ Ordered from innermost (where error occurred) to outermost (main)        ║"
echo "║ ✓ Automatically captured on every error                                    ║"
echo "║ ✓ Thread-safe with mutex-protected global stack                            ║"
echo "╚════════════════════════════════════════════════════════════════════════════╝"
