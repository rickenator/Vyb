// SPDX-License-Identifier: Apache-2.0

#include "vyb/parser/ast.hpp"
#include "vyb/parser/token.hpp"
#include "vyb/parser/parser.hpp"
#include <vector>
#include <memory>
#include <stdexcept>

namespace vyb {

ModuleParser::ModuleParser(const std::vector<vyb::token::Token>& tokens, size_t& pos, const std::string& file_path, DeclarationParser& declaration_parser)
    : BaseParser(tokens, pos, file_path), declaration_parser_(declaration_parser) {}

bool ModuleParser::isTopLevelStarter(vyb::TokenType tt) {
    switch (tt) {
        case vyb::TokenType::KEYWORD_STRUCT:
        case vyb::TokenType::KEYWORD_ENUM:
        case vyb::TokenType::KEYWORD_TYPE:
        case vyb::TokenType::KEYWORD_IMPORT:
        case vyb::TokenType::KEYWORD_SMUGGLE:
        case vyb::TokenType::KEYWORD_ASPECT:
        case vyb::TokenType::KEYWORD_BIND:
        case vyb::TokenType::KEYWORD_CLASS:
        case vyb::TokenType::KEYWORD_MODULE:
        case vyb::TokenType::KEYWORD_USE:
        case vyb::TokenType::KEYWORD_FN:
        case vyb::TokenType::KEYWORD_EXTERN:
            return true;
        default:
            return false;
    }
}

// Skip tokens until the next top-level declaration starts (a known declaration
// keyword at brace depth 0) or EOF. Anything in an unclosed block in the failed
// region is skipped too, so recovery never re-enters a half-parsed body.
void ModuleParser::synchronizeToNextDeclaration() {
    // Error recovery must ALWAYS make forward progress or the top-level parse
    // loop can spin forever (issue #245). On entry, consume the token that
    // failed to parse *before* scanning, so a token we cannot consume can never
    // wedge the loop. This matters for top-level starter keywords (e.g. \`use\`,
    // KEYWORD_USE) that appear in isTopLevelStarter() but have no declaration or
    // statement handler anywhere in the grammar: otherwise the depth-0
    // isTopLevelStarter() break below fires without consuming, and
    // ModuleParser::parse() retries the same unparseable token indefinitely,
    // hanging the compiler. This path is only reachable after a failed parse,
    // so valid files are unaffected.
    if (this->peek().type != vyb::TokenType::END_OF_FILE) {
        this->consume();
    }

    int depth = 0;
    while (this->peek().type != vyb::TokenType::END_OF_FILE) {
        vyb::TokenType tt = this->peek().type;
        if (tt == vyb::TokenType::LBRACE) {
            ++depth;
        } else if (tt == vyb::TokenType::RBRACE) {
            if (depth > 0) --depth;
        } else if (depth == 0 && isTopLevelStarter(tt)) {
            break;
        }
        this->consume();
    }
}

// Pre-register every module global's name before any function body is parsed
// (#5). A bare `name = expr` inside a function is an inferred local declaration
// when `name` is not yet a known variable, so without this pre-scan a function
// that assigns a global declared further down the file silently got a fresh
// local instead (and its writes were lost). The scan only looks at statements
// that start at nesting depth 0: `name<Type> = ...` / `name<Type>`, `name = ...`,
// and `let|mut|var|const|auto name ...`. Generic functions (`name<T>(...)`) are
// excluded because a '(' follows their angle brackets.
void ModuleParser::prescanModuleGlobals() {
    using TT = vyb::TokenType;
    const auto& t = this->tokens_;
    const size_t n = t.size();
    int depth = 0;
    bool atStart = true;
    for (size_t i = this->pos_; i < n && t[i].type != TT::END_OF_FILE; ++i) {
        TT tt = t[i].type;
        if (tt == TT::COMMENT) continue;
        if (tt == TT::LBRACE || tt == TT::LPAREN || tt == TT::LBRACKET || tt == TT::INDENT) {
            ++depth; atStart = false; continue;
        }
        if (tt == TT::RBRACE || tt == TT::RPAREN || tt == TT::RBRACKET || tt == TT::DEDENT) {
            if (depth > 0) --depth;
            atStart = (depth == 0 && (tt == TT::RBRACE || tt == TT::DEDENT));
            continue;
        }
        if (tt == TT::NEWLINE || tt == TT::SEMICOLON) { if (depth == 0) atStart = true; continue; }
        if (depth != 0 || !atStart) { atStart = false; continue; }
        atStart = false;
        if ((tt == TT::KEYWORD_LET || tt == TT::KEYWORD_MUT || tt == TT::KEYWORD_VAR ||
             tt == TT::KEYWORD_CONST || tt == TT::KEYWORD_AUTO) &&
            i + 1 < n && t[i + 1].type == TT::IDENTIFIER) {
            this->declaration_parser_.declareGlobalVariable(t[i + 1].lexeme);
            continue;
        }
        if (tt != TT::IDENTIFIER || i + 1 >= n) continue;
        if (t[i + 1].type == TT::EQ) {
            this->declaration_parser_.declareGlobalVariable(t[i].lexeme);
            continue;
        }
        if (t[i + 1].type != TT::LT) continue;
        int angle = 0;
        size_t j = i + 1;
        for (; j < n; ++j) {
            TT a = t[j].type;
            if (a == TT::LT) ++angle;
            else if (a == TT::GT) --angle;
            else if (a == TT::RSHIFT) angle -= 2;
            else if (a == TT::NEWLINE || a == TT::END_OF_FILE || a == TT::LBRACE) { angle = -100; break; }
            if (angle <= 0) break;
        }
        if (angle != 0 || j + 1 >= n) continue;
        TT after = t[j + 1].type;
        if (after == TT::EQ || after == TT::NEWLINE || after == TT::SEMICOLON || after == TT::END_OF_FILE) {
            this->declaration_parser_.declareGlobalVariable(t[i].lexeme);
        }
    }
}

std::unique_ptr<vyb::ast::Module> ModuleParser::parse() {
    vyb::SourceLocation module_loc = this->current_location();
    std::vector<vyb::ast::StmtPtr> module_body;

    this->skip_comments_and_newlines();

    prescanModuleGlobals();

    while (this->peek().type != vyb::TokenType::END_OF_FILE) {
        vyb::token::Token current_token_before_parse = this->peek();

        std::unique_ptr<vyb::ast::Declaration> decl_node;
        try {
            decl_node = this->declaration_parser_.parse();
        } catch (const std::exception& e) {
            // A top-level declaration failed to parse. Record it, skip past the
            // broken region to the next declaration boundary, and keep going so
            // every error in the file is reported, not just the first.
            std::string msg = "Parse error at " + current_token_before_parse.location.toString()
                              + ": " + e.what();
            errors_.push_back(msg);
            synchronizeToNextDeclaration();
            continue;
        }

        if (decl_node) {
            module_body.push_back(std::move(decl_node));
            // Consume all consecutive semicolons after top-level declaration (e.g., class, struct, etc.)
            while (this->peek().type == vyb::TokenType::SEMICOLON) {
                this->consume();
            }
        } else {
            // Declaration parse returned null (failed without throwing): report it
            // and recover. If no progress was made the error loop still terminates
            // because synchronizeToNextDeclaration() always advances the position.
            std::string msg = "Parse error at " + this->peek().location.toString()
                              + ": unexpected token '" + vyb::token_type_to_string(this->peek().type) + "'";
            errors_.push_back(msg);
            // Always advance past the failure via sync; this also terminates the
            // loop even if no progress was made during the failed parse itself.
            synchronizeToNextDeclaration();
        }
    }

    return std::make_unique<vyb::ast::Module>(module_loc, std::move(module_body));
}

} // namespace vyb
