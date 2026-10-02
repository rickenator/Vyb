# Installing the Vyb SDK

The SDK ships as a per-platform tarball: `vyb-sdk-<version>-<os>-<arch>.tar.gz`.
The supported matrix is:

| Target | Artifact suffix | Shipped? |
| --- | --- | --- |
| Linux / x86_64 | `linux-x86_64` | ✅ shipped on every release |
| macOS | `macos-x86_64` / `macos-aarch64` | ❌ **not shipped** — the runtime's async/fibre layer uses Linux-only `ucontext` context-switch routines and the `pipe2` syscall, so the release workflow cannot build the runtime on macOS. No macOS artifact exists on any release; a macOS port is a separate runtime effort, not a packaging gap |

## Steps

1. **Download** the tarball matching your OS/arch from the release page. The
   release attaches the tarball; the `manifest.json` describing its contents
   (per-file sha256 digests included) is **inside** the tarball.

2. **Extract** to a location of your choice:

   ```bash
   tar -xzf vyb-sdk-<version>-<os>-<arch>.tar.gz
   cd vyb-sdk-<version>
   ```

3. **Verify integrity** — the tarball digest, then every file against the
   `manifest.json` shipped inside the tarball (it records a sha256 per file):

   ```bash
   sha256sum ../vyb-sdk-<version>-<os>-<arch>.tar.gz
   python3 -c "import json,hashlib; m=json.load(open('manifest.json')); \
   [print(('BAD ' if hashlib.sha256(open(e['path'],'rb').read()).hexdigest()!=e['sha256'] else 'OK ')+e['path']) \
   for e in m['files']]" | grep BAD || echo "all files match manifest"
   ```

4. **Set up the environment** by sourcing `env.sh` (adds `bin/` to `PATH` and
   sets `VYB_STDLIB`):

   ```bash
   source env.sh
   ```

5. **Verify** the toolchain responds:

   ```bash
   vyb --version       # prints version + build configuration
   vyb help            # subcommand overview
   vyb hello.vyb       # run a program (JIT)
   ```

## Environment

`sdk/env.sh` is meant to be `source`d, not executed. It:
- prepends `bin/` to `PATH` (idempotent — running it twice is safe);
- exports `VYB_STDLIB` to the bundled `stdlib/`;
- only touches `LD_LIBRARY_PATH` if the SDK actually ships shared libraries.

Prefer `source env.sh` for interactive use. In scripts, equivalently:

```bash
export PATH="<sdk>/bin:$PATH"
export VYB_STDLIB="<sdk>/stdlib"
```

Building from a source checkout instead of the packaged SDK? The Vyb repository
ships a toplevel `SOURCEME_VYB` that resolves that checkout and exports
`VYBHOME` plus the derived `VYB` (`$VYBHOME/build/vyb`) and `VYB_STDLIB`
(`$VYBHOME/stdlib`) -- one knob, no path literals:

```sh
. "$HOME/Projects/Vyb/SOURCEME_VYB"
```

It leaves `PATH` alone (this SDK's `env.sh` is the one that prepends `bin/`), so a
project can source either depending on whether it is pointed at a checkout or at
an installed SDK.

## Contents

The SDK bundles `bin/vyb` (Release compiler) and `bin/vyb-bindgen`, the standard
library sources, the runtime objects, an empty `include/vendor/` for optional
bindings, the full documentation set under `docs/` (refman + manifest +
getting-started), and `manifest.json`. See `GETTING_STARTED.md` for the quick
tour.
