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
#include <termios.h>
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
// Runs `exe <args...>` capturing stdout+stderr. Returns process exit code.
int runProgram(const std::string& exe, const std::vector<std::string>& args,
               std::string& out, std::string& err) {
    int outPipe[2], errPipe[2];
    if (pipe(outPipe) != 0 || pipe(errPipe) != 0) return -1;
    pid_t pid = fork();
    if (pid == 0) {
        dup2(outPipe[1], STDOUT_FILENO);
        dup2(errPipe[1], STDERR_FILENO);
        close(outPipe[0]); close(outPipe[1]); close(errPipe[0]); close(errPipe[1]);
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(exe.c_str()));
        for (auto& a : args) argv.push_back(const_cast<char*>(a.c_str()));
        argv.push_back(nullptr);
        execv(exe.c_str(), argv.data());
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

// Minimal raw-mode line editor with history recall (interactive TTY only). Falls
// back to std::getline when stdin is not a terminal, so piped/CI sessions are
// unaffected. Supports: enter (submit), backspace, left/right, home/end,
// up/down history recall, Ctrl-D (EOF), Ctrl-U (clear line), Ctrl-W (kill word).
class LineEditor {
    std::vector<std::string> history_;
    size_t histPos_ = 0;   // index into history_ during recall
    std::string draft_;    // in-progress line saved when starting a recall

    void draw(const std::string& prompt, const std::string& line, size_t pos) {
        std::printf("\r\033[K%s%s", prompt.c_str(), line.c_str());
        std::fflush(stdout);
        if (pos < line.size())
            std::printf("\033[%zuD", line.size() - pos);
        std::fflush(stdout);
    }

public:
    bool isTty() const { return isatty(STDIN_FILENO) != 0; }

    // Add a committed line to history (dedupe against the last entry).
    void remember(const std::string& line) {
        if (!line.empty() && (history_.empty() || history_.back() != line))
            history_.push_back(line);
    }

    // Read one edited line from stdin. Returns false on EOF (empty line).
    bool read(const std::string& prompt, std::string& out) {
        if (!isTty()) {
            std::string l;
            if (!std::getline(std::cin, l)) return false;
            out = l;
            return true;
        }
        struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        newt.c_cc[VMIN] = 1;
        newt.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        std::string line; size_t pos = 0;
        histPos_ = history_.size();
        draft_.clear();
        draw(prompt, line, pos);
        bool eof = false;
        for (;;) {
            unsigned char c;
            ssize_t n = ::read(STDIN_FILENO, &c, 1);
            if (n <= 0) { eof = true; break; }
            if (c == '\n' || c == '\r') {
                std::printf("\r\n"); std::fflush(stdout);
                break;
            }
            if (c == 0x04) { // Ctrl-D
                if (line.empty()) { eof = true; break; }
                continue;
            }
            if (c == 0x1b) { // ESC: arrow/home/end = "[A"/"[B"/"[C"/"[D"/"[H"/"[F"
                unsigned char b1 = 0, b2 = 0;
                ::read(STDIN_FILENO, &b1, 1);
                if (b1 == '[') {
                    ::read(STDIN_FILENO, &b2, 1);
                    if (b2 == 'A') { // up: history recall (earlier)
                        if (histPos_ > 0) {
                            if (histPos_ == history_.size()) draft_ = line;
                            --histPos_;
                            line = history_[histPos_];
                            pos = line.size();
                            draw(prompt, line, pos);
                        }
                    } else if (b2 == 'B') { // down: forward through history
                        if (histPos_ < history_.size()) {
                            ++histPos_;
                            line = (histPos_ == history_.size()) ? draft_ : history_[histPos_];
                            pos = line.size();
                            draw(prompt, line, pos);
                        }
                    } else if (b2 == 'C') { // right
                        if (pos < line.size()) { ++pos; draw(prompt, line, pos); }
                    } else if (b2 == 'D') { // left
                        if (pos > 0) { --pos; draw(prompt, line, pos); }
                    } else if (b2 == 'H') { pos = 0; draw(prompt, line, pos); }
                    else if (b2 == 'F') { pos = line.size(); draw(prompt, line, pos); }
                }
                continue;
            }
            if (c == 0x7f || c == '\b') { // backspace
                if (pos > 0) {
                    line.erase(pos - 1, 1); --pos;
                    draw(prompt, line, pos);
                }
                continue;
            }
            if (c == 0x15) { line.clear(); pos = 0; draw(prompt, line, pos); continue; }   // Ctrl-U
            if (c == 0x17) { // Ctrl-W: kill word before cursor
                size_t s = pos;
                while (s > 0 && std::isspace((unsigned char)line[s - 1])) --s;
                while (s > 0 && !std::isspace((unsigned char)line[s - 1])) --s;
                line.erase(s, pos - s);
                pos = s;
                draw(prompt, line, pos);
                continue;
            }
            if (c >= 0x20) { line.insert(line.begin() + pos, (char)c); ++pos; draw(prompt, line, pos); }
        }
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        if (eof) { out.clear(); return false; }
        out = line;
        return true;
    }
};

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

    LineEditor lineEditor;

    prompt("vyb> ");
    std::string line;
    while (lineEditor.read("vyb> ", line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        std::string entry = line;
        int depth = 0;
        // multi-line: keep reading until brackets balance
        while (true) {
            int d = bracketDelta(line);
            depth += d;
            if (depth > 0) { if (!lineEditor.read("... ", line)) break; if (!line.empty() && line.back()=='\r') line.pop_back(); entry += "\n" + line; }
            else break;
        }
        lineEditor.remember(entry);
        std::string t = trim(entry);
        if (t.empty()) { prompt("vyb> "); continue; }

        // REPL commands
        if (t == ":q" || t == ":quit" || t == ":exit" || t == ":bye") break;
        if (t == ":help" || t == ":?") {
            std::printf("Vyb REPL commands:\n"
                        "  :help       show this help\n"
                        "  :clear      reset the session (declarations + main body)\n"
                        "  :decls      print the accumulated program\n"
                        "  :type <expr>  print the inferred type of <expr>\n"
                        "  :quit :q :exit  leave the REPL\n"
                        "Declarations (fn/struct/enum/type/...) persist across entries;\n"
                        "a bare expression evaluates and prints its value.\n");
            prompt("vyb> "); continue;
        }
        if (t == ":clear") { decls.clear(); body.clear(); prompt("vyb> "); continue; }
        if (t == ":decls") { std::printf("%s", printSession().c_str()); prompt("vyb> "); continue; }
        if (t.rfind(":type", 0) == 0) {
            std::string expr = trim(t.substr(5));
            if (expr.empty()) { std::printf("usage: :type <expr>\n"); prompt("vyb> "); continue; }
            // Type <expr> in the accumulated session context: bind it to a probe
            // variable in the (copied) main body and ask `vyb type` for the type.
            std::string prog;
            for (auto& d : decls) { prog += d; prog += "\n"; }
            prog += "\nmain() -> {\n";
            for (auto& b : body) { prog += b; prog += "\n"; }
            prog += "    vtypeprobe = " + expr + "\n";
            prog += "}\n";
            std::string path = makeTempFile(prog);
            std::string out, err;
            int rc = runProgram(selfExe, {"type", path, "vtypeprobe"}, out, err);
            std::remove(path.c_str());
            std::string typeres = out;
            while (!typeres.empty() && (typeres.back() == '\n' || typeres.back() == '\r')) typeres.pop_back();
            std::printf(":type %s = %s\n", expr.c_str(), typeres.empty() ? "(error)" : typeres.c_str());
            if (rc != 0 && !err.empty()) std::fputs(err.c_str(), stderr);
            prompt("vyb> "); continue;
        }

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
        int rc = runProgram(selfExe, {path}, out, err);
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
