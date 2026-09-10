// SPDX-License-Identifier: Apache-2.0

// Vyb LSP server implementation (see lsp.hpp). A minimal self-contained JSON
// value + parser/serializer and LSP framing (Content-Length) support the
// JSON-RPC 2.0 message loop. On each open/change the document is parsed (via the
// caller's ParseFn) and linted, diagnostics are published, and a symbol table of
// the document's declarations drives go-to-definition / hover / completion.

#include "vyb/lsp.hpp"
#include "vyb/lint.hpp"
#include <cctype>
#include <cstdio>
#include <functional>
#include <map>
#include <memory>
#include <sstream>

namespace vyb {
namespace lsp {

// ---------------------------------------------------------------------------
// Minimal JSON value
// ---------------------------------------------------------------------------
struct Json {
    enum Type { Null, Bool, Num, Str, Arr, Obj };
    Type type = Null;
    bool b = false;
    double num = 0;
    std::string s;
    std::vector<Json> arr;
    std::vector<std::pair<std::string, Json>> obj;

    static Json makeNull() { return Json(); }
    static Json makeBool(bool v) { Json j; j.type = Bool; j.b = v; return j; }
    static Json makeNum(double v) { Json j; j.type = Num; j.num = v; return j; }
    static Json makeStr(std::string v) { Json j; j.type = Str; j.s = std::move(v); return j; }
    static Json makeArr() { Json j; j.type = Arr; return j; }
    static Json makeObj() { Json j; j.type = Obj; return j; }

    bool has(const std::string& k) const {
        for (auto& kv : obj) if (kv.first == k) return true;
        return false;
    }
    const Json* get(const std::string& k) const {
        for (auto& kv : obj) if (kv.first == k) return &kv.second;
        return nullptr;
    }
    Json* getMut(const std::string& k) {
        for (auto& kv : obj) if (kv.first == k) return &kv.second;
        return nullptr;
    }
    void set(const std::string& k, Json v) {
        type = Obj; // setting a member makes this an object (Null -> Obj upgrade)
        if (auto* e = getMut(k)) { *e = std::move(v); return; }
        obj.push_back({k, std::move(v)});
    }
};

// Recursive-descent JSON parser over a string. On error returns a Null Json.
struct JsonParser {
    const std::string& t;
    size_t p = 0;
    JsonParser(const std::string& src) : t(src) {}
    void ws() { while (p < t.size() && (t[p] == ' ' || t[p] == '\t' || t[p] == '\n' || t[p] == '\r')) ++p; }
    bool ok() const { return p < t.size(); }
    Json parse() { ws(); Json v = value(); (void)v; return v; }
    Json value() {
        ws();
        if (!ok()) return Json::makeNull();
        char c = t[p];
        if (c == '{') return object();
        if (c == '[') return array();
        if (c == '"') return string();
        if (c == 't' || c == 'f') return boolean();
        if (c == 'n') { p += 4; return Json::makeNull(); }
        return number();
    }
    Json object() {
        Json o = Json::makeObj();
        ++p; ws();
        if (ok() && t[p] == '}') { ++p; return o; }
        while (ok()) {
            ws();
            std::string key = string().s;
            ws();
            if (ok() && t[p] == ':') ++p;
            Json v = value();
            o.set(key, v);
            ws();
            if (ok() && t[p] == ',') { ++p; continue; }
            if (ok() && t[p] == '}') { ++p; break; }
            break;
        }
        return o;
    }
    Json array() {
        Json a = Json::makeArr();
        ++p; ws();
        if (ok() && t[p] == ']') { ++p; return a; }
        while (ok()) {
            a.arr.push_back(value());
            ws();
            if (ok() && t[p] == ',') { ++p; continue; }
            if (ok() && t[p] == ']') { ++p; break; }
            break;
        }
        return a;
    }
    Json string() {
        Json v = Json::makeStr("");
        if (ok() && t[p] == '"') ++p;
        while (ok() && t[p] != '"') {
            if (t[p] == '\\' && p + 1 < t.size()) {
                ++p;
                char e = t[p];
                if (e == 'n') v.s += '\n';
                else if (e == 't') v.s += '\t';
                else if (e == 'r') v.s += '\r';
                else if (e == 'u') { // minimal \uXXXX
                    v.s += '?';
                    p += 4;
                }
                else v.s += e;
                ++p;
            } else v.s += t[p++];
        }
        if (ok() && t[p] == '"') ++p;
        return v;
    }
    Json boolean() {
        if (t.substr(p, 4) == "true") { p += 4; return Json::makeBool(true); }
        p += 5; return Json::makeBool(false);
    }
    Json number() {
        size_t start = p;
        if (ok() && (t[p] == '-' || t[p] == '+')) ++p;
        while (ok() && (std::isdigit((unsigned char)t[p]) || t[p] == '.' || t[p] == 'e' || t[p] == 'E' || t[p] == '-' || t[p] == '+')) ++p;
        std::string num = t.substr(start, p - start);
        return Json::makeNum(num.empty() ? 0 : std::atof(num.c_str()));
    }
};

static Json parseJson(const std::string& s) { return JsonParser(s).parse(); }

static void jsonEscape(std::ostringstream& o, const std::string& v) {
    o << '"';
    for (char c : v) {
        switch (c) {
            case '"': o << "\\\""; break;
            case '\\': o << "\\\\"; break;
            case '\n': o << "\\n"; break;
            case '\r': o << "\\r"; break;
            case '\t': o << "\\t"; break;
            default:
                if ((unsigned char)c < 0x20) { char buf[8]; snprintf(buf, sizeof buf, "\\u%04x", c); o << buf; }
                else o << c;
        }
    }
    o << '"';
}

static void writeJson(std::ostringstream& o, const Json& v) {
    switch (v.type) {
        case Json::Null: o << "null"; break;
        case Json::Bool: o << (v.b ? "true" : "false"); break;
        case Json::Num: { char buf[32]; snprintf(buf, sizeof buf, "%g", v.num); o << buf; break; }
        case Json::Str: jsonEscape(o, v.s); break;
        case Json::Arr:
            o << '[';
            for (size_t i = 0; i < v.arr.size(); ++i) { if (i) o << ','; writeJson(o, v.arr[i]); }
            o << ']';
            break;
        case Json::Obj:
            o << '{';
            for (size_t i = 0; i < v.obj.size(); ++i) {
                if (i) o << ',';
                jsonEscape(o, v.obj[i].first);
                o << ':';
                writeJson(o, v.obj[i].second);
            }
            o << '}';
            break;
    }
}

// ---------------------------------------------------------------------------
// LSP framing + server state
// ---------------------------------------------------------------------------
struct Symbol {
    std::string name;
    std::string kind;     // "function" | "struct" | "enum" | "alias" | "aspect" | ...
    unsigned line = 0;    // 1-based
    unsigned col = 0;     // 1-based
    std::string signature;
    std::string doc;
};

class Server {
public:
    ParseFn parse;
    std::string uri;
    std::string text;
    std::vector<Symbol> symbols;
    bool shuttingDown = false;

    explicit Server(ParseFn p) : parse(std::move(p)) {}

    void writeResponse(const Json* id, const Json& result) {
        Json msg = Json::makeObj();
        msg.set("jsonrpc", Json::makeStr("2.0"));
        if (id) msg.set("id", *id); else msg.set("id", Json::makeNull());
        msg.set("result", result);
        send(msg);
    }
    void writeError(const Json* id, int code, const std::string& message) {
        Json err = Json::makeObj(); err.set("code", Json::makeNum(code)); err.set("message", Json::makeStr(message));
        Json msg = Json::makeObj();
        msg.set("jsonrpc", Json::makeStr("2.0"));
        if (id) msg.set("id", *id); else msg.set("id", Json::makeNull());
        msg.set("error", err);
        send(msg);
    }
    void notify(const std::string& method, Json params) {
        Json msg = Json::makeObj();
        msg.set("jsonrpc", Json::makeStr("2.0"));
        msg.set("method", Json::makeStr(method));
        msg.set("params", params);
        send(msg);
    }
    void send(const Json& msg) {
        std::ostringstream o;
        writeJson(o, msg);
        std::string body = o.str();
        std::printf("Content-Length: %zu\r\n\r\n", body.size());
        fwrite(body.data(), 1, body.size(), stdout);
        fflush(stdout);
    }

    // ---- document model ----
    void reindex() {
        symbols.clear();
        std::vector<std::string> errors;
        auto module = parse(text, uri, errors);
        if (module) {
            for (auto& s : module->body) {
                auto* d = dynamic_cast<ast::Declaration*>(s.get());
                if (!d) continue;
                Symbol sym;
                sym.line = d->loc.line; sym.col = d->loc.column;
                if (!buildSymbol(d, sym)) continue;
                symbols.push_back(sym);
            }
        }
    }

    bool buildSymbol(ast::Declaration* d, Symbol& sym) {
        switch (d->getType()) {
            case ast::NodeType::FUNCTION_DECLARATION: {
                auto* n = static_cast<ast::FunctionDeclaration*>(d);
                if (!n->id) return false;
                sym.name = n->id->name; sym.kind = "function";
                std::ostringstream s; s << sym.name << "(";
                for (size_t i = 0; i < n->params.size(); ++i) {
                    if (i) s << ", ";
                    if (n->params[i].name) s << n->params[i].name->name;
                    if (n->params[i].typeNode) s << "<" << n->params[i].typeNode->toString() << ">";
                }
                s << ")";
                if (n->returnTypeNode) s << "<" << n->returnTypeNode->toString() << ">";
                sym.signature = s.str();
                return true;
            }
            case ast::NodeType::STRUCT_DECLARATION: {
                auto* n = static_cast<ast::StructDeclaration*>(d);
                if (!n->name) return false;
                sym.name = n->name->name; sym.kind = "struct";
                sym.signature = "struct " + sym.name;
                return true;
            }
            case ast::NodeType::ENUM_DECLARATION: {
                auto* n = static_cast<ast::EnumDeclaration*>(d);
                if (!n->name) return false;
                sym.name = n->name->name; sym.kind = "enum";
                sym.signature = "enum " + sym.name;
                return true;
            }
            case ast::NodeType::TYPE_ALIAS_DECLARATION: {
                auto* n = static_cast<ast::TypeAliasDeclaration*>(d);
                if (!n->name) return false;
                sym.name = n->name->name; sym.kind = "alias";
                sym.signature = "type " + sym.name;
                return true;
            }
            case ast::NodeType::ASPECT_DECLARATION: {
                auto* n = static_cast<ast::AspectDeclaration*>(d);
                if (!n->name) return false;
                sym.name = n->name->name; sym.kind = "aspect";
                sym.signature = "aspect " + sym.name;
                return true;
            }
            case ast::NodeType::IMPORT_DECLARATION: {
                auto* n = static_cast<ast::ImportDeclaration*>(d);
                if (!n->source) return false;
                sym.name = n->source->value; sym.kind = "import";
                sym.signature = "import " + sym.name;
                return true;
            }
            case ast::NodeType::BIND_DECLARATION: {
                auto* n = static_cast<ast::BindDeclaration*>(d);
                sym.name = n->selfType ? "bind " + n->selfType->toString() : "bind";
                sym.kind = "bind"; sym.signature = sym.name;
                return true;
            }
            default: return false;
        }
    }

    // Extract the identifier (allowing dots) at a 0-based position (line, col).
    std::string wordAt(unsigned line0, unsigned col0) {
        std::vector<std::string> lines; std::string cur;
        for (size_t i = 0; i <= text.size(); ++i) {
            if (i == text.size() || text[i] == '\n') { lines.push_back(cur); cur.clear(); }
            else cur += text[i];
        }
        if (line0 >= lines.size()) return "";
        const std::string& ln = lines[line0];
        auto isw = [&](size_t i) { char c = ln[i]; return std::isalnum((unsigned char)c) || c == '_' || c == '.'; };
        if (col0 > ln.size()) col0 = (unsigned)ln.size();
        size_t b = col0, e = col0;
        while (b > 0 && isw(b - 1)) --b;
        while (e < ln.size() && isw(e)) ++e;
        // Cursor centered on a non-word char (e.g. after '('): take the word to the left.
        if (b == e && col0 > 0 && b > 0) { /* nothing more to do; empty */ }
        return ln.substr(b, e - b);
    }

    int findSymbolIndex(const std::string& name) const {
        for (size_t i = 0; i < symbols.size(); ++i) if (symbols[i].name == name) return (int)i;
        return -1;
    }

    // ---- handlers ----
    Json capabilities() {
        Json sync = Json::makeObj();
        sync.set("openClose", Json::makeBool(true));
        sync.set("change", Json::makeNum(1));
        Json cap = Json::makeObj();
        cap.set("textDocumentSync", sync);
        cap.set("hoverProvider", Json::makeBool(true));
        cap.set("definitionProvider", Json::makeBool(true));
        Json comp = Json::makeObj();
        Json trig = Json::makeArr(); trig.arr.push_back(Json::makeStr("."));
        comp.set("triggerCharacters", trig);
        cap.set("completionProvider", comp);
        return cap;
    }

    void publishDiagnostics() {
        std::vector<std::string> errors;
        auto module = parse(text, uri, errors);
        Json diags = Json::makeArr();
        auto push = [&](unsigned line0, unsigned char0, unsigned endLine, std::string msg, int sev) {
            Json range = Json::makeObj();
            Json st = Json::makeObj(); st.set("line", Json::makeNum(line0)); st.set("character", Json::makeNum(char0));
            Json en = Json::makeObj(); en.set("line", Json::makeNum(endLine)); en.set("character", Json::makeNum(char0 + 1));
            range.set("start", st); range.set("end", en);
            Json d = Json::makeObj();
            d.set("range", range); d.set("severity", Json::makeNum(sev)); d.set("message", Json::makeStr(msg));
            diags.arr.push_back(d);
        };
        for (auto& e : errors) push(0, 0, 1, "parse error: " + e, 1);
        if (module) {
            std::vector<std::string> warns;
            std::string lintFile = uri; // strip scheme so "file:line:col:" parses cleanly
            size_t sch = lintFile.find("://");
            if (sch != std::string::npos) lintFile = lintFile.substr(sch + 3);
            vyb::lint::lintModule(module.get(), lintFile, warns);
            for (auto& w : warns) {
                // "file:line:col: warning: msg"
                unsigned line = 0, col = 0; std::string msg = w;
                if (sscanf(w.c_str(), "%*[^:]:%u:%u: warning: ", &line, &col) == 2) {
                    msg = w.substr(w.find("warning:") + 8);
                    // strip a leading space
                    if (!msg.empty() && msg[0] == ' ') msg = msg.substr(1);
                    push(line > 0 ? line - 1 : 0, col > 0 ? col - 1 : 0, line > 0 ? line : 1, msg, 2);
                }
            }
        }
        Json params = Json::makeObj();
        params.set("uri", Json::makeStr(uri));
        params.set("diagnostics", diags);
        notify("textDocument/publishDiagnostics", params);
    }

    Json hoverAt(const Json& params) {
        Json result = Json::makeNull();
        const Json* pos = params.get("position");
        const Json* td = params.get("textDocument");
        if (!pos || !td) return result;
        unsigned line = (unsigned)pos->get("line")->num;
        unsigned ch = (unsigned)pos->get("character")->num;
        std::string w = wordAt(line, ch);
        if (getenv("VYB_LSP_DBG")) fprintf(stderr, "[lsp] hover wordAt(l=%u,c=%u)='%s'\n", line, ch, w.c_str());
        if (w.empty()) return result;
        int idx = findSymbolIndex(w);
        if (getenv("VYB_LSP_DBG")) fprintf(stderr, "[lsp] hover idx=%d syms=%zu\n", idx, symbols.size());
        if (idx < 0) {
            // heuristic: last word of dotted name
            size_t dot = w.rfind('.');
            if (dot != std::string::npos) idx = findSymbolIndex(w.substr(dot + 1));
        }
        if (idx < 0) return result;
        const Symbol& s = symbols[idx];
        std::string contents = "**" + s.kind + "** " + s.signature + "\n\n" + s.doc;
        Json val = Json::makeObj();
        Json k = Json::makeObj(); k.set("kind", Json::makeStr("markdown")); k.set("value", Json::makeStr(contents));
        val.set("contents", k);
        return val;
    }

    Json definitionAt(const Json& params) {
        Json result = Json::makeNull();
        const Json* pos = params.get("position");
        if (!pos) return result;
        unsigned line = (unsigned)pos->get("line")->num;
        unsigned ch = (unsigned)pos->get("character")->num;
        std::string w = wordAt(line, ch);
        if (getenv("VYB_LSP_DBG")) fprintf(stderr, "[lsp] def wordAt(l=%u,c=%u)='%s'\n", line, ch, w.c_str());
        if (w.empty()) return result;
        int idx = findSymbolIndex(w);
        if (getenv("VYB_LSP_DBG")) fprintf(stderr, "[lsp] def idx=%d syms=%zu\n", idx, symbols.size());
        if (idx < 0) { size_t dot = w.rfind('.'); if (dot != std::string::npos) idx = findSymbolIndex(w.substr(dot + 1)); }
        if (idx < 0) return result;
        const Symbol& s = symbols[idx];
        Json range = Json::makeObj();
        Json st = Json::makeObj(); st.set("line", Json::makeNum(s.line - 1)); st.set("character", Json::makeNum(s.col - 1));
        Json en = Json::makeObj(); en.set("line", Json::makeNum(s.line - 1)); en.set("character", Json::makeNum(s.col - 1 + s.name.size()));
        range.set("start", st); range.set("end", en);
        result.set("range", range);
        result.set("uri", Json::makeStr(uri));
        return result;
    }

    Json completions(const Json& params) {
        Json items = Json::makeArr();
        const Json* td = params.get("textDocument");
        const Json* pos = params.get("position");
        std::string prefix;
        if (pos) {
            std::string w = wordAt((unsigned)pos->get("line")->num + 0, (unsigned)pos->get("character")->num);
            // wordAt may have grabbed a trailing '.'; restrict to identifier prefix before cursor
            prefix = w;
        }
        for (auto& s : symbols) {
            if (prefix.empty() || s.name.rfind(prefix, 0) == 0) {
                Json it = Json::makeObj();
                it.set("label", Json::makeStr(s.name));
                it.set("kind", Json::makeNum(s.kind == "function" ? 3 : (s.kind == "struct" || s.kind == "class" ? 7 : 6)));
                it.set("detail", Json::makeStr(s.kind));
                items.arr.push_back(it);
            }
        }
        return items;
    }

    bool handleMessageDispatch(const std::string& body) {
        Json req = parseJson(body);
        const Json* method = req.get("method");
        const Json* msgId = req.get("id");
        if (!method) return false; // response, ignore
        std::string m = method->s;
        if (m == "initialize") {
            Json r = Json::makeObj();
            r.set("capabilities", capabilities());
            r.set("serverInfo", Json::makeObj());
            r.getMut("serverInfo")->set("name", Json::makeStr("vyb-lsp"));
            r.getMut("serverInfo")->set("version", Json::makeStr("0.1"));
            writeResponse(msgId, r);
        }
        else if (m == "initialized") { /* noop */ }
        else if (m == "shutdown") { shuttingDown = true; writeResponse(msgId, Json::makeNull()); }
        else if (m == "exit") { return true; }
        else if (m == "textDocument/didOpen" || m == "textDocument/didChange") {
            const Json* params = req.get("params");
            const Json* td = params ? params->get("textDocument") : nullptr;
            if (td && td->get("uri")) {
                uri = td->get("uri")->s;
                if (m == "textDocument/didOpen") {
                    const Json* txt = td->get("text");
                    if (txt) text = txt->s;
                } else {
                    const Json* changes = params->get("contentChanges");
                    if (changes && !changes->arr.empty())
                        { const Json& last = changes->arr.back(); if (last.get("text")) text = last.get("text")->s; }
                }
                reindex();
                publishDiagnostics();
            }
        }
        else if (m == "textDocument/didClose") {
            uri.clear(); text.clear(); symbols.clear();
        }
        else if (m == "textDocument/hover") {
            const Json* params = req.get("params");
            writeResponse(msgId, params ? hoverAt(*params) : Json::makeNull());
        }
        else if (m == "textDocument/definition") {
            const Json* params = req.get("params");
            writeResponse(msgId, params ? definitionAt(*params) : Json::makeNull());
        }
        else if (m == "textDocument/completion") {
            const Json* params = req.get("params");
            writeResponse(msgId, params ? completions(*params) : Json::makeNull());
        }
        else if (m == "$/cancelRequest" || m == "textDocument/didSave") {
            /* noop */
        }
        else {
            writeResponse(msgId, Json::makeNull());
        }
        return false;
    }
};

int serveLsp(ParseFn parse) {
    Server server(std::move(parse));
    while (true) {
        std::string line;
        size_t contentLen = 0;
        bool gotHeader = false;
        while (std::getline(std::cin, line)) {
            gotHeader = true;
            if (line == "\r") break;
            // Trim trailing \r
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.rfind("Content-Length:", 0) == 0)
                contentLen = (size_t)std::strtoul(line.c_str() + 15, nullptr, 10);
        }
        if (!gotHeader) break;             // EOF
        if (contentLen == 0) break;
        std::string body(contentLen, '\0');
        std::cin.read(&body[0], (std::streamsize)contentLen);
        if (!std::cin) break;
        if (server.handleMessageDispatch(body)) break; // 'exit' request
    }
    return 0;
}

} // namespace lsp
} // namespace vyb
