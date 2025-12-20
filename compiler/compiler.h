//
// Created by MCris112 on 12/18/2025.
//

#ifndef KRISLANG_COMPILER_H
#define KRISLANG_COMPILER_H
#include "../parser/parser.h"


//-------------------------------------------------
//
// VARIABLES
//
//-------------------------------------------------

typedef struct {
    ASTNodeType type;
    union {
        char *text;
        int integer;
    };
} VarValue;

typedef struct {
    char *name;
    VarType type;
    VarValue value;
} Symbol;


typedef struct {
    Symbol *symbols;
    int count;
    int capacity;
} SymbolTable;

void printSymbolTable();


void runCompiler();

#endif //KRISLANG_COMPILER_H