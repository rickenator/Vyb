// SPDX-License-Identifier: Apache-2.0

// Vyb source formatter (`--format` / `--check`).
// Walks the parsed AST and re-emits canonical, idempotent source: 4-space
// indentation, one statement per line, comma-joined field/variant lists,
// precedence-aware parenthesization (the AST dissolves explicit grouping, so a
// nested lower-precedence operand is re-parenthesized to preserve semantics).
// Comments are intentionally dropped (out of scope for the core formatter).

#ifndef VYB_FORMATTER_HPP
#define VYB_FORMATTER_HPP

#include "vyb/parser/ast.hpp"
#include <sstream>
#include <string>

namespace vyb {
namespace fmt {

// Formats a parsed module to canonical source. Implements ast::Visitor.
class SourcePrinter : public ast::Visitor {
public:
    // Format a whole module and return the canonical text.
    static std::string format(ast::Module* module);

    std::string str() const { return out_.str(); }

    void visit(ast::Module*) override;
    void visit(ast::Identifier*) override;
    void visit(ast::IntegerLiteral*) override;
    void visit(ast::FloatLiteral*) override;
    void visit(ast::StringLiteral*) override;
    void visit(ast::BooleanLiteral*) override;
    void visit(ast::NilLiteral*) override;
    void visit(ast::ObjectLiteral*) override;
    void visit(ast::ArrayLiteral*) override;

    void visit(ast::UnaryExpression*) override;
    void visit(ast::BinaryExpression*) override;
    void visit(ast::CallExpression*) override;
    void visit(ast::MemberExpression*) override;
    void visit(ast::AssignmentExpression*) override;
    void visit(ast::BorrowExpression*) override;
    void visit(ast::PointerDerefExpression*) override;
    void visit(ast::AddrOfExpression*) override;
    void visit(ast::FromIntToLocExpression*) override;
    void visit(ast::ArrayElementExpression*) override;
    void visit(ast::LocationExpression*) override;
    void visit(ast::ListComprehension*) override;
    void visit(ast::IfExpression*) override;
    void visit(ast::ConstructionExpression*) override;
    void visit(ast::ArrayInitializationExpression*) override;
    void visit(ast::GenericInstantiationExpression*) override;
    void visit(ast::LogicalExpression*) override;
    void visit(ast::ConditionalExpression*) override;
    void visit(ast::SequenceExpression*) override;
    void visit(ast::FunctionExpression*) override;
    void visit(ast::ThisExpression*) override;
    void visit(ast::SuperExpression*) override;
    void visit(ast::AwaitExpression*) override;
    void visit(ast::RangeExpression*) override;
    void visit(ast::BlockExpression*) override;
    void visit(ast::SelectExpression*) override;
    void visit(ast::TypeofExpression*) override;
    void visit(ast::TypenameExpression*) override;
    void visit(ast::AsExpression*) override;
    void visit(ast::MatchExpression*) override;
    void visit(ast::ComparisonPattern*) override;
    void visit(ast::StructPattern*) override;
    void visit(ast::SetPattern*) override;

    void visit(ast::BlockStatement*) override;
    void visit(ast::ExpressionStatement*) override;
    void visit(ast::IfStatement*) override;
    void visit(ast::ForStatement*) override;
    void visit(ast::WhileStatement*) override;
    void visit(ast::ReturnStatement*) override;
    void visit(ast::PassStatement*) override;
    void visit(ast::BreakStatement*) override;
    void visit(ast::ContinueStatement*) override;
    void visit(ast::TryStatement*) override;
    void visit(ast::FreedomStatement*) override;
    void visit(ast::EmptyStatement*) override;
    void visit(ast::ExternStatement*) override;
    void visit(ast::ThrowStatement*) override;
    void visit(ast::MatchStatement*) override;
    void visit(ast::YieldStatement*) override;
    void visit(ast::YieldReturnStatement*) override;
    void visit(ast::AssertStatement*) override;
    void visit(ast::FailStatement*) override;
    void visit(ast::RefailStatement*) override;
    void visit(ast::PanicStatement*) override;
    void visit(ast::ExitStatement*) override;
    void visit(ast::DeferStatement*) override;
    void visit(ast::TrapClause*) override;
    void visit(ast::EnsureClause*) override;
    void visit(ast::TupleDestructureAssignment*) override;

    void visit(ast::VariableDeclaration*) override;
    void visit(ast::FunctionDeclaration*) override;
    void visit(ast::TypeAliasDeclaration*) override;
    void visit(ast::ImportDeclaration*) override;
    void visit(ast::StructDeclaration*) override;
    void visit(ast::ClassDeclaration*) override;
    void visit(ast::FieldDeclaration*) override;
    void visit(ast::BindDeclaration*) override;
    void visit(ast::EnumDeclaration*) override;
    void visit(ast::EnumVariant*) override;
    void visit(ast::GenericParameter*) override;
    void visit(ast::TemplateDeclaration*) override;
    void visit(ast::AspectDeclaration*) override;
    void visit(ast::NamespaceDeclaration*) override;

    void visit(ast::TypeNode*) override { emit("typename"); }
    void visit(ast::TypeName*) override;
    void visit(ast::PointerType*) override;
    void visit(ast::ArrayType*) override;
    void visit(ast::VecType*) override;
    void visit(ast::FutureType*) override;
    void visit(ast::FunctionType*) override;
    void visit(ast::OptionalType*) override;
    void visit(ast::TupleTypeNode*) override;

    // Print a type node as <...> generic suffix including the angle brackets.
    void printGenericArgs(const std::vector<ast::TypeNodePtr>& args);

private:
    // --- low-level emission helpers ---
    std::ostringstream out_;
    int indent_ = 0;
    bool atLineStart_ = true;

    void emit(const std::string& s);
    void space() { emit(" "); }
    void newline();
    void push() { indent_++; }
    void pop() { indent_--; }

    // Print a type node (no generic suffix special-casing beyond TypeName).
    void printType(const ast::TypeNode* t);
    void printParamList(const std::vector<ast::FunctionParameter>& params);
    void printGenericParams(const std::vector<std::unique_ptr<ast::GenericParameter>>& gps);
    // Print a statement list inside a { } block, indented.
    void printBlock(ast::BlockStatement* block);

    // Issue a statement then a newline; dedent-first for closing constructs.
    void stmt(ast::Statement* s) { s->accept(*this); newline(); }

    // Precedence-aware expression emission: prints \p e, wrapping it in parens
    // if its own precedence is lower than \p parentPrec (0 = no constraint).
    void printExpr(ast::Expression* e, int parentPrec = 0);
    // Operator precedence of a node kind; higher binds tighter.
    static int precOf(const ast::Expression* e);
};

} // namespace fmt
} // namespace vyb

#endif // VYB_FORMATTER_HPP
