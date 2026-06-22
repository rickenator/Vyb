#!/bin/bash
# Test compilation of VyB examples
# Tests both JIT execution and AOT compilation with build

echo "=========================================="
echo "VyB Compilation Test Suite v0.4.4"
echo "=========================================="
echo

TESTS_PASSED=0
TESTS_FAILED=0

# Test function
test_compile() {
    local test_file=$1
    local expected_exit=$2
    local test_name=$(basename "$test_file" .vyb)

    echo "Testing: $test_name"

    # Build the executable
    if ! build/vyb "$test_file" --build "test_output_$test_name" -O2 > /dev/null 2>&1; then
        echo "✗ Build failed: $test_name"
        TESTS_FAILED=$((TESTS_FAILED + 1))
        return 1
    fi

    # Run and check exit code
    ./test_output_$test_name > /dev/null 2>&1
    local actual_exit=$?

    if [ "$actual_exit" -eq "$expected_exit" ]; then
        echo "✓ Pass: $test_name (exit code: $actual_exit)"
        TESTS_PASSED=$((TESTS_PASSED + 1))
        rm -f "test_output_$test_name" "test_output_$test_name.o"
        return 0
    else
        echo "✗ Fail: $test_name (expected: $expected_exit, got: $actual_exit)"
        TESTS_FAILED=$((TESTS_FAILED + 1))
        return 1
    fi
}

# Run tests
echo "Running compilation tests..."
echo

test_compile "test/compilation/test_compile.vyb" 49
test_compile "test/compilation/binary_tree.vyb" 60
test_compile "test/compilation/binary_tree_complex.vyb" 94

echo
echo "=========================================="
echo "Test Summary"
echo "=========================================="
echo "Passed: $TESTS_PASSED"
echo "Failed: $TESTS_FAILED"
echo

if [ $TESTS_FAILED -eq 0 ]; then
    echo "All tests passed! ✓"
    exit 0
else
    echo "Some tests failed! ✗"
    exit 1
fi
