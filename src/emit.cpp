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

// Takes in an operation and formats its operands eg. "addq $1, %rax", The first argument whether it be an operand or a register should be stored as register1/operand1, and likewise for the second argument.
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

    } else if (instruction->register1 != R_NULL) { // R_NULL is just an enum since we can't use NULL for enums
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
    }

    return result;

}

std::string jump_instruction(struct Instruction * instruction) {
     std::string result = {};

    switch(instruction->op) {
        case O_JMP:
            result = "\tjmp";
            break;
        case O_JZ:
            result = "\tjz";
            break;
        case O_JNZ:
            result = "\tjnz";
            break;          
        case O_JE:
            result = "\tje";
            break;
        case O_JNE:
            result = "\tjne";
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
            result = "\taddq";
            break;
        case O_SUB:
            result = "\tsubq";
            break;
        case O_MUL:
            result = "\timulq";
            break;
        case O_POP:
            result = "\tpopq";
            break;
        case O_PUSH:
            result = "\tpushq";
            break;
        case O_MOV:
            result = "\tmovq";
            break;
        case O_CALL:
            result = "\tcall";
            break;
        case O_CMP:
            result = "\tcmpb";
            break;
        default: 
            fprintf(stderr, "Emit\t::\tCannot find simple instruction for operator '%s'\n", instruction_operator_to_string(instruction->op).c_str());
            exit(-1);    
    }

    result += format_arguments(instruction);

    printf("Emit\t::\tCreating Simple Instruction\t::\t%s\n", result.c_str());
    return result;
}

std::string comparison_instruction(struct Instruction * instruction) {
    std::string result = {};

    result += "\tcmpq";
    result += format_arguments(instruction);

    switch(instruction->op) {
        case O_EQ:
            result += "\n\tsete %al\n";
            break;
        case O_NEQ:
            result += "\n\tsetne %al\n";
            break;
        case O_LEQ:
            result += "\n\tsetle %al\n";
            break;
        case O_GEQ:
            result += "\n\tsetge %al\n";
            break;
        case O_GREAT:
            result += "\n\tsetg %al\n";
            break;
        case O_LESS:
            result += "\n\tsetl %al\n";
            break;
            
        default:
            fprintf(stderr, "Emit\t::\tcomparison_operator\t::\tUnhandled comparison operator\n");
            exit(-1);
    }

    result += "\tmovzbq %al , %rax\n";
    return result;
}

std::string x86_preamble(){

    printf("Emit\t::\tCreating x86 Preamble\n");

    std::string result = {};

    // This is split into separate lines to make it easier to read.
    result += "iform:\n.string \"%d\\n\"\n";
    result += "fform:\n.string \"%f\\n\"\n";
    result += "sform:\n.string \"%s\\n\"\n";
    result += "cform:\n.string \"%c\\n\"\n";
    result += ".text\n.globl main\n";
    return result;    
}

std::string division_operator() {
    std::string output = "\t# PERFORMING DIVISION\n";
    output += "\tpopq %rbx # Popping first value into dividend\n";
    output += "\tpopq %rax # Popping second value into divider\n";
    output += "\tcqo # sign extension\n";
    output += "\tidivq %rbx # Carry out division\n";
    output += "\tpushq %rax # Push result to stack\n";
    return output;
}

std::string modulo_operator() {
    std::string output = "\t# PERFORMING MODULO\n";
    output += "\tpopq %rbx # Popping first value into dividend\n";
    output += "\tpopq %rax # Popping second value into divider\n";
    output += "\tcqo # sign extension\n";
    output += "\tidivq %rbx # Carry out division\n";
    output += "\tpushq %rdx # Push result to stack\n";
    return output;
}

//@TODO : This is a mega hack, fix pls.
std::string print_operator(struct Instruction * instruction){
    std::string result = {};
	result += "\tmovq     $1, %rax    # sys_write call number \n";
	result += "\tmovq     $1, %rdi    # write to stdout (fd=1)\n";
    result += "\tmovq     %rsp, %rsi  # use char on stack\n";
    result += "\tmovq     $1, %rdx    # write 1 char\n";
	result += "\tsyscall\n";
	result += "\taddq     $8, %rsp    # restore sp\n\n";
    return result;
}

std::string x86_epilogue(){

    printf("Emit\t::\tCreating x86 Epilogue\n");
    std::string result = {};

    result += "\n# PROGRAM EPILOGUE\n";
    result += "movl $1 , %eax\n";
    result += "int $0x80\n";
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
        
        // This is a simple comparison instruction, no jumps.
        case O_CMP:
            output += simple_instruction(instruction);
            break;

        case O_JMP_COND:
        case O_JMP:
        case O_JNZ:
        case O_JZ:
        case O_JE:
        case O_JNE:
            printf("Emit\t::\tCreating Jump Instruction\t:.\t%s\n", jump_instruction(instruction).c_str());
            output += jump_instruction(instruction);
            break;
        case O_GEQ:
        case O_LEQ:
        case O_GREAT:
        case O_LESS:
        case O_EQ:
        case O_NEQ:
            printf("Emit\t::\tCreating Comparison Instruction\t::\t%s\n", instruction_operator_to_string(instruction->op).c_str());
            output += comparison_instruction(instruction);
            break;
        case O_RET:
            printf("Emit\t::\tCreating Return Instruction\n");
            output += "\tret\n";
            break;
        case O_ADD:
        case O_SUB:
        case O_POP:
        case O_MUL:
        case O_PUSH:
        case O_LEAQ:
        case O_MOV:
        case O_CALL:
            printf("Emit\t::\tCreating Simple Instruction\t::\t%s\n", simple_instruction(instruction).c_str());
            output += simple_instruction(instruction);
            break;
        case O_DIV:
            printf("Emit\t::\tCreating Division Instruction\n");
            division_operator();
            break;
        case O_MOD:
            printf("Emit\t::\tCreating Modulo Instruction\n");
            modulo_operator();
            break;
        case O_LABEL:
            printf("Emit\t::\tInserting Label\t::\t%s\n", instruction->label.c_str());
            output += format_string("\t%s:", instruction->label.c_str());
            break;
        case O_STRING_DATA:
            printf("Emit\t::\tInserting STRING LITERAL\t::\t%s\n", instruction->label.c_str());
            output = format_string("%s: .string %s\n", instruction->label.c_str(), instruction->operand1->strval.c_str()) + output;
            newline = false; // Don't add a newline after this instruction as it only changes the .data section
            break;
        case O_PRINT:
            printf("Emit\t::\tCreating Print Instruction\n");
            output += print_operator(instruction);
            break;
        default:
            fprintf(stderr, "Emit\t::\tUnknown instruction type '%s'\n", instruction_operator_to_string(instruction->op).c_str());
            exit(-1);
        }

        // @NOTE : Hack
        wrap_instruction_comment(output, instruction);
        if(newline){output += "\n";}
    }


    output += x86_epilogue();

    return output;
}