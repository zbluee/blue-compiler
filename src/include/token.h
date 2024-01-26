#pragma once

#include <optional>

enum class TokenTypes
{
    int_literals,
    exit,
    semicolon,
    open_parenthesis,
    open_curly,
    close_parenthesis,
    close_curly,
    ident,
    let,
    eq,
    mul,
    div,
    plus,
    sub,
    _if,
    elif,
    _else
};

struct Token
{
    /* data */
    TokenTypes type;
    std::optional<std::string> value{};
};
