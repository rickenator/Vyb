# Vyn Test System

This directory contains the Vyn language test suite and Python test harness.
There are currently 658 `.vyn` tests.

## Test Directives

Tests use top-of-file comments to describe expected behavior:

```vyn
// @test: Short name
// @description: What this test validates
// @category: parser, semantic, runtime
// @expect: pass
// @parse-only: true
// @semantic-only: true
// @expect-output: Expected stdout text
// @expect-error: Expected error text
// @expect-return: 0
```

| Directive | Description |
|-----------|-------------|
| `@test` | Human-readable test name |
| `@description` | Specific behavior under test |
| `@category` | Comma-separated category tags |
| `@expect` | `pass` or `fail` |
| `@expect-error` | Error substring expected for failing tests |
| `@expect-output` | Stdout substring expected for runtime tests |
| `@expect-return` | Expected process return value when used by a runner that supports it |
| `@parse-only` | Stop after parsing |
| `@semantic-only` | Stop after parsing and semantic analysis |

Use `@expect-output: n/a` when a parse-only or semantic-only test should not
assert runtime output.

## Running Tests

```bash
python3 test/run_tests.py --vyn build/vyn --test-dir test/new_features --execute-jit
python3 test/run_tests.py --vyn build/vyn --test-dir test/ffi --execute-jit
python3 test/run_tests.py --vyn build/vyn --test-dir test/parser
```

The harness defaults to `test/units` and `--no-execute`. Pass `--execute-jit`
for runtime/output tests.

## Writing Tests

Use current name-first Vyn syntax:

```vyn
// @test: Integer Addition
// @description: Tests basic integer addition
// @category: runtime, arithmetic
// @expect: pass
// @expect-output: Result: 15

main()<Int> -> {
    a<Int> = 5
    b<Int> = 10
    c<Int> = a + b
    println("Result: " + c.to_string())
    return 0
}
```

Runtime tests should return `0` unless the return code is the behavior being
tested.
