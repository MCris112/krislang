cmake_minimum_required(VERSION 4.0)
project(KrisLang C)

set(CMAKE_C_STANDARD 17)

add_executable(KrisLang main.c
        src/lexer/lexer.c
        src/lexer/lexer.h
        src/parser/parser.c
        src/parser/parser.h
        src/debug.c
        src/debug.h
        src/parser/expression.c
        src/parser/node/binary.c
        src/parser/nodeIf.c
        src/runtime/runtime.c
        src/runtime/runtime.h
        src/runtime/environment.c
        src/runtime/environment.h
        src/helpers/helper.c
        src/helpers/helper.h
        src/runtime/run_function.c
)
