
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
    V_VOID,
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



void push_scope();

struct SCOPE *pop_scope();

// Certain AST_NODEs such as the literal token 'int' or 'str' needs to be converted from a NODE type to a VARIABLE type 'integer' or 'string'
struct VARIABLE_TYPE *get_type_from_subtree(struct AST_NODE *node);

int are_arithmetiically_compatible(enum AST_NODE_TYPE type, struct VARIABLE_TYPE *type1, struct VARIABLE_TYPE *type2);

int are_relationally_comparable(struct VARIABLE_TYPE *type1, struct VARIABLE_TYPE *type2);

int are_equivalent_types(struct VARIABLE_TYPE *a, struct VARIABLE_TYPE *b);

int is_literal(struct AST_NODE *node);
// This function is used to check if a variable is declared in the current scope
struct VARIABLE * find_variable(std::string name);

struct VARIABLE_TYPE *check_expression(struct AST_NODE *expr);
void check_statement(struct AST_NODE *statement);

// Checks a block of statements, creating a new scope if necessary
void check_block(struct AST_NODE *block, std::vector<struct VARIABLE *> params);

struct AST_NODE *typecheck(struct AST_NODE *root);