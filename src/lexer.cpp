#include "lexer.h"
#include <fstream>
#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>


#ifdef _WIN32
    #include <experimental/filesystem>
    #define FILE_SEPARATOR '\\'
    #include <windows.h>
    namespace fs = std::experimental::filesystem;
#else
    #include <filesystem>
    #define FILE_SEPARATOR '/'
    #include <unistd.h>
    namespace fs = std::filesystem;
#endif

std::string getExecutablePath() {
    #ifdef _WIN32
        char buffer[256] = { 0 };
        GetModuleFileNameA(NULL, buffer, 256);
        return std::string(buffer);
    #else
        char buffer[256] = { 0 };
        ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (len != -1) {
            buffer[len] = '\0';
            return std::string(buffer);
        } else {
            return "";
        }
    #endif
}

std::string get_file_path(std::string path) {
    int var_pos = path.find(':');
    bool has_var = var_pos != std::string::npos;
    bool uses_slash = (path.find(FILE_SEPARATOR) != std::string::npos) | has_var;
    if (has_var) {
        std::string var = path.substr(0, var_pos);
        std::string rel_path = path.substr(var_pos+1);
        std::unordered_map<std::string, std::string> vars = {
            {"CWD", fs::current_path().string()}
        };
        std::string dirpath = getExecutablePath();
        dirpath = dirpath.substr(0, dirpath.rfind(FILE_SEPARATOR)) + FILE_SEPARATOR + "libraries";
        if (!fs::exists(dirpath))
            fs::create_directory(dirpath);
        for (const auto & entry : fs::directory_iterator(dirpath)) {
            vars.insert({entry.path().filename().string(), entry.path().string()});
        }
        return vars.at(var) + rel_path;
    }
    return "";
}

Opcode parse_opcode(std::string opcode) {
    static const std::unordered_map<std::string, Opcode> opMap = {
        {"MOV", MOV}, {"ADD", ADD}, {"SUB", SUB}, {"MUL", MUL}, {"DIV", DIV}, {"INC", INC}, {"JMP", JMP},
        {"CMP", CMP}, {"JE", JE}, {"JL", JL}, {"CALL", CALL}, {"RET", RET}, {"PUSH", PUSH}, {"POP", POP},
        {"OUT", OUT}, {"COUT", COUT}, {"OUTSTR", OUTSTR}, {"OUTCHAR", OUTCHAR}, {"HLT", HLT},
        {"ARGC", ARGC}, {"GETARG", GETARG}, {"DB", DB}, {"LBL", LBL}, {"AND", AND}, {"OR", OR},
        {"XOR", XOR}, {"NOT", NOT}, {"SHL", SHL}, {"SHR", SHR}, {"MOVADDR", MOVADDR}, {"MOVTO", MOVTO},
        {"JNE", JNE}, {"JG", JG}, {"JLE", JLE}, {"JGE", JGE}, {"ENTER", ENTER}, {"LEAVE", LEAVE},
        {"COPY", COPY}, {"FILL", FILL}, {"CMP_MEM", CMP_MEM}, {"MNI", MNI}, {"IN", IN}, 
        {"MOVB", MOVB}, {"SYSCALL", SYSCALL}, {"INCLUDE", INCLUDE}
    };
    return (Opcode)opMap.at(opcode);
}

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
    ret.simplify = false;
    bool backward;
    ret.opcode = op;
    if (std::toupper(first[0]) == 'R') {
        ret.reg = parse_reg(first);
    } else {
        ret.other.imm = std::stoi(first);
        backward = true;
        ret.is_other_imm = true;
    }
    if (second[0] == 'R') {
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
        } else {
            ret.other.imm = std::stoi(second);
            ret.is_other_imm = true;
        }
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
    std::string line;
    std::string part;
    std::vector<std::vector<std::string>> lines;
    std::vector<std::vector<std::string>> lowerlines;
    code += '\n';
    while (code.find('\n') != std::string::npos) {
        line = code.substr(0, code.find('\n'));
        code = code.substr(code.find('\n')+1);
        bool hit_real_char = false;
        std::string cur_part = "";
        std::string lower_cur_part = "";
        std::vector<std::string> parts;
        std::vector<std::string> lower_parts;
        for (int i=0;i<line.size();i++) {
            if (line[i] == ';') break;
            // real char
            if ((int)(line[i])-32>0) {
                if (!hit_real_char) hit_real_char = true;
                cur_part += std::toupper(line[i]);
                lower_cur_part += line[i];
            } else {
                if (!hit_real_char) continue;
                parts.push_back(cur_part);
                lower_parts.push_back(lower_cur_part);
                cur_part = "";
                lower_cur_part = "";
                hit_real_char = false;
            }
        }
        if (hit_real_char) {
            parts.push_back(cur_part);
            lower_parts.push_back(lower_cur_part);
        }
        
        if (parts[0] == "#INCLUDE") {
            std::string file_path = lower_parts[1];
            file_path = file_path.substr(1, file_path.length()-2);
            file_path = get_file_path(file_path);
            std::ifstream file(file_path);
            char* file_data = (char*)malloc(fs::file_size(file_path)+1);
            file_data[file.read(file_data, fs::file_size(file_path)).gcount()] = '\0';
            code = file_data + code;
            free(file_data);
        } else {
            lines.push_back(parts);
            lowerlines.push_back(lower_parts);
        }
    }
    for (int i=0;i<lines.size();i++) {
        std::vector<std::string> line = lines[i];
        Instruction ins;
        Operand op;
        for (int p=0;p<line.size();p++) {
            if (p==0) {
                ins.opcode = parse_opcode(line[p]);
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
                if (op.data.mathop.simplify == true) {
                    op.type = MemImm;
                    op.data.imm = op.data.mathop.other.imm;
                }
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