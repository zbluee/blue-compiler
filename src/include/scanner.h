#pragma once
#include <iostream>
#include <sstream>


class Scanner
{
private:
    std::string m_Content;
    size_t m_Count;

    inline std::optional<char> lookAhead(const int ahead = 1) const
    {
        if (m_Count + ahead > m_Content.length())
            return {};
        return m_Content[m_Count];
    }

    inline char getNextChar()
    {
        return m_Content[m_Count++];
    }

public:
    inline Scanner(const std::string &content) : m_Content(content), m_Count(0) {}

    std::vector<Token> tokenize()
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
                    continue;
                }
                else
                {
                    std::cerr << "Error : Invalid syntax" << std::endl;
                    exit(EXIT_FAILURE);
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
                continue;
            }
            else if (lookAhead().value() == ';')
            {
                getNextChar();
                tokens.push_back({.type = TokenTypes::semicolon});
                continue;
            }
            else if (std::isspace(lookAhead().value()))
            {
                getNextChar();
                continue;
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