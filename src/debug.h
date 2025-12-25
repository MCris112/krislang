//
// Created by crisv on 12/21/2025.
//

#ifndef KRISLANG_DEBUG_H
#define KRISLANG_DEBUG_H
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "runtime/environment.h"

//------------------------------------
//
// LEXER
//
//------------------------------------
const char *lexerTokenToString(TokenType type);

void lexerPrintSingleToken(const Token *tok);

void lexerPrintTokens(const Token *tokens, int count);

//------------------------------------
//
// PARSER
//
//------------------------------------
void parserPrintASTNode(ASTNode *node, int indent);

void parserPrintAST(ASTNode *root);

char *parserVarTypeToString(VarType type);

char *parseEnvValueTypeToString( EnvValueType type );


//------------------------------------
//
// VARIABLES
//
//------------------------------------
void printSymbolTable(SymbolTable *variableTable);

#endif //KRISLANG_DEBUG_H