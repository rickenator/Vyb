// SPDX-License-Identifier: Apache-2.0

// Vyb doc generator (`vyb doc`) — turns `///` doc comments on declarations plus
// the parsed AST into a self-contained HTML reference page per module. Doc
// comments are `/// text` (Vyb's third slash goes on the lines immediately above
// a declaration); consecutive `///` lines form a block attached to the following
// declaration. No execution; parse-only.

#ifndef VYB_DOCGEN_HPP
#define VYB_DOCGEN_HPP

#include "vyb/parser/ast.hpp"
#include <map>
#include <string>
#include <vector>

namespace vyb {
namespace docgen {

// Render a parsed module (with raw \p source for doc-comment extraction) to an
// HTML document body fragment (declaration sections). Returns 0 on success.
int renderModule(ast::Module* module, const std::string& source,
                 const std::string& title, std::string& htmlBody);

// Build the full HTML page (head + TOC + body) for \p title / \p bodyFragments.
std::string buildPage(const std::string& title, const std::vector<std::string>& bodyFragments,
                      const std::vector<std::pair<std::string,std::string>>& toc);

} // namespace docgen
} // namespace vyb

#endif // VYB_DOCGEN_HPP
