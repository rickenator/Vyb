# Vyb project claims audit: handoff for the vyb-os.org contractor

**Date:** Tuesday, October 20, 2026
**Prepared by:** Ursela, for Rick Goldberg (Aniviza LLC)
**Audience:** the outside contractor or agent building **vyb-os.org** and editing project READMEs
**Status:** read-only audit, revised Oct 20, ~8:40 AM PT after a rescan of Rick's working tree. No repo, README, site, or DNS record was changed to write it. This file is uncommitted.
**Baseline audited:** Vyb is audited against **HEAD `c14d4768` plus Rick's uncommitted working-tree edits on Godzilla** (`~/Projects/Vyb`, edited Oct 19, 7:02–7:15 PM PT). The other repos are audited at their public HEADs.

---

## 1. Purpose and scope

You're building the vyb-os.org website, and you'll probably also edit the READMEs it draws from. This audit lists **what the public code actually supports today**, so the site and READMEs can be accurate from day one. Every statement on the site and in a README has to be traceable to code, CI, or a measurement at a named commit.

**In scope:** the public `main` branches of the repos below, pinned at the commits audited, plus the live and unlaunched sites.

| Repo | Audited HEAD (main) | HEAD date (PT) | README size |
|---|---|---|---|
| rickenator/Vyb | `c14d4768` | 2026-10-10 19:53 | 3,739 lines |
| rickenator/VybForge | `77aed0d0` | 2026-10-11 05:43 | 228 lines |
| rickenator/VybFly | `055748ce` | 2026-09-28 14:41 | 237 lines |
| rickenator/VybOS | `628025ff` | 2026-09-23 01:45 | 256 lines |
| Vyb `docs/web/landing_page/index.html` (vyb-lang.org source) | `c14d4768` + working tree | edited 2026-10-19 19:15 | 1.49 MB (README render) |

For Vyb, line numbers are **working-tree** line numbers. That's HEAD +2 after line ~70, because Rick's Quick Start edit added two lines. For the other repos, line numbers refer to the listed commits. If main has moved by the time you read this, re-check the line before you edit it.

**Severity key**
- **High:** false or contradicted by the code or CI. Must not appear on the site.
- **Medium:** overstated, stale, or self-contradictory. Fix the wording before reusing it.
- **Low:** unverified, or a minor doc slip. Reuse only with a qualifier, or verify first.

---

## 2. Ground rules for the contractor

1. **Every claim has to match what the code does.** No overclaims, no aspirations written in the present tense. If a feature is planned, say "planned". If it's partial, say "partial". Keep honest scoping that's already there, such as "P0 feasibility probe", "fixture-scale", "Linux-only async", and "untested targets". Don't delete it.
2. **VybForge speed:** the public `main` branch decodes Qwen3-4B at **about 16 tok/s (≈15.7 tok/s measured after issue #23, Oct 11)**. **Do not cite 58 tok/s (GB10) or ~112 tok/s (RTX 3090).** Those come from unpublished local commits and aren't on GitHub. Any number above ~16 tok/s is unsupported by the public code until Rick publishes that work.
3. **Don't claim 27B support.** Qwen3-27B has no generation loop and no detokenizer on main (open item A5). It has never generated text end to end. Don't say "runs 27B", "supports 27B", or anything similar.
4. **Credit third-party code** as `VybForge/THIRD-PARTY.md` describes. Today that covers the llama.cpp (MIT) Q4_K/Q6_K block-decode ports in `native/kernels/q4k.vyb` and `q6k.vyb`. Any page that describes the kernels must keep that attribution. Copyleft code is consume-only and never ported. Don't change the LICENSE (Apache-2.0, "Copyright 2026 Aniviza LLC") or the author or copyright lines.
5. **Open pull requests for Rick's review. Never push to `main`.** Use one PR per repo with a clear description. Don't merge your own PRs, don't add collaborators, and don't add deploy workflows (Pages, Netlify, Wrangler, Vercel) or a `CNAME` file unless Rick asks for them.
6. **Numbers need a source.** Test counts, tok/s, neuron and synapse counts, `sm_` architectures, and version strings all have to be checked against code or CI at the commit you cite. Put the commit in the PR description.
7. **Don't introduce** tracking or analytics, external CDNs, new contact emails, or new domains without Rick's sign-off.
8. **vyb-lang.org's body is generated from the Vyb README** (see §3). Fix the README first and regenerate the page from it, following `docs/web/landing_page/README.md`. Don't hand-edit the README-derived body of `index.html`.
9. **Build on Rick's in-progress edits, not around them.** Don't overwrite or revert his uncommitted Vyb changes (§3.3). Wait until he commits them, or until he says which branch to base on.
10. **Don't run `deploy.sh` or upload to any host.** Publishing vyb-lang.org or vyb-os.org is Rick's step (see §3.4).

---

## 3. Current state of the sites

### 3.1 vyb-os.org: not live
- Registered through **Cloudflare** (registrar) on 2026-08-26, expires 2027-08-26. Nameservers are `elijah.ns.cloudflare.com` and `mira.ns.cloudflare.com`, the same pair as vyb-lang.org, so it's very likely the same Cloudflare account.
- At baseline (Oct 19, 6:40 PM PT) the apex had **no A, AAAA, or CNAME records**. The zone held only SOA and NS records (SOA serial `2413391667`). `www.vyb-os.org` returns NXDOMAIN. No page or TLS certificate is served.
- No GitHub repo under rickenator references the domain yet.

### 3.2 vyb-lang.org: live but very stale
- Served by Bluehost Apache (69.89.24.177) behind Cloudflare DNS. `Last-Modified: Sat, 22 Aug 2026 21:31:29 GMT`.
- The deployed page says **v0.7.3** and **"1061 .vyb tests"**. The current release is **v0.7.7**.
- Its source, `Vyb/docs/web/landing_page/index.html` on main, says v0.7.6, links release v0.7.5, advertises a "Linux x64 / macOS x64" SDK (no macOS SDK exists), cites 1310 tests, and says "mature". So **deployed ≠ main ≠ the open PR**.
- Open PR **#405** (by rickenator, unmerged) updates the version to 0.7.7, fixes the counts, drops the macOS SDK, and corrects the lifetime wording.
- `og:image` `/vyb.png` returned 404 on the live root (noted in Rick's working-tree landing-page README).

### 3.3 Rick's in-progress edits (uncommitted, on Godzilla `~/Projects/Vyb`)
These are Rick's own edits from Oct 19, 7:02–7:15 PM PT. They aren't committed or pushed, so **GitHub `main` doesn't have them yet**. A rescan on Oct 20 (~8:35 AM PT) found nothing new since then.

| File | State | What it does |
|---|---|---|
| `README.md` | modified (18 lines) | NVPTX range corrected from "`sm_75` → `sm_90a`" to "`sm_75` → `sm_90` in CI", plus a new "`sm_90a` additionally validates on a local toolchain" (lines 30, 236, 248, 472, 3287). Quick Start now points to the releases page and the `vyb-sdk-<version>-linux-x86_64.tar.gz` name pattern. |
| `TODO.md` | modified | The landing-page entry now documents that the page body is a render of `README.md`, and points to the recipe and `deploy.sh`. |
| `docs/web/landing_page/index.html` | modified (+1,088/−573 overall diff) | Regenerated from the edited README. Version v0.7.7. The old 1310 / v0.7.5 / v0.7.6 / "macOS x64 SDK" strings are gone. JSON-LD and meta updated. The body inherits every remaining README finding. |
| `docs/web/landing_page/README.md` | new, untracked | How the page is built (hand-maintained shell + README-rendered body), the regeneration recipe, and deploy notes. |
| `docs/web/landing_page/deploy.sh` | new, untracked, executable | Uploads the page to Bluehost over scp and checks the live site. Reviewed in §3.4. |

### 3.4 `deploy.sh` review (read-only; it was not run)
**What it does:** it `cd`s to `docs/web/landing_page/` and checks that six files exist: `index.html vyb.png favicon.ico favicon-16x16.png favicon-32x32.png apple-touch-icon.png`. It then `scp`s them to `anivizac@69.89.24.177:${DEST:-public_html/vyb-lang.org/}` on port 2222. Finally it curls `https://vyb-lang.org/`, greps for the token `1314`, and checks that `/vyb.png` and the icons resolve. `--check` runs only the curl verification.

**Secrets:** **no hardcoded passwords, tokens, or key material.** It names a key *path* (`~/.ssh/id_ed25519_aniviza`) and hardcodes the Bluehost account user and IP (`anivizac@69.89.24.177`, port 2222). That's not a secret, but it is infrastructure detail in a file that becomes public if committed.

**Concerns for the contractor and Rick:**
1. **Host-key checking is disabled** (`StrictHostKeyChecking=no`, `UserKnownHostsFile=/dev/null`) because "the host's ssh key changed". That accepts any server, including a man-in-the-middle, and it can fall back to **password auth** (`PreferredAuthentications=publickey,password,…`, three prompts). A cPanel password typed into an unverified host could be captured. The safer fix is to confirm the new host-key fingerprint in cPanel or with Bluehost support and pin it in known_hosts.
2. **It will refuse to upload as written:** `vyb.png` is at the repo root (`~/Projects/Vyb/vyb.png`), not in `docs/web/landing_page/`, so the existence check exits with "missing vyb.png". That's harmless, but the og:image fix isn't actually wired up yet.
3. **`DEST` is unconfirmed.** The script itself says the addon-domain document root hasn't been verified. `scp` silently overwrites files of the same name at the destination, and that account also hosts aniviza.com. A wrong `DEST` could clobber another site's `index.html` or favicons. Confirm the root in cPanel (Domains → vyb-lang.org → Document Root) before the first run.
4. **The live check is weak.** The token `1314` already appears 7 times in the page, and it would stay unchanged across future edits that don't touch the test count. A stale page with the same count would pass. A per-build token, such as a commit hash or a hash of the upload, would be stronger.
5. **The check is cache-sensitive.** The site is behind Cloudflare DNS. If proxying or caching is ever enabled, `curl` may see a cached copy.
6. There's no backup of the existing remote files before overwrite, and no dry-run mode beyond `--check`.
7. It's scoped to **vyb-lang.org** only. It does nothing for vyb-os.org.

---

## 4. Findings by repo

### 4.1 Vyb: `README.md` (working tree on HEAD `c14d4768`)

At HEAD, the README is still byte-identical to the version audited on Oct 10 (md5 `8a8ef8daf0871dd388e0ece9026a94d9`). **Rick's working-tree edits fix one finding (the CI range in V5) and introduce one new unverified claim (V5a). The other 13 problems remain.** The full 132-row list is in `prior-vyb-readme-audit-DRAFT.md` in the watch baseline.

| # | Claim (README line) | What the code or CI shows | Sev. | Suggested wording |
|---|---|---|---|---|
| V1 | "1314 `.vyb` tests, all passing", "100% (1314/1314)" (3174, 3221, 3501, 3504, 3620) — **remains** | CI runs **1307 passed + 7 skipped** (CUDA FFI tests skip when the CUDA libs are absent on hosted runners). Debian 13: 1297 pass / 10 async fail. Line 3618 credits 1314 tests to v0.4.2 (anachronism). | High | "1314 `.vyb` tests: 1307 pass and 7 CUDA-library tests are skipped on hosted CI (Ubuntu 24.04)." |
| V2 | "`asm`-based fiber context switch" (64) — **remains** | `runtime/vyb_runtime.c:4082-4177` uses `ucontext` (`swapcontext`/`makecontext`). README line 76 itself says "ucontext/pipe2". | High | "cooperative multi-threaded executor (ucontext-based stackful fibers, Linux-only)" |
| V3 | "mature … The core language is stable and well tested" (1021, 3741) — **remains** | `TODO.md:224`: "approximately 60-65% complete toward a production 1.0 release". Self-hosting is ~0% (`TODO.md:222`). | High | "an actively developed pre-1.0 systems language (v0.7.7)" |
| V4 | "Cross-Platform: Linux and macOS support" (457) — **remains** | No macOS SDK (README 72-76 says so). Async is Linux-only. Only Ubuntu 24.04 is CI-tested. `release-sdk.yml` has a `macos-latest` job, but its outcome hasn't been checked. | High | "Linux x86-64 (CI-tested on Ubuntu 24.04). macOS is not supported yet." |
| V5 | ~~NVPTX "validated `sm_75` → `sm_90a`" in CI~~ → now "`sm_75` → `sm_90` in CI" (30, 236, 248, 472, 3287) — **fixed by Rick's edit** | Matches `.github/workflows/gpu-kernel.yml:144` (`sm_75 sm_80 sm_86 sm_90`). Still worth noting: CI covers 4 kernels, not "every kernel" (3287), and `doc/CUDA.md:48-53` excludes sm_75 from 1.0. | Low (residual) | Optional: "(4 kernels)" and "sm_75 is outside 1.0 support". |
| V5a | **New:** "`sm_90a` additionally emits and validates on a local toolchain" (30, 236, 248, 472, 3287; also in the landing page JSON-LD) | **No recorded evidence found on Godzilla:** no log, script, test, or CI step mentions sm_90a. The repo has only prose: `doc/FEATURE_STATUS.md:211` and `TODO.md:167` ("supported by ptxas"). Plausible: Godzilla has ptxas 12.0 / 12.5 / 12.8, which accept `sm_90a`, and `--gpu <arch>` is passed through (`src/main.cpp:1242, 5018`). But a passing run isn't recorded. | Medium (unverified) | Either add a reproducible check (e.g. a `test/` script or a CI arch entry for `sm_90a`) and cite it, or say "`sm_90a` is accepted by ptxas 12.x; not CI-tested". |
| V6 | "Deploy native executables without any runtime dependencies" (456) — **remains** | Dynamically linked to libc/libm by default (README 168-174, 242). Standalone only with `--static`. | Medium | "With `--static`, produces standalone executables; by default links libc/libm dynamically." |
| V7 | "Borrow checking and lifetime analysis prevent dangling pointers" (890); `their<T>` "lifetime checked" (3659, 3699) — **remains** | `doc/FEATURE_STATUS.md:94,119`: borrows are lexical by design, with **no lifetime inference**. | Medium | "Lexically scoped borrow checking (no lifetime inference)." |
| V8 | "Modern parallel test runner/harness" (994, 3560) — **remains** | Lines 3191, 3220, 3485: the runner is single-threaded and `--workers` is ignored. Parallelism comes only from 4 CI shards. | Medium | "Test harness with HTML/JSON reporting; CI runs the suite in 4 parallel shards." |
| V9 | Escaping closures share "one refcounted cell" (1106-1112) vs "the per-binding shared cell … is not implemented" (1136-1138) — **remains** | The README contradicts itself within 30 lines. | Medium | Reconcile against the closure tests, then keep one statement. |
| V10 | `main()<Int> -> { return 42 }` → "Exit code 42" (3681) — **remains** | Line 90 says "Prints 42 and exits 0". Measured: prints 42, exits 0. | Medium | "→ prints 42, exits 0" |
| V11 | `await` "(synchronous resolution today)" (3630) — **remains** | Stale v0.4.2 text. Lines 561-578 and 1159 describe the multi-threaded fiber executor. | Medium | Delete, or move to a dated changelog. |
| V12 | Project tree: `src/lexer.cpp`, `src/parser.cpp`, `src/ast.cpp` (~3295-3299) — **remains** | They live in `src/parser/`. `src/vre/` (semantic analysis + LLVM codegen) is missing. | Low | Regenerate the tree from `ls src/`. |
| V13 | `v.map(...)`, `v.filter(...)`, `v.first()` as `Vec` dot-calls (1935-1939) — **remains** | `FEATURE_STATUS.md:154`: "No such method on `Vec` yet". Free-function combinators exist. Unresolved which doc is wrong. | Low | Run the example. If it fails, show the free-function form. |
| V14 | "signed repositories", `std = "^1.0.0" # Signed, from Vyb registry` (288, 338, ~1828, ~3705) — **remains** | No signing registry found in the tree. `std` 1.0.0 vs language 0.7.7. | Low (risky) | Remove "signed" unless a signing registry exists. |
| V15 | `select` "uniquely Vyb, not found in other languages" (58, ~411) — **remains** | Value-yielding `match` exists in Rust and Scala. Marketing claim. | Low | "Vyb's `select` expression…" (drop "unique"). |

**Accurate claims you can reuse:** v0.7.7; x86-64/Linux release-tested; other LLVM targets "untested"; Full JIT (LLVM ORC); pthread-backed threads, Mutex, Cond, atomics; CUDA cuBLAS/cuFFT/cuDNN bindings verified on an RTX 3090; NVPTX kernel mode is a "P0 feasibility probe"; ASCII-only `to_upper`/`to_lower`; DWARF debug info validated in CI.

**Omissions to keep in mind:** the README has **no** model, inference, or tok/s claims. Don't attribute VybForge results to Vyb. Regex is POSIX ERE over bytes (no Unicode classes). The JIT can't pass argv.

### 4.2 Vyb landing page: `docs/web/landing_page/index.html` → vyb-lang.org

| # | Claim | What's true | Sev. | Suggested action |
|---|---|---|---|---|
| L1 | **Deployed** page: v0.7.3, "1061 .vyb tests" (Last-Modified Aug 22) — **remains** | Current release v0.7.7. Suite is 1314 (1307 pass + 7 skipped). | High | Redeploy from a corrected README render (Rick's step). |
| L2 | `main` source: v0.7.6, release link v0.7.5, "Linux x64 / macOS x64" SDK, 1310 tests — **fixed in Rick's working tree** (not yet committed) | The working-tree page says v0.7.7, and the macOS-SDK and 1310 strings are gone. GitHub main still has the old page, and PR #405 is open. | Medium | Rick to commit, and to decide on PR #405. |
| L3 | Working-tree page inherits V1–V4, V6–V15 and the new V5a from the README — **remains** | It's a README render ("1314" appears 7 times, "mature" twice, "Linux and macOS support" once). | High | Fix the README first, then regenerate. |
| L4 | **New (shell):** `<meta name="keywords">` includes "self-hosting programming language" and "Rust alternative" | Self-hosting is ~0% (`TODO.md:222`). "Rust alternative" is positioning, not a capability. | Medium | Drop "self-hosting programming language" (or say "planned"). Rick decides on "Rust alternative". |
| L5 | **New (shell):** JSON-LD says "cross-compilation to a wide range of target architectures" | Only x86-64/Linux is release-tested. NVPTX is ptxas-validated only. Other targets are untested (README ~237). | Medium | "x86-64/Linux (release-tested) and NVPTX; other LLVM targets untested". |
| L6 | Example links `github.com/user/utils`, `github.com/dev/tools`, `https://...` | Placeholders | Low | Fine in code samples. Never in real navigation. |
| L7 | `og:image /vyb.png` 404 on the live root — **remains** | `deploy.sh` now lists it, but `vyb.png` isn't in `docs/web/landing_page/`, so the script stops before uploading (§3.4). | Low | Copy or link `vyb.png` into the page directory (Rick's call). |
| L8 | External link and badge to `deepwiki.com/rickenator/Vyb` | Third-party site. Not a claim. | Info | Keep only if Rick wants it. Don't add more third-party embeds without sign-off. |

### 4.3 VybOS: `README.md` (HEAD `628025ff`)

| # | Claim (line) | What the code or docs show | Sev. | Suggested wording |
|---|---|---|---|---|
| O1 | "PID 1 is Vyb." (16, 21) | `doc/ARCHITECTURE.md:78` ("A Vyb service manager / PID 1 is a late…") and `:318` (Phase 5, optional). Boot today uses a config-driven BusyBox/POSIX `/init` stand-in (`PLAN_BOOTABLE_IMAGE.md:108, 211-212`). | High | "Configuration is Vyb; boot currently uses a BusyBox `/init`. A Vyb PID 1 is planned for a later phase." |
| O2 | Repo description: "entire build/config/init framework is written in JIT Vyb" | Init isn't Vyb. `tools/vybos-run` and `tools/vybctl` are bash. | Medium | "The build and configuration framework is written in Vyb, with a few bash helper tools." |
| O3 | "no Python or Rust runtime in the core design" (26) | Consistent: 77 `.vyb` files, 2 `.sh` + 2 bash tools. | OK | Reuse. |
| O4 | Derived linux-6.6 bzImage boots; derived QEMU 8.2.2; "only store artifacts — no host qemu, no host toolchain" (82-88) | Not verified. A strong claim. | Low | Verify against `doc/STATUS.md` before reusing. |
| O5 | Byte-deterministic kernel, reproducibility proofs (244-245) | Not verified | Low | Reuse only with a link to the proof artifacts. |
| O6 | CUDA/NVPTX/SIMD/training/inference "being developed" (23) | Correct as future tense | OK | **Keep it in future tense.** |
| O7 | M0/M1/M2 done; boots to `VYBOS_READY` in a container and QEMU (44-67) | Consistent with the repo, dated Aug 23-25 | OK | Reuse with dates. |

### 4.4 VybFly: `README.md` (HEAD `055748ce`)

| # | Claim (line) | What the code or docs show | Sev. | Suggested wording |
|---|---|---|---|---|
| F1 | "roughly **500 million synapses**" (18) | The README's own table (line 53): 1× real brain = 139,255 neurons, 2,700,513 connections, **34,153,566 synapses**. FlyWire reports ~50M. Off by ~10×. | High | "about 139,000 neurons and roughly 50 million synapses (34 million in the FlyWire dataset we use)" |
| F2 | FlyWire is "the first complete wiring diagram of a brain that anyone has ever had" (20-21) | C. elegans (1986) and the larval *Drosophila* connectome (2023) came first. | High | "the first complete wiring diagram of an adult fly brain" |
| F3 | 139,255 neurons; 2×/5×/10× table up to 1.39M neurons / 27.8M connections (27, 51-56) | Consistent with repo results | OK | Reuse. |
| F4 | M3 GPU kernels match the CPU tick for tick on a 512-neuron fixture; whole-connectome GPU still fixture-scale (100, 192, 205) | Honest scoping | OK | **Keep "fixture-scale".** Don't imply the whole connectome runs on the GPU. |
| F5 | Downscaled replicas overlap 85-95% (59) | Not verified | Low | Cite the result file or drop it. |
| F6 | Energy 438 → 2,022 J measured on GPU (74, 181) | Partly verified. No brain-efficiency comparison is made. | Low | Don't add efficiency-vs-brain claims. |
| F7 | M15 100× scale: gated, not attempted (210) | Consistent | OK | Keep as "not attempted". |

### 4.5 VybForge: `README.md` (HEAD `77aed0d0`)

| # | Claim (line) | What the code or measurements show | Sev. | Suggested wording / rule |
|---|---|---|---|---|
| G1 | Vyb-native GPU decode of Qwen3-4B, zero Python in production; Python is the reference only (76-78, 225) | Consistent: token-exact real-model chat (`doc/CHAT-SERVER.md`). | OK | Reuse. |
| G2 | Performance: (no tok/s in README) | Public main ≈ **15.7 tok/s** (≈16) on the RTX 3090 after issue #23. 58 / ~112 tok/s are **local-only, unpublished** commits. For reference, llama.cpp does ~209 tok/s decode on the same 3090. | High (if added) | If the site gives a number, use "about 16 tok/s on an RTX 3090 (Qwen3-4B Q4_K_M, public main)". Never 58 or 112 until Rick publishes. |
| G3 | 27B / Qwen3-27B | No generation loop or detokenizer on main (A5 open). Never generated text end to end. | High (if added) | Don't claim it. At most: "27B bring-up in progress". |
| G4 | "the bar is anything PyTorch can do" (79) | An aspiration, not a capability | Medium | Keep as a goal ("aims to…"). Never present it as parity. |
| G5 | Tested on Godzilla's RTX 3090 (175) | Consistent. GB10/DGX Spark results are **not** in the README or on main. | OK | Don't add GB10/Spark claims yet. |
| G6 | "`tools/apply.vyb` (Vyb 0.7.3)" (50) | Stale. Vyb is 0.7.7. | Low | "Vyb 0.7.7" |
| G7 | "216 train / 24 eval records" (131) vs chat render "byte-identical on all 720 records" (217) | 216 + 24 = 240, not 720. Possibly a different corpus. Unreconciled. | Low | Name the corpus each number refers to. |
| G8 | Repo description: "… LoRA fine-tuning … without Python or PyTorch as the runtime" | Inference is Python-free. **Training still uses Python QLoRA** (`training/train_lora.py`). | Medium (if extended) | Keep "no Python" scoped to inference. |
| G9 | Kernel provenance | `THIRD-PARTY.md` credits llama.cpp (MIT) for the Q4_K/Q6_K block decode in `native/kernels/q4k.vyb`, `q6k.vyb`. The upstream revision isn't pinned yet. | Medium | Any kernel or performance page must link THIRD-PARTY.md. |

---

## 5. Top findings (priority order)

1. **VybOS "PID 1 is Vyb" is false today.** Boot uses BusyBox `/init`, and a Vyb PID 1 is a later, optional phase. This is the most likely claim to end up on a site called vyb-os.org.
2. **Vyb test claims "1314/1314, all passing" are wrong.** CI is 1307 pass + 7 skipped. The same claim repeats in five places and flows into vyb-lang.org.
3. **VybFly "500 million synapses" is off by ~10×.** Its own table says 34.2M. The "first complete brain wiring diagram" claim is also false (C. elegans and the larval fly came first).
4. **VybForge speed must be quoted as ~16 tok/s (public main).** The 58 / ~112 tok/s results are unpublished local work. 27B doesn't generate text.
5. **Vyb still overclaims maturity and platforms:** "mature", "Linux and macOS", and "asm-based fibers" contradict the code (60-65% to 1.0, Linux-only, ucontext). Rick's edit fixed the CI arch range, but the new "sm_90a validates on a local toolchain" has no recorded evidence yet, and the page keywords add "self-hosting programming language".

Also: vyb-lang.org is serving v0.7.3 from August. Rick's regenerated v0.7.7 page exists only in his working tree. The new `deploy.sh` disables ssh host-key checking and would currently stop because `vyb.png` is missing (§3.4).

---

## 6. Open questions for Rick

1. **Hosting for vyb-os.org:** Cloudflare Pages, Bluehost (like vyb-lang.org), or something else? Should the contractor have access to the Cloudflare zone, or should they hand you the DNS records to add?
2. **Content scope:** is vyb-os.org about VybOS only, or the whole Vyb family (language, VybForge, VybFly, VybOS)? Should vyb-lang.org stay the language site?
3. **PR #405:** merge, close, or supersede it with a fresh landing-page PR from the corrected README?
4. **Your in-progress Vyb edits:** will you commit them (and on which branch) before the contractor starts, so their PRs base on your version? Can you back "`sm_90a` validates on a local toolchain" with a log or test, or should it say "accepted by ptxas 12.x, not CI-tested"?
4b. **`deploy.sh`:** confirm the vyb-lang.org document root and the new Bluehost host-key fingerprint. Pin that key instead of `StrictHostKeyChecking=no`. Also: should `deploy.sh` be committed, given it names the account and IP? And where should `vyb.png` live?
5. **VybForge performance:** do you want to publish the perf branch (~112 tok/s on the 3090, ~58 on GB10) before launch, so the site can cite it? Until then, the site says ~16 tok/s.
6. **Default model decision** (imatrix / Q6_K-mix) is still pending. It affects any quality or speed number on the site.
7. **Branding and legal:** confirm the attribution line ("Aniviza LLC"), the contact email, and whether analytics are allowed.
8. **Review process:** one PR per repo, reviewed by you before merge. Is anyone else allowed to approve?

---

## 7. Source and method

- Claims checked against shallow clones of each repo at the HEADs above (`/workspace/vyb-os-watch/clones/`) and the baseline snapshot taken Oct 19, 6:39 PM PT (`/workspace/vyb-os-watch/baseline/`: `manifest.json`, `README-claims-checklist.md`, `prior-vyb-readme-audit-DRAFT.md`, `site/`, `meta/`).
- Rescanned Rick's working tree on Godzilla on Oct 20, ~8:35 AM PT: `git status`, `git diff`, the untracked files, and `deploy.sh` (read only, not executed). I searched for sm_90a evidence in the repo, the CUDA toolchains, and the `src/main.cpp` `--gpu` path.
- Re-verified for this handoff: Vyb README lines 30/64/234/246/454/455/470/888/992/1019/3285/3502/3558/3679/3739; `gpu-kernel.yml:144`; `TODO.md:224`; `vyb_runtime.c:4082-4177`; VybOS README 16/21/26 and `ARCHITECTURE.md:78,318`; VybFly README 18-21/53; VybForge README 50/79/131/175/217 and `THIRD-PARTY.md`.
- Items marked "not verified" were not run. Verify them before publishing.
- A watch runs on weekdays and compares README and site changes against this list.
