#include "lexer.h"
#include <string>
#include <iostream>

void compile_masm(std::string code) {\
    std::vector<Instruction> ins = parse_masm(code);
    for (int i=0;i<ins.size();i++) {
        for (int j=0;j<ins[i].operands.size();j++) {
            std::cout << "type: " << std::to_string(ins[i].operands[j].type) << std::endl;
            std::cout << "imm: " << std::to_string(ins[i].operands[j].data.imm) << std::endl;
            std::cout << "reg: " << std::to_string(ins[i].operands[j].data.reg) << std::endl;
            std::cout << "is_other_imm: " << std::to_string(ins[i].operands[j].data.mathop.is_other_imm) << std::endl;
            std::cout << "op: " << std::to_string(ins[i].operands[j].data.mathop.opcode) << std::endl;
            std::cout << "o imm: " << std::to_string(ins[i].operands[j].data.mathop.other.imm) << std::endl;
            std::cout << "o reg: " << std::to_string(ins[i].operands[j].data.mathop.other.reg) << std::endl;
            std::cout << "m reg: " << std::to_string(ins[i].operands[j].data.mathop.reg) << std::endl;
        }
    }
}