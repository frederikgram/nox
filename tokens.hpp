/**/

using namespace std;
#include <string>

enum TOKEN_TYPE {
    T_EMPTY,
    T_FOR,
    T_WHILE,
    T_IF,
    T_INTEGER,
    T_STR,
    T_INT,
    T_LESS,
    T_GREAT,
    T_ASSIGN,
    T_EQ,
    T_GEQ,
    T_LEQ,
    T_AND,
    T_OR,
    T_AMP,
    T_AEQ,
    T_SEQ,
    T_PIPE,
    T_MOD,
    T_INC,
    T_DEC,
    T_BANG,
    T_ADD,
    T_SUB,
    T_STAR,
    T_DIV,
    T_LPARENS,
    T_RPARENS,
    T_LBRACKET,
    T_RBRACKET,
    T_RBRACE,
    T_LBRACE,
    T_DOT,
    T_COMMA,
    T_COLON,
    T_SEMICOLON,
    T_STRING,
    T_INPUT,
    T_IDENTIFIER,
    T_NEQ,
    T_ELSE,
    T_ELSEIF,
    T_FUNC,
    T_RETURN,
    T_USE,
    T_OUTPUT,
};




union Value {
    int intval;
    float floatval;
    char charval;
    char * strval;
};


struct TOKEN {
    enum TOKEN_TYPE type;
    union Value value;
    int row;
    int col;
    struct TOKEN * next;
    struct TOKEN * prev;
    char * literal_value;

};
