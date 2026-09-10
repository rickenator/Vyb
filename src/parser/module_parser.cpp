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

std::unique_ptr<vyb::ast::Module> ModuleParser::parse() {
    vyb::SourceLocation module_loc = this->current_location();
    std::vector<vyb::ast::StmtPtr> module_body;

    this->skip_comments_and_newlines();

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
