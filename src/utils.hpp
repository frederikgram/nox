#pragma once
#include "lexer.hpp"
#include "parser.hpp"
#include "typecheck.hpp"
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
using namespace std;

char *ast_node_type_to_string(enum AST_NODE_TYPE type);
char *variable_type_to_string(struct VARIABLE_TYPE *type);
enum VARIABLE_TYPE_ENUM ast_type_to_variable_type_enum(enum AST_NODE_TYPE type);
enum AST_NODE_TYPE token_type_to_node_type(enum TOKEN_TYPE type);
char *ast_node_type_to_string(enum AST_NODE_TYPE type);