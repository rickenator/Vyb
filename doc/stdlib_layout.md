# Standard Library Module Layout (Current Foundation)

This document is the canonical stdlib module layout for the current milestone.

## Discovery

Stdlib import resolution uses the normal module resolver search order documented
in `doc/module_visibility.md`, including:

1. `VYN_STDLIB` (if set)
2. Executable-relative probes (`<exe_dir>/../stdlib`, then `<exe_dir>/stdlib`)

No `--module-path` flag is required when one of those stdlib roots is available.

## Layout

```text
stdlib/
  prelude.vyn            # top-level prelude re-export module
  core/
    prelude.vyn          # canonical prelude contents
    option.vyn           # transitional OptionInt bridge + Option<T> notes
    result.vyn           # placeholder for future Result<T,E>
  collections/
    mod.vyn              # placeholder scaffold
  io/
    mod.vyn              # placeholder scaffold
```

## Prelude behavior (current decision)

Prelude is **explicit-only** right now.

- `core::prelude` is **not auto-imported**.
- `prelude` is **not auto-imported**.
- Users explicitly import whichever prelude path they want:
  - `import core::prelude`
  - `import prelude`

This keeps module behavior deterministic while Option/Result/iterator/core-aspect
work is still evolving.

## Option/Result status

- `core::option` currently ships a transitional, non-generic `OptionInt` model.
- Canonical generic `Option<T>` with `Some(value)` / `None` is still future work.
- `core::result` is a placeholder module that documents future `Result<T,E>`.
