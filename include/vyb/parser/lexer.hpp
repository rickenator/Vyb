// SPDX-License-Identifier: Apache-2.0

#ifndef VYB_PARSER_LEXER_HPP
#define VYB_PARSER_LEXER_HPP

#include <string>
#include <vector>
#include <functional>
#include <stdexcept>
#include <unordered_map>
#include "token.hpp" // Provides vyb::token::Token and vyb::TokenType
#include "source_location.hpp" // Provides vyb::SourceLocation

// class Lexer is in the global namespace
class Lexer {
public:
  explicit Lexer(const std::string& source, const std::string& filePath); // Added filePath
  std::vector<vyb::token::Token> tokenize(); // Changed Token to vyb::token::Token
  void set_verbose(bool v) { verbose_ = v; }

private:
  std::string consume_while(std::function<bool(char)> pred) {
    std::string result;
    while (pos_ < source_.size() && pred(source_[pos_])) {
      result += source_[pos_];
      pos_++;
      // Do not increment column_ here, it's handled by the caller
      // or by the specific logic within tokenize() after calling consume_while.
    }
    return result;
  }
  bool is_letter(char c);
  bool is_digit(char c);
  // Consumes an exponent suffix -- `e`/`E`, an optional sign, at least one digit
  // (`1e-6`, `2.5E+9`) -- and returns it, or "" when the next character is not an
  // exponent marker at all. A marker with no digits is an error rather than being
  // silently handed back as an identifier (`1e` used to lex as `1` + `e`).
  std::string consume_exponent() {
    if (pos_ >= source_.size() || (source_[pos_] != 'e' && source_[pos_] != 'E')) return "";
    size_t probe = pos_ + 1;
    if (probe < source_.size() && (source_[probe] == '+' || source_[probe] == '-')) probe++;
    if (probe >= source_.size() || !is_digit(source_[probe])) {
      throw std::runtime_error("Invalid number format (scientific-notation exponent has no digits): "
        + std::string(1, source_[pos_]) + " at line " + std::to_string(line_) + ", column "
        + std::to_string(column_ + 1));
    }
    std::string result;
    result += source_[pos_++];
    if (pos_ < source_.size() && (source_[pos_] == '+' || source_[pos_] == '-'))
      result += source_[pos_++];
    while (pos_ < source_.size() && is_digit(source_[pos_])) result += source_[pos_++];
    return result;
  }
  vyb::TokenType get_keyword_type(const std::string& word); // Corrected namespace
  // Removed: std::string token_type_to_string(vyb::TokenType type); - Use vyb::token_type_to_string from token.hpp/token.cpp
  void handle_newline(std::vector<vyb::token::Token>& tokens); // Changed Token to vyb::token::Token

  std::string source_;
  std::string current_file_path_; // Added filePath member
  size_t pos_;
  int line_;
  int column_;
  std::vector<int> indent_levels_;
  int nesting_level_;
  bool verbose_ = false;
};

#endif // VYB_PARSER_LEXER_HPP