cmake_minimum_required(VERSION 4.0)
project(KrisLang C)

set(CMAKE_C_STANDARD 17)

add_executable(KrisLang main.c
        lexer/lexer.c
        lexer/lexer.h
        compiler/compiler.c
        compiler/compiler.h
        parser/parser.c
        parser/parser.h
        parser/print.c
        parser/content.c
        parser/content.h
)
