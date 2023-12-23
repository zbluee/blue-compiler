#pragma once
#include <string>
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
    eq
};

struct Token
{
    /* data */
    TokenTypes type;
    std::optional<std::string> value{};
};
