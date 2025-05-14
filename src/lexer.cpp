#include "lexer.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

Register parse_reg(std::string reg) {
    static const std::unordered_map<std::string, Register> regMap = {
        {"RAX", RAX}, {"RBX", RBX}, {"RCX", RCX}, {"RDX", RDX},
        {"RSI", RSI}, {"RDI", RDI}, {"RBP", RBP}, {"RSP", RSP},
        {"R0", R0}, {"R1", R1}, {"R2", R2}, {"R3", R3}, {"R4", R4},
        {"R5", R5}, {"R6", R6}, {"R7", R7}, {"R8", R8}, {"R9", R9},
        {"R10", R10}, {"R11", R11}, {"R12", R12}, {"R13", R13},
        {"R14", R14}, {"R15", R15}
    };
    return (Register)regMap.at(reg);
}

MathOperator parse_math(std::string math) {
    math = math.substr(1, math.length()-2);
    std::string first = "";
    std::string second = "";
    MathOpcode op = op_NONE;
    std::string part = "";
    bool have_first = false;
    for (int i=0;i<math.length();i++) {
        if (math[i]==' ') {continue;}
        switch (math[i])
        {
            case '+':
                op = op_ADD;
                break;
            case '-':
                op = op_SUB;
                break;
            case '*':
                op = op_MUL;
                break;
            case '/':
                op = op_DIV;
                break;
            case '>':
                op = op_LSR;
                break;
            case '<':
                op = op_LSL;
                break;
            case '&':
                op = op_AND;
                break;
            case '|':
                op = op_OR;
                break;
            case '^':
                op = op_XOR;
                break;
            
            default:
                part += math[i];
                if (have_first) second = part; else first = part;
                continue;
        }
        part = "";
        have_first = true;
    }
    MathOperator ret;
    bool backward;
    ret.opcode = op;
    if (std::toupper(first[0]) == 'R') {
        ret.reg = parse_reg(first);
    } else {
        ret.other.imm = std::stoi(first);
        backward = true;
        ret.is_other_imm = true;
    }
    if (std::toupper(second[0]) == 'R') {
        if (backward) {
            ret.reg = parse_reg(second);
        } else {
            ret.other.reg = parse_reg(second);
            ret.is_other_imm = false;
        }
    } else {
        if (backward) {
            ret.simplify = true;
            switch (op)
            {
                case op_ADD:
                    ret.other.imm += std::stoi(second);
                    break;
                case op_SUB:
                    ret.other.imm -= std::stoi(second);
                    break;
                case op_MUL:
                    ret.other.imm *= std::stoi(second);
                    break;
                case op_DIV:
                    ret.other.imm /= std::stoi(second);
                    break;
                case op_LSR:
                    ret.other.imm >>= std::stoi(second);
                    break;
                case op_LSL:
                    ret.other.imm <<= std::stoi(second);
                    break;
                case op_AND:
                    ret.other.imm &= std::stoi(second);
                    break;
                case op_OR:
                    ret.other.imm |= std::stoi(second);
                    break;
                case op_XOR:
                    ret.other.imm ^= std::stoi(second);
                    break;
                default:
                    std::cout << "huh" << std::endl;
            }
        }
        ret.other.imm = std::stoi(second);
        ret.is_other_imm = true;
    }
    if (backward) {
        if (ret.opcode == op_SUB) ret.opcode = op_BSUB;
        if (ret.opcode == op_DIV) ret.opcode = op_BDIV;
        if (ret.opcode == op_LSL) ret.opcode = op_BLSL;
        if (ret.opcode == op_LSR) ret.opcode = op_BLSR;
    }
    return ret;
}

std::vector<Instruction> parse_masm(std::string code) {
    std::vector<Instruction> ret;
    std::stringstream code_stream(code);
    std::string line;
    std::string part;
    std::vector<std::vector<std::string>> lines;
    while (std::getline(code_stream, line, '\n')) {
        bool hit_real_char = false;
        std::string cur_part = "";
        std::vector<std::string> parts;
        for (int i=0;i<line.size();i++) {
            if (line[i] == ';') break;
            // real char
            if ((int)(line[i])-32>0) {
                if (!hit_real_char) hit_real_char = true;
                cur_part += std::toupper(line[i]);
            } else {
                if (!hit_real_char) continue;
                parts.push_back(cur_part);
                cur_part = "";
                hit_real_char = false;
            }
        }
        if (hit_real_char) {
            parts.push_back(cur_part);
        }
        lines.push_back(parts);
    }
    for (int i=0;i<lines.size();i++) {
        std::vector<std::string> line = lines[i];
        Instruction ins;
        Operand op;
        for (int p=0;p<line.size();p++) {
            if (p==0) {

                continue;
            }
            std::string part = line[p];
            bool is_mem = false;
            bool is_reg = false;
            if (part[0] == '$') {
                is_mem = true;
                part = part.substr(1);
            }
            if (part[0] == 'R') {
                is_reg = true;
                if (is_mem) op.type = MemReg; else op.type = Reg;
                op.data.reg = parse_reg(part);
            } else if (part[0] == '[') {
                op.type = Math;
                op.data.mathop = parse_math(part);
            } else {
                if (is_mem) op.type = MemImm; else op.type = Imm;
                op.data.imm = std::stoi(part);
            }
            ins.operands.push_back(op);
        }
        ret.push_back(ins);
    }
    return ret;
}