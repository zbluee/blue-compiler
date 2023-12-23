#pragma once
#include <vector>
#include <variant>
#include "./token.h"

namespace node
{
    struct ExprIntLit
    {
        Token int_literal;
    };
    struct ExprIdent
    {
        Token ident;
    };
    struct Expr
    {
        std::variant<node::ExprIntLit, node::ExprIdent> variant;
    };

    struct StatementExit
    {
        Expr expr;
    };

    struct StatementLet
    {
        Token ident;
        Expr expr;
    };

    struct Statement
    {
        std::variant<node::StatementExit, node::StatementLet> variant;
    };
    struct Prog
    {
        std::vector<node::Statement> statements;
    };
}

class Parser
{
private:
    std::vector<Token> m_Tokens;
    mutable size_t m_Count;

    inline std::optional<Token> lookAhead(const int ahead = 0) const
    {
        if (m_Count + ahead >= m_Tokens.size())
            return {};
        return m_Tokens[m_Count + ahead];
    }

    inline Token getNextToken() const
    {
        return m_Tokens[m_Count++];
    }

public:
    inline Parser(const std::vector<Token> &tokens) : m_Tokens(tokens), m_Count(0) {}

    std::optional<node::Expr> parseExpr()
    {
        if (lookAhead().has_value() && lookAhead().value().type == TokenTypes::int_literals)
            return node::Expr{.variant = node::ExprIntLit{.int_literal = getNextToken()}};
        else if (lookAhead().has_value() && lookAhead().value().type == TokenTypes::ident)
            return node::Expr{.variant = node::ExprIdent{.ident = getNextToken()}};
        return {};
    }

    std::optional<node::Statement> parseStatement()
    {
        if (lookAhead().value().type == TokenTypes::exit && lookAhead(1).has_value() && lookAhead(1).value().type == TokenTypes::open_parenthesis)
        {
            getNextToken();
            getNextToken();

            node::StatementExit exit_statement;
            if (auto expr_node = parseExpr())
            {
                exit_statement = {.expr = expr_node.value()};
            }
            else
            {
                std::cerr << "Error : Invalid Expression" << std::endl;
                exit(EXIT_FAILURE);
            }

            if (lookAhead().has_value() && lookAhead().value().type == TokenTypes::close_parenthesis)
            {
                getNextToken();
            }
            else
            {
                std::cerr << "Error : Expected `)`" << std::endl;
                exit(EXIT_FAILURE);
            }
            if (lookAhead().has_value() && lookAhead().value().type == TokenTypes::semicolon)
            {
                getNextToken();
            }
            else
            {
                std::cerr << "Error : Expected `;`" << std::endl;
                exit(EXIT_FAILURE);
            }
            return node::Statement{.variant = exit_statement};
        }
        else if (lookAhead().has_value() && lookAhead().value().type == TokenTypes::let &&
                 lookAhead(1).has_value() && lookAhead(1).value().type == TokenTypes::ident &&
                 lookAhead(2).has_value() && lookAhead(2).value().type == TokenTypes::eq)
        {
            // let statement, consume it
            getNextToken();
            auto statementLet = node::StatementLet{.ident = getNextToken()};
            // equal sign, consume it
            getNextToken();
            // and then we have z expr
            if (auto expr = parseExpr())
            {
                statementLet.expr = expr.value();
            }
            else
            {
                std::cerr << "Error : Invalid Expression" << std::endl;
                exit(EXIT_FAILURE);
            }
            if (lookAhead().has_value() && lookAhead().value().type == TokenTypes::semicolon)
                getNextToken();
            else
            {
                std::cerr << "Error : Exprected `;`" << std::endl;
                exit(EXIT_FAILURE);
            }
            return node::Statement{.variant = statementLet};
        }
        return {};
    }
    std::optional<node::Prog> parseProg()
    {
        node::Prog prog;

        while (lookAhead().has_value())
        {
            if (auto statement = parseStatement())
            {
                prog.statements.push_back(statement.value());
            }
            else
            {
                std::cerr << "Error : Invalid Statement" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        return prog;
    }
};
