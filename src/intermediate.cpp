/**/

#include "intermediate.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "tokens.hpp"
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
using namespace std;

// @TODO : We should probably do this without a global variable.
std::vector<struct Instruction> instructions;
int label_count = 0;



struct Instruction comparison_operator(enum Operator op, enum Operator jump_type) {

    int label       = label_count++;
    int true_label  = label_count++;
    int false_label = label_count++;

    struct Instruction instruction = {
        .op = op,
    };

}


// Entry point for the intermediate code generator.
std::vector<struct Instruction> generate_intermediate_representation(struct AST_NODE * root) {



    return instructions;
}

