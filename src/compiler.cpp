#include "lexer.h"
#include <string>
#include <iostream>

void print_ins(Instruction ins) {
    std::cout << "Opcode: " << std::to_string(ins.opcode) << std::endl;
    for (int i=0;i<ins.operands.size();i++) {
        Operand op = ins.operands[i];
        std::cout << "  Type: " << std::to_string(op.type) << std::endl;
             if (op.type == Reg)    std::cout << "    Reg      : " << op.data.reg << std::endl;
        else if (op.type == Imm)    std::cout << "    Imm      : " << op.data.imm << std::endl;
        else if (op.type == MemReg) std::cout << "    (M)Reg   : " << op.data.reg << std::endl;
        else if (op.type == MemImm) std::cout << "    (M)Imm   : " << op.data.imm << std::endl;
        else if (op.type == Math) { std::cout << "    Math Reg : " << op.data.mathop.reg << std::endl;
                                    std::cout << "    Math Op  : " << op.data.mathop.opcode << std::endl;
   if (op.data.mathop.is_other_imm) std::cout << "    Math Imm : " << op.data.mathop.other.imm << std::endl;
                               else std::cout << "    Other Reg: " << op.data.mathop.other.reg << std::endl;}
    }
}

void compile_masm(std::string code) {
    std::vector<Instruction> instructions = parse_masm(code);
    for (int i=0;i<instructions.size();i++) {
        Instruction ins = instructions[i];
        print_ins(ins);
        
    }
}