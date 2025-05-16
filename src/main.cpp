#include <iostream>
#include "compiler.h"

int main(int argc, char* argv[]) {
    compile_masm("#include \"STDLIB:/printf.masm\"\nADD $[10<<4] rax");
    return 0;
}