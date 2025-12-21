//
// Created by MCris112 on 12/18/2025.
//

#ifndef KRISLANG_PARSER_H
#define KRISLANG_PARSER_H
#include <stdbool.h>
#include "../src/lexer/lexer.h"

typedef enum {
    AST_PROGRAM, // root program
    AST_BLOCK,
    AST_PRINT_STMT,
    AST_TEXT,
    AST_NUMBER,
    AST_ERROR,

    AST_CONCAT, // CONCAT CERTAIN VALUES

    AST_VARIABLE_DEFINITION, // Define a new var $varname = "something"
    AST_VARIABLE_CAST // call a var like: echo $varname;
} ASTNodeType;

typedef enum {
    VARIABLE_TYPE_STRING,
    VARIABLE_TYPE_INT
} VarType;

typedef struct ASTNode {
    ASTNodeType type;

    union {
        // Literals
        char *text;
        int number;

        // Binary operations
        struct {
            struct ASTNode *left;
            struct ASTNode *right;
        } binary;

        // Variable declaration
        struct {
            VarType varType;
            char *name;
            struct ASTNode *value;
        } varDecl;

        // Statements
        struct {
            struct ASTNode **children;
            int count;
            int capacity;
        } block;
    };
} ASTNode;

void syntaxError(const char *message, int line, int column);

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