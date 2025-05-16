#ifndef CMASN_LEXER
#define CMASM_LEXER

#include <regex>
typedef enum Opcode {
    MOV, ADD, SUB, MUL, DIV, INC, JMP,
    CMP, JE, JL, CALL, RET, PUSH, POP,
    OUT, COUT, OUTSTR, OUTCHAR, HLT,
    ARGC, GETARG, DB, LBL, AND, OR,
    XOR, NOT, SHL, SHR, MOVADDR, MOVTO,
    JNE, JG, JLE, JGE, ENTER, LEAVE,
    COPY, FILL, CMP_MEM, MNI, IN, 
    MOVB, SYSCALL, INCLUDE
} Opcode;

typedef enum OperandType {
    Math, Reg, Imm, MemReg, MemImm
} OperandType;

typedef enum Register {
    RAX, RBX, RCX, RDX,
    RSI, RDI, RBP, RSP,
    R0, R1, R2, R3, R4,
    R5, R6, R7, R8, R9,
    R10, R11, R12, R13,
    R14, R15
} Register;

typedef enum MathOpcode {
    op_ADD,
    op_SUB,
    op_MUL,
    op_DIV,
    op_BDIV,
    op_LSR,
    op_LSL,
    op_AND,
    op_OR,
    op_XOR,
    op_BSUB,
    op_BLSR,
    op_BLSL,
    op_NONE
} MathOpcode;

typedef struct MathOperator {
    Register reg;
    MathOpcode opcode;
    union {
        Register reg;
        int imm;
    } other;
    bool is_other_imm;
    bool simplify;
} MathOperator;

typedef struct Operand {
    OperandType type;
    union {
        Register reg;
        int imm;
        MathOperator mathop;
    } data;
} Operand;

typedef struct Instruction {
    Opcode opcode;
    std::vector<Operand> operands;
} Instruction;

std::vector<Instruction> parse_masm(std::string code);

#endif