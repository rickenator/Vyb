// SPDX-License-Identifier: Apache-2.0
// Analysis facts: what the semantic pass RECORDS about a node, kept off the AST.
//
// The immutable-AST half of the TypeTable migration (#392,
// doc/TYPETABLE_MIGRATION.md) asks that the parse tree stop being a scratchpad
// for the later passes. Types already moved to the node-id TypeTable; these
// are the remaining analysis-recorded values: whether a function contains a
// `fail` (and therefore lowers to the dual-return {T, i8*} error ABI), and
// which origin an operand of a cast/typename expression has.
//
// The analyzer owns them keyed by the stable Node::typeId() -- the same key
// and the same ownership model as the TypeTable -- and codegen queries them
// through one binding (LLVMCodegen::nodeFacts, bound from the Driver's
// SemanticAnalyzer in cgen_main.cpp) instead of reading and writing mutable
// node fields in either pass.
//
// Shared by vyb::SemanticAnalyzer (writer, single authority) and
// vyb::LLVMCodegen (reader): the same two-caller shape as
// include/vyb/vre/thread_boundary.hpp.
#pragma once

namespace vyb {
namespace analysis {

// Bit flags recorded against one node id.
enum NodeFact : unsigned {
    // The function holds a `fail` itself or reaches one through a callee.
    FuncCanFail = 1u << 0,
    // Lowering must use the dual-return {T, i8*} error-return ABI.
    FuncNeedsErrorReturn = 1u << 1,
    // A cast/typename operand is a wildcard trap error (`e<?>`): its runtime
    // type name is loaded at codegen rather than known statically.
    OperandWildcardError = 1u << 2,
    // A cast/typename operand's static type is the opaque `Type`, so the actual
    // type name must be resolved at runtime from the type id.
    OperandFromTypeValue = 1u << 3,
    // A cast/typename operand is a wildcard error with no static type at all.
    OperandIsWildcardError = 1u << 4,
    // The closure has mutable captures AND its value can outlive the defining
    // frame (returned, stored into a field/element, handed to a thread/call).
    // Codegen must therefore box each mutable capture into a heap cell owned by
    // the closure environment instead of storing the frame's stack address
    // (#384 checkpoint b(1)). A closure without this fact keeps the stack path
    // and its write-back-through-the-frame contract.
    ClosureMutablesBoxed = 1u << 5,
    // #384 checkpoint (c): this declaration's binding is mutably captured by a
    // closure that can escape, so it is promoted to one refcounted heap cell shared
    // by the defining frame and every environment that captures it -- instead of the
    // frame owning its own storage and each escaping closure getting a private copy.
    // Marked on the declaration (not the closure), because the frame's storage is
    // what changes and codegen must know before the closure literal is reached.
    CellPromotedBinding = 1u << 6,
};

} // namespace analysis
} // namespace vyb
