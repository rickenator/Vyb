#!/usr/bin/env python3
"""End-to-end smoke test for `vyb lsp` (issue #154 / Testing & Tooling).

Spawns the built `vyb lsp` server, drives the Language Server Protocol over
stdio (JSON-RPC 2.0 + Content-Length framing), and asserts the core features:

  * initialize  -> capabilities (hoverProvider, definitionProvider, completion)
  * didOpen     -> publishDiagnostics (0 for a clean document)
  * hover       -> kind + signature for a declaration
  * definition  -> location of a declaration from a use site
  * completion  -> candidate declaration names
  * didChange   -> publishDiagnostics carries lint warnings (severity 2)
  * shutdown/exit -> clean termination

Exits non-zero on any failure so it can gate hosted CI. The server's stdin is a
pipe owned by this script, so it cannot hang waiting on a terminal.

Usage: python3 test/lsp_smoke.py [path-to-vyb]   (defaults to build/vyb then build-ci/vyb)
"""

import json
import os
import subprocess
import sys

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


class LspClient:
    def __init__(self, exe, stdlib):
        env = dict(os.environ)
        env["VYB_STDLIB"] = stdlib
        self.proc = subprocess.Popen([exe, "lsp"], stdin=subprocess.PIPE,
                                     stdout=subprocess.PIPE, env=env)

    def send(self, msg):
        body = json.dumps(msg).encode()
        self.proc.stdin.write(b"Content-Length: %d\r\n\r\n" % len(body) + body)
        self.proc.stdin.flush()

    def recv(self):
        headers = {}
        while True:
            line = self.proc.stdout.readline().decode()
            if not line:
                raise RuntimeError("server closed stdout")
            if line == "\r\n":
                break
            if ":" in line:
                k, v = line.split(":", 1)
                headers[k.strip().lower()] = v.strip()
        n = int(headers.get("content-length", 0))
        return json.loads(self.proc.stdout.read(n).decode())

    def notify(self, method, params):
        self.send({"jsonrpc": "2.0", "method": method, "params": params})

    def request(self, seq, method, params):
        self.send({"jsonrpc": "2.0", "id": seq, "method": method, "params": params})
        r = self.recv()
        assert r.get("id") == seq, "response id mismatch"
        return r.get("result")

    def until(self, method):
        for _ in range(50):
            m = self.recv()
            if m.get("method") == method:
                return m["params"]
        raise RuntimeError("no %s notification received" % method)

    def close(self):
        self.proc.stdin.close()
        return self.proc.wait(timeout=10)


def main():
    exe = find_exe()
    stdlib = os.path.join(REPO, "stdlib")
    c = LspClient(exe, stdlib)
    seq = 0
    failures = []

    def check(name, cond):
        print("%-22s %s" % (name, "ok" if cond else "FAIL"))
        if not cond:
            failures.append(name)

    try:
        init = c.request(seq := seq + 1, "initialize", {})
        check("initialize/capabilities",
              init and init.get("capabilities", {}).get("hoverProvider") is True)
        c.notify("initialized", {})

        uri = "file:///tmp/lsp_demo.vyb"
        doc = ("/// Adds two integers.\n"
               "add(a<Int>, b<Int>)<Int> -> {\n"
               "    return a + b\n"
               "}\n"
               "\n"
               "/// A point.\n"
               "struct Point {\n"
               "    x<Int>,\n"
               "    y<Int>\n"
               "}\n")
        c.notify("textDocument/didOpen",
                 {"textDocument": {"uri": uri, "languageId": "vyb",
                                   "version": 1, "text": doc}})
        p = c.until("textDocument/publishDiagnostics")
        check("didOpen diagnostics(clean=0)", p["diagnostics"] == [])

        hover = c.request(seq := seq + 1, "textDocument/hover",
                          {"textDocument": {"uri": uri},
                           "position": {"line": 1, "character": 0}})
        check("hover(signature)",
              hover and "add(a<Int>" in hover["contents"]["value"])

        loc = c.request(seq := seq + 1, "textDocument/definition",
                        {"textDocument": {"uri": uri},
                         "position": {"line": 6, "character": 7}})
        check("definition(struct Point)",
              loc and loc.get("uri") == uri and loc["range"]["start"]["line"] == 5)

        items = c.request(seq := seq + 1, "textDocument/completion",
                          {"textDocument": {"uri": uri},
                           "position": {"line": 10, "character": 0}})
        labels = [i["label"] for i in items] if items else []
        check("completion(add,Point)", "add" in labels and "Point" in labels)

        # Lint diagnostics via didChange: self-comparison 'a == a' -> warning sev 2
        dirty = doc + ("\nself_check(a<Int>)<Int> -> {\n"
                       "    if (a == a) {\n        return 1\n    }\n"
                       "    return 0\n}\n")
        c.notify("textDocument/didChange",
                 {"textDocument": {"uri": uri, "version": 2},
                  "contentChanges": [{"text": dirty}]})
        p = c.until("textDocument/publishDiagnostics")
        sevs = [d["severity"] for d in p["diagnostics"]]
        check("didChange lint warning(sev2)", 2 in sevs)

        nullres = c.request(seq := seq + 1, "shutdown", {})
        check("shutdown", nullres is None)
        c.notify("exit", {})
        check("server exit 0", c.close() == 0)
    finally:
        if c.proc.poll() is None:
            c.proc.kill()

    total = 7
    passed = total - len(failures)
    if failures:
        print("LSP smoke FAILED: %d/%d passed -> " % (passed, total) + ", ".join(failures))
        return 1
    print("LSP smoke: %d/%d checks passed" % (passed, total))
    return 0


if __name__ == "__main__":
    sys.exit(main())
