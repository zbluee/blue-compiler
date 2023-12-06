#pragma once
#include <string>
#include <optional>

enum class TokenTypes
{
    int_literals,
    exit,
    semicolon
};

struct Token
{
    /* data */
    TokenTypes type;
    std::optional<std::string> value{};
};
