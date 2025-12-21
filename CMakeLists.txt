cmake_minimum_required(VERSION 4.0)
project(KrisLang C)

set(CMAKE_C_STANDARD 17)

add_executable(KrisLang main.c
        src/lexer/lexer.c
        src/lexer/lexer.h
        src/compiler/compiler.c
        src/compiler/compiler.h
        src/parser/parser.c
        src/parser/parser.h
        src/parser/print.c
        src/parser/content.c
        src/parser/content.h
        src/debug.c
        src/debug.h
        src/compiler/variables/Variables.c
        src/compiler/variables/Variables.h
        src/compiler/expression.c
        src/compiler/expression.h
)
