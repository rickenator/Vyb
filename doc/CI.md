# CI workflows

All three Gates run on every push to `main` and on pull requests to it. They were
re-enabled 2026-09-11 (fixing #220: `ci` and `refman-check` had been
`disabled_manually` in the repository Actions settings, so only `gpu-kernel`
attached to pushes).

## Branch protection (ruleset `main-protection`, #222)

A repository ruleset on `refs/heads/main` (enforcement: active) requires, for any
merge/push to `main`:

- **Required status checks** (a PR/commit cannot merge to `main` unless these
  pass):
  1. `Build + JIT suite + AOT/native-link` (hosted, ci.yml)
  2. `refman` (refman-check.yml)
  3. `Emit PTX + ptxas across arches + compile FFI runners` (hosted CPU-side GPU
     validation, gpu-kernel.yml)
- **No force pushes** (non-fast-forward blocked).
- **No branch deletion**.

Deliberately NOT required (non-blocking, so hardware availability / scheduled
runs never deadlock `main`): the self-hosted `Execute + verify kernels +
bindings on RTX 3090` job (gpu-kernel.yml) and the scheduled-only
`Memory-safety (ASan full suite)` job. Both still run/report for visibility.

Changes to `main` therefore go through a pull request: push a branch, open a PR,
wait for the three required checks to pass, then merge.

| Workflow file | Check names (for branch protection) | What it gates |
|---|---|---|
| `.github/workflows/ci.yml` | `ci` — "Build + JIT suite + AOT/native-link", plus the scheduled daily ASan/UBSan run (06:00 UTC) | canonical 1141-test JIT suite, AOT/native link, memory safety |
| `.github/workflows/gpu-kernel.yml` | `gpu-kernel` — "Emit PTX + ptxas across arches + compile FFI runners", "Execute + verify kernels + bindings on RTX 3090" (self-hosted), plus the LSP/REPL/git-dep/registry smoke tests | GPU/NVPTX + CLI protocol smokes |
| `.github/workflows/refman-check.yml` | `refman-check` — "Validate generated refman matches stdlib" | doc/refman drift + link integrity |

Notes
- Re-enabling a manually-disabled workflow is `PUT /repos/{owner}/{repo}/actions/workflows/{id}/enable` (the API uses **PUT**; POST 404s) — `gh api -X PUT`.
- The daily scheduled ASan/UBSan run in `ci.yml` runs on the default branch only (GitHub scheduled-workflow rule), so it is independent of push frequency.
- `release-sdk.yml` builds/publishes the SDK and is intentionally trigger-scoped (tags/release), not on every push.
