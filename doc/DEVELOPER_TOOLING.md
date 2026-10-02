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
- **`github:`** — `vyb build` AUTO-FETCHES `name = { github = "owner/repo/path" }`
  into `.vybmod/<name>/` when it is not already materialized (#388) and consumes it
  like a git dep; `vyb mod install github:owner/repo/path` remains the pinned
  (`@sha256:HEX`) / `--require-signed` channel. A package that declares a freedom or
  capability boundary is refused on build — that trust decision belongs to
  `vyb mod install`. The raw transport's base is overridable with
  `VYB_GITHUB_RAW_URL`.
- **`version:`** — matched against the package registry (`VYB_REGISTRY`, else
  `~/.vyb/registry`), picking the highest version satisfying the spec (`""` or
  `*` matches any); `vyb mod publish` populates it. A dep already materialized in
  `.vybmod/<name>/` rebuilds with the registry absent (offline cache).
- **Lockfile** — `vyb.lock` records the resolved pins, so a build is reproducible.

The design notes that used to be "staged" are now the implemented behaviour, and
the acceptance cases are fixtures rather than promises: `test/gitdep_smoke.vyb`
drives clone-on-build, lockfile, build and run end to end
(`gitdep smoke: 4/4 checks passed`), `test/githubdep_smoke.vyb` the same for
auto-fetch-on-build against a hermetic `file://` mirror plus the privileged-package
refusal (`githubdep smoke: 6/6 checks passed`), and `test/registry_smoke.vyb` the
registry/`version:` path including an offline rebuild
(`registry smoke: 9/9 checks passed`). All three are wired into CI
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
