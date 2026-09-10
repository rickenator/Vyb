#!/usr/bin/env python3
"""End-to-end smoke test for `git:`-clone dependencies in `vyb build` (#165).

Sets up a scratch directory:
  * a local git repo whose root is a Vyb module (share(all) + one function)
  * a consumer project whose vyb.toml declares a `git:` dependency pointing at it

Then runs `vyb build` and asserts: the dep is auto-cloned into .vybmod/<name>/,
vyb.lock records `source="git"` + the resolved path, the build succeeds, and the
linked binary prints the module function's output. This is the auto-fetch-on-build
path for git deps (no `vyb mod install` step needed).

Requires `git` to be on PATH. Usage: python3 test/gitdep_smoke.py [path-to-vyb]
"""

import os
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(HERE)


def find_exe():
    for cand in sys.argv[1:]:
        if os.path.exists(cand):
            return os.path.abspath(cand)
    for cand in ("build/vyb", "build-ci/vyb"):
        p = os.path.join(REPO, cand)
        if os.path.exists(p):
            return os.path.abspath(p)
    raise SystemExit("no built `vyb` binary found (build/vyb or build-ci/vyb); pass a path")


def sh(env, *args, check=True):
    r = subprocess.run(args, env=env, capture_output=True, text=True)
    if check and r.returncode != 0:
        raise SystemExit("command failed: %s\n%s\n%s" % (
            " ".join(args), r.stdout, r.stderr))
    return r


def main():
    if shutil.which("git") is None:
        print("SKIP: git not available")
        return 0
    exe = find_exe()
    env = dict(os.environ)
    env["VYB_STDLIB"] = os.path.join(REPO, "stdlib")
    env.pop("VYB_MODULE_PATH", None)

    tmp = tempfile.mkdtemp(prefix="vyb_gitdep_")
    repo = os.path.join(tmp, "grepmod")
    consumer = os.path.join(tmp, "consumer")
    try:
        # 1. git repo whose root is a Vyb module
        os.makedirs(repo)
        with open(os.path.join(repo, "mod.vyb"), "w") as f:
            f.write("share(all)\n"
                    "exclaim(s<String>)<String> -> {\n"
                    "    return s + \"!\"\n"
                    "}\n")
        sh(env, "git", "-C", repo, "init", "-q")
        sh(env, "git", "-C", repo, "config", "user.email", "t@t")
        sh(env, "git", "-C", repo, "config", "user.name", "t")
        sh(env, "git", "-C", repo, "add", ".")
        sh(env, "git", "-C", repo, "commit", "-qm", "init")

        # 2. consumer with a git: dependency
        os.makedirs(os.path.join(consumer, "src"))
        with open(os.path.join(consumer, "vyb.toml"), "w") as f:
            f.write('[package]\nname = "grepcon"\nversion = "0.1.0"\n\n'
                    '[[bin]]\nname = "grepcon"\npath = "src/main.vyb"\n\n'
                    '[dependencies]\n'
                    'greeter = { git = "file://%s" }\n' % repo)
        with open(os.path.join(consumer, "src/main.vyb"), "w") as f:
            f.write("import greeter::{exclaim}\n"
                    "main()<Int> -> {\n"
                    "    println(exclaim(\"ab\"))\n"
                    "    return 0\n"
                    "}\n")

        # 3. build (auto-clones the dep); run inside the consumer dir
        r = subprocess.run([exe, "build"], cwd=consumer,
                           capture_output=True, text=True, env=env)
        if r.returncode != 0:
            print("FAIL: vyb build failed\n%s\n%s" % (r.stdout, r.stderr))
            return 1

        mod = os.path.join(consumer, ".vybmod/greeter/mod.vyb")
        if not os.path.exists(mod):
            print("FAIL: git dep not materialized at %s" % mod)
            return 1

        lock = open(os.path.join(consumer, "vyb.lock")).read()
        if 'source = "git"' not in lock or "greeter" not in lock:
            print("FAIL: vyb.lock missing git:\n%s" % lock)
            return 1

        binpath = os.path.join(consumer, "target/grepcon")
        if not os.path.exists(binpath):
            print("FAIL: binary not built: %s" % binpath)
            return 1
        out = subprocess.run([binpath], capture_output=True, text=True, env=env)
        if out.returncode != 0 or "ab!" not in out.stdout:
            print("FAIL: git dep binary bad: rc=%s stdout=%r" % (out.returncode, out.stdout))
            return 1

        print("gitdep smoke: 4/4 checks passed (clone-on-build, lockfile, build, run)")
        return 0
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


if __name__ == "__main__":
    sys.exit(main())
