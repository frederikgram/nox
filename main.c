/**/

#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include <string.h>




int main(int argc, char * argv[]) {

    
    char * test = "1234+12312";


    struct TOKEN * head = lex(test, strlen(test));
    for(;head != NULL; head=head->next) {
        printf("%s", head->literal_value);
    }
    printf("\n");

}