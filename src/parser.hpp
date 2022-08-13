/**/
#pragma once
#include <map>
#include <string>
#include <vector>
using namespace std;

enum AST_NODE_TYPE
{
    A_EQ,
    A_IF,
    A_OR,
    A_ADD,
    A_AND,
    A_DIV,
    A_FOR,
    A_GEQ,
    A_INT,
    A_LEQ,
    A_LOR,
    A_MUL,
    A_NEQ,
    A_NOT,
    A_STR,A_VOID,
    A_MOD,
    A_SUB,
    A_USE,
    A_CHAR,
    A_LAND,
    A_LESS,
    A_ARRAY,
    A_BLOCK,
    A_DEREF,
    A_EMPTY,
    A_GREAT,
    A_INPUT,
    A_PRINT,
    A_WHILE,
    A_ARR,
    A_ASSIGN,
    A_MEMBER,
    A_RETURN,
    A_STRING,
    A_INTEGER,
    A_LPARENS,
    A_RPARENS,
    A_FUNC_DEF,
    A_CHARACTER,
    A_FUNC_CALL,
    A_IDENTIFIER,
    A_UNARYMINUS,
};

struct AST_NODE
{
    struct TOKEN *token;
    struct AST_NODE *parent;
    enum AST_NODE_TYPE type;

    struct AST_NODE *lhs;
    struct AST_NODE *rhs;
    struct AST_NODE *asg;
    struct AST_NODE *els;
    struct AST_NODE *name;
    struct AST_NODE *body;
    struct AST_NODE *block;
    struct AST_NODE *vartype;
    struct AST_NODE *conditional;
    std::vector<struct AST_NODE *> args;
    std::vector<struct AST_NODE *> statements;


    struct SCOPE *scope;
};

struct PARSER_STATUS
{
    struct TOKEN *head;
    struct TOKEN *current;
    struct AST_NODE *root;
    int vpos; // 'Virtual' position of current token in the linked list of tokens
    int input_size;
    std::string input;
};

struct AST_NODE *parse(char *input, int input_size, struct TOKEN *head);
struct AST_NODE *make_node(struct PARSER_STATUS *status, enum AST_NODE_TYPE type, struct AST_NODE *parent);
struct AST_NODE *parse_inner(struct PARSER_STATUS *status, struct AST_NODE *parent);
struct AST_NODE *parse_expression(struct PARSER_STATUS *status, struct AST_NODE *PARENT);
struct AST_NODE *parse_statement(struct PARSER_STATUS *status, struct AST_NODE *parent);
