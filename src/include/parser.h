#pragma once
#include <vector>
#include <variant>
#include "./arenaAllocator.h"
#include "./token.h"
#include "./scanner.h"

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

    struct Statement
    {
        std::variant<node::StatementExit *, node::StatementLet *, node::Scope *, node::StatementIf *> variant;
    };
    struct Prog
    {
        std::vector<node::Statement *> statements;
    };
}

class Parser
{
private:
    const std::vector<Token> m_Tokens;
    mutable size_t m_Count;
    ArenaAllocator m_ArenaAllocator;

    inline std::optional<Token> lookAhead(const size_t ahead = 0) const
    {
        if (m_Count + ahead >= m_Tokens.size())
            return {};
        return m_Tokens.at(m_Count + ahead);
    }

    inline Token getNextToken() const
    {
        return m_Tokens.at(m_Count++);
    }

    inline Token trytoGetNextToken(const TokenTypes type, const std::string &errMsg) const
    {
        if (lookAhead().has_value() && lookAhead().value().type == type)
            return getNextToken();
        else
        {
            std::cerr << errMsg << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    inline std::optional<Token> trytoGetNextToken(const TokenTypes type) const
    {
        if (lookAhead().has_value() && lookAhead().value().type == type)
            return getNextToken();

        return {};
    }

public:
    inline Parser(const std::vector<Token> &tokens) : m_Tokens(tokens), m_Count(0), m_ArenaAllocator(1024 * 1024 * 4) {}

    std::optional<node::Term *> parseTerm()
    {
        if (auto intLit = trytoGetNextToken(TokenTypes::int_literals))
        {
            auto termIntLit = m_ArenaAllocator.allocate<node::TermIntLit>();
            termIntLit->int_literal = intLit.value();
            auto term = m_ArenaAllocator.allocate<node::Term>();
            term->variant = termIntLit;
            return term;
        }
        if (auto ident = trytoGetNextToken(TokenTypes::ident))
        {
            auto exprIdent = m_ArenaAllocator.allocate<node::TermIdent>();
            exprIdent->ident = ident.value();
            auto term = m_ArenaAllocator.allocate<node::Term>();
            term->variant = exprIdent;
            return term;
        }
        if (auto openParenthesis = trytoGetNextToken(TokenTypes::open_parenthesis))
        {
            auto expr = parseExpr();
            if (!expr.has_value())
            {
                std::cerr << "Error: Expected Expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            trytoGetNextToken(TokenTypes::close_parenthesis, "Expected `)`");
            auto termParenthesis = m_ArenaAllocator.allocate<node::TermParenthesis>();
            termParenthesis->expr = expr.value();
            auto term = m_ArenaAllocator.allocate<node::Term>();
            term->variant = termParenthesis;
            return term;
        }
        return {};
    }

    std::optional<node::Expr *> parseExpr(const int minPrecedence = 0)
    {
        std::optional<node::Term *> termLhs = parseTerm();
        if (!termLhs.has_value())
            return {};

        auto exprLhs = m_ArenaAllocator.allocate<node::Expr>();
        exprLhs->variant = termLhs.value();

        while (true)
        {
            std::optional<Token> currToken = lookAhead();

            if (!currToken.has_value())
                break;

            std::optional<int> precedence = exprsPrecedence(currToken->type);

            if (!precedence.has_value() || precedence < minPrecedence)
                break;

            const Token opr = getNextToken();
            const int nextMinPrecedence = precedence.value() + 1;
            auto exprRhs = parseExpr(nextMinPrecedence);

            if (!exprRhs.has_value())
            {

                std::cerr << "Error : Unable to parse expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            auto exprs = m_ArenaAllocator.allocate<node::Exprs>();
            auto exprLhs2 = m_ArenaAllocator.allocate<node::Expr>();

            if (opr.type == TokenTypes::plus)
            {
                auto add = m_ArenaAllocator.allocate<node::ExprsAdd>();
                exprLhs2->variant = exprLhs->variant;
                add->lhs = exprLhs2;
                add->rhs = exprRhs.value();
                exprs->variant = add;
            }
            else if (opr.type == TokenTypes::mul)
            {
                auto mul = m_ArenaAllocator.allocate<node::ExprsMul>();
                exprLhs2->variant = exprLhs->variant;
                mul->lhs = exprLhs2;
                mul->rhs = exprRhs.value();
                exprs->variant = mul;
            }
            else if (opr.type == TokenTypes::sub)
            {
                auto sub = m_ArenaAllocator.allocate<node::ExprsSub>();
                exprLhs2->variant = exprLhs->variant;
                sub->lhs = exprLhs2;
                sub->rhs = exprRhs.value();
                exprs->variant = sub;
            }
            else if (opr.type == TokenTypes::div)
            {
                auto div = m_ArenaAllocator.allocate<node::ExprsDiv>();
                exprLhs2->variant = exprLhs->variant;
                div->lhs = exprLhs2;
                div->rhs = exprRhs.value();
                exprs->variant = div;
            }
            else
            {
                std::cout << "Error : Unkown operation unable to parse" << std::endl;
                exit(EXIT_FAILURE);
            }

            exprLhs->variant = exprs;
        }
        return exprLhs;
    }

    std::optional<node::ConditionalBranch *> parseConditionalBr()
    {
        if (trytoGetNextToken(TokenTypes::elif))
        {
            trytoGetNextToken(TokenTypes::open_parenthesis, "Error : Expected `(`");
            const auto conditionalBrElif = m_ArenaAllocator.allocate<node::ConditionalBranchElif>();
            if (const auto expr = parseExpr())
            {
                conditionalBrElif->expr = expr.value();
            }
            else
            {
                std::cerr << "Error : Expected Expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            trytoGetNextToken(TokenTypes::close_parenthesis, "Error : Expected `)`");
            if (const auto scope = parseScope())
            {
                conditionalBrElif->scope = scope.value();
            }
            else
            {
                std::cerr << "Error : Expected Scope" << std::endl;
                exit(EXIT_FAILURE);
            }
            conditionalBrElif->conditionalBr = parseConditionalBr();
            auto conditionalBr = m_ArenaAllocator.allocate<node::ConditionalBranch>();
            conditionalBr->variant = conditionalBrElif;
            return conditionalBr;
        }
        if (trytoGetNextToken(TokenTypes::_else))
        {
            auto conditionalBrElse = m_ArenaAllocator.allocate<node::ConditionalBranchElse>();
            if (const auto scope = parseScope())
            {
                conditionalBrElse->scope = scope.value();
            }
            else
            {
                std::cerr << "Error : Expected Scope" << std::endl;
                exit(EXIT_FAILURE);
            }
            auto conditionalBr = m_ArenaAllocator.allocate<node::ConditionalBranch>();
            conditionalBr->variant = conditionalBrElse;
            return conditionalBr;
        }
        return {};
    }

    std::optional<node::Scope *> parseScope()
    {
        if (!trytoGetNextToken(TokenTypes::open_curly).has_value())
            return {};
        auto scope = m_ArenaAllocator.allocate<node::Scope>();
        while (auto statement = parseStatement())
            scope->statements.push_back(statement.value());

        trytoGetNextToken(TokenTypes::close_curly, "Error : Expected `}`");
        return scope;
    }

    std::optional<node::Statement *> parseStatement()
    {
        if (lookAhead().value().type == TokenTypes::exit && lookAhead(1).has_value() && lookAhead(1).value().type == TokenTypes::open_parenthesis)
        {
            getNextToken();
            getNextToken();

            auto exit_statement = m_ArenaAllocator.allocate<node::StatementExit>();

            if (const auto expr_node = parseExpr())
            {
                exit_statement->expr = expr_node.value();
            }
            else
            {
                std::cerr << "Error : Invalid Expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            trytoGetNextToken(TokenTypes::close_parenthesis, "Error : Expected `)`");
            trytoGetNextToken(TokenTypes::semicolon, "Error : Exprected `;`");
            auto statement = m_ArenaAllocator.allocate<node::Statement>();
            statement->variant = exit_statement;
            return statement;
        }
        if (lookAhead().has_value() && lookAhead().value().type == TokenTypes::let &&
            lookAhead(1).has_value() && lookAhead(1).value().type == TokenTypes::ident &&
            lookAhead(2).has_value() && lookAhead(2).value().type == TokenTypes::eq)
        {
            // let statement, consume it
            getNextToken();
            auto statementLet = m_ArenaAllocator.allocate<node::StatementLet>();
            statementLet->ident = getNextToken();
            // equal sign, consume it
            getNextToken();
            // and then we have z expr
            if (const auto expr = parseExpr())
            {
                statementLet->expr = expr.value();
            }
            else
            {
                std::cerr << "Error : Invalid Expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            trytoGetNextToken(TokenTypes::semicolon, "Error : Expected `;`");
            auto statement = m_ArenaAllocator.allocate<node::Statement>();
            statement->variant = statementLet;
            return statement;
        }
        if (lookAhead().has_value() && lookAhead().value().type == TokenTypes::open_curly)
        {
            if (auto scope = parseScope())
            {
                auto statement = m_ArenaAllocator.allocate<node::Statement>();
                statement->variant = scope.value();
                return statement;
            }
            std::cerr << "Error : Invalid Scope" << std::endl;
            exit(EXIT_FAILURE);
        }
        if (auto _if = trytoGetNextToken(TokenTypes::_if))
        {
            trytoGetNextToken(TokenTypes::open_parenthesis, "Error : Expected '('");
            auto statementIf = m_ArenaAllocator.allocate<node::StatementIf>();
            if (const auto expr = parseExpr())
            {
                statementIf->expr = expr.value();
            }
            else
            {
                std::cerr << "Error : Invalid Expression" << std::endl;
                exit(EXIT_FAILURE);
            }

            trytoGetNextToken(TokenTypes::close_parenthesis, "Error : Expected ')'");
            if (const auto scope = parseScope())
            {
                statementIf->scope = scope.value();
            }
            else
            {
                std::cerr << "Error : Invalid Scope" << std::endl;
                exit(EXIT_FAILURE);
            }
            statementIf->conditionalBr = parseConditionalBr();
            auto statement = m_ArenaAllocator.allocate<node::Statement>();
            statement->variant = statementIf;
            return statement;
        }
        return {};
    }

    std::optional<node::Prog> parseProg()
    {
        node::Prog prog;
        while (lookAhead().has_value())
        {
            if (auto statement = parseStatement())
                prog.statements.push_back(statement.value());

            else
            {
                std::cerr << "Error : Invalid Statement" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        return prog;
    }
};
