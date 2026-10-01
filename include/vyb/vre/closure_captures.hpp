// SPDX-License-Identifier: Apache-2.0
// A closure's capture lists as recorded by the semantic pass.
//
// Splitting these out of SemanticAnalyzer (and out of the AST) is the #392
// immutable-AST work: the closure node used to carry three mutable
// `std::vector<std::string>` fields that semantic filled in and codegen read
// back. They are analysis output, not parse-tree structure, so the analyzer now
// owns them keyed by the stable Node::typeId() (SemanticAnalyzer::resetCaptures
// / addCapture / capturesOf) and codegen reads them through its bound
// `nodeCaptures` query.
//
// Shared by vyb::SemanticAnalyzer and vyb::LLVMCodegen, so it lives in its own
// header rather than inside semantic.hpp (which codegen must not depend on).
#pragma once

#include <string>
#include <vector>

namespace vyb {
namespace analysis {

struct ClosureCaptures {
    // Every free variable the body references from an enclosing scope: codegen
    // copies each value (or the frame address, when mutable) into the closure's
    // environment at creation time.
    std::vector<std::string> captured;
    // The subset the body writes to. Codegen stores the OUTER variable's address
    // in the env so writes propagate back; a mutable capture therefore holds a
    // pointer into the defining frame and cannot cross a thread boundary.
    std::vector<std::string> mutableCaptured;
    // The subset that is `our<T>` (shared): codegen bumps the strong count at
    // capture so the shared value stays alive for the life of the closure.
    std::vector<std::string> ourCaptured;
};

} // namespace analysis
} // namespace vyb
