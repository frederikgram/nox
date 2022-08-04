/**/

#include <stdio.h>
#include <stdlib.h>
#include "lexer.hpp"
#include "parser.hpp"
#include <string.h>
#include "symbols.hpp"


int main(int argc, char * argv[]) {


    /* HANDLE INPUT FILE */

    
    //char * test = "a=5;b=5;if(a+b==10){True};vector_value=2319038;2d_matrix;";
    //char * test = "if(a=5)(\n  a=7\n}";
    //char * test = "{b=1;c=2;};";
    //char * test = "{threshold = 18;\nint age = input();\nif(age >= threshold)\n{\noutput(\"Accepted\");\n    return;\n}\n output(\"Rejected\");\n}";
    //char * test = "{\nthreshold: int = 18;if(threshold>=18){return 1;};\n};";
    

    char * test = "func int test(int a) {\nthreshold: str = 18;\nif (a >= threshold) {\nreturn 1; \n} else {\nreturn 0;\n}\n}";
    struct TOKEN * head = lex(test, strlen(test));
    struct AST_NODE * tree = parse(test, strlen(test), head);

}
