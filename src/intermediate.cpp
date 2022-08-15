#include "intermediate.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "tokens.hpp"
#include "utils.hpp"
#include "typecheck.hpp"
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

// @TODO : We should probably do this without a global variable.
static std::vector<struct Instruction *> instructions;

std::string get_label() {

    static int label_count = 0;
    return "L" + std::to_string(label_count++);
}

// Wrapper for instruction.push_back() to allow for logging information
void push_instruction(struct Instruction * instruction) {

    printf("Intermediate\t::\tPushing Instruction\t::\t\"%s\"\n", instruction->comment.c_str());
    printf("..... %d-%p\n", instruction->register1, instruction);   
    instructions.push_back(instruction);
}

// Push instructions for a comparison operation to the global instructions vector
void comparison_operator(enum Operator jump_type, enum VARIABLE_TYPE_ENUM type) {

    std::string label       = get_label();
    std::string true_label  = get_label();
    std::string false_label = get_label();

    // Pop the second operand from the stack
    push_instruction(new Instruction {
        .op = O_POP,
        .register1 = R_R10,
        .comment = "Pop second operand from stack"
    });

    // Pop the first operand from the stack
    push_instruction(new Instruction {
        .op = O_POP,
        .register1 = R_R11,
        .comment = "Pop first operand from stack"
    });

    // Compare the two operands
    push_instruction(new Instruction {
        .op = type == V_INTEGER ? O_CMP : O_CMPB, // @TODO : This is a hack. O_CMPB is for characters when type == V_CHARACTER, so when we add floats we can't just use this ternary.
        .register1 = R_R10,
        .register2 = R_R11,
        .comment = "Compare the two operands"
    });

    // Jump to the true label if the comparison is true
    push_instruction(new Instruction {
        .op = jump_type,
        .label = true_label,
        .comment = "Jump to the true label if the comparison is true"
    });

    // Push 0 to the stack
    push_instruction(new Instruction {
        .op = O_PUSH,
        .operand1 = new Operand {
            .mode = AM_DIRECT,
            .type = type,
            .value = 0,
        },
        .comment = "Push 0 to the stack"
    });

    // Jump to the false label
    push_instruction(new Instruction {
        .op = O_JMP,
        .label = false_label,
        .comment = "Jump to the false label"
    });

    // Push True label
    push_instruction(new Instruction {
        .op = O_LABEL,
        .label = true_label,
        .comment = "Push True label"
    });

    // Push 1 to the stack
    push_instruction(new Instruction {
        .op = O_PUSH,
        .operand1 =  new Operand {
            .value = 1,
        },
        .comment = "Push 1 to the stack"
    });

    // Push False label
    push_instruction(new Instruction {
        .op = O_LABEL,
        .label = false_label,
        .comment = "Push False label"
    });

}

// Arithmetic operators
void arithmetic_operator(enum Operator op, enum VARIABLE_TYPE_ENUM type) {

    // Pop the second operand from the stack
    push_instruction(new Instruction {
        .op = O_POP,
        .register1 = R_R10,
        .comment = "Pop second operand",
    });

    // Pop the first operand from the stack
    push_instruction(new Instruction {
        .op = O_POP,
        .register1 = R_R11,
        .comment = "Pop first operand",
    });

    // Perform the operation
    push_instruction(new Instruction {
        .op = op,
        .register1 = R_R10,
        .register2 = R_R11,
        .comment = "Perform operation",
    });

    // Push the result to the stack
    push_instruction(new Instruction {
        .op = O_PUSH,
        .register1 = R_R10,
        .comment = "Push result",
    });
}



void generate_code(struct AST_NODE * node) {

    switch(node->type) {

        case A_BLOCK: {

            printf("Intermediate\t::\tGenerating code for block\n");
            // Generate code for each statement in the block

            for(auto stmt : node->statements) {
                generate_code(stmt);
            }

            break;
        }

        case A_EQ:
        case A_NEQ:
        case A_LESS:
        case A_GREAT:
        case A_LEQ:
        case A_GEQ:
            printf("Intermediate\t::\tComparison operator %s\n", ast_node_type_to_string(node->type));
            comparison_operator(ast_node_type_to_operator(node->type), node->expression_type->type);
            break;

    }



}

// Entry point for the intermediate code generator.
std::vector<struct Instruction *> & generate_intermediate_representation(struct AST_NODE * root) {


    generate_code(root);
    return instructions;
}

