//
// Created by crisv on 12/21/2025.
//
#include "../compiler.h"

#ifndef KRISLANG_VARIABLES_H
#define KRISLANG_VARIABLES_H

void initSymbolTable(SymbolTable **variableTable);

void declareVariableByASTNode(SymbolTable *variableTable, ASTNode *node);

VarValue *getVariableValue(SymbolTable *variableTable,char *name);

ASTNode *astNodeFromVarValue(const VarValue *value);

ASTNode *getVariableNode(SymbolTable *table, const char *name);

#endif //KRISLANG_VARIABLES_H