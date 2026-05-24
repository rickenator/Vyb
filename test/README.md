# Vyn Test System

This directory contains the Vyn language test suite and Python test harness.
There are currently 687 `.vyn` tests.

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
// @vyn-args: --module-path test/modules/pathlib
// @env: VYN_MODULE_PATH=test/modules/envlib
```

| Directive | Description |
|-----------|-------------|
| `@test` | Human-readable test name |
| `@description` | Specific behavior under test |
| `@category` | Comma-separated category tags |
| `@expect` | `pass` or `fail` |
| `@expect-error` | Error substring expected for failing tests |
| `@expect-output` | Stdout substring expected for runtime tests |
| `@expect-return` | Expected final stdout line from `main` return serialization when JIT execution is enabled |
| `@vyn-args` | Extra CLI arguments passed to `vyn` before the test filename |
| `@env` | Semicolon-separated environment overrides (`KEY=value;KEY2=value2`) |
| `@parse-only` | Stop after parsing |
| `@semantic-only` | Stop after parsing and semantic analysis |

Use `@expect-output: n/a` when a parse-only or semantic-only test should not
assert runtime output.

## Running Tests

```bash
python3 test/run_tests.py --vyn build/vyn --test-dir test/new_features --execute-jit
python3 test/run_tests.py --vyn build/vyn --test-dir test/ffi --execute-jit
python3 test/run_tests.py --vyn build/vyn --test-dir test/parser
python3 test/run_milestone_tests.py --vyn build/vyn
```

The harness defaults to `test/units` and `--no-execute`. Pass `--execute-jit`
for runtime/output tests. The milestone runner aggregates the stable runtime,
module, FFI, introspection, primitive type, range, Vec iteration, and stdlib
suites and enforces at least 122 passing tests before the next milestone.

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
