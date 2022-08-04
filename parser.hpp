/**/
#pragma once
#include <string>
#include <vector>

using namespace std;

enum AST_NODE_TYPE {
    A_EMPTY,
    A_BLOCK,
    A_IF,
    A_FOR,
    A_WHILE,
    A_INT,
    A_STR,
    A_INTEGER,
    A_STRING,
    A_ADD,
    A_DEREF,
    A_MUL,
    A_DIV,
    A_SUB,
    A_IDENTIFIER,
    A_LESS,
    A_LEQ,
    A_GEQ,
    A_EQ,
    A_NEQ,
    A_OR,
    A_GREAT,
    A_AND,
    A_ASSIGN,
    A_ADDRESS,
    A_NOT,
    A_MEMBER,
    A_LPARENS,
    A_RPARENS,
    A_UNARYMINUS,
    A_LOR,
    A_LAND,
    A_AEQ,
    A_SEQ,
    A_RETURN,
    A_FUNC_DEF,
    A_USE,
    A_OUTPUT,
    A_INPUT,
};


struct AST_NODE {
    enum AST_NODE_TYPE type;
    struct TOKEN * token;
    struct AST_NODE * parent;

    struct AST_NODE * lhs;
    struct AST_NODE * rhs;
    struct AST_NODE * name;
    struct AST_NODE * body;
    struct AST_NODE * els;
    struct AST_NODE * conditional;
    struct AST_NODE * block;
    std::vector<struct AST_NODE*> statements;
    struct AST_NODE * asg;
    struct AST_NODE * vartype;
    std::vector<struct AST_NODE*> args;

    struct SYMBOL_TABLE * table;

};

struct PARSER_STATUS {
    struct TOKEN * head;
    struct TOKEN * current;
    struct AST_NODE * root;
    int vpos; // 'Virtual' position of current token in the linked list of tokens
    std::string input;
    int input_size;
};



struct AST_NODE * parse(char * input, int input_size, struct TOKEN * head);
struct AST_NODE * make_node(struct PARSER_STATUS * status, enum AST_NODE_TYPE type, struct AST_NODE * parent);
struct AST_NODE * parse_inner(struct PARSER_STATUS * status, struct AST_NODE * parent);
struct AST_NODE * parse_expression(struct PARSER_STATUS * status, struct AST_NODE * PARENT);
struct AST_NODE * parse_statement(struct PARSER_STATUS * status, struct AST_NODE * parent);
