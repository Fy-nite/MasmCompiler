#include <iostream>
#include "compiler.h"

int main(int argc, char* argv[]) {
    compile_masm("ADD $rax $1\nSUB $[rbx-4] $[4-rbx]");
    return 0;
}