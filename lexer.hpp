#pragma once
#include "tokens.hpp"

using namespace std;

struct LEXER_STATUS
{
    struct TOKEN *head;
    struct TOKEN *tail;
    int current_row;
    int current_col;
};

struct TOKEN *lex(char *input, int size);