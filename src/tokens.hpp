/**/

#pragma once
using namespace std;
#include <string>

enum TOKEN_TYPE
{
    T_IF,
    T_EQ,
    T_OR,
    T_FOR,
    T_STR,
    T_INT,
    T_GEQ,
    T_LEQ,
    T_AND,
    T_AMP,
    T_MOD,
    T_ADD,
    T_SUB,
    T_DIV,
    T_DOT,
    T_NEQ,
    T_LESS,
    T_PIPE,
    T_BANG,
    T_STAR,
    T_ELSE,
    T_FUNC,
    T_CHAR,
    T_EMPTY,
    T_WHILE,
    T_GREAT,
    T_COMMA,
    T_COLON,
    T_INPUT,
    T_PRINT,
    T_ASSIGN,
    T_RBRACE,
    T_LBRACE,
    T_STRING,
    T_ELSEIF,
    T_RETURN,
    T_INCLUDE,
    T_INTEGER,
    T_LPARENS,
    T_RPARENS,
    T_LBRACKET,
    T_RBRACKET,
    T_SEMICOLON,
    T_CHARACTER,
    T_IDENTIFIER,
};

union Value {
    int intval;
    float floatval;
    char charval;
    char *strval;
};

struct TOKEN
{
    enum TOKEN_TYPE type;
    union Value value;
    int row;
    int col;
    struct TOKEN *next;
    struct TOKEN *prev;
    char *literal_value;
};
