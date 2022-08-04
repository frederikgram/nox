/**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <string>
#include "lexer.hpp"
#include "parser.hpp"

using namespace std;

int main(int argc, char * argv[]) {


    /* HANDLE INPUT FILE */

    
    //char * test = "a=5;b=5;if(a+b==10){True};vector_value=2319038;2d_matrix;";
    //char * test = "if(a=5)(\n  a=7\n}";
    //char * test = "{b=1;c=2;};";
    //char * test = "{threshold = 18;\nint age = input();\nif(age >= threshold)\n{\noutput(\"Accepted\");\n    return;\n}\n output(\"Rejected\");\n}";
    //char * test = "{\nthreshold: int = 18;if(threshold>=18){return 1;};\n};";
    


    FILE *fp;
    long lSize;
    char *buffer;

    fp = fopen ( argv[1] , "rb" );
    if( !fp ) perror(argv[1]),exit(1);

    fseek( fp , 0L , SEEK_END);
    lSize = ftell( fp );
    rewind( fp );

    /* allocate memory for entire content */
    buffer = (char *)calloc( 1, lSize+1 );
    if( !buffer ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);

    /* copy the file into the buffer */
    if( 1!=fread( buffer , lSize, 1 , fp) )
    fclose(fp),free(buffer),fputs("entire read fails",stderr),exit(1);

    struct TOKEN * head = lex(buffer, lSize);
    struct AST_NODE * tree = parse(buffer, lSize, head);

    fclose(fp);
    free(buffer);

}
