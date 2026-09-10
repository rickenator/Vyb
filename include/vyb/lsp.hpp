// SPDX-License-Identifier: Apache-2.0

// Vyb Language Server Protocol implementation (`vyb lsp`) — a JSON-RPC 2.0 server
// over stdio (LSP framing: Content-Length headers). Provides text-sync driven
// diagnostics (parse errors + lint warnings) and the core features of
// go-to-definition, hover (signature + doc comment), and completion over the
// declarations of the open document.

#ifndef VYB_LSP_HPP
#define VYB_LSP_HPP

#include "vyb/parser/ast.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace vyb {
namespace lsp {

// Parse a source buffer into an AST (or nullptr + errors on failure). Provided by
// the caller (main.cpp) because the Vyb parser lives in that translation unit.
using ParseFn = std::function<std::unique_ptr<ast::Module>(
    const std::string& source, const std::string& uri, std::vector<std::string>& errors)>;

// Serve the LSP protocol on stdin/stdout until shutdown. Returns 0 on clean exit.
int serveLsp(ParseFn parse);

} // namespace lsp
} // namespace vyb

#endif // VYB_LSP_HPP
