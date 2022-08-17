/**/

#include "emit.hpp"
#include "utils.hpp"

// Prefix literals with $ and registers % for register names.
std::string format_operand(struct Operand * operand) {

    switch(operand->type) {
        case V_CHARACTER:
        case V_INTEGER:
            return format_string("$%d", operand->value);
        default:
            fprintf(stderr, "Emit\t::\tformat_operand\t::\tUnknown operand type\n");
            exit(-1);
    }

}

// Adds a '%' to the beginning of the string
std::string format_register(enum Register reg) {
    return format_string("%%%s", register_to_string(reg));
}

std::string format_arguments(struct Instruction * instruction) {
    std::string result = " ";

    // If a label is given, use that instead of any operands, as we can assume that we're pushing a label to the stack.
    if(instruction->label.size() != 0) {
        result += instruction->label;
        return result;

    // Handle the first argument
    } else if(instruction->operand1 != NULL && instruction->register1 != R_NULL) {
        fprintf(stderr, "Emit\t::\tInstruction was given both an operand and a register as its first argument (instruction->operand1 && instruction->register1)\n");
        exit(-1);

    } else if(instruction->operand1 != NULL) {
        result += format_operand(instruction->operand1);

    } else if (instruction->register1 != R_NULL) {
        result += format_register(instruction->register1);

    } else {
        fprintf(stderr, "Intermediate\t::\tInstruction has no first argument.\n");
        exit(-1);
    }

    // Handle the second argument
    if(instruction->operand2 != NULL && instruction->register2 != R_NULL) {
        fprintf(stderr, "Emit\t::\tInstruction was given both an operand and a register as its second argument (instruction->operand2 && instruction->register2)\n");
        exit(-1);

    } else if(instruction->operand2 != NULL) {

        result += format_string(" , %s", format_operand(instruction->operand2).c_str());

    } else if (instruction->register2 != R_NULL) {

                result += format_string(" , %s", format_register(instruction->register2).c_str());


    } else {
        fprintf(stderr, "Emit\t::\t> Warning!: Instruction '%s' has no second argument.\n", instruction_operator_to_string(instruction->op).c_str());
    }

    return result;

}

std::string jump_instruction(struct Instruction * instruction) {
     std::string result = {};

    switch(instruction->op) {
        case O_JMP:
            result = "jge";
            break;
        case O_JMP_COND:
            result = "jmp";
            break;
        default:
            fprintf(stderr, "Emit\t::\tjump_instruction\t::\tUnhandled Jump instruction\n");
            exit(-1);
    }

    result += format_string(" %s", instruction->label.c_str());
    return result;
}

std::string simple_instruction(struct Instruction * instruction) {

    std::string result = {};

    switch(instruction->op) {
        case O_ADD:
            result = "addq";
            break;
        case O_SUB:
            result = "subq";
            break;
        case O_MUL:
            result = "imulq";
            break;
        case O_POP:
            result = "popq";
            break;
        case O_PUSH:
            result = "pushq";
            break;
        default: 
            fprintf(stderr, "Emit\t::\tCannot find simple instruction for operator '%s'\n", instruction_operator_to_string(instruction->op).c_str());
            exit(-1);    
    }

    result += format_arguments(instruction);

    printf("Emit\t::\tCreated Simple Instruction\t::\t%s\n", result.c_str());
    return result;
}

std::string comparison_instruction(struct Instruction * instruction) {
    std::string result = {};

    result += "cmpq";
    result += format_arguments(instruction);

    switch(instruction->op) {
        case O_EQ:
            result += "\nsete %al\n";
            break;
        case O_NEQ:
            result += "\nsetne %al\n";
            break;
        case O_LEQ:
            result += "\nsetle %al\n";
            break;
        case O_GEQ:
            result += "\nsetge %al\n";
            break;
        case O_GREAT:
            result += "\nsetg %al\n";
            break;
        case O_LESS:
            result += "\nsetl %al\n";
            break;
        default:
            fprintf(stderr, "Emit\t::\tcomparison_operator\t::\tUnhandled comparison operator\n");
            exit(-1);
    }

    result += "movzbq %al , %rax\n";
    return result;
}

std::string x86_preamble(){

    printf("Emit\t::\tCreating x86 Preamble\n");

    std::string result = {};

    // This is split into separate lines to make it easier to read.
    result += "\n# START OF PROGRAM PREAMBLE\n";
    result += "iform:\n.string \"%d\\n\"\n";
    result += "fform:\n.string \"%f\\n\"\n";
    result += "sform:\n.string \"%s\\n\"\n";
    result += "cform:\n.string \"%c\\n\"\n";
    result += ".text\n.globl main\nmain:\n\n";
    result += "# END OF PROGRAM PREAMBLE\n\n";
    return result;    
}

std::string x86_epilogue(){

    printf("Emit\t::\tCreating x86 Epilogue\n");
    std::string result = {};

    result += "\n# START OF PROGRAM EPILOGUE\n\n";
    result += "popq %rbx\n";
    result += "movl $1 , %eax\n";
    //result += "movl $0, %ebx\n";
    result += "int $0x80\n";
    result += "leave\n";
    result += "ret\n";
    return result;
}

// If a comment is given, add it to the end of the assembly instruction.
void wrap_instruction_comment(std::string &result, struct Instruction * instruction) {
    if(instruction->comment.size() != 0) {
        result += format_string("\t# %s", instruction->comment.c_str());
    }
}

// Main function for emitting assembly code.
std::string emit(std::vector<struct Instruction *> instructions) {

    std::string output = x86_preamble();

    bool newline = false;

    for (auto instruction : instructions) {

        newline = true;

        switch(instruction->op) {
        
        case O_JMP_COND:
        case O_JMP:
            printf("Emit\t::\tCreate Jump Instruction %s\n", jump_instruction(instruction).c_str());
            output += jump_instruction(instruction);
            break;
        case O_GEQ:
        case O_LEQ:
        case O_GREAT:
        case O_LESS:
        case O_EQ:
        case O_NEQ:
            printf("Emit\t::\tCreated Comparison Instruction\t::\t%s\n", instruction_operator_to_string(instruction->op).c_str());
            output += comparison_instruction(instruction);
            break;
        case O_ADD:
        case O_SUB:
        case O_POP:
        case O_MUL:
        case O_PUSH:
        case O_LEAQ:
        case O_CALL:
            printf("Emit\t::\tCreated Simple Instruction\t::\t%s\n", simple_instruction(instruction).c_str());
            output += simple_instruction(instruction);
            break;
        case O_DIV:
            printf("Emit\t::\tCreated Division Instruction\n");
            output += "# PERFORMING DIVISION\n";
            output += "popq %rbx # Popping first value into dividend\n";
            output += "popq %rax # Popping second value into divider\n";
            output += "cqo # sign extension\n";
            output += "idivq %rbx # Carry out division\n";
            output += "pushq %rax # Push result to stack\n";
            break;
        case O_MOD:
            printf("Emit\t::\tCreated Modulo Instruction\n");
            output += "# PERFORMING MODULO OPERATION\n";
            output += "popq %rbx # Popping first value into dividend\n";
            output += "popq %rax # Popping second value into divider\n";
            output += "cqo # sign extension\n";
            output += "idivq %rbx # Carry out division\n";
            output += "pushq %rdx # Push result to stack\n";
            break;
        case O_LABEL:
            printf("Emit\t::\tInserted Label\t::\t%s\n", instruction->label.c_str());
            output += format_string("%s:", instruction->label.c_str());
            break;
        case O_STRING_DATA:
            printf("Emit\t::\tInserted STRING LITERAL\t::\t%s\n", instruction->label.c_str());
            output = format_string("%s: .string %s\n", instruction->label.c_str(), instruction->operand1->strval.c_str()) + output;
            newline = false; // Don't add a newline after this instruction as it only changes the .data section
            break;
        default:
            fprintf(stderr, "Emit\t::\tUnknown instruction type '%s'\n", instruction_operator_to_string(instruction->op).c_str());
            exit(-1);
        }

        wrap_instruction_comment(output, instruction);
        if(newline){output += "\n";}
    }


    output += x86_epilogue();

    return output;
}