#include "intermediate.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "tokens.hpp"
#include "typecheck.hpp"
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

// @TODO : We should probably do this without a global variable.
static std::vector<struct Instruction> instructions;

std::string get_label() {

    static int label_count = 0;
    return "L" + std::to_string(label_count++);
}

void comparison_operator(enum Operator jump_type, enum VARIABLE_TYPE_ENUM type) {

    std::string label       = get_label();
    std::string true_label  = get_label();
    std::string false_label = get_label();

    // Pop the second operand from the stack
    instructions.push_back(Instruction {
        .op = O_POP,
        .register1 = R_R10,
    });

    // Pop the first operand from the stack
    instructions.push_back(Instruction {
        .op = O_POP,
        .register1 = R_R11,
    });

    // Compare the two operands
    instructions.push_back(Instruction {
        .op = type == V_INTEGER ? O_CMP : O_CMPB, // @TODO : This is a hack. CMPB is for characters when type == V_CHARACTER
        .register1 = R_R10,
        .register2 = R_R11,
    });

    // Jump to the true label if the comparison is true
    instructions.push_back(Instruction {
        .op = jump_type,
        .label = true_label,
    });

    // Push 0 to the stack
    instructions.push_back(Instruction {
        .op = O_PUSH,
        .operand1 = Operand {
            .value = 0,
        },
    });

    // Jump to the false label
    instructions.push_back(Instruction {
        .op = O_JMP,
        .label = false_label,
    });

    // Push True label
    instructions.push_back(Instruction {
        .op = O_LABEL,
        .label = true_label,
    });

    // Push 1 to the stack
    instructions.push_back(Instruction {
        .op = O_PUSH,
        .operand1 =  Operand {
            .value = 1,
        },
    });

    // Push False label
    instructions.push_back(Instruction {
        .op = O_LABEL,
        .label = false_label,
    });

}


// Entry point for the intermediate code generator.
std::vector<struct Instruction> generate_intermediate_representation(struct AST_NODE * root) {



    return instructions;
}

