/**/

#include "emit.hpp"
#include "utils.hpp"


std::string format_operand(struct Operand * operand) {

    switch(operand->type) {
        case V_INTEGER:
            return format_string("$%d", operand->value);

        default:
            fprintf(stderr, "Emit\t::\tformat_operand\t::\tUnknown operand type\n");
            exit(-1);
    }

}

std::string format_arguments(std::string &result, struct Instruction * instruction) {
    printf(",,,,%p\n", instruction);
    printf("Emit\t::\tformat_arguments %d\n", instruction->register1);
    if(instruction->operand1 != NULL) {

        result += format_operand(instruction->operand1);

    } else if (instruction->register1 != R_NULL) {

        
        //result += format_register(instruction.register1);


    } else {
        fprintf(stderr, "Error: Instruction has no first argument.\n");
        exit(-1);
    }

}

std::string simple_instruction(struct Instruction * instruction) {

    std::string result = {};


    switch(instruction->op) {

    case O_JE:
        result = "je";
        break;
    case O_JNE:
        result = "jne";
        break;
    case O_JG:
        result = "jg";
        break;
    case O_JL:
        result = "jl";
        break;
    case O_JGE:
        result = "jge";
        break;
    case O_JLE:
        result = "jle";
        break;
    case O_JMP:
        result = "jmp";
        break;
    case O_ADD:
        result = "addq";
        break;
    case O_SUB:
        result = "subq";
        break;
    case O_MUL:
        result = "imulq";
        break;
    case O_CMP:
        result = "leaq";
        break;
    case O_CMPB:
        result = "cmpb";
        break;
    default: 
        fprintf(stderr, "Cannot find simple instruction\n");
        exit(-1);    
    }

    format_arguments(result, instruction);

    printf("Emit\t::\tCreated Simple Instruction\t::\t%s\n", result.c_str());
}



std::string emit(std::vector<struct Instruction *> instructions) {

    std::string output = "";

    for (auto instruction : instructions) {

        switch(instruction->op) {

        case O_JE:
        case O_JNE:
        case O_JG:
        case O_JL:
        case O_JGE:
        case O_JLE:
        case O_JMP:
        case O_ADD:
        case O_SUB:
            output += simple_instruction(instruction);
            break;
        }   
    }

    return output;
}