//
// Created by crisv on 12/24/2025.
//

#ifndef KRISLANG_RUNTIME_H
#define KRISLANG_RUNTIME_H
#include <stdbool.h>

#include "environment.h"
#include "../parser/parser.h"


EnvValue *runExpression( SymbolTable *symbolTable, ASTNode *node  );

EnvValue *runFunctionCall(  SymbolTable *table, ASTNode *node );

bool runExpressionBoolean( SymbolTable *symbolTable, ASTNode *node );

void runFunctionDefinition( SymbolTable *variableTable, ASTNode *node);

void runBody(SymbolTable *varTable, ASTBlock *block);

void runtime(void);

#endif //KRISLANG_RUNTIME_H