/**/





enum SYMBOL_CATEGORY {S_VAR, S_FUNC, S_PARAM};


struct SYMBOL {
    int position; //Position in its respective symbol table
    enum SYMBOL_CATEGORY category; 
    struct TOKEN * token; //@TODO : This might have to get decoupled so we just store some literal value instead of the token, otherwise we could get problems with compounds such asd >= and !=
    struct SYMBOL * next; // Linked list structure, could be converted into a hash table
};


struct SYMBOL_TABLE {
    int id;
    struct SYMBOL_TABLE * parent;
    struct SYMBOL * head; 
    int number_of_symbols;
};