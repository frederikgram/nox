#pragma once
#include "lexer.hpp"
#include "parser.hpp"
#include "typecheck.hpp"

using namespace std;

enum AST_NODE_TYPE token_type_to_node_type(enum TOKEN_TYPE type);
char *ast_node_type_to_string(enum AST_NODE_TYPE type);
char *variable_type_to_string(struct VARIABLE_TYPE *type);
enum VARIABLE_TYPE_ENUM ast_type_to_variable_type_enum(enum AST_NODE_TYPE type);