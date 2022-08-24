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

static std::vector<std::vector<struct Instruction *>> instructions_stack;
static std::vector<std::vector<struct Instruction *>> functions_stack;

std::string get_label() {

    static int label_count = 0;
    return "L_" + std::to_string(label_count++);
}

// Wrapper for instruction.push_back() to allow for logging information
static void push_instruction(struct Instruction * instruction) {

    printf("Intermediate\t::\tPushing Instruction\t::\t\"%s\"\n", instruction->comment.c_str());
    instructions_stack.back().push_back(instruction);
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

// push RBP; mov RSP, RBP;
void setup_stack() {
    push_instruction(new Instruction {
        .op = O_PUSH,
        .register1 = R_RBP,
        .comment = "Push RBP",
    });
    push_instruction(new Instruction {
        .op = O_MOV,
        .register1 = R_RSP,
        .register2 = R_RBP,
        .comment = "Set RSP to RBP",
    });
}

// pop RBP; ret;
void return_from_function() {
    push_instruction(new Instruction {
        .op = O_POP,
        .register1 = R_RBP,
        .comment = "Pop RBP",
    });
    push_instruction(new Instruction {
        .op = O_RET,
    });
}

void generate_code(struct AST_NODE * node) {

    switch(node->type) {
    
        case A_BLOCK:
        case A_PROGRAM:

            // Generate code for each statement in the block
            for(auto stmt : node->statements) {
                generate_code(stmt);
            }
            break;

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
        case A_GEQ: 

            // Generate code for the left and right operands before the comparison
            generate_code(node->lhs);
            generate_code(node->rhs);

            printf("Intermediate\t::\tComparison operator %s\n", ast_node_type_to_string(node->type));
            comparison_operator(ast_node_type_to_operator(node->type), node->expression_type->type);
            break;

        // String Literals
        case A_STRING: {
            std::string label = "ST_" + get_label();
            node->label = label;

            // Add the string literal to the .data section
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
        
        case A_FUNC_DEF: {
            instructions_stack.push_back({});
            
            // Make the function label
            push_instruction(new Instruction {
                .op = O_LABEL,
                .label = node->name->token->value.strval,
            });

            setup_stack();

            // Generate code for the function body
            generate_code(node->block);
            return_from_function();

            //std::vector<struct Instruction *> & first = instructions_stack[0];
            //std::vector<struct Instruction *> & last = instructions_stack.back();
//
            //// Merge the instructions from the stack into the main instruction list
            //first.insert(first.begin(), last.begin(), last.end());
            //instructions_stack.pop_back();

            std::vector<struct Instruction *> & current = instructions_stack.back();
            functions_stack.push_back(current);
            instructions_stack.pop_back();
            break;
        }

        case A_FUNC_CALL:

            printf("Intermediate\t::\tGenerating code for function call %s\n",  node->name->token->literal_value.c_str());
            for(auto arg: node->args) {
                // Generate code for the argument
                generate_code(arg);
            }
            
            push_instruction(new Instruction {
                .op = O_CALL,
                .label = node->name->token->literal_value,
            });
            break;

        default: 
            printf("\x1b[32m !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! default case hit in %s\x1b[0m\n", __FUNCTION__);
            break;

    }
}


// Entry point for the intermediate code generator.
std::vector<struct Instruction *> & generate_intermediate_representation(struct AST_NODE * root) {
    instructions_stack.push_back({});
    generate_code(root);

    //@TODO : Hack
    for (auto func : functions_stack) {
        instructions_stack[0].insert(instructions_stack[0].end(), func.begin(), func.end());
    }
    

    return instructions_stack.back();
}

