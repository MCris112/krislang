//
// Created by crisv on 12/21/2025.
//

#ifndef KRISLANG_EXPRESSION_H
#define KRISLANG_EXPRESSION_H

#include "compiler.h"

ASTNode *parseExpression(int deep );

ASTNode *compileExpression(SymbolTable *symbolTable, ASTNode *node );

#endif //KRISLANG_EXPRESSION_H