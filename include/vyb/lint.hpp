// SPDX-License-Identifier: Apache-2.0

// Vyb lint pass (`vyb check`) — AST-based warnings beyond compile errors.
// Walks the parsed module and emits diagnostics for a focused, low-false-positive
// rule set: constant boolean conditions, empty blocks, self-comparison,
// unreachable code after a terminator, and unused local variables/parameters.
// Non-executing by design; exit code reflects whether any warnings were emitted.

#ifndef VYB_LINT_HPP
#define VYB_LINT_HPP

#include "vyb/parser/ast.hpp"
#include <string>
#include <vector>

namespace vyb {
namespace lint {

// Lint a parsed module for \p filename. Returns a list of "line:col: <msg>"
// diagnostics (line/col 1-based). Does not modify the AST.
int lintModule(ast::Module* module, const std::string& filename, std::vector<std::string>& warnings);

} // namespace lint
} // namespace vyb

#endif // VYB_LINT_HPP
