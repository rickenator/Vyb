// SPDX-License-Identifier: Apache-2.0

// Vyb doc generator implementation (see docgen.hpp). Extracts `///` doc comments
// from the raw source, walks the parsed AST for declarations, and emits a
// self-contained HTML reference page. Signature rendering is header-only (no
// function bodies); nesting (struct fields, enum variants, aspect methods) is
// shown in the signature block itself, not as separate sections.

#include "vyb/docgen.hpp"
#include <cctype>
#include <sstream>

namespace vyb {
namespace docgen {

namespace {

struct DocBlock { int start; int end; std::string text; };

std::string html_escape(const std::string& s) {
    std::string o;
    o.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '&': o += "&amp;"; break;
            case '<': o += "&lt;"; break;
            case '>': o += "&gt;"; break;
            case '"': o += "&quot;"; break;
            case '\'': o += "&#39;"; break;
            default: o += c;
        }
    }
    return o;
}

bool isDocLine(const std::string& t) { return t.size() >= 3 && t[0] == '/' && t[1] == '/' && t[2] == '/'; }

// Trim a doc line's leading "///" + one space.
std::string docLineText(const std::string& line) {
    size_t i = 0;
    while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) ++i;
    i += 3; // "///"
    while (i < line.size() && line[i] == ' ') ++i;        // optional single space
    std::string t = line.substr(i);
    if (!t.empty() && t.back() == '\r') t.pop_back();
    return t;
}

// Extract contiguous `///` doc-comment blocks from the source.
std::vector<DocBlock> extractDocBlocks(const std::string& source) {
    std::vector<DocBlock> out;
    std::vector<std::string> lines;
    {
        std::istringstream ss(source);
        std::string l;
        while (std::getline(ss, l)) lines.push_back(l);
    }
    int i = 0, n = (int)lines.size();
    while (i < n) {
        std::string raw = lines[i];
        size_t b = 0;
        while (b < raw.size() && (raw[b] == ' ' || raw[b] == '\t' || raw[b] == '\r')) ++b;
        if (!isDocLine(raw.substr(b))) { ++i; continue; }
        DocBlock blk;
        blk.start = i + 1;
        std::string joined;
        while (i < n) {
            std::string r = lines[i];
            size_t b2 = 0;
            while (b2 < r.size() && (r[b2] == ' ' || r[b2] == '\t' || r[b2] == '\r')) ++b2;
            if (!isDocLine(r.substr(b2))) break;
            if (!joined.empty()) joined += " ";
            joined += docLineText(r);
            blk.end = i + 1;
            ++i;
        }
        blk.text = joined;
        out.push_back(blk);
    }
    return out;
}

// Find the doc block a declaration at (1-based) line \p L belongs to or directly
// follows: the parser places some declarations' loc on the doc-comment start line
// (functions/enums) and others on the code line (structs), so accept any block with
// start <= L <= end+1 (nearest such block below).
std::string docForLine(const std::vector<DocBlock>& blocks, int L) {
    const DocBlock* best = nullptr;
    for (const auto& blk : blocks) {
        if (L >= blk.start && L <= blk.end + 1) best = &blk;
    }
    return best ? best->text : "";
}

// Header-only signature rendering for a declaration.
std::string declSignature(ast::Declaration* d) {
    std::ostringstream o;
    switch (d->getType()) {
        case ast::NodeType::FUNCTION_DECLARATION: {
            auto* n = static_cast<ast::FunctionDeclaration*>(d);
            if (n->isAsync) o << "async ";
            if (n->id) o << n->id->name;
            if (!n->genericParams.empty()) {
                o << "<";
                for (size_t i = 0; i < n->genericParams.size(); ++i) {
                    if (i) o << ", ";
                    if (n->genericParams[i] && n->genericParams[i]->name) o << n->genericParams[i]->name->name;
                }
                o << ">";
            }
            o << "(";
            for (size_t i = 0; i < n->params.size(); ++i) {
                if (i) o << ", ";
                const auto& p = n->params[i];
                if (p.name) o << p.name->name;
                if (p.typeNode) { o << "<"; o << p.typeNode->toString(); o << ">"; }
            }
            if (n->variadic) o << ", ...";
            o << ")";
            if (n->returnTypeNode) { o << "<"; o << n->returnTypeNode->toString(); o << ">"; }
            return o.str();
        }
        case ast::NodeType::STRUCT_DECLARATION: {
            auto* n = static_cast<ast::StructDeclaration*>(d);
            o << (n->reprC ? "struct reprC " : "struct ");
            if (n->name) o << n->name->name;
            if (!n->genericParams.empty()) {
                o << "<";
                for (size_t i = 0; i < n->genericParams.size(); ++i) {
                    if (i) o << ", ";
                    if (n->genericParams[i] && n->genericParams[i]->name) o << n->genericParams[i]->name->name;
                }
                o << ">";
            }
            o << " { ";
            for (size_t i = 0; i < n->fields.size(); ++i) {
                if (i) o << ", ";
                const auto& f = n->fields[i];
                if (f->name) o << f->name->name;
                if (f->typeNode) { o << "<"; o << f->typeNode->toString(); o << ">"; }
            }
            o << " }";
            return o.str();
        }
        case ast::NodeType::ENUM_DECLARATION: {
            auto* n = static_cast<ast::EnumDeclaration*>(d);
            o << "enum ";
            if (n->name) o << n->name->name;
            o << " { ";
            for (size_t i = 0; i < n->variants.size(); ++i) {
                if (i) o << ", ";
                auto* v = n->variants[i].get();
                if (v && v->name) o << v->name->name;
                if (v && !v->associatedTypes.empty()) {
                    o << "(";
                    for (size_t j = 0; j < v->associatedTypes.size(); ++j) {
                        if (j) o << ", ";
                        if (v->associatedTypes[j]) o << v->associatedTypes[j]->toString();
                    }
                    o << ")";
                } else if (v && v->hasValue) { o << " = " << v->value; }
            }
            o << " }";
            return o.str();
        }
        case ast::NodeType::TYPE_ALIAS_DECLARATION: {
            auto* n = static_cast<ast::TypeAliasDeclaration*>(d);
            o << "type ";
            if (n->name) o << n->name->name;
            o << " = ";
            if (n->typeNode) o << n->typeNode->toString();
            return o.str();
        }
        case ast::NodeType::IMPORT_DECLARATION: {
            auto* n = static_cast<ast::ImportDeclaration*>(d);
            o << "import ";
            if (n->source) o << n->source->value;
            if (n->locator) { o << " from \""; o << n->locator->value; o << "\""; }
            return o.str();
        }
        case ast::NodeType::ASPECT_DECLARATION: {
            auto* n = static_cast<ast::AspectDeclaration*>(d);
            o << "aspect ";
            if (n->name) o << n->name->name;
            if (!n->superTypes.empty()) {
                o << " : ";
                for (size_t i = 0; i < n->superTypes.size(); ++i) {
                    if (i) o << " + ";
                    if (n->superTypes[i]) o << n->superTypes[i]->name;
                }
            }
            return o.str();
        }
        case ast::NodeType::BIND_DECLARATION: {
            auto* n = static_cast<ast::BindDeclaration*>(d);
            o << "bind ";
            if (n->selfType) o << n->selfType->toString();
            return o.str();
        }
        case ast::NodeType::CLASS_DECLARATION: {
            auto* n = static_cast<ast::ClassDeclaration*>(d);
            o << "class ";
            if (n->name) o << n->name->name;
            o << " { ... }";
            return o.str();
        }
        default:
            return d->toString();
    }
}

std::string anchor(std::string name) {
    for (auto& c : name) if (!(std::isalnum((unsigned char)c) || c == '_')) c = '-';
    return name;
}

// Identifier / display name for a declaration (used for the heading + anchor).
std::string declName(ast::Declaration* d) {
    switch (d->getType()) {
        case ast::NodeType::FUNCTION_DECLARATION:
            if (auto* n = static_cast<ast::FunctionDeclaration*>(d); n->id) return n->id->name;
            break;
        case ast::NodeType::STRUCT_DECLARATION:
            if (auto* n = static_cast<ast::StructDeclaration*>(d); n->name) return n->name->name;
            break;
        case ast::NodeType::ENUM_DECLARATION:
            if (auto* n = static_cast<ast::EnumDeclaration*>(d); n->name) return n->name->name;
            break;
        case ast::NodeType::TYPE_ALIAS_DECLARATION:
            if (auto* n = static_cast<ast::TypeAliasDeclaration*>(d); n->name) return n->name->name;
            break;
        case ast::NodeType::ASPECT_DECLARATION:
            if (auto* n = static_cast<ast::AspectDeclaration*>(d); n->name) return n->name->name;
            break;
        case ast::NodeType::BIND_DECLARATION:
            if (auto* n = static_cast<ast::BindDeclaration*>(d))
                return n->selfType ? "bind " + n->selfType->toString() : "bind";
            break;
        case ast::NodeType::CLASS_DECLARATION:
            if (auto* n = static_cast<ast::ClassDeclaration*>(d); n->name) return n->name->name;
            break;
        default: break;
    }
    return "decl";
}

} // namespace

void renderDeclSections(ast::Module* module, const std::vector<DocBlock>& blocks,
                        std::vector<std::string>& frags,
                        std::vector<std::pair<std::string,std::string>>& toc);

int renderModule(ast::Module* module, const std::string& source, const std::string& title,
                 std::string& htmlBody) {
    (void)title;
    auto blocks = extractDocBlocks(source);
    std::vector<std::string> frags;
    std::vector<std::pair<std::string, std::string>> toc;
    renderDeclSections(module, blocks, frags, toc);
    std::ostringstream nav;
    nav << "<ul class=\"toc\">";
    for (auto& t : toc) nav << "<li><a href=\"#" << anchor(t.first) << "\">" << html_escape(t.first) << "</a></li>";
    nav << "</ul>";
    htmlBody = nav.str();
    for (auto& f : frags) htmlBody += f;
    return 0;
}

// Walk top-level declarations; emit a section per declaration.
void renderDeclSections(ast::Module* module, const std::vector<DocBlock>& blocks,
                        std::vector<std::string>& frags,
                        std::vector<std::pair<std::string,std::string>>& toc) {
    for (auto& s : module->body) {
        auto* d = dynamic_cast<ast::Declaration*>(s.get());
        if (!d) continue;
        if (d->getType() == ast::NodeType::IMPORT_DECLARATION) continue; // not API
        std::string sig = declSignature(d);
        std::string name = declName(d);
        std::string doc = docForLine(blocks, (int)d->loc.line);
        std::ostringstream frag;
        frag << "<section id=\"" << anchor(name) << "\">\n"
             << "  <h3>" << html_escape(name) << "</h3>\n";
        if (!doc.empty()) frag << "  <p class=\"doc\">" << html_escape(doc) << "</p>\n";
        frag << "  <pre>" << html_escape(sig) << "</pre>\n"
             << "</section>\n";
        frags.push_back(frag.str());
        toc.push_back({name, name});
    }
}

std::string buildPage(const std::string& title, const std::vector<std::string>& bodyFragments,
                      const std::vector<std::pair<std::string,std::string>>& toc) {
    (void)toc;
    std::string css =
        "body{font-family:system-ui,-apple-system,sans-serif;margin:2rem auto;max-width:52rem;"
        "padding:0 1rem;line-height:1.5;color:#1a1a1a;background:#fff;}"
        "h1{border-bottom:2px solid #eee;padding-bottom:.3rem;}"
        "h3{margin:.6rem 0 .2rem;color:#0b5394;}"
        ".doc{background:#f6f8fa;padding:.4rem .7rem;border-left:3px solid #0969da;}"
        "pre{background:#0d1117;color:#e6edf3;padding:.8rem;border-radius:6px;overflow-x:auto;"
        "font-size:.85em;}section{margin:1.2rem 0;}"
        "ul.toc{column-width:14rem;list-style:none;padding:0;}ul.toc a{color:#0969da;text-decoration:none;}";
    std::ostringstream o;
    o << "<!DOCTYPE html>\n<html lang=\"en\"><head><meta charset=\"utf-8\">\n"
      << "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">\n"
      << "<title>" << html_escape(title) << "</title>\n"
      << "<style>" << css << "</style>\n</head>\n<body>\n"
      << "<h1>" << html_escape(title) << "</h1>\n";
    for (auto& f : bodyFragments) o << f << "\n";
    o << "</body>\n</html>\n";
    return o.str();
}

} // namespace docgen
} // namespace vyb
