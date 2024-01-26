#pragma once

inline std::optional<int> exprsPrecedence(const TokenTypes &type)
{
    switch (type)
    {
    case TokenTypes::plus:
    case TokenTypes::sub:
        return 0;
    case TokenTypes::mul:
    case TokenTypes::div:
        return 1;
    default:
        return {};
    }
}

class Scanner
{
private:
    const std::string m_Content;
    size_t m_Count;

    inline std::optional<char> lookAhead(const size_t ahead = 0) const
    {
        if (m_Count + ahead >= m_Content.length())
            return {};
        return m_Content[m_Count + ahead];
    }

    inline char getNextChar()
    {
        return m_Content[m_Count++];
    }

public:
    inline explicit Scanner(const std::string &content) : m_Content(content), m_Count(0) {}

    inline std::vector<Token> tokenize()
    {
        std::vector<Token> tokens;
        std::string buf;

        while (lookAhead().has_value())
        {
            if (std::isalpha(lookAhead().value()))
            {
                buf.push_back(getNextChar());

                while (lookAhead().has_value() && std::isalnum(lookAhead().value()))
                {
                    buf.push_back(getNextChar());
                }

                if (buf == "exit")
                {
                    tokens.push_back({.type = TokenTypes::exit});
                    buf.clear();
                }
                else if (buf == "let")
                {
                    tokens.push_back({.type = TokenTypes::let});
                    buf.clear();
                }
                else if (buf == "if")
                {
                    tokens.push_back({.type = TokenTypes::_if});
                    buf.clear();
                }
                else if (buf == "elif")
                {
                    tokens.push_back({.type = TokenTypes::elif});
                    buf.clear();
                }
                else if (buf == "else")
                {
                    tokens.push_back({.type = TokenTypes::_else});
                    buf.clear();
                }
                else
                {
                    tokens.push_back({.type = TokenTypes::ident, .value = buf});
                    buf.clear();
                }
            }
            else if (std::isdigit(lookAhead().value()))
            {
                buf.push_back(getNextChar());
                while (lookAhead().has_value() && std::isdigit(lookAhead().value()))
                {
                    buf.push_back(getNextChar());
                }

                tokens.push_back({.type = TokenTypes::int_literals, .value = buf});
                buf.clear();
            }
            else if (lookAhead().value() == '-' && lookAhead(1).has_value() && lookAhead(1).value() == '-')
            {
                getNextChar();
                getNextChar();
                while (lookAhead().has_value() && lookAhead().value() != '\n')
                    getNextChar();
            }
            else if (lookAhead().value() == '-' && lookAhead(1).has_value() && lookAhead(1).value() == '#')
            {
                getNextChar();
                getNextChar();
                while (lookAhead().has_value())
                {
                    if (lookAhead().value() == '#' && lookAhead(1).has_value() && lookAhead(1).value() == '-')
                        break;
                    getNextChar();
                }

                if (lookAhead().has_value())
                    getNextChar();

                if (lookAhead().has_value())
                    getNextChar();
            }
            else if (lookAhead().value() == '(')
            {
                getNextChar();
                tokens.push_back({.type = TokenTypes::open_parenthesis});
            }
            else if (lookAhead().value() == ')')
            {
                getNextChar();
                tokens.push_back({.type = TokenTypes::close_parenthesis});
            }
            else if (lookAhead().value() == ';')
            {
                getNextChar();
                tokens.push_back({.type = TokenTypes::semicolon});
            }
            else if (lookAhead().value() == '=')
            {
                getNextChar();
                tokens.push_back({.type = TokenTypes::eq});
            }
            else if (lookAhead().value() == '+')
            {
                getNextChar();
                tokens.push_back({.type = TokenTypes::plus});
            }
            else if (lookAhead().value() == '*')
            {
                getNextChar();
                tokens.push_back({.type = TokenTypes::mul});
            }
            else if (lookAhead().value() == '-')
            {
                getNextChar();
                tokens.push_back({.type = TokenTypes::sub});
            }
            else if (lookAhead().value() == '/')
            {
                getNextChar();
                tokens.push_back({.type = TokenTypes::div});
            }
            else if (lookAhead().value() == '{')
            {
                getNextChar();
                tokens.push_back({.type = TokenTypes::open_curly});
            }
            else if (lookAhead().value() == '}')
            {
                getNextChar();
                tokens.push_back({.type = TokenTypes::close_curly});
            }
            else if (std::isspace(lookAhead().value()))
            {
                getNextChar();
            }
            else
            {
                std::cerr << "Error: Invalid syntax." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        m_Count = 0;
        return tokens;
    }
};