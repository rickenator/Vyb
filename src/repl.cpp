// SPDX-License-Identifier: Apache-2.0

// Vyb REPL implementation (see repl.hpp).

#define _GNU_SOURCE  // for mkstemps()

#include "vyb/repl.hpp"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <vector>

namespace vyb {
namespace repl {

namespace {

enum class Kind { DECL, ASSIGN, STMT, EXPR };

std::string trim(const std::string& s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace((unsigned char)s[a])) ++a;
    while (b > a && std::isspace((unsigned char)s[b - 1])) --b;
    return s.substr(a, b - a);
}

bool startsWithWord(const std::string& s, const char* w) {
    size_t n = std::strlen(w);
    if (s.size() < n || s.compare(0, n, w) != 0) return false;
    return s.size() == n || !(std::isalnum((unsigned char)s[n]) || s[n] == '_');
}

bool isIdentStart(char c) { return std::isalpha((unsigned char)c) || c == '_'; }

// Net bracket depth change for a line, skipping "..." strings and // # comments.
int bracketDelta(const std::string& line) {
    int depth = 0;
    bool inStr = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (inStr) {
            if (c == '\\') { ++i; continue; }
            if (c == '"') inStr = false;
            continue;
        }
        if (c == '"') { inStr = true; continue; }
        if (c == '#' || (c == '/' && i + 1 < line.size() && line[i + 1] == '/')) break;
        if (c == '{' || c == '(' || c == '[') ++depth;
        else if (c == '}' || c == ')' || c == ']') --depth;
    }
    return depth;
}

bool containsTopLevelAssign(const std::string& line) {
    int depth = 0;
    bool inStr = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (inStr) { if (c == '\\'){++i;continue;} if (c == '"') inStr = false; continue; }
        if (c == '"') { inStr = true; continue; }
        if (c == '#' || (c == '/' && i + 1 < line.size() && line[i + 1] == '/')) break;
        if (c == '(' || c == '[' || c == '{') { ++depth; continue; }
        if (c == ')' || c == ']' || c == '}') { if (depth) --depth; continue; }
        if (c == '=' && depth == 0) {
            char next = (i + 1 < line.size()) ? line[i + 1] : 0;
            char prev = (i > 0) ? line[i - 1] : 0;
            // '==', '=>', '<=', '>=', '!=', '+=', ... are not assignments
            if (next == '=' || next == '>' ||
                std::strchr("<>!=+-*/%&|^", prev) != nullptr) continue;
            return true; // assignment / definition
        }
    }
    return false;
}

// Is this the signature line of a function declaration (name(...)<T> -> { ... })?
bool isFunctionDeclLine(const std::string& line) {
    if (line.empty() || !isIdentStart(line[0])) return false;
    int depth = 0; bool inStr = false;
    bool sawOpen = false;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (inStr) { if (c == '\\'){++i;continue;} if (c == '"') inStr = false; continue; }
        if (c == '"') { inStr = true; continue; }
        if (c == '#' || (c == '/' && i + 1 < line.size() && line[i + 1] == '/')) break;
        if (c == '(') { ++depth; sawOpen = true; continue; }
        if (c == ')' && depth) { --depth; continue; }
        if (c == '-' && i + 1 < line.size() && line[i + 1] == '>' && depth == 0 && sawOpen)
            return true;
    }
    return false;
}

Kind classify(const std::string& line0) {
    const std::string& s = trim(line0);
    for (const char* kw : {"import", "struct", "enum", "type", "aspect", "bind", "class"})
        if (startsWithWord(s, kw)) return Kind::DECL;
    if (isFunctionDeclLine(s)) return Kind::DECL;
    if (containsTopLevelAssign(s)) return Kind::ASSIGN;
    for (const char* kw : {"if", "for", "while", "return", "match", "else", "break",
                           "continue", "assert", "println", "print", "eprintln", "flush"})
        if (startsWithWord(s, kw)) return Kind::STMT;
    // bare identifier / literal / expression -> auto-display value
    return Kind::EXPR;
}

// ---- subprocess evaluation -------------------------------------------------
// Runs `exe <path>` capturing stdout+stderr. Returns process exit code.
int runProgram(const std::string& exe, const std::string& path,
               std::string& out, std::string& err) {
    int outPipe[2], errPipe[2];
    if (pipe(outPipe) != 0 || pipe(errPipe) != 0) return -1;
    pid_t pid = fork();
    if (pid == 0) {
        dup2(outPipe[1], STDOUT_FILENO);
        dup2(errPipe[1], STDERR_FILENO);
        close(outPipe[0]); close(outPipe[1]); close(errPipe[0]); close(errPipe[1]);
        execl(exe.c_str(), exe.c_str(), path.c_str(), (char*)nullptr);
        _exit(127);
    }
    if (pid < 0) return -1;
    close(outPipe[1]); close(errPipe[1]);
    char buf[4096];
    ssize_t n;
    // Drain both pipes (they may block if one fills); read out first fully, then err.
    bool outOpen = true, errOpen = true;
    while (outOpen || errOpen) {
        fd_set rfds; FD_ZERO(&rfds);
        if (outOpen) FD_SET(outPipe[0], &rfds);
        if (errOpen) FD_SET(errPipe[0], &rfds);
        int maxfd = std::max(outPipe[0], errPipe[0]) + 1;
        if (select(maxfd, &rfds, nullptr, nullptr, nullptr) <= 0) break;
        if (outOpen && FD_ISSET(outPipe[0], &rfds)) {
            n = read(outPipe[0], buf, sizeof buf);
            if (n <= 0) { outOpen = false; close(outPipe[0]); }
            else out.append(buf, (size_t)n);
        }
        if (errOpen && FD_ISSET(errPipe[0], &rfds)) {
            n = read(errPipe[0], buf, sizeof buf);
            if (n <= 0) { errOpen = false; close(errPipe[0]); }
            else err.append(buf, (size_t)n);
        }
    }
    int status = 0;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) ? WEXITSTATUS(status) : (WIFSIGNALED(status) ? 128 + WTERMSIG(status) : 1);
}

std::string makeTempFile(const std::string& body) {
    const char* td = getenv("TMPDIR");
    std::string tmpl = std::string(td && *td ? td : "/tmp") + "/vyb_repl_XXXXXX.vyb";
    std::vector<char> buf(tmpl.begin(), tmpl.end());
    buf.push_back('\0');
    int fd = mkstemps(buf.data(), 4); // 4 = keep ".vyb"
    if (fd < 0) return "";
    std::string path(buf.data());
    (void)!write(fd, body.data(), body.size());
    close(fd);
    return path;
}

} // namespace

int runRepl(const std::string& selfExe) {
    bool tty = isatty(STDIN_FILENO) != 0;
    std::vector<std::string> decls, body;

    auto prompt = [&](const char* p) {
        if (tty) { std::fputs(p, stdout); std::fflush(stdout); }
        else { std::fputs(p, stderr); std::fflush(stderr); }
    };

    auto printSession = [&]() {
        std::string prog;
        for (auto& d : decls) { prog += d; prog += "\n"; }
        prog += "\nmain() -> {\n";
        for (auto& b : body) { prog += b; prog += "\n"; }
        prog += "}\n";
        return prog;
    };

    if (tty) {
        std::printf("Vyb REPL (JIT backend). :help for commands, :quit to exit.\n");
    }

    prompt("vyb> ");
    std::string line;
    while (std::getline(std::cin, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::string entry = line;
        int depth = 0;
        // multi-line: keep reading until brackets balance
        while (true) {
            int d = bracketDelta(line);
            depth += d;
            if (depth > 0) { prompt("... "); if (!std::getline(std::cin, line)) break; if (!line.empty() && line.back()=='\r') line.pop_back(); entry += "\n" + line; }
            else break;
        }
        std::string t = trim(entry);
        if (t.empty()) { prompt("vyb> "); continue; }

        // REPL commands
        if (t == ":q" || t == ":quit" || t == ":exit" || t == ":bye") break;
        if (t == ":help" || t == ":?") {
            std::printf("Vyb REPL commands:\n"
                        "  :help       show this help\n"
                        "  :clear      reset the session (declarations + main body)\n"
                        "  :decls      print the accumulated program\n"
                        "  :quit :q :exit  leave the REPL\n"
                        "Declarations (fn/struct/enum/type/...) persist across entries;\n"
                        "a bare expression evaluates and prints its value.\n");
            prompt("vyb> "); continue;
        }
        if (t == ":clear") { decls.clear(); body.clear(); prompt("vyb> "); continue; }
        if (t == ":decls") { std::printf("%s", printSession().c_str()); prompt("vyb> "); continue; }

        // classify + accumulate
        Kind k = classify(entry);
        std::vector<std::string> evalBody = body; // body used for THIS eval
        bool persisted = false;
        if (k == Kind::DECL) {
            decls.push_back(entry);
            persisted = true;
        } else if (k == Kind::ASSIGN || k == Kind::STMT) {
            // persist side-effect-free assignments / user statements: re-run each
            // eval so later entries can see prior variables/state.
            std::istringstream iss(entry); std::string aline;
            while (std::getline(iss, aline)) {
                if (!trim(aline).empty()) body.push_back("    " + aline);
            }
            evalBody = body;
            persisted = true;
        } else {
            // EXPR: auto-display this entry's value; do NOT persist it, so it
            // cannot re-fire on later evals (no repeated output).
            evalBody.push_back("    println(" + trim(entry) + ")");
        }

        // build + evaluate
        std::string prog;
        for (auto& d : decls) { prog += d; prog += "\n"; }
        prog += "\nmain() -> {\n";
        for (auto& b : evalBody) { prog += b; prog += "\n"; }
        prog += "}\n";

        std::string path = makeTempFile(prog);
        if (path.empty()) { std::fputs("repl: could not create temp file\n", stderr); break; }
        std::string out, err;
        int rc = runProgram(selfExe, path, out, err);
        std::remove(path.c_str());

        if (rc == 0) {
            if (!out.empty()) std::fputs(out.c_str(), stdout);
            if (!err.empty()) std::fputs(err.c_str(), stderr);
        } else {
            // revert the entry that broke the program
            (void)persisted;
            if (k == Kind::DECL && !decls.empty()) decls.pop_back();
            else if ((k == Kind::ASSIGN || k == Kind::STMT) && !body.empty()
                     && body == evalBody) body.pop_back();
            std::fputs("Error:\n", stderr);
            std::fputs(err.c_str(), stderr);
        }
        prompt("vyb> ");
    }
    if (tty) std::printf("\n");
    return 0;
}

} // namespace repl
} // namespace vyb
