#pragma once

#include "parser.hpp"
#include "typecheck.hpp"
#include <string.h>

enum Operator {

    O_JE,
    O_JNE,
    O_JG,
    O_JL,
    O_JGE,
    O_JLE,
    O_JMP,

    O_ADD,
    O_SUB,
    O_MUL,
    O_DIV,
    O_MOD,

    O_CMP,
    O_CMPB,

    O_ALLOC_STACK,
    O_ALLOC_HEAP,
    O_FREE_STACK,
    O_FREE_HEAP,

    O_LABEL,

    O_PUSH,
    O_POP,
    O_CALL,
    O_RET,

    O_PRINT,
    // O_SCANF, @TODO : for when we get input working.

    O_WHILE,
    O_IF,
    O_ELSE,

    O_INT_TO_FLOAT,
    O_FLOAT_TO_INT,
};



enum Register {
    
        // 64 bit registers.
        R_RAX,
        R_RCX,
        R_RDX,
        R_RBX,
        R_RSP,
        R_RBP,
        R_RSI,
        R_RDI,
        R_R8,
        R_R9,
        R_R10,
        R_R11,
        R_R12,
        R_R13,
        R_R14,
        R_R15,

        // 32 bit registers.
        R_EAX,
        R_ECX,
        R_EDX,
        R_EBX,
        R_ESP,
        R_EBP,
        R_ESI,
        R_EDI,
        R_R8D,
        R_R9D,
        R_R10D,
        R_R11D,
        R_R12D,
        R_R13D,
        R_R14D,
        R_R15D,

        // 16 bit registers.
        R_AX,
        R_CX,
        R_DX,
        R_BX,
        R_SP,
        R_BP,
        R_SI,
        R_DI,
        R_R8W,
        R_R9W,
        R_R10W,
        R_R11W,
        R_R12W,
        R_R13W,
        R_R14W,
        R_R15W,

        // 8 bit registers.
        R_AL,
        R_CL,
        R_DL,
        R_BL,
        R_AH,
        R_CH,
        R_DH,
        R_BH,
        R_SPL,
        R_BPL,
        R_SIL,
        R_DIL,
        R_R8B,
        R_R9B,
        R_R10B,
        R_R11B,
        R_R12B,
        R_R13B,
        R_R14B,
        R_R15B,

};



enum AddressingMode {
    AM_IMMEDIATE,
    AM_DIRECT,
    AM_INDIRECT,
    AM_INDIRECT_OFFSET,
    AM_INDIRECT_OFFSET_INDEX,
    AM_INDIRECT_INDEX,
    AM_INDIRECT_INDEX_OFFSET,
    AM_INDIRECT_INDEX_OFFSET_INDEX,
};

struct Operand {
    AddressingMode mode;
    int value;
    int offset;
};

struct Instruction {
    Operator op;
    
    Operand operand1;
    Operand operand2;

    enum Register register1;
    enum Register register2;
    enum AddressingMode addressing_mode;

    std::string label;
    std::string comment;

};

void comparison_operator(enum Operator jump_type, enum VARIABLE_TYPE_ENUM type);