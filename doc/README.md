# VyB Documentation Index

This directory contains the authoritative documentation for the VyB programming language.

---

## Start Here

| Document | Purpose |
|----------|---------|
| [`../README.md`](../README.md) | Project overview, quick start, examples |
| [`FEATURE_STATUS.md`](FEATURE_STATUS.md) | Current implementation status per feature |
| [`VYB_PROJECT_STUDY_GUIDE.md`](VYB_PROJECT_STUDY_GUIDE.md) | Living project study guide for practitioners |
| [`../TODO.md`](../TODO.md) | Living road-to-1.0 checklist |
| [`../SUGGESTIONS.md`](../SUGGESTIONS.md) | Sprint-organized improvement suggestions |

---

## Language Reference

| Document | Purpose |
|----------|---------|
| [`Canonical_Reference_Syntax.md`](Canonical_Reference_Syntax.md) | **Authoritative ownership syntax** (`my`, `our`, `their`, `borrow`, `view`) |
| [`Declaration_Syntax.md`](Declaration_Syntax.md) | Function and variable declaration syntax |
| [`VyB_Function_Declaration_Syntax.md`](VyB_Function_Declaration_Syntax.md) | Name-first function syntax deep-dive |
| [`MATCH_SYNTAX.md`](MATCH_SYNTAX.md) | Pattern matching (`match`/`select`) |
| [`LAMBDAS.md`](LAMBDAS.md) | Lambda expressions and closures |
| [`VEC_ITERATION.md`](VEC_ITERATION.md) | `Vec<T>` and `for (item in vec)` |
| [`OWNERSHIP_MILD.md`](OWNERSHIP_MILD.md) | `mild<T>` weak references |
| [`Memory_Operations.md`](Memory_Operations.md) | `freedom` blocks and raw pointer operations |

---

## Design Documents

| Document | Purpose |
|----------|---------|
| [`ROADMAP.md`](ROADMAP.md) | High-level language roadmap |
| [`VRE.md`](VRE.md) | VyB Runtime Environment internals |
| [`RUNTIME.md`](RUNTIME.md) | Runtime design |
| [`WHY_TRAITS_NOT_CLASSES.md`](WHY_TRAITS_NOT_CLASSES.md) | Rationale for aspect/bind over classes (uses `aspect`/`bind` terminology) |
| [`TRAIT_SYSTEM_DESIGN.md`](TRAIT_SYSTEM_DESIGN.md) | Aspect system design (`aspect`/`bind`) |
| [`ERROR_TRAP.md`](ERROR_TRAP.md) | `fail`/`trap` error propagation design |
| [`FFI_DESIGN.md`](FFI_DESIGN.md) | Foreign Function Interface design |
| [`MODULE_FFI_BINARY_ROADMAP.md`](MODULE_FFI_BINARY_ROADMAP.md) | Module system roadmap |

---

## AST Reference

| Document | Purpose |
|----------|---------|
| [`AST_Overview.md`](AST_Overview.md) | AST node hierarchy overview |
| [`AST_Core.md`](AST_Core.md) | Core AST nodes |
| [`AST_Declarations.md`](AST_Declarations.md) | Declaration nodes |
| [`AST_Expressions.md`](AST_Expressions.md) | Expression nodes |
| [`AST_Statements.md`](AST_Statements.md) | Statement nodes |
| [`AST_Types.md`](AST_Types.md) | Type nodes |
| [`AST_Literals.md`](AST_Literals.md) | Literal nodes |
| [`AST_Patterns.md`](AST_Patterns.md) | Pattern nodes |

---

*VyB is not Rust. It is not C++. It is its own thing.*
