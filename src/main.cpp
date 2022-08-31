/**/

#include "lexer.hpp"
#include "parser.hpp"
#include "typecheck.hpp"
#include "utils.hpp"
#include "intermediate.hpp"
#include "emit.hpp"
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>

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

    printf("/* BEGIN LEXING */\n\n");
    struct TOKEN *head = lex(buffer, lSize);

    if(head == NULL)
    {
        printf("/* LEXING RETURNED EMPTY */\n");
        return 0;
    }

    printf("\n/* BEGIN PARSING */\n\n");
    struct AST_NODE *tree = parse(buffer, lSize, head);


    if(tree == NULL)
    {
        printf("/* PARSING RETURNED EMPTY */\n");
        return 0;
    }

    printf("\n/* BEGINNING TYPECHECKING */\n\n");

    tree = typecheck(tree);
    printf("\n/* FINISHED TYPECHECKING */\n/* BEGINNING INTERMEDIATE CODE GENERATION */\n\n");

    std::vector<struct Instruction *> & instructions = generate_intermediate_representation(tree);
    printf("\n/* FINISHED INTERMEDIATE CODE GENERATION */\n/* BEGINNING X86 ASSEMBLY GENERATION */\n\n");
    std::string output = emit(instructions);
    printf("\n/* FINISHED X86 ASSEMBLY GENERATION */\n/* WRITING X86 TO FILE './output.s' */\n");
    
    fstream outfile;
    outfile.open("output.s", std::ios_base::out);
    if (!outfile.is_open()) {
        std::cout << "failed to open " << "output.s" << '\n';
    } else {
        outfile.write(output.data(), output.size());
    }
    printf("/* FINISHED WRITING X86 TO FILE */\n");

    fclose(fp);
    free(buffer);
}
