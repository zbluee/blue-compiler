#pragma once

#include "./token.h"
#include <vector>
#include <variant>

namespace node
{
    struct TermIntLit
    {
        Token int_literal;
    };

    struct TermIdent
    {
        Token ident;
    };

    struct Expr;

    struct TermParenthesis
    {
        Expr *expr;
    };

    struct ExprsAdd
    {
        Expr *lhs, *rhs;
    };

    struct ExprsMul
    {
        Expr *lhs, *rhs;
    };

    struct ExprsSub
    {
        Expr *lhs, *rhs;
    };

    struct ExprsDiv
    {
        Expr *lhs, *rhs;
    };

    struct Exprs
    {
        std::variant<node::ExprsAdd *, node::ExprsSub *, node::ExprsMul *, node::ExprsDiv *> variant;
    };

    struct Term
    {
        std::variant<node::TermIntLit *, node::TermIdent *, node::TermParenthesis *> variant;
    };

    struct Expr
    {
        std::variant<node::Term *, node::Exprs *> variant;
    };
    struct StatementExit
    {
        Expr *expr;
    };

    struct StatementLet
    {
        Token ident;
        Expr *expr{};
    };

    struct Statement;

    struct Scope
    {
        std::vector<node::Statement *> statements;
    };

    struct ConditionalBranch;

    struct ConditionalBranchElif
    {
        Expr *expr{};
        Scope *scope{};
        std::optional<node::ConditionalBranch *> conditionalBr;
    };

    struct ConditionalBranchElse
    {
        Scope *scope;
    };

    struct ConditionalBranch
    {
        std::variant<node::ConditionalBranchElif *, node::ConditionalBranchElse *> variant;
    };

    struct StatementIf
    {
        Expr *expr{};
        Scope *scope{};
        std::optional<node::ConditionalBranch *> conditionalBr;
    };

    struct StatementAssignment
    {
        Token ident;
        Expr *expr{};
    };

    struct Statement
    {
        std::variant<node::StatementExit *, node::StatementLet *, node::Scope *, node::StatementIf *, node::StatementAssignment*> variant;
    };
    struct Prog
    {
        std::vector<node::Statement *> statements;
    };
}