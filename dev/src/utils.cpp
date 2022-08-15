
#include "utils.hpp"

using namespace std;

// @TODO : this is temporary, it sucks ! Print both the row and its two
// surrounding rows in both directions, with a row containing the given error
// message pointing at the proper column
void print_error(int col, int row, const char *message, ...)
{
    int row_number = 1;
    int shown = 0;
    int i = 0;
    va_list lst;
    va_start(lst, message);
    fprintf(stderr, message, lst);
    va_end(lst);
    fprintf(stderr, " at line '%d', column '%d'\n", row + 1, col - 2);
}
// Calls print_error and exits with flag -1
void print_error_exit(int col, int row, const char *message, ...)
{
    va_list lst;
    va_start(lst, message);
    print_error(col, row, message, lst);
    va_end(lst);
    exit(-1);
}




// Converts _some_ enum TOKEN_TYPE's to enum AST_NODE_TYPE's
enum AST_NODE_TYPE token_type_to_node_type(enum TOKEN_TYPE type)
{
    switch (type)
    {
    case T_ASSIGN:
        return A_ASSIGN;
    case T_LESS:
        return A_LESS;
    case T_ADDRESSOF:
        return A_ADDRESSOF;
    case T_DEREF:
        return A_DEREF;
    case T_VOID:
        return A_VOID;
    case T_GREAT:
        return A_GREAT;
    case T_GEQ:
        return A_GEQ;
    case T_MOD:
        return A_MOD;   
    case T_LEQ:
        return A_LEQ;
    case T_DIV:
        return A_DIV;
    case T_ADD:
        return A_ADD;
    case T_EQ:
        return A_EQ;
    case T_STAR:
        return A_MUL; //@TODO : Fix this when adding deref
    case T_NEQ:
        return A_NEQ;
    case T_INT:
        return A_INT;
    case T_STR:
        return A_STR;
    case T_CHAR:
        return A_CHAR;
    case T_INTEGER:
        return A_INTEGER;
    case T_STRING: 
        return A_STRING;
    default:
        fprintf(stderr, "token_type_to_node_type: Missing Case\n");
        exit(-1);
    }
}

char *ast_node_type_to_string(enum AST_NODE_TYPE type)
{
    switch (type)
    {
    case A_ADD:
        return "A_ADD";
    case A_VOID:
        return "A_VOID";
    case A_ARR:
        return "A_ARR";
    case A_DEREF:
        return "A_DEREF";
    case A_ADDRESSOF:
        return "A_ADDRESSOF";
    case A_ARRAY:
        return "A_ARRAY";
    case A_SUB:
        return "A_SUB";
    case A_MUL:
        return "A_MUL";
    case A_DIV:
        return "A_DIV";
    case A_INTEGER:
        return "A_INTEGER";
    case A_STRING:
        return "A_STRING";
    case A_CHARACTER:
        return "A_CHARACTER";
    case A_IDENTIFIER:
        return "A_IDENTIFIER";
    case A_FUNC_CALL:
        return "A_FUNC_CALL";
    case A_FUNC_DEF:
        return "A_FUNC_DEF";
    case A_BLOCK:
        return "A_BLOCK";
    case A_ASSIGN:
        return "A_ASSIGN";
    case A_RETURN:
        return "A_RETURN";
    case A_IF:
        return "A_IF";
    case A_WHILE:
        return "A_WHILE";
    case A_PRINT:
        return "A_PRINT";
    case A_EMPTY:
        return "A_EMPTY";
    case A_INT:
        return "A_INT";
    case A_STR: 
        return "A_STR";
    case A_CHAR:
        return "A_CHAR";
    
    default: {
        fprintf(stderr, "ast_node_type_to_string :: Missing Case\n");
        exit(-1);
    }
    }
}

char *variable_type_to_string(struct VARIABLE_TYPE *type)
{

    switch (type->type)
    {
    case V_INTEGER:
        return "V_INTEGER";
    case V_STRING:
        return "V_STRING";
    case V_CHARACTER:
        return "V_CHARACTER";
    case V_UNKNOWN:
        return "V_UNKNOWN";
    case V_POINTER:
        return "V_POINTER";
    case V_FUNCTION:
        return "V_FUNCTION";
    case V_ARRAY:
        return "V_ARRAY";
    case V_VOID:
        return "V_VOID";
    case V_IDENTIFIER:
        return "V_IDENTIFIER";
    default: {
        fprintf(stderr, "variable_type_to_string :: Missing Case\n");
    exit(-1);
    }
    }

    
}
enum VARIABLE_TYPE_ENUM ast_type_to_variable_type_enum(enum AST_NODE_TYPE type)
{
    switch (type)
    {
    case A_INTEGER:
        return V_INTEGER;
    case A_STRING:
        return V_STRING;
    case A_IDENTIFIER:
        return V_IDENTIFIER;
    case A_CHARACTER:
        return V_CHARACTER;
    case A_ARRAY:
        return V_ARRAY;
    case A_VOID:
        return V_VOID;
    default:
        fprintf(stderr, "ast_type_to_variable_type_enum :: Missing Case\n");
        exit(-1);
    }
}
