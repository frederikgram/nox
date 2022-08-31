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
    
    printf("Intermediate\t::\tComparison Operator\t::\t\"%s\"\n", instruction_operator_to_string(cmp_type).c_str());

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
    
        case A_BLOCK: {
            // Generate code for each statement in the block
            for(auto stmt : node->statements) {
                generate_code(stmt);
            }
            break;
        }
   
        // Arithmetic operators
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
        case A_INTEGER: {
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
}
        // Character Literals
        case A_CHARACTER: {
            push_instruction(new Instruction {
                .op = O_PUSH,
                .operand1 = new Operand {
                    .mode = AM_DIRECT,
                    .type = V_CHARACTER,
                    .value = node->token->literal_value[1],
                },
                .comment = (std::string) format_string("Push character %s to stack", node->token->literal_value.c_str()) // This should probably use node->token->value.charval instead
            });
            break;
        }
    
        case A_IF: {

            printf("Intermediate\t::\tGenerating code for if statement\n");
            
            std::string label = get_label();

            // Generate code for the condition
            generate_code(node->conditional);
            
            if(node->els == NULL) {
                push_instruction(new Instruction {
                    .op = O_JNE,
                    .label = "end_if_" + label,
                    .comment = "Skip if condition is false",
                });

                // Generate code for the then statement
                generate_code(node->body);

            } else {

                push_instruction(new Instruction {
                    .op = O_CMP,
                    .operand1 = new Operand {
                        .mode = AM_DIRECT,
                        .type = V_INTEGER,
                        .value = 0,
                    },
                    .register2 = R_AL,
                    
                });

                push_instruction(new Instruction {
                    .op = O_JE,
                    .label = "else_" + label,
                    .comment = "Goto else statement if condition is false",
                });

                // Generate code for the then statement
                generate_code(node->body);

                // Goto the end of the if statement
                push_instruction(new Instruction {
                    .op = O_JMP,
                    .label = "end_if_" + label,
                    .comment = "Goto end of if statement",
                });

                // Insert a label for the else statement
                push_instruction(new Instruction {
                    .op = O_LABEL,
                    .label = "else_" + label,
                    .comment = "Start of else statement",
                });

                // Generate code for the else statement
                generate_code(node->els);
            }

            // Insert a label for the end of the if statement
            push_instruction(new Instruction {
                    .op = O_LABEL,
                    .label = "end_if_" + label,
                    .comment = "End of if statement",
                });
            
            break;
        }
        
        case A_WHILE: {
                
                printf("Intermediate\t::\tGenerating code for while statement\n");

                std::string label = get_label();

                // Insert a label for the start of the while statement
                push_instruction(new Instruction {
                    .op = O_LABEL,
                    .label = "while_" + label,
                    .comment = "Start of while statement",
                });

                // Generate code for the condition
                generate_code(node->conditional);

                push_instruction(new Instruction {
                    .op = O_CMP,
                    .operand1 = new Operand {
                        .mode = AM_DIRECT,
                        .type = V_INTEGER,
                        .value = 0,
                    },
                    .register2 = R_AL,
                    
                });

                // Jump to the end of the while statement if the condition is false
                push_instruction(new Instruction {
                    .op = O_JE,
                    .label = "end_while_" + label,
                    .comment = "Goto end of while statement if condition is false",
                });

                // Generate code for the body of the while statement
                generate_code(node->body);

                // Jump back to the start of the while statement
                push_instruction(new Instruction {
                    .op = O_JMP,
                    .label = "while_" + label,
                    .comment = "Goto start of while statement",
                });

                // Insert a label for the end of the while statement
                push_instruction(new Instruction {
                    .op = O_LABEL,
                    .label = "end_while_" + label,
                    .comment = "End of while statement",
                });

                break;

        }
      
        // Main function
        case A_PROGRAM:{
            instructions_stack.push_back({});

            // Make the function label
            push_instruction(new Instruction {
                .op = O_LABEL,
                .label = "main",
            });

            // Generate code for the function body
            for(auto stmt : node->statements) {
                generate_code(stmt);
            }

            std::vector<struct Instruction *> & current = instructions_stack.back();
            functions_stack.push_back(current);
            instructions_stack.pop_back();
            break;
        }

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

            std::vector<struct Instruction *> & current = instructions_stack.back();
            functions_stack.push_back(current);
            instructions_stack.pop_back();
            break;
        }

        case A_FUNC_CALL:

            printf("Intermediate\t::\tGenerating code for function call %s\n",  node->name->token->literal_value.c_str());

            // Generate code for the argument
            for(auto arg: node->args) {
                generate_code(arg);
            }

            // Call the function
            push_instruction(new Instruction {
                .op = O_CALL,
                .label = node->name->token->literal_value,
            });
            break;

        //@TODO : Gotta figure out how we do return values since we're stack-only
        case A_RETURN:
            //generate_code(node->lhs);
            break;

        // @TODO : Print stuff pls
        case A_PRINT: {
            generate_code(node->lhs);

            push_instruction(new Instruction {
                .op = O_PRINT,
                .operand1 = new Operand {
                    .mode = AM_DIRECT,
                    .type = ast_type_to_variable_type_enum(node->lhs->type),
                },
            });
            break;
        }

        default: 
            printf("\x1b[32m !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! default case hit in %s\x1b[0m\n", __FUNCTION__);
            exit(-1);
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

