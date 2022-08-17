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
    return "L_" + std::to_string(label_count++);
}

// Wrapper for instruction.push_back() to allow for logging information
void push_instruction(struct Instruction * instruction) {

    printf("Intermediate\t::\tPushing Instruction\t::\t\"%s\"\n", instruction->comment.c_str());
    instructions.push_back(instruction);
}


// Comparisons using compq and setXX instructions.
void comparison_operator(enum Operator cmp_type, enum VARIABLE_TYPE_ENUM type) {
    
        // Pop the second operand from the stack
    push_instruction(new Instruction {
        .op = O_POP,
        .register1 = R_RBX,
        .comment = "Pop second operand from stack to R10"
    });

    // Pop the first operand from the stack
    push_instruction(new Instruction {
        .op = O_POP,
        .register1 = R_RCX,
        .comment = "Pop first operand from stack R11"
    });

    // Compare the two operands
    push_instruction(new Instruction {
        .op = cmp_type, 
        .register1 = R_RBX,
        .register2 = R_RCX,
    });

    // Push the result of the comparison onto the stack
    push_instruction(new Instruction {
        .op = O_PUSH,
        .register1 = R_RAX,
        .comment = "Push the result of the comparison onto the stack"
    });
}


// Arithmetic operators
void arithmetic_operator(enum Operator op, enum VARIABLE_TYPE_ENUM type) {

    // Pop the second operand from the stack
    push_instruction(new Instruction {
        .op = O_POP,
        .register1 = R_RBX,
        .comment = "Pop second operand",
    });

    // Pop the first operand from the stack
    push_instruction(new Instruction {
        .op = O_POP,
        .register1 = R_RCX,
        .comment = "Pop first operand",
    });

    // Perform the operation
    push_instruction(new Instruction {
        .op = op,
        .register1 = R_RBX,
        .register2 = R_RCX,
        .comment = "Perform operation",
    });

    // Push the result to the stack
    push_instruction(new Instruction {
        .op = O_PUSH,
        .register1 = R_RCX,
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

        //@TODO : case A_MOD: 
        case A_ADD:
        case A_SUB:
        case A_MUL:
        case A_DIV:

            // Generate code for the left and right operands before the operator
            generate_code(node->lhs);
            generate_code(node->rhs);

            printf("Intermediate\t::\tGenerating code for arithmetic operator %s\n", ast_node_type_to_string(node->type));  
            arithmetic_operator(ast_node_type_to_operator(node->type), node->expression_type->type);
            break;

        // Comparison operators
        case A_EQ:
        case A_NEQ:
        case A_LESS:
        case A_GREAT:
        case A_LEQ:
        case A_GEQ: {

            // Generate code for the left and right operands before the comparison
            generate_code(node->lhs);
            generate_code(node->rhs);

            printf("Intermediate\t::\tComparison operator %s\n", ast_node_type_to_string(node->type));
            comparison_operator(ast_node_type_to_operator(node->type), node->expression_type->type);
            break;
}
        // String Literals
        case A_STRING: {
            std::string label = "ST_" + get_label();
            node->label = label;

            push_instruction(new Instruction {
                .op = O_STRING_DATA,
                .operand1 = new Operand {
                    .mode = AM_DIRECT,
                    .type = V_STRING,
                    .strval = node->token->value.strval,
                },
                .label = label,
            });

            push_instruction(new Instruction {
                .op = O_PUSH,
                .label = label,
                .comment = "Push string label to stack",
            });
            break;    
        }  

        // Integer Literals
        case A_INTEGER: 
            push_instruction(new Instruction {
                .op = O_PUSH,
                .operand1 = new Operand {
                    .mode = AM_DIRECT,
                    .type = V_INTEGER,
                    .value = node->token->value.intval,
                },
                .comment = "Push integer to stack",
            });
            break;

        // Character Literals
        case A_CHARACTER:
            push_instruction(new Instruction {
                .op = O_PUSH,
                .operand1 = new Operand {
                    .mode = AM_DIRECT,
                    .type = V_CHARACTER,
                    .value = node->token->value.charval,
                },
                .comment = (std::string) format_string("Push character %s to stack", node->token->literal_value.c_str()) // This should probably use node->token->value.charval instead
            });
            break;
            

    }
}

// Entry point for the intermediate code generator.
std::vector<struct Instruction *> & generate_intermediate_representation(struct AST_NODE * root) {


    generate_code(root);
    return instructions;
}

