# CI workflows

All three Gates run on every push to `main` and on pull requests to it. They were
re-enabled 2026-09-11 (fixing #220: `ci` and `refman-check` had been
`disabled_manually` in the repository Actions settings, so only `gpu-kernel`
attached to pushes).

| Workflow file | Check names (for branch protection) | What it gates |
|---|---|---|
| `.github/workflows/ci.yml` | `ci` — "Build + JIT suite + AOT/native-link", plus the scheduled daily ASan/UBSan run (06:00 UTC) | canonical 1141-test JIT suite, AOT/native link, memory safety |
| `.github/workflows/gpu-kernel.yml` | `gpu-kernel` — "Emit PTX + ptxas across arches + compile FFI runners", "Execute + verify kernels + bindings on RTX 3090" (self-hosted), plus the LSP/REPL/git-dep/registry smoke tests | GPU/NVPTX + CLI protocol smokes |
| `.github/workflows/refman-check.yml` | `refman-check` — "Validate generated refman matches stdlib" | doc/refman drift + link integrity |

Notes
- Re-enabling a manually-disabled workflow is `PUT /repos/{owner}/{repo}/actions/workflows/{id}/enable` (the API uses **PUT**; POST 404s) — `gh api -X PUT`.
- The daily scheduled ASan/UBSan run in `ci.yml` runs on the default branch only (GitHub scheduled-workflow rule), so it is independent of push frequency.
- `release-sdk.yml` builds/publishes the SDK and is intentionally trigger-scoped (tags/release), not on every push.
