// SPDX-License-Identifier: Apache-2.0

// Vyb source formatter implementation (see format.hpp). AST-visitor based:
// re-emits canonical idempotent source with 4-space indentation and
// precedence-aware parenthesization.

#include "vyb/format.hpp"
#include <iomanip>

namespace vyb {
namespace fmt {

std::string SourcePrinter::format(ast::Module* module) {
    SourcePrinter p;
    module->accept(p);
    std::string s = p.str();
    // Normalize trailing whitespace / EOF: strip trailing blank lines, ensure
    // exactly one trailing newline.
    while (!s.empty() && (s.back() == '\n' || s.back() == ' ' || s.back() == '\t'))
        s.pop_back();
    s.push_back('\n');
    return s;
}

// --- low level ---
void SourcePrinter::emit(const std::string& s) {
    if (atLineStart_) {
        for (int i = 0; i < indent_; ++i) out_ << "    ";
        atLineStart_ = false;
    }
    out_ << s;
}
void SourcePrinter::newline() {
    out_ << "\n";
    atLineStart_ = true;
}

// --- precedence ---
int SourcePrinter::precOf(const ast::Expression* e) {
    switch (e->getType()) {
        case ast::NodeType::ASSIGNMENT_EXPRESSION:
        case ast::NodeType::SEQUENCE_EXPRESSION: return 1;
        case ast::NodeType::CONDITIONAL_EXPRESSION:
        case ast::NodeType::MATCH_EXPRESSION:
        case ast::NodeType::SELECT_EXPRESSION:
        case ast::NodeType::IF_EXPRESSION:
        case ast::NodeType::AS_EXPRESSION: return 2;
        case ast::NodeType::RANGE_EXPRESSION: return 3;
        case ast::NodeType::LOGICAL_EXPRESSION: {
            auto* le = static_cast<const ast::LogicalExpression*>(e);
            const std::string& op = le->op.lexeme;
            if (op == "||") return 4;
            if (op == "&&") return 5;
            return 4;
        }
        case ast::NodeType::BINARY_EXPRESSION: {
            auto* be = static_cast<const ast::BinaryExpression*>(e);
            const std::string& op = be->op.lexeme;
            if (op == "||") return 4;
            if (op == "&&") return 5;
            if (op == "|") return 6;
            if (op == "^") return 7;
            if (op == "&") return 8;
            if (op == "==" || op == "!=") return 9;
            if (op == "<" || op == ">" || op == "<=" || op == ">=") return 10;
            if (op == "<<" || op == ">>") return 11;
            if (op == "+" || op == "-") return 12;
            return 13; // * / %
        }
        case ast::NodeType::UNARY_EXPRESSION: return 14;
        default: return 15; // atoms / postfix
    }
}

void SourcePrinter::printExpr(ast::Expression* e, int parentPrec) {
    if (!e) return;
    int myPrec = precOf(e);
    bool needParen = parentPrec > myPrec;
    if (needParen) emit("(");
    e->accept(*this);
    if (needParen) emit(")");
}

// --- types ---
void SourcePrinter::printGenericArgs(const std::vector<ast::TypeNodePtr>& args) {
    if (args.empty()) return;
    emit("<");
    for (size_t i = 0; i < args.size(); ++i) {
        if (i) emit(", ");
        if (args[i]) args[i]->accept(*this);
    }
    emit(">");
}

void SourcePrinter::printType(const ast::TypeNode* t) {
    if (!t) return;
    switch (t->getType()) {
        case ast::NodeType::TYPE_NAME: {
            auto* tn = static_cast<const ast::TypeName*>(t);
            if (tn->identifier) emit(tn->identifier->name);
            printGenericArgs(tn->genericArgs);
            break;
        }
        case ast::NodeType::VEC_TYPE: {
            auto* vt = static_cast<const ast::VecType*>(t);
            emit("Vec<");
            if (vt->elementType) vt->elementType->accept(*this);
            emit(">");
            break;
        }
        case ast::NodeType::OPTIONAL_TYPE: {
            auto* ot = static_cast<const ast::OptionalType*>(t);
            if (ot->containedType) ot->containedType->accept(*this);
            emit("?");
            break;
        }
        case ast::NodeType::FUNCTION_TYPE: {
            auto* ft = static_cast<const ast::FunctionType*>(t);
            emit("fn(");
            for (size_t i = 0; i < ft->parameterTypes.size(); ++i) {
                if (i) emit(", ");
                if (ft->parameterTypes[i]) ft->parameterTypes[i]->accept(*this);
            }
            emit(")");
            if (ft->returnType) { emit(" -> "); ft->returnType->accept(*this); }
            break;
        }
        case ast::NodeType::POINTER_TYPE: {
            auto* pt = static_cast<const ast::PointerType*>(t);
            emit("loc<");
            if (pt->pointeeType) pt->pointeeType->accept(*this);
            emit(">");
            break;
        }
        case ast::NodeType::ARRAY_TYPE: {
            auto* at = static_cast<const ast::ArrayType*>(t);
            emit("[");
            if (at->elementType) at->elementType->accept(*this);
            if (at->sizeExpression) {
                emit("; ");
                at->sizeExpression->accept(*this);
            }
            emit("]");
            break;
        }
        case ast::NodeType::FUTURE_TYPE: {
            auto* fut = static_cast<const ast::FutureType*>(t);
            emit("Future<");
            if (fut->resultType) fut->resultType->accept(*this);
            emit(">");
            break;
        }
        case ast::NodeType::TUPLE_TYPE: {
            auto* tt = static_cast<const ast::TupleTypeNode*>(t);
            emit("(");
            for (size_t i = 0; i < tt->memberTypes.size(); ++i) {
                if (i) emit(", ");
                if (tt->memberTypes[i]) tt->memberTypes[i]->accept(*this);
            }
            emit(")");
            break;
        }
        default:
            emit(t->toString());
            break;
    }
}

void SourcePrinter::printGenericParams(const std::vector<std::unique_ptr<ast::GenericParameter>>& gps) {
    if (gps.empty()) return;
    emit("<");
    for (size_t i = 0; i < gps.size(); ++i) {
        if (i) emit(", ");
        if (gps[i]->name) emit(gps[i]->name->name);
        if (!gps[i]->bounds.empty()) {
            emit(": ");
            for (size_t j = 0; j < gps[i]->bounds.size(); ++j) {
                if (j) emit(" + ");
                if (gps[i]->bounds[j]) gps[i]->bounds[j]->accept(*this);
            }
        }
    }
    emit(">");
}

void SourcePrinter::printParamList(const std::vector<ast::FunctionParameter>& params) {
    emit("(");
    for (size_t i = 0; i < params.size(); ++i) {
        if (i) emit(", ");
        if (params[i].name) emit(params[i].name->name);
        if (params[i].typeNode) { emit("<"); params[i].typeNode->accept(*this); emit(">"); }
    }
    emit(")");
}

void SourcePrinter::printBlock(ast::BlockStatement* block) {
    emit("{");
    if (block && !block->body.empty()) {
        newline(); push();
        for (auto& s : block->body) { stmt(s.get()); }
        pop(); emit("}");
    } else {
        emit(" }");
    }
}

// --- Module / declarations ---
void SourcePrinter::visit(ast::Module* m) {
    bool first = true;
    for (auto& s : m->body) {
        if (!first) newline();
        first = false;
        stmt(s.get());
    }
}

void SourcePrinter::visit(ast::VariableDeclaration* n) {
    // isConst distinguishes `let` (explicit) from default `var`/bare; a bare
    // declaration is the repo's canonical style, so only emit `let` for const.
    if (n->isConst) emit("let ");
    if (n->id) emit(n->id->name);
    if (n->typeNode) { emit("<"); n->typeNode->accept(*this); emit(">"); }
    if (n->init) { emit(" = "); n->init->accept(*this); }
}

void SourcePrinter::visit(ast::FunctionDeclaration* n) {
    if (n->id) emit(n->id->name);
    printGenericParams(n->genericParams);
    printParamList(n->params);
    if (n->variadic) emit("...");
    if (n->returnTypeNode) { emit("<"); n->returnTypeNode->accept(*this); emit(">"); }
    if (n->body && !n->body->body.empty()) {
        emit(" -> ");
        printBlock(n->body.get());
    }
}

void SourcePrinter::visit(ast::StructDeclaration* n) {
    emit(n->reprC ? "struct reprC " : "struct ");
    if (n->name) emit(n->name->name);
    printGenericParams(n->genericParams);
    emit(" {");
    newline(); push();
    for (size_t i = 0; i < n->fields.size(); ++i) {
        visit(n->fields[i].get());
        emit((i + 1 < n->fields.size() || !n->constructors.empty()) ? "," : "");
        newline();
    }
    for (size_t i = 0; i < n->constructors.size(); ++i) {
        visit(n->constructors[i].get());
        if (i + 1 < n->constructors.size()) emit(",");
        newline();
    }
    pop(); emit("}");
}

void SourcePrinter::visit(ast::FieldDeclaration* n) {
    if (n->name) emit(n->name->name);
    if (n->typeNode) { emit("<"); n->typeNode->accept(*this); emit(">"); }
    if (n->initializer) { emit(" = "); n->initializer->accept(*this); }
}

void SourcePrinter::visit(ast::EnumDeclaration* n) {
    emit("enum ");
    if (n->name) emit(n->name->name);
    printGenericParams(n->genericParams);
    emit(" {");
    newline(); push();
    for (size_t i = 0; i < n->variants.size(); ++i) {
        visit(n->variants[i].get());
        emit((i + 1 < n->variants.size()) ? "," : "");
        newline();
    }
    pop(); emit("}");
}

void SourcePrinter::visit(ast::EnumVariant* n) {
    if (n->name) emit(n->name->name);
    if (!n->associatedTypes.empty()) {
        emit("(");
        for (size_t i = 0; i < n->associatedTypes.size(); ++i) {
            if (i) emit(", ");
            if (n->associatedTypes[i]) n->associatedTypes[i]->accept(*this);
        }
        emit(")");
    } else if (n->hasValue) {
        emit(" = ");
        emit(std::to_string(n->value));
    }
}

void SourcePrinter::visit(ast::ImportDeclaration* n) {
    emit("import ");
    if (n->defaultImport) {
        emit(n->defaultImport->name);
        emit(" from ");
    } else if (n->namespaceImport) {
        emit("* as ");
        emit(n->namespaceImport->name);
        emit(" from ");
    } else if (!n->specifiers.empty()) {
        emit("{ ");
        for (size_t i = 0; i < n->specifiers.size(); ++i) {
            if (i) emit(", ");
            if (n->specifiers[i].importedName) emit(n->specifiers[i].importedName->name);
            if (n->specifiers[i].localName) { emit(" as "); emit(n->specifiers[i].localName->name); }
        }
        emit(" } from ");
    }
    if (n->source) emit(n->source->value);
    if (n->locator) { emit(" from \""); emit(n->locator->value); emit("\""); }
}

void SourcePrinter::visit(ast::TypeAliasDeclaration* n) {
    emit("type ");
    if (n->name) emit(n->name->name);
    emit(" = ");
    if (n->typeNode) n->typeNode->accept(*this);
}

void SourcePrinter::visit(ast::GenericParameter* n) {
    if (n->name) emit(n->name->name);
}

// --- statements ---
void SourcePrinter::visit(ast::BlockStatement* n) { printBlock(n); }

void SourcePrinter::visit(ast::EmptyStatement* n) { (void)n; }

void SourcePrinter::visit(ast::ExpressionStatement* n) {
    if (n->expression) n->expression->accept(*this);
}

void SourcePrinter::visit(ast::IfStatement* n) {
    emit("if (");
    if (n->test) n->test->accept(*this);
    emit(") ");
    if (n->consequent) n->consequent->accept(*this);
    if (n->alternate) {
        emit(" else ");
        n->alternate->accept(*this);
    }
}

void SourcePrinter::visit(ast::ForStatement* n) {
    if (!n->label.empty()) { emit(n->label); emit(": "); }
    emit("for (");
    if (n->init) n->init->accept(*this);
    emit("; ");
    if (n->test) n->test->accept(*this);
    emit("; ");
    if (n->update) n->update->accept(*this);
    emit(") ");
    if (n->body) n->body->accept(*this);
}

void SourcePrinter::visit(ast::WhileStatement* n) {
    if (!n->label.empty()) { emit(n->label); emit(": "); }
    emit("while (");
    if (n->test) n->test->accept(*this);
    emit(") ");
    if (n->body) n->body->accept(*this);
}

void SourcePrinter::visit(ast::ReturnStatement* n) {
    emit("return");
    if (n->argument) { emit(" "); n->argument->accept(*this); }
}

void SourcePrinter::visit(ast::PassStatement* n) {
    emit("pass(");
    if (n->argument) n->argument->accept(*this);
    emit(")");
}

void SourcePrinter::visit(ast::BreakStatement* n) {
    emit("break");
    if (!n->label.empty()) { emit(" "); emit(n->label); }
}

void SourcePrinter::visit(ast::ContinueStatement* n) {
    emit("continue");
    if (!n->label.empty()) { emit(" "); emit(n->label); }
}

void SourcePrinter::visit(ast::TryStatement* n) {
    emit("try ");
    if (n->tryBlock) n->tryBlock->accept(*this);
    if (n->catchBlock) {
        emit(" catch");
        if (n->catchIdent) { emit(" ("); emit(*n->catchIdent); emit(")"); }
        emit(" ");
        n->catchBlock->accept(*this);
    }
    if (n->finallyBlock) { emit(" finally "); n->finallyBlock->accept(*this); }
}

void SourcePrinter::visit(ast::FreedomStatement* n) {
    emit("freedom ");
    if (n->block) printBlock(n->block.get());
}

void SourcePrinter::visit(ast::ExternStatement* n) {
    emit("extern ");
    if (n->name) emit(n->name->name);
    printParamList(n->parameters);
    if (n->returnType) { emit("<"); n->returnType->accept(*this); emit(">"); }
}

void SourcePrinter::visit(ast::ThrowStatement* n) {
    emit("throw ");
    if (n->expr) n->expr->accept(*this);
}

void SourcePrinter::visit(ast::MatchStatement* n) {
    emit("match (");
    if (n->expr) n->expr->accept(*this);
    emit(") {");
    newline(); push();
    for (size_t i = 0; i < n->cases.size(); ++i) {
        if (n->cases[i].first) n->cases[i].first->accept(*this);
        else emit("?");
        // Optional guard: `pattern if cond -> value`.
        if (i < n->guards.size() && n->guards[i]) {
            emit(" if ");
            n->guards[i]->accept(*this);
        }
        emit(" -> ");
        if (n->cases[i].second) n->cases[i].second->accept(*this);
        if (i + 1 < n->cases.size()) emit(","); // no trailing comma on last arm
        newline();
    }
    pop(); emit("}");
}

void SourcePrinter::visit(ast::YieldStatement* n) {
    emit("yield");
    if (n->expression) { emit(" "); n->expression->accept(*this); }
}

void SourcePrinter::visit(ast::YieldReturnStatement* n) {
    emit("yield return");
    if (n->expression) { emit(" "); n->expression->accept(*this); }
}

void SourcePrinter::visit(ast::AssertStatement* n) {
    emit("assert(");
    if (n->condition) n->condition->accept(*this);
    if (n->message) { emit(", "); n->message->accept(*this); }
    emit(")");
}

void SourcePrinter::visit(ast::FailStatement* n) {
    emit("fail(");
    if (n->error) n->error->accept(*this);
    emit(")");
}

void SourcePrinter::visit(ast::RefailStatement* n) {
    emit("refail");
    if (n->wrappedError) { emit(" "); n->wrappedError->accept(*this); }
}

void SourcePrinter::visit(ast::PanicStatement* n) {
    emit("panic(");
    if (n->message) n->message->accept(*this);
    emit(")");
}

void SourcePrinter::visit(ast::ExitStatement* n) {
    emit("exit(");
    if (n->code) n->code->accept(*this);
    emit(")");
}

void SourcePrinter::visit(ast::DeferStatement* n) {
    emit("defer ");
    if (n->statement) n->statement->accept(*this);
}

void SourcePrinter::visit(ast::TrapClause* n) {
    emit("trap (");
    if (n->errorName) emit(n->errorName->name);
    if (n->isWildcard) emit("<?>");
    else if (n->isMultiType) {
        emit("<");
        for (size_t i = 0; i < n->errorTypes.size(); ++i) {
            if (i) emit(" | ");
            if (n->errorTypes[i]) n->errorTypes[i]->accept(*this);
        }
        emit(">");
    }
    else if (n->errorType) { emit("<"); n->errorType->accept(*this); emit(">"); }
    emit(") ");
    if (n->handler) n->handler->accept(*this);
}

void SourcePrinter::visit(ast::EnsureClause* n) {
    emit("ensure ");
    if (n->cleanupBlock) n->cleanupBlock->accept(*this);
}

void SourcePrinter::visit(ast::TupleDestructureAssignment* n) {
    emit("(");
    for (size_t i = 0; i < n->identifiers.size(); ++i) {
        if (i) emit(", ");
        if (n->identifiers[i]) emit(n->identifiers[i]->name);
    }
    emit(") = ");
    if (n->expression) n->expression->accept(*this);
}

// --- expressions ---
void SourcePrinter::visit(ast::Identifier* n) { emit(n->name); }
void SourcePrinter::visit(ast::IntegerLiteral* n) { emit(std::to_string(n->isUnsigned ? (long long)n->uvalue : (long long)n->value) + (n->isUnsigned ? "u" : "")); }
void SourcePrinter::visit(ast::FloatLiteral* n) {
    std::ostringstream ss;
    ss << std::setprecision(15) << n->value;
    std::string s = ss.str();
    // A bare integral representation ("100") would reparse as an Int literal,
    // changing meaning — force a float form ("100.0").
    if (s.find('.') == std::string::npos && s.find('e') == std::string::npos &&
        s.find('E') == std::string::npos)
        s += ".0";
    emit(s);
}
void SourcePrinter::visit(ast::StringLiteral* n) { emit("\""); emit(n->value); emit("\""); }
void SourcePrinter::visit(ast::BooleanLiteral* n) { emit(n->value ? "true" : "false"); }
void SourcePrinter::visit(ast::NilLiteral*) { emit("Nil"); }

void SourcePrinter::visit(ast::ArrayLiteral* n) {
    emit("[");
    for (size_t i = 0; i < n->elements.size(); ++i) {
        if (i) emit(", ");
        if (n->elements[i]) n->elements[i]->accept(*this);
    }
    emit("]");
}

void SourcePrinter::visit(ast::ObjectLiteral* n) {
    if (n->typePath) { n->typePath->accept(*this); emit(" "); }
    emit("{ ");
    for (size_t i = 0; i < n->properties.size(); ++i) {
        if (i) emit(", ");
        if (n->properties[i].key) emit(n->properties[i].key->name);
        if (n->properties[i].value) { emit(" = "); n->properties[i].value->accept(*this); }
    }
    emit(" }");
}

void SourcePrinter::visit(ast::UnaryExpression* n) {
    emit(n->op.lexeme);
    printExpr(n->operand.get(), 14);
}

void SourcePrinter::visit(ast::BinaryExpression* n) {
    int p = precOf(n);
    printExpr(n->left.get(), p);
    emit(" ");
    emit(n->op.lexeme);
    emit(" ");
    printExpr(n->right.get(), p + 1);
}

void SourcePrinter::visit(ast::LogicalExpression* n) {
    int p = precOf(n);
    printExpr(n->left.get(), p);
    emit(" ");
    emit(n->op.lexeme);
    emit(" ");
    printExpr(n->right.get(), p + 1);
}

void SourcePrinter::visit(ast::CallExpression* n) {
    if (!n->explicitTypeArgs.empty()) {
        // emit `callee<TypeArgs>(args)`
        n->callee->accept(*this);
        emit("<");
        for (size_t i = 0; i < n->explicitTypeArgs.size(); ++i) {
            if (i) emit(", ");
            if (n->explicitTypeArgs[i]) n->explicitTypeArgs[i]->accept(*this);
        }
        emit(">");
    } else {
        n->callee->accept(*this);
    }
    emit("(");
    for (size_t i = 0; i < n->arguments.size(); ++i) {
        if (i) emit(", ");
        if (n->arguments[i]) n->arguments[i]->accept(*this);
    }
    emit(")");
}

void SourcePrinter::visit(ast::MemberExpression* n) {
    printExpr(n->object.get(), 15);
    if (n->computed) { emit("["); if (n->property) n->property->accept(*this); emit("]"); }
    else { emit("."); if (n->property) n->property->accept(*this); }
}

void SourcePrinter::visit(ast::AssignmentExpression* n) {
    int p = precOf(n);
    printExpr(n->left.get(), p);
    emit(" ");
    emit(n->op.lexeme);
    emit(" ");
    printExpr(n->right.get(), p); // right-assoc
}

void SourcePrinter::visit(ast::BorrowExpression* n) {
    emit(n->kind == ast::BorrowKind::MUTABLE_BORROW ? "borrow(" : "view(");
    if (n->expression) n->expression->accept(*this);
    emit(")");
}

void SourcePrinter::visit(ast::PointerDerefExpression* n) {
    emit("at(");
    if (n->pointer) n->pointer->accept(*this);
    emit(")");
}

void SourcePrinter::visit(ast::AddrOfExpression* n) {
    emit("addr(");
    if (n->getLocation()) n->getLocation()->accept(*this);
    emit(")");
}

void SourcePrinter::visit(ast::FromIntToLocExpression* n) {
    emit("from<");
    if (n->getTargetType()) n->getTargetType()->accept(*this);
    emit(">(");
    if (n->getAddressExpression()) n->getAddressExpression()->accept(*this);
    emit(")");
}

void SourcePrinter::visit(ast::ArrayElementExpression* n) {
    printExpr(n->array.get(), 15);
    emit("[");
    if (n->index) n->index->accept(*this);
    emit("]");
}

void SourcePrinter::visit(ast::LocationExpression* n) {
    emit("loc(");
    if (n->expression) n->expression->accept(*this);
    emit(")");
}

void SourcePrinter::visit(ast::ListComprehension* n) {
    emit("[");
    if (n->elementExpr) n->elementExpr->accept(*this);
    emit(" for ");
    if (n->loopVariable) emit(n->loopVariable->name);
    emit(" in ");
    if (n->iterableExpr) n->iterableExpr->accept(*this);
    if (n->conditionExpr) { emit(" if "); n->conditionExpr->accept(*this); }
    emit("]");
}

void SourcePrinter::visit(ast::IfExpression* n) {
    emit("if (");
    if (n->condition) n->condition->accept(*this);
    emit(") ");
    if (n->thenBranch) n->thenBranch->accept(*this);
    emit(" else ");
    if (n->elseBranch) n->elseBranch->accept(*this);
}

void SourcePrinter::visit(ast::ConstructionExpression* n) {
    if (n->constructedType) n->constructedType->accept(*this);
    emit("(");
    for (size_t i = 0; i < n->arguments.size(); ++i) {
        if (i) emit(", ");
        if (n->arguments[i]) n->arguments[i]->accept(*this);
    }
    emit(")");
}

void SourcePrinter::visit(ast::ArrayInitializationExpression* n) {
    emit("[");
    if (n->elementType) n->elementType->accept(*this);
    emit("; ");
    if (n->sizeExpression) n->sizeExpression->accept(*this);
    emit("]()");
}

void SourcePrinter::visit(ast::GenericInstantiationExpression* n) {
    if (n->baseExpression) n->baseExpression->accept(*this);
    emit("<");
    for (size_t i = 0; i < n->genericArguments.size(); ++i) {
        if (i) emit(", ");
        if (n->genericArguments[i]) n->genericArguments[i]->accept(*this);
    }
    emit(">");
}

void SourcePrinter::visit(ast::ConditionalExpression* n) {
    printExpr(n->condition.get(), 2);
    emit(" ? ");
    printExpr(n->thenExpr.get(), 0);
    emit(" : ");
    printExpr(n->elseExpr.get(), 2);
}

void SourcePrinter::visit(ast::SequenceExpression* n) {
    for (size_t i = 0; i < n->expressions.size(); ++i) {
        if (i) emit(", ");
        if (n->expressions[i]) n->expressions[i]->accept(*this);
    }
}

void SourcePrinter::visit(ast::FunctionExpression* n) {
    if (n->isAsync) emit("async ");
    if (!n->params.empty()) {
        emit("|");
        for (size_t i = 0; i < n->params.size(); ++i) {
            if (i) emit(", ");
            if (n->params[i].name) emit(n->params[i].name->name);
            if (n->params[i].typeNode) { emit("<"); n->params[i].typeNode->accept(*this); emit(">"); }
        }
        emit("|");
    } else {
        emit("||");
    }
    emit(" -> ");
    if (n->body) n->body->accept(*this);
}

void SourcePrinter::visit(ast::ThisExpression*) { emit("this"); }
void SourcePrinter::visit(ast::SuperExpression*) { emit("super"); }

void SourcePrinter::visit(ast::AwaitExpression* n) {
    emit("await ");
    if (n->expr) n->expr->accept(*this);
}

void SourcePrinter::visit(ast::RangeExpression* n) {
    if (n->start) n->start->accept(*this);
    emit("..");
    if (n->end) n->end->accept(*this);
    if (n->step) { emit(".."); n->step->accept(*this); }
}

void SourcePrinter::visit(ast::BlockExpression* n) {
    if (n->block) printBlock(n->block.get());
}

void SourcePrinter::visit(ast::SelectExpression* n) {
    emit("select(");
    if (n->expr) n->expr->accept(*this);
    emit(") -> {");
    newline(); push();
    for (size_t i = 0; i < n->cases.size(); ++i) {
        if (n->cases[i].first) n->cases[i].first->accept(*this);
        else emit("?"); // wildcard/default arm
        emit(" -> ");
        if (n->cases[i].second) n->cases[i].second->accept(*this);
        if (i + 1 < n->cases.size()) emit(",");
        newline();
    }
    pop(); emit("}");
}

void SourcePrinter::visit(ast::TypeofExpression* n) {
    emit("typeof");
    if (n->typeArg) { emit("<"); n->typeArg->accept(*this); emit(">"); emit("()"); }
    else { emit("("); if (n->operand) n->operand->accept(*this); emit(")"); }
}

void SourcePrinter::visit(ast::TypenameExpression* n) {
    emit("typename(");
    if (n->operand) n->operand->accept(*this);
    emit(")");
}

void SourcePrinter::visit(ast::AsExpression* n) {
    if (n->operand) n->operand->accept(*this);
    emit(" as ");
    if (n->targetType) n->targetType->accept(*this);
}

void SourcePrinter::visit(ast::MatchExpression* n) {
    if (n->match) n->match->accept(*this);
}

void SourcePrinter::visit(ast::ComparisonPattern* n) {
    emit(n->op.lexeme);
    emit(" ");
    if (n->value) n->value->accept(*this);
}

void SourcePrinter::visit(ast::StructPattern* n) {
    if (n->typeName) n->typeName->accept(*this);
    emit(" { ");
    for (size_t i = 0; i < n->bindings.size(); ++i) {
        if (i) emit(", ");
        if (n->bindings[i]) emit(n->bindings[i]->name);
    }
    emit(" }");
}

void SourcePrinter::visit(ast::SetPattern* n) {
    emit("{ ");
    for (size_t i = 0; i < n->elements.size(); ++i) {
        if (i) emit(", ");
        if (n->elements[i]) n->elements[i]->accept(*this);
    }
    emit(" }");
}

// Type printer visit methods route into printType (kept for visitor dispatch).
void SourcePrinter::visit(ast::TypeName* n) { printType(n); }
void SourcePrinter::visit(ast::PointerType* n) { printType(n); }
void SourcePrinter::visit(ast::ArrayType* n) { printType(n); }
void SourcePrinter::visit(ast::VecType* n) { printType(n); }
void SourcePrinter::visit(ast::FutureType* n) { printType(n); }
void SourcePrinter::visit(ast::FunctionType* n) { printType(n); }
void SourcePrinter::visit(ast::OptionalType* n) { printType(n); }
void SourcePrinter::visit(ast::TupleTypeNode* n) { printType(n); }

// Uncommon declaration visitors: NodeType::escape for places the corpus rarely
// exercises; emit a best-effort form rather than nothing.
void SourcePrinter::visit(ast::ClassDeclaration* n) {
    emit("class ");
    if (n->name) emit(n->name->name);
    printGenericParams(n->genericParams);
    emit(" {");
    newline(); push();
    for (auto& m : n->members) { stmt(m.get()); }
    pop(); emit("}");
}

void SourcePrinter::visit(ast::BindDeclaration* n) {
    emit("bind ");
    if (n->selfType) n->selfType->accept(*this);
    emit(" -> {");
    newline(); push();
    for (auto& m : n->methods) { visit(m.get()); newline(); }
    pop(); emit("}");
}

void SourcePrinter::visit(ast::TemplateDeclaration* n) {
    emit("template ");
    if (n->name) emit(n->name->name);
    printGenericParams(n->genericParams);
    emit(" ");
    if (n->body) n->body->accept(*this);
}

void SourcePrinter::visit(ast::AspectDeclaration* n) {
    emit("aspect ");
    if (n->name) emit(n->name->name);
    printGenericParams(n->genericParams);
    if (!n->superTypes.empty()) {
        emit(" : ");
        for (size_t i = 0; i < n->superTypes.size(); ++i) {
            if (i) emit(" + ");
            if (n->superTypes[i]) emit(n->superTypes[i]->name);
        }
    }
    emit(" {");
    newline(); push();
    for (auto& m : n->methods) { visit(m.get()); newline(); }
    pop(); emit("}");
}

void SourcePrinter::visit(ast::NamespaceDeclaration* n) {
    emit("namespace ");
    if (n->name) emit(n->name->name);
    emit(" {");
    newline(); push();
    for (auto& m : n->members) { stmt(m.get()); }
    pop(); emit("}");
}

} // namespace fmt
} // namespace vyb
