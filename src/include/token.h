#pragma once

#include <optional>

enum class TokenTypes
{
    int_literals,
    exit,
    semicolon,
    open_parenthesis,
    close_parenthesis,
    ident,
    let,
    eq,
    mul,
    div,
    plus,
    sub,
};

struct Token
{
    /* data */
    TokenTypes type;
    std::optional<std::string> value{};
};
