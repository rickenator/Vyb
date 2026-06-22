#!/usr/bin/env python3
"""
Vyb Test Harness with Logging
"""

import os
import re
import subprocess
import sys
import argparse
from pathlib import Path
from dataclasses import dataclass
from typing import Optional, List
import time
import json
from datetime import datetime

# === Logging Setup ===
timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
log_path = f"test_output_{timestamp}.log"
log_file = open(log_path, "w")

def log_print(msg):
    print(msg)
    log_file.write(msg + "\n")

def find_vyb_root():
    script_dir = Path(os.path.dirname(os.path.abspath(__file__)))
    current_dir = script_dir
    while True:
        if (current_dir / "CMakeLists.txt").exists() and (current_dir / "src" / "tests.cpp").exists():
            return current_dir
        parent_dir = current_dir.parent
        if parent_dir == current_dir:
            raise RuntimeError("Could not find Vyb repository root directory")
        current_dir = parent_dir

@dataclass
class TestCase:
    filename: str
    name: str = "Unnamed test"
    description: str = ""
    category: List[str] = None
    expect: str = "pass"
    expect_error: Optional[str] = None
    expect_output: Optional[str] = None
    expect_return: Optional[str] = None
    parse_only: bool = False
    semantic_only: bool = False
    def __post_init__(self):
        if self.category is None:
            self.category = ["uncategorized"]

def parse_directives(file_path):
    test = TestCase(filename=str(file_path))
    try:
        with open(file_path, 'r') as f:
            content = f.read()
        m = lambda p: re.search(p, content, re.MULTILINE)
        g = lambda m: m.group(1).strip() if m else None

        test.name = g(m(r'// @test:\s*(.*?)$')) or test.name
        test.description = g(m(r'// @description:\s*(.*?)$')) or test.description
        cats = g(m(r'// @category:\s*(.*?)$'))
        if cats: test.category = [c.strip() for c in cats.split(',')]
        test.expect = g(m(r'// @expect:\s*(.*?)$')) or test.expect
        test.expect_error = g(m(r'// @expect-error:\s*(.*?)$'))
        test.expect_output = g(m(r'// @expect-output:\s*(.*?)$'))
        test.expect_return = g(m(r'// @expect-return:\s*(.*?)$'))
        test.parse_only = (g(m(r'// @parse-only:\s*(.*?)$')) or '').lower() in ('true', 'yes', '1')
        test.semantic_only = (g(m(r'// @semantic-only:\s*(.*?)$')) or '').lower() in ('true', 'yes', '1')
    except Exception as e:
        log_print(f"Error parsing directives in {file_path}: {str(e)}")
    return test

def run_test(test, vyb_executable, verbose=False, execute_jit=False):
    cmd = [vyb_executable]
    if test.parse_only:
        cmd.append("--parse-only")
    elif test.semantic_only:
        cmd.append("--semantic-only")
    elif not execute_jit:
        cmd.append("--no-execute")
    if verbose:
        cmd.append("--emit-llvm")
    cmd.append(test.filename)

    start_time = time.time()
    try:
        result = subprocess.run(cmd, capture_output=True, text=True)
        elapsed = time.time() - start_time
        expected_code = 0 if test.expect == "pass" else 1
        success = (result.returncode == expected_code)

        if test.expect_output and test.expect_output != "n/a":
            if test.expect_output not in result.stdout:
                success = False
        if test.expect == "fail" and test.expect_error and test.expect_error != "n/a":
            if test.expect_error not in result.stderr:
                success = False

        return {
            "success": success,
            "stdout": result.stdout,
            "stderr": result.stderr,
            "return_code": result.returncode,
            "execution_time": elapsed
        }

    except Exception as e:
        return {
            "success": False,
            "stdout": "",
            "stderr": str(e),
            "return_code": -1,
            "execution_time": time.time() - start_time
        }

def format_result(result, test, verbose=False):
    status = "PASS" if result["success"] else "FAIL"
    if verbose:
        output = f"\n{status}: {test.name} ({test.filename})\n"
        output += f"Description: {test.description}\n"
        output += f"Category: {', '.join(test.category)}\n"
        output += f"Execution time: {result['execution_time']:.4f}s\n"
        if not result["success"] or verbose > 1:
            output += "Output:\n" + result["stdout"] + "\n"
            output += "Error:\n" + result["stderr"] + "\n"
    else:
        output = f"{status}: {test.name}"
    return output

def main():
    try:
        vyb_root = find_vyb_root()
    except RuntimeError as e:
        log_print(f"Error: {e}")
        sys.exit(1)

    parser = argparse.ArgumentParser(description='Vyb test harness')
    parser.add_argument('--vyb', default=None)
    parser.add_argument('--test-dir', default=None)
    parser.add_argument('--pattern', default='*.vyb')
    parser.add_argument('--verbose', '-v', action='count', default=0)
    parser.add_argument('--category')
    parser.add_argument('--json')
    parser.add_argument('--execute-jit', action='store_true')
    args = parser.parse_args()

    vyb_executable = str(vyb_root / "build" / "vyb") if args.vyb is None else args.vyb
    test_dir = vyb_root / "test" / "units" if args.test_dir is None else Path(args.test_dir)

    if not os.path.isfile(vyb_executable):
        log_print(f"Error: Cannot find Vyb executable at {vyb_executable}")
        sys.exit(1)
    if not test_dir.exists():
        log_print(f"Error: Test directory {test_dir} doesn't exist")
        sys.exit(1)

    if args.verbose:
        log_print(f"Vyb repo: {vyb_root}")
        log_print(f"Executable: {vyb_executable}")
        log_print(f"Tests in: {test_dir}")
        if args.execute_jit:
            log_print("JIT: Enabled")

    results = []
    pass_count = fail_count = 0
    test_files = sorted(list(test_dir.glob(f"**/{args.pattern}")))
    if args.verbose:
        log_print(f"Found {len(test_files)} test files")

    for file in test_files:
        test = parse_directives(file)
        if args.category and args.category not in test.category:
            continue
        test_result = run_test(test, vyb_executable, args.verbose > 1, args.execute_jit)
        log_print(format_result(test_result, test, args.verbose))
        if test_result["stdout"]:
            log_file.write(f"\n--- stdout from {test.name} ---\n")
            log_file.write(test_result["stdout"] + "\n")

if test_result["stderr"]:
    log_file.write(f"\n--- stderr from {test.name} ---\n")
    log_file.write(test_result["stderr"] + "\n")
        if test_result["success"]:
            pass_count += 1
        else:
            fail_count += 1
        results.append({"test": test.__dict__, "result": test_result})

    log_print(f"\nRan {len(results)} tests")
    log_print(f"Passed: {pass_count}")
    log_print(f"Failed: {fail_count}")

    if args.json:
        with open(args.json, 'w') as f:
            json.dump(results, f, indent=2)
        if args.verbose:
            log_print(f"Saved JSON: {args.json}")

    log_file.close()
    if fail_count > 0:
        sys.exit(1)

if __name__ == "__main__":
    main()

