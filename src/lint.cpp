// SPDX-License-Identifier: Apache-2.0

// Vyb lint pass implementation (see lint.hpp). Recursively walks the AST and
// emits diagnostics. Unused-variable/parameter detection is done per function
// with a conservative name-count heuristic (flag only when a declared name never
// appears as an identifier beyond its own declaration), to keep false positives
// near zero.

#include "vyb/lint.hpp"
#include <map>
#include <sstream>

namespace vyb {
namespace lint {

namespace {

struct Frame {
    std::map<std::string, int> declared; // local var/param declared counts
    std::map<std::string, std::pair<unsigned,unsigned>> declaredLoc; // first decl loc
    std::map<std::string, int> uses;     // identifier use counts (incl. decl ids)
};

class Linter {
public:
    std::string filename;
    std::vector<std::string>& warnings;
    std::vector<Frame> frames;

    Linter(const std::string& fn, std::vector<std::string>& w) : filename(fn), warnings(w) {}

    void warn(unsigned line, unsigned col, const std::string& msg) {
        std::ostringstream o;
        o << filename << ":" << line << ":" << col << ": warning: " << msg;
        warnings.push_back(o.str());
    }
    void warnNode(const ast::Node* n, const std::string& msg) {
        if (n) warn(n->loc.line, n->loc.column, msg);
        else warn(1, 1, msg);
    }
    bool isTerminator(const ast::Statement* s) const {
        ast::NodeType t = s->getType();
        return t == ast::NodeType::RETURN_STATEMENT ||
               t == ast::NodeType::BREAK_STATEMENT ||
               t == ast::NodeType::CONTINUE_STATEMENT;
    }

    void walk(ast::Expression* e);
    void walk(ast::Statement* s);
    void walk(ast::Declaration* d);
    void walkBlock(ast::BlockStatement* b);

    // Register a declaration (first occurrence's location) in the current frame.
    void declare(const std::string& name, unsigned line, unsigned col) {
        if (name.empty() || frames.empty()) return;
        Frame& f = frames.back();
        f.declared[name]++;
        if (!f.declaredLoc.count(name)) f.declaredLoc[name] = {line, col};
    }
    // Evaluate + pop the current frame: warn names declared but never used.
    void finishFrame() {
        if (frames.empty()) return;
        Frame f = frames.back();
        frames.pop_back();
        for (auto& kv : f.declared) {
            int uses = f.uses.count(kv.first) ? f.uses[kv.first] : 0;
            if (uses <= kv.second) { // only its own declaration(s), no real use
                auto lit = f.declaredLoc.find(kv.first);
                unsigned ln = 1, cl = 1;
                if (lit != f.declaredLoc.end()) { ln = lit->second.first; cl = lit->second.second; }
                warn(ln, cl, "unused variable '" + kv.first + "'");
            }
        }
    }
};

// Count an identifier reference into the innermost function frame (if inside one).
void countUse(Linter& L, const std::string& name) {
    if (name.empty()) return;
    if (!L.frames.empty()) L.frames.back().uses[name]++;
}

void Linter::walk(ast::Expression* e) {
    if (!e) return;
    switch (e->getType()) {
        case ast::NodeType::IDENTIFIER: {
            auto* n = static_cast<ast::Identifier*>(e);
            countUse(*this, n->name);
            return;
        }
        case ast::NodeType::INTEGER_LITERAL: case ast::NodeType::FLOAT_LITERAL:
        case ast::NodeType::STRING_LITERAL: case ast::NodeType::BOOLEAN_LITERAL:
        case ast::NodeType::NIL_LITERAL: case ast::NodeType::THIS_EXPRESSION:
        case ast::NodeType::SUPER_EXPRESSION:
            return;
        case ast::NodeType::ARRAY_LITERAL: {
            auto* n = static_cast<ast::ArrayLiteral*>(e);
            for (auto& el : n->elements) walk(el.get());
            return;
        }
        case ast::NodeType::OBJECT_LITERAL: {
            auto* n = static_cast<ast::ObjectLiteral*>(e);
            for (auto& p : n->properties)
                if (p.value) walk(p.value.get());
            return;
        }
        case ast::NodeType::UNARY_EXPRESSION: {
            auto* n = static_cast<ast::UnaryExpression*>(e);
            walk(n->operand.get());
            return;
        }
        case ast::NodeType::BINARY_EXPRESSION: {
            auto* n = static_cast<ast::BinaryExpression*>(e);
            const auto* lhs = dynamic_cast<const ast::Identifier*>(n->left.get());
            const auto* rhs = dynamic_cast<const ast::Identifier*>(n->right.get());
            if (lhs && rhs && lhs->name == rhs->name &&
                (n->op.lexeme == "==" || n->op.lexeme == "!=" || n->op.lexeme == "<" ||
                 n->op.lexeme == ">" || n->op.lexeme == "<=" || n->op.lexeme == ">="))
                warnNode(n, "comparing '" + lhs->name + "' to itself");
            walk(n->left.get());
            walk(n->right.get());
            return;
        }
        case ast::NodeType::LOGICAL_EXPRESSION: {
            auto* n = static_cast<ast::LogicalExpression*>(e);
            walk(n->left.get()); walk(n->right.get());
            return;
        }
        case ast::NodeType::CALL_EXPRESSION: {
            auto* n = static_cast<ast::CallExpression*>(e);
            walk(n->callee.get());
            for (auto& a : n->arguments) walk(a.get());
            return;
        }
        case ast::NodeType::MEMBER_EXPRESSION: {
            auto* n = static_cast<ast::MemberExpression*>(e);
            walk(n->object.get());
            if (n->computed) walk(n->property.get());
            return;
        }
        case ast::NodeType::ASSIGNMENT_EXPRESSION: {
            auto* n = static_cast<ast::AssignmentExpression*>(e);
            walk(n->left.get()); walk(n->right.get());
            return;
        }
        case ast::NodeType::BORROW_EXPRESSION: {
            auto* n = static_cast<ast::BorrowExpression*>(e);
            walk(n->expression.get());
            return;
        }
        case ast::NodeType::POINTER_DEREF_EXPRESSION: {
            auto* n = static_cast<ast::PointerDerefExpression*>(e);
            walk(n->pointer.get());
            return;
        }
        case ast::NodeType::ADDR_OF_EXPRESSION: {
            auto* n = static_cast<ast::AddrOfExpression*>(e);
            walk(n->getLocation().get());
            return;
        }
        case ast::NodeType::FROM_INT_TO_LOC_EXPRESSION: {
            auto* n = static_cast<ast::FromIntToLocExpression*>(e);
            walk(n->getAddressExpression().get());
            return;
        }
        case ast::NodeType::ARRAY_ELEMENT_EXPRESSION: {
            auto* n = static_cast<ast::ArrayElementExpression*>(e);
            walk(n->array.get()); walk(n->index.get());
            return;
        }
        case ast::NodeType::LOCATION_EXPRESSION: {
            auto* n = static_cast<ast::LocationExpression*>(e);
            walk(n->expression.get());
            return;
        }
        case ast::NodeType::LIST_COMPREHENSION: {
            auto* n = static_cast<ast::ListComprehension*>(e);
            walk(n->elementExpr.get());
            walk(n->iterableExpr.get());
            if (n->conditionExpr) walk(n->conditionExpr.get());
            return;
        }
        case ast::NodeType::IF_EXPRESSION: {
            auto* n = static_cast<ast::IfExpression*>(e);
            if (auto* bl = dynamic_cast<ast::BooleanLiteral*>(n->condition.get()))
                warnNode(n, "condition is always " + std::string(bl->value ? "true" : "false"));
            walk(n->condition.get()); walk(n->thenBranch.get()); walk(n->elseBranch.get());
            return;
        }
        case ast::NodeType::CONSTRUCTION_EXPRESSION: {
            auto* n = static_cast<ast::ConstructionExpression*>(e);
            for (auto& a : n->arguments) walk(a.get());
            return;
        }
        case ast::NodeType::GENERIC_INSTANTIATION_EXPRESSION: {
            auto* n = static_cast<ast::GenericInstantiationExpression*>(e);
            walk(n->baseExpression.get());
            return;
        }
        case ast::NodeType::CONDITIONAL_EXPRESSION: {
            auto* n = static_cast<ast::ConditionalExpression*>(e);
            walk(n->condition.get()); walk(n->thenExpr.get()); walk(n->elseExpr.get());
            return;
        }
        case ast::NodeType::SEQUENCE_EXPRESSION: {
            auto* n = static_cast<ast::SequenceExpression*>(e);
            for (auto& x : n->expressions) walk(x.get());
            return;
        }
        case ast::NodeType::FUNCTION_EXPRESSION: {
            auto* n = static_cast<ast::FunctionExpression*>(e);
            frames.push_back(Frame{});
            for (auto& p : n->params)
                if (p.name) { countUse(*this, p.name->name); declare(p.name->name, p.name->loc.line, p.name->loc.column); }
            walk(n->body.get());
            finishFrame();
            return;
        }
        case ast::NodeType::AWAIT_EXPRESSION: {
            auto* n = static_cast<ast::AwaitExpression*>(e);
            walk(n->expr.get());
            return;
        }
        case ast::NodeType::RANGE_EXPRESSION: {
            auto* n = static_cast<ast::RangeExpression*>(e);
            walk(n->start.get()); walk(n->end.get());
            if (n->step) walk(n->step.get());
            return;
        }
        case ast::NodeType::BLOCK_EXPRESSION: {
            auto* n = static_cast<ast::BlockExpression*>(e);
            if (n->block) walkBlock(n->block.get());
            return;
        }
        case ast::NodeType::SELECT_EXPRESSION: {
            auto* n = static_cast<ast::SelectExpression*>(e);
            walk(n->expr.get());
            for (auto& c : n->cases) { walk(c.first.get()); walk(c.second.get()); }
            return;
        }
        case ast::NodeType::TYPEOF_EXPRESSION: {
            auto* n = static_cast<ast::TypeofExpression*>(e);
            if (n->operand) walk(n->operand.get());
            return;
        }
        case ast::NodeType::TYPENAME_EXPRESSION: {
            auto* n = static_cast<ast::TypenameExpression*>(e);
            if (n->operand) walk(n->operand.get());
            return;
        }
        case ast::NodeType::AS_EXPRESSION: {
            auto* n = static_cast<ast::AsExpression*>(e);
            walk(n->operand.get());
            return;
        }
        case ast::NodeType::MATCH_EXPRESSION: {
            auto* n = static_cast<ast::MatchExpression*>(e);
            if (n->match) walk(n->match.get());
            return;
        }
        case ast::NodeType::COMPARISON_PATTERN: {
            auto* n = static_cast<ast::ComparisonPattern*>(e);
            walk(n->value.get());
            return;
        }
        case ast::NodeType::STRUCT_PATTERN: {
            auto* n = static_cast<ast::StructPattern*>(e);
            for (auto& b : n->bindings) countUse(*this, b ? b->name : "");
            return;
        }
        case ast::NodeType::SET_PATTERN: {
            auto* n = static_cast<ast::SetPattern*>(e);
            for (auto& el : n->elements) walk(el.get());
            return;
        }
        default:
            return;
    }
}

void Linter::walkBlock(ast::BlockStatement* b) {
    if (!b) return;
    if (b->body.empty()) { warnNode(b, "empty block"); return; }
    for (size_t i = 0; i < b->body.size(); ++i) {
        ast::Statement* s = b->body[i].get();
        walk(s);
        if (i + 1 < b->body.size() && isTerminator(s))
            warnNode(b->body[i + 1].get(), "unreachable statement after terminator");
    }
}

void Linter::walk(ast::Statement* s) {
    if (!s) return;
    switch (s->getType()) {
        case ast::NodeType::BLOCK_STATEMENT: { auto* n = static_cast<ast::BlockStatement*>(s); walkBlock(n); return; }
        case ast::NodeType::EMPTY_STATEMENT: return;
        case ast::NodeType::EXPRESSION_STATEMENT: { auto* n = static_cast<ast::ExpressionStatement*>(s); walk(n->expression.get()); return; }
        case ast::NodeType::IF_STATEMENT: {
            auto* n = static_cast<ast::IfStatement*>(s);
            if (auto* bl = dynamic_cast<ast::BooleanLiteral*>(n->test.get()))
                warnNode(n, "condition is always " + std::string(bl->value ? "true" : "false"));
            walk(n->test.get());
            walk(n->consequent.get());
            walk(n->alternate.get());
            return;
        }
        case ast::NodeType::FOR_STATEMENT: {
            auto* n = static_cast<ast::ForStatement*>(s);
            if (auto* bl = dynamic_cast<ast::BooleanLiteral*>(n->test.get()))
                warnNode(n, "condition is always " + std::string(bl->value ? "true" : "false"));
            if (n->init) {
                if (auto* var = dynamic_cast<ast::VariableDeclaration*>(n->init.get())) walk(static_cast<ast::Declaration*>(var));
                else if (auto* es = dynamic_cast<ast::ExpressionStatement*>(n->init.get())) walk(es);
            }
            walk(n->test.get());
            walk(n->update.get());
            walk(n->body.get());
            return;
        }
        case ast::NodeType::WHILE_STATEMENT: {
            auto* n = static_cast<ast::WhileStatement*>(s);
            if (auto* bl = dynamic_cast<ast::BooleanLiteral*>(n->test.get()))
                warnNode(n, "condition is always " + std::string(bl->value ? "true" : "false"));
            walk(n->test.get());
            walk(n->body.get());
            return;
        }
        case ast::NodeType::RETURN_STATEMENT: { auto* n = static_cast<ast::ReturnStatement*>(s); walk(n->argument.get()); return; }
        case ast::NodeType::PASS_STATEMENT: { auto* n = static_cast<ast::PassStatement*>(s); walk(n->argument.get()); return; }
        case ast::NodeType::BREAK_STATEMENT: case ast::NodeType::CONTINUE_STATEMENT: return;
        case ast::NodeType::TRY_STATEMENT: {
            auto* n = static_cast<ast::TryStatement*>(s);
            if (n->tryBlock) walkBlock(n->tryBlock.get());
            if (n->catchBlock) walkBlock(n->catchBlock.get());
            if (n->finallyBlock) walkBlock(n->finallyBlock.get());
            return;
        }
        case ast::NodeType::FREEDOM_STATEMENT: { auto* n = static_cast<ast::FreedomStatement*>(s); if (n->block) walkBlock(n->block.get()); return; }
        case ast::NodeType::EXTERN_STATEMENT: return;
        case ast::NodeType::THROW_STATEMENT: { auto* n = static_cast<ast::ThrowStatement*>(s); walk(n->expr.get()); return; }
        case ast::NodeType::MATCH_STATEMENT: {
            auto* n = static_cast<ast::MatchStatement*>(s);
            walk(n->expr.get());
            for (size_t i = 0; i < n->cases.size(); ++i) {
                walk(n->cases[i].first.get());
                walk(n->cases[i].second.get());
                if (i < n->guards.size()) walk(n->guards[i].get());
            }
            return;
        }
        case ast::NodeType::YIELD_STATEMENT: { auto* n = static_cast<ast::YieldStatement*>(s); walk(n->expression.get()); return; }
        case ast::NodeType::YIELD_RETURN_STATEMENT: { auto* n = static_cast<ast::YieldReturnStatement*>(s); walk(n->expression.get()); return; }
        case ast::NodeType::ASSERT_STATEMENT: {
            auto* n = static_cast<ast::AssertStatement*>(s);
            walk(n->condition.get());
            if (n->message) walk(n->message.get());
            return;
        }
        case ast::NodeType::FAIL_STATEMENT: { auto* n = static_cast<ast::FailStatement*>(s); walk(n->error.get()); return; }
        case ast::NodeType::REFAIL_STATEMENT: { auto* n = static_cast<ast::RefailStatement*>(s); walk(n->wrappedError.get()); return; }
        case ast::NodeType::PANIC_STATEMENT: { auto* n = static_cast<ast::PanicStatement*>(s); walk(n->message.get()); return; }
        case ast::NodeType::EXIT_STATEMENT: { auto* n = static_cast<ast::ExitStatement*>(s); walk(n->code.get()); return; }
        case ast::NodeType::DEFER_STATEMENT: { auto* n = static_cast<ast::DeferStatement*>(s); walk(n->statement.get()); return; }
        case ast::NodeType::TUPLE_DESTRUCTURE_ASSIGNMENT: {
            auto* n = static_cast<ast::TupleDestructureAssignment*>(s);
            for (auto& id : n->identifiers) countUse(*this, id ? id->name : "");
            walk(n->expression.get());
            return;
        }
        case ast::NodeType::VARIABLE_DECLARATION:
            walk(static_cast<ast::Declaration*>(static_cast<ast::VariableDeclaration*>(s)));
            return;
        case ast::NodeType::FUNCTION_DECLARATION:
            walk(static_cast<ast::Declaration*>(static_cast<ast::FunctionDeclaration*>(s)));
            return;
        default:
            return;
    }
}

void Linter::walk(ast::Declaration* d) {
    if (!d) return;
    switch (d->getType()) {
        case ast::NodeType::VARIABLE_DECLARATION: {
            auto* n = static_cast<ast::VariableDeclaration*>(d);
            if (n->id) {
                countUse(*this, n->id->name);
                declare(n->id->name, n->id->loc.line, n->id->loc.column);
            }
            if (n->init) walk(n->init.get());
            return;
        }
        case ast::NodeType::FUNCTION_DECLARATION: {
            auto* n = static_cast<ast::FunctionDeclaration*>(d);
            frames.push_back(Frame{});
            for (auto& p : n->params)
                if (p.name) { countUse(*this, p.name->name); declare(p.name->name, p.name->loc.line, p.name->loc.column); }
            if (n->body) walkBlock(n->body.get());
            finishFrame();
            return;
        }
        case ast::NodeType::TYPE_ALIAS_DECLARATION: case ast::NodeType::IMPORT_DECLARATION:
        case ast::NodeType::FIELD_DECLARATION: case ast::NodeType::ENUM_DECLARATION:
        case ast::NodeType::ENUM_VARIANT: case ast::NodeType::GENERIC_PARAMETER:
            return;
        case ast::NodeType::STRUCT_DECLARATION: {
            auto* n = static_cast<ast::StructDeclaration*>(d);
            for (auto& f : n->fields) walk(static_cast<ast::Declaration*>(f.get()));
            for (auto& c : n->constructors) walk(static_cast<ast::Declaration*>(c.get()));
            return;
        }
        case ast::NodeType::CLASS_DECLARATION: {
            auto* n = static_cast<ast::ClassDeclaration*>(d);
            for (auto& m : n->members) {
                if (m) {
                    if (dynamic_cast<ast::FunctionDeclaration*>(m.get())) walk(static_cast<ast::Declaration*>(m.get()));
                    else if (dynamic_cast<ast::FieldDeclaration*>(m.get())) walk(static_cast<ast::Declaration*>(m.get()));
                }
            }
            return;
        }
        case ast::NodeType::BIND_DECLARATION: {
            auto* n = static_cast<ast::BindDeclaration*>(d);
            for (auto& m : n->methods) walk(static_cast<ast::Declaration*>(m.get()));
            return;
        }
        case ast::NodeType::TEMPLATE_DECLARATION: {
            auto* n = static_cast<ast::TemplateDeclaration*>(d);
            if (n->body) walk(static_cast<ast::Declaration*>(n->body.get()));
            return;
        }
        case ast::NodeType::ASPECT_DECLARATION: {
            auto* n = static_cast<ast::AspectDeclaration*>(d);
            for (auto& m : n->methods) walk(static_cast<ast::Declaration*>(m.get()));
            return;
        }
        case ast::NodeType::NAMESPACE_DECLARATION: {
            auto* n = static_cast<ast::NamespaceDeclaration*>(d);
            for (auto& m : n->members) walk(static_cast<ast::Declaration*>(m.get()));
            return;
        }
        default:
            return;
    }
}

} // namespace

int lintModule(ast::Module* module, const std::string& filename, std::vector<std::string>& warnings) {
    if (!module) return 0;
    Linter L(filename, warnings);
    // Walk the module body (declarations and top-level statements).
    for (auto& s : module->body) {
        auto* st = s.get();
        if (auto* d = dynamic_cast<ast::Declaration*>(st)) L.walk(d);
        else L.walk(st);
    }
    return (int)warnings.size();
}

} // namespace lint
} // namespace vyb
