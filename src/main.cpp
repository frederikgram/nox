/**/

#include "lexer.hpp"
#include "parser.hpp"
#include "typecheck.hpp"
#include "utils.hpp"
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

using namespace std;

int main(int argc, char *argv[])
{

    FILE *fp;
    long lSize;
    char *buffer;

    fp = fopen(argv[1], "rb");
    if (!fp)
        perror(argv[1]), exit(1);

    fseek(fp, 0L, SEEK_END);
    lSize = ftell(fp);
    rewind(fp);

    /* allocate memory for entire content */
    buffer = (char *)calloc(1, lSize + 1);
    if (!buffer)
        fclose(fp), fputs("memory alloc fails", stderr), exit(1);

    /* copy the file into the buffer */
    if (1 != fread(buffer, lSize, 1, fp))
        fclose(fp), free(buffer), fputs("entire read fails", stderr), exit(1);

#ifdef DEBUG
    printf("/* BEGIN LEXING */\n\n");
#endif
    struct TOKEN *head = lex(buffer, lSize);
#ifdef DEBUG
    printf("\n/* FINISHED LEXING */");
    printf("\n/* BEGIN PARSING */\n\n");
#endif
    struct AST_NODE *tree = parse(buffer, lSize, head);
#ifdef DEBUG
    printf("\n/* FINISHED PARSING */");
    printf("\n/* BEGINNING TYPECHECKING */\n\n");
#endif

    tree = typecheck(tree);
#ifdef DEBUG
    printf("\n/* FINISHED TYPECHECKING */\n");
#endif

    fclose(fp);
    free(buffer);
}
