#!/usr/bin/env python3
"""End-to-end smoke test for the directory package registry + `version:` deps.

Publishes a package through `vyb mod publish`, then a consumer that declares
`name = { version = "..." }` resolves it from the registry (VYB_REGISTRY),
materializes it into .vybmod/, records source="version" in vyb.lock, builds, and
runs a binary whose output comes from the published module. Also checks version
selection (a "1" spec resolves to the highest 1.x, not 2.0.0).
"""
import os, shutil, subprocess, sys, tempfile

VYB = os.environ.get("VYB", os.path.join(os.path.dirname(__file__), "..", "build", "vyb"))
VYB = os.path.abspath(VYB)
STDLIB = os.path.join(os.path.dirname(__file__), "..", "stdlib")

def sh(cmd, cwd=None, env=None, check=True):
    r = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True, env=env or os.environ)
    if check and r.returncode != 0:
        raise RuntimeError("cmd failed %s\n%s\n%s" % (cmd, r.stdout, r.stderr))
    return r

def write(path, text):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    open(path, "w").write(text)

def main():
    tmp = tempfile.mkdtemp(prefix="vybreg_")
    reg = os.path.join(tmp, "registry")
    env = dict(os.environ, VYB_STDLIB=STDLIB, VYB_REGISTRY=reg)

    # package v1.0.0 and sibling 1.1.0 (to exercise prefix selection)
    for ver, msg in [("1.0.0", "v1"), ("1.1.0", "v11"), ("2.0.0", "v2")]:
        pkg = os.path.join(tmp, "pkg_" + ver)
        write(pkg + "/vyb.toml", '[package]\nname = "Gdemo"\nversion = "%s"\n' % ver)
        write(pkg + "/mod.vyb", 'share(all)\ngreet()<String> -> { return "hi-%s" }\n' % msg)
        sh([VYB, "mod", "publish", pkg], env=env)

    # each published version dir must contain mod.vyb
    for ver in ("1.0.0", "1.1.0", "2.0.0"):
        assert os.path.isfile(os.path.join(reg, "Gdemo", ver, "mod.vyb")), ver

    # consumer A: spec "1.1.x via exact" -> 1.1.0; consumer B: prefix "1" -> 1.1.0
    cons = os.path.join(tmp, "consumer")
    write(cons + "/vyb.toml",
        '[package]\nname = "consumer"\nversion = "0.1.0"\n\n[dependencies]\nGdemo = { version = "1" }\n')
    write(cons + "/src/main.vyb",
        'import Gdemo::{greet}\nmain()<Int> -> {\n    s<String> = greet()\n    println(s)\n    return 0\n}\n')

    r = sh([VYB, "build"], cwd=cons, env=env)
    assert "Fetching Gdemo@1.1.0" in r.stderr, r.stderr   # prefix "1" picks highest 1.x
    lock = open(os.path.join(cons, "vyb.lock")).read()
    assert 'source = "version"' in lock, lock
    assert r.returncode == 0

    run = sh([os.path.join(cons, "target", "consumer")])   # built binary needs no VYB env
    assert "hi-v11" in run.stdout, run.stdout

    # consumer B: exact 1.0.0
    consB = os.path.join(tmp, "consumerB")
    write(consB + "/vyb.toml",
        '[package]\nname = "consumerB"\nversion = "0.1.0"\n\n[dependencies]\nGdemo = { version = "1.0.0" }\n')
    write(consB + "/src/main.vyb",
        'import Gdemo::{greet}\nmain()<Int> -> {\n    s<String> = greet()\n    println(s)\n    return 0\n}\n')
    rb = sh([VYB, "build"], cwd=consB, env=env)
    assert "Fetching Gdemo@1.0.0" in rb.stderr, rb.stderr
    rb_run = sh([os.path.join(consB, "target", "consumerB")])
    assert "hi-v1" in rb_run.stdout, rb_run.stdout

    print("registry smoke: 5/5 checks passed (publish, prefix->highest, exact, lockfile source=version, run)")
    shutil.rmtree(tmp, ignore_errors=True)

if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print("FAIL: %s" % e)
        sys.exit(1)
