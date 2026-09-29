# Developer tooling: dependencies, formatter, LSP, REPL (#154)

## Dependency resolution — shipped scope

**Shipped:** all four dependency source kinds resolve (tracked by
`doc/FEATURE_STATUS.md` "URL/Git dependency fetching", #165 / #175, and documented
in full in `doc/MANIFEST.md`):

- **`path`** — `name = { path = "relative/dir" }` is resolved by `vyb build`
  (`project_module_paths`): the directory joins the module search path and its
  modules are importable in the project.
- **`git:`** — `name = { git = "url" }` with a `rev`/`tag`/`branch` is shallow-cloned
  into `.vybmod/<name>/` on build and consumed from there.
- **`github:`** — `vyb mod install github:owner/repo/path` materializes the module
  into `.vybmod/<name>/`; the build then consumes it like a git dep.
- **`version:`** — matched against the package registry (`VYB_REGISTRY`, else
  `~/.vyb/registry`), picking the highest version satisfying the spec (`""` or
  `*` matches any); `vyb mod publish` populates it.
- **Lockfile** — `vyb.lock` records the resolved pins, so a build is reproducible.

The design notes that used to be "staged" are now the implemented behaviour, and
the acceptance case is a fixture rather than a promise: `test/gitdep_smoke.vyb`
drives clone-on-build, lockfile, build and run end to end
(`gitdep smoke: 4/4 checks passed`), and the smoke is wired into CI
(`.github/workflows/gpu-kernel.yml`, see `doc/CI.md`).

**Remaining:** broader ecosystem polish (registry publishing UX, transitive
version conflict reporting) — tracked in `TODO.md` under the project system.

## Post-release tooling roadmap (in order)

After the safety/release validation now in CI (#160 build profiles + #158
CTest/reproducible evidence), add these in order:

1. **Formatter** -- deterministic `vyb fmt` (whitespace/layout only, no
   semantic rewrites) driven by the existing token stream, so it can never
   reorder or drop code. Round-trip must be a no-op (fixpoint) and should per
   file under version control before being trusted.
2. **LSP** -- a `vyb lsp` server over stdio implementing the Language Server
   Protocol on top of the parser + semantic analyzer (diagnostics, then
   hover/go-to-definition once the index is in place). Uses the compiler as a
   library, not a subprocess-per-keystroke.
3. **REPL** -- a `vyb repl` interactive loop that JIT-compiles each statement
   (mirroring the existing single-file JIT pipeline) and prints the result.
