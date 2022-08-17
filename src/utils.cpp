
#include "utils.hpp"
#include <iostream>
#include <stdarg.h>

char* format_string(const char* format, ...) {
  va_list args;
  va_start(args, format);
  char *ret;
  vasprintf(&ret, format, args);
  va_end(args);
  return ret;
}


std::string instruction_operator_to_string(enum Operator op) {
    switch(op) {
 
    case O_ADD:
        return "O_ADD";
    case O_STRING_DATA:
        return "O_STRING_DATA";
    case O_FUNC:
        return "O_FUNC";
    case O_SUB:
        return "O_SUB";
    case O_MUL:
        return "O_MUL";
    case O_DIV:
        return "O_DIV";
    case O_MOD:
        return "O_MOD";
    case O_LEQ:
        return "O_LEQ";
    case O_GEQ:
        return "O_GEW";
    case O_GREAT:
        return "O_GREAT";
    case O_LESS:
        return "O_LESS";
    case O_EQ:
        return "O_EQ";
    case O_NEQ:
        return "O_NEQ";
    case O_LABEL:
        return "O_LABEL";
    case O_PUSH:
        return "O_PUSH";
    case O_POP:
        return "O_POP";
    case O_CALL:
        return "O_CALL";
    case O_LEAQ:
        return "O_LEAQ";
    case O_PRINT:
        return "O_PRINT";
    case O_WHILE:
        return "O_WHILE";
    case O_IF:
        return "O_IF";
    case O_ELSE:
        return "O_ELSE";
  
    default:
        fprintf(stderr, "instruction_operator_to_string, unknown operator\n");
        exit(-1);
    }
}
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
    case T_BREAK:
        return A_BREAK;
    case T_CONTINUE:
        return A_CONTINUE;
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
const char *ast_node_type_to_string(enum AST_NODE_TYPE type)
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
    case A_BREAK:
        return "A_BREAK";
    case A_CONTINUE:
        return "A_CONTINUE";
    case A_EQ:
        return "A_EQ";
    case A_NEQ:
        return "A_NEQ";
    case A_LESS:
        return "A_LESS";
    case A_GREAT:
        return "A_GREAT";
    case A_LEQ:
        return "A_LEQ";
    case A_GEQ:
        return "A_GEQ";
    
    default: {
        std::cout << "ast_node_type_to_string: Missing Case: " << type << std::endl;
        fprintf(stderr, "ast_node_type_to_string :: Missing Case\n");
        exit(-1);
    }
    }
}
char * register_to_string(enum Register reg) {
    switch(reg) {

    case R_RAX:
        return "rax";
    case R_RCX:
        return "rcx";
    case R_RDX:
        return "rdx";
    case R_RBX:
        return "rbx";
    case R_RSP:
        return "rsp";
    case R_RBP:
        return "rbp";
    case R_RSI:
        return "rsi";
    case R_RDI:
        return "rdi";
    case R_R8:
     return "r8";
    case R_R9:
     return "r9";
    case R_R10:
        return "r10";
    case R_R11:
        return "r11";
    case R_R12:
        return "r12";
    case R_R13:
        return "r13";
    case R_R14:
        return "r14";
    case R_R15:
        return "r15";
    case R_EAX:
        return "eax";
    case R_ECX:
        return "ecx";
    case R_EDX:
        return "edx";
    case R_EBX:
        return "ebx";
    case R_ESP:
        return "esp";
    case R_EBP:
        return "ebp";
    case R_ESI:
        return "esi";
    case R_EDI:
        return "edi";
    case R_R8D:
        return "r8d";
    case R_R9D:
        return "r9d";
    case R_R10D:
        return "r10d";
    case R_R11D:
        return "r11d";
    case R_R12D:
        return "r12d";
    case R_R13D:
        return "r13d";
    case R_R14D:
        return "r14d";
    case R_R15D:
        return "r15d";
    case R_AX:
     return "ax";
    case R_CX:
     return "cx";
    case R_DX:
     return "dx";
    case R_BX:
     return "bx";
    case R_SP:
     return "sp";
    case R_BP:
     return "bp";
    case R_SI:
     return "si";
    case R_DI:
     return "di";
    case R_R8W:
        return "r8w";
    case R_R9W:
        return "r9w";
    case R_R10W:
        return "r10w";
    case R_R11W:
        return "r11w";
    case R_R12W:
        return "r12w";
    case R_R13W:
        return "r13w";
    case R_R14W:
        return "r14w";
    case R_R15W:
        return "r15w";
    case R_AL:
     return "al";
    case R_CL:
     return "cl";
    case R_DL:
     return "dl";
    case R_BL:
     return "bl";
    case R_AH:
     return "ah";
    case R_CH:
     return "ch";
    case R_DH:
     return "dh";
    case R_BH:
     return "bh";
    case R_SPL:
        return "spl";
    case R_BPL:
        return "bpl";
    case R_SIL:
        return "sil";
    case R_DIL:
        return "dil";
    case R_R8B:
        return "r8b";
    case R_R9B:
        return "r9b";
    case R_R10B:
        return "r10b";
    case R_R11B:
        return "r11b";
    case R_R12B:
        return "r12b";
    case R_R13B:
        return "r13b";
    case R_R14B:
        return "r14b";
    case R_R15B:
        return "r15b";
    }

}
enum Operator ast_node_type_to_operator(enum AST_NODE_TYPE type) {

    switch(type) {
        case A_ADD:
            return O_ADD;
        case A_SUB:
            return O_SUB;
        case A_MUL:
            return O_MUL;
        case A_DIV:
            return O_DIV;
        case A_MOD:
            return O_MOD;
        case A_EQ:
            return O_EQ;
        case A_NEQ:
            return O_NEQ;
        case A_GREAT:
            return O_GREAT;
        case A_LESS:
            return O_LESS;
        case A_GEQ:
            return O_GEQ;
        case A_LEQ:
            return O_LEQ;
        case A_PRINT:
            return O_PRINT;
        default:
            fprintf(stderr, "ast_node_type_to_operator: Unknown operator type\n");
            exit(1);
    }
}
const char * variable_type_to_string(struct VARIABLE_TYPE *type)
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
