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
    T_EQUAL,
    T_AMP,
    T_PIPE,
    T_MOD,
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
    T_STRING
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
