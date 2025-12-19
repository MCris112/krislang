//
// Created by MCris112 on 12/18/2025.
//

#ifndef KRISLANG_PARSER_H
#define KRISLANG_PARSER_H
#include <stdbool.h>
#include "../lexer/lexer.h"

typedef enum {
    AST_PROGRAM, // root program
    AST_PRINT_STMT,
    AST_TEXT,
    AST_NUMBER,
    AST_ERROR,

    AST_CONCAT, // CONCAT CERTAIN VALUES

    AST_VARIABLE_DEFINITION, // Define a new var $varname = "something"
    AST_VARIABLE_CAST // call a var like: echo $varname;
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    char *text; // for strings, identifiers
    int number; // for numbers
    struct ASTNode *children;
    int childCount;
    int childCapacity;
} ASTNode;

int getASTPosition();

void nextPos();

bool isEnd();

Token currentToken();

ASTNode getAST();

void astParsePrint(ASTNode *parent);

void showAST(ASTNode *root);

char *astNodeTypeToString(ASTNodeType type);

ASTNode *addASTNode(ASTNode *parent, ASTNode child);

#endif //KRISLANG_PARSER_H