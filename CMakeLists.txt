cmake_minimum_required(VERSION 3.10)

#add_compile_options(-fsanitize=address -fsanitize=leak -fsanitize=undefined)
#add_compile_options(-g)
#add_link_options(-fsanitize=address -fsanitize=leak -fsanitize=undefined)

add_link_options(-lm)

# Project Name
project(MicroASM_Toolchain CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-fsanitize=address -fsanitize=leak -fsanitize=undefined)
add_compile_options(-g)
add_link_options(-fsanitize=address -fsanitize=leak -fsanitize=undefined)

project(MasmCompiler)

set(SOURCES
    src/compiler.cpp
    src/compiler.h
    src/lexer.cpp
    src/lexer.h
    src/main.cpp
)

# Create the main executable
add_executable(masm_to_binary ${SOURCES})