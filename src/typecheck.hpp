
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

enum VARIABLE_TYPE_ENUM
{
    V_INTEGER,
    V_STRING,
    V_CHARACTER,
    V_UNKNOWN,
    V_IDENTIFIER,
    V_FUNCTION,
    V_ADDRESS,
    V_POINTER,
    V_ARRAY
};

struct VARIABLE_TYPE
{
    VARIABLE_TYPE_ENUM type;
    int array_size = 0;
    struct VARIABLE_TYPE *return_type;
    struct VARIABLE_TYPE *array_type;
    struct VARIABLE_TYPE *pointer_to;
};

struct VARIABLE
{
    void *value;
    struct VARIABLE_TYPE *type;
    std::string name;
    std::vector<VARIABLE *> parameters;
};

struct SCOPE
{
    map<std::string, struct VARIABLE *> variables;
    struct SCOPE *parent;
    int id;
};

struct AST_NODE *typecheck(struct AST_NODE *root);
void check_block(struct AST_NODE *block, std::vector<struct VARIABLE *> params);