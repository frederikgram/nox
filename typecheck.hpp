
#pragma once

#include "lexer.hpp"
#include "parser.hpp"
#include "tokens.hpp"
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
using namespace std;

enum VARIABLE_TYPE
{
    V_INTEGER,
    V_CHARARACTER,
    V_STRING,
    V_ARRAY, // Unused
    V_FUNCTION,
    V_VOID, // Unused
    V_UNKNOWN,
    V_IDENTIFIER
};

struct VARIABLE
{
    std::string name;
    enum VARIABLE_TYPE type;
    void *value;
};

struct SCOPE
{
    map<std::string, struct VARIABLE *> variables;
    struct SCOPE *parent;
};

struct AST_NODE *typecheck(struct AST_NODE *root);
void check_block(struct AST_NODE *block);
void check_function_block(struct AST_NODE *function_node);