// SPDX-License-Identifier: Apache-2.0
//
// Seam extracted from src/vre/semantic.cpp (#347): the member definitions below are moved
// VERBATIM -- no reformatting, no renames -- so the split stays behaviour-neutral.
// Everything that is file-local (static helpers, macros) travels with them or is
// promoted into the shared internal header, never duplicated.

#include "vyb/semantic.hpp"
#include "vyb/parser/token.hpp"
#include "vyb/parser/ast.hpp"
#include "vyb/driver.hpp"
#include "vyb/vre/semantic_internal.hpp"
#include "vyb/vre/thread_boundary.hpp"
#include <stdexcept>
#include <memory>
#include <unordered_set>
#include <string>
#include <map>
#include <set>
#include <functional>
#include <algorithm>

namespace vyb {
bool SemanticAnalyzer::isRawLocationType(ast::Expression* expr) {
    auto it = expressionTypes.find(exprKey(expr));
    if (it == expressionTypes.end() || !it->second) return false;
    return true;
}
bool SemanticAnalyzer::isRawLocationType(ast::TypeNode* type) {
    // Determine if the type is a raw location type (loc<T>)
    if (auto* typeName = dynamic_cast<ast::TypeName*>(type)) {
        return typeName->identifier && typeName->identifier->name == "loc" &&
               !typeName->genericArgs.empty();
       }
    return false;
}

} // namespace vyb
