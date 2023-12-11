#pragma once
#include <vector>
#include "./token.h"

namespace node
{
    struct Expr
    {
        Token int_literal;
    };
    struct Exit
    {
        Expr expr;
    };
}

class Parser
{
private:
    std::vector<Token> m_Tokens;
    mutable size_t m_Count;

    inline std::optional<Token> lookAhead(const int ahead = 1) const
    {
        if (m_Count + ahead > m_Tokens.size())
            return {};
        return m_Tokens[m_Count];
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
            return node::Expr{.int_literal = getNextToken()};
        return {};
    }
    std::optional<node::Exit> parse()
    {
        std::optional<node::Exit> exit_node;

        while (lookAhead().has_value())
        {
            if (lookAhead().value().type == TokenTypes::exit)
            {
                getNextToken();
                // if expr has value or not null
                if (auto expr_node = parseExpr())
                    exit_node = node::Exit{.expr = expr_node.value()};

                else
                {
                    std::cerr << "Error : Invalid Expression" << std::endl;
                    exit(EXIT_FAILURE);
                }

                if (lookAhead().has_value() && lookAhead().value().type == TokenTypes::semicolon)
                    getNextToken();

                else
                {
                    std::cerr << "Error : Invalid Expression" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
        }
        m_Count = 0;
        return exit_node;
    }
};
