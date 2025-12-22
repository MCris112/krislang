//
// Created by MCris112 on 12/18/2025.
//

#ifndef KRISLANG_PARSER_H
#define KRISLANG_PARSER_H
#include <stdbool.h>
#include "../lexer/lexer.h"

typedef enum {
    AST_PROGRAM, // root program
    AST_BLOCK,
    AST_PRINT_STMT,

    AST_FUNCTION_CALL,

    AST_TEXT,
    AST_NUMBER,
    AST_NUMBER_DECIMAL,
    AST_BOOLEAN,
    AST_CHAR,
    AST_ERROR,

    AST_CONCAT, // CONCAT CERTAIN VALUES

    AST_VARIABLE_DEFINITION, // Define a new var $varname = "something"
    AST_VARIABLE_CAST // call a var like: echo $varname;
} ASTNodeType;

typedef enum {
    VARIABLE_TYPE_STRING,
    VARIABLE_TYPE_INT,
    VARIABLE_TYPE_FLOAT,
    VARIABLE_TYPE_BOOLEAN,
    VARIABLE_TYPE_CHAR,
} VarType;

typedef struct ASTNode {
    ASTNodeType type;

    union {
        // Literals
        char *text;
        int number;
        double decimal;
        bool boolean;

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
            int size; /* // -1 means autosize */
        } varDecl;

        // Statements
        struct {
            struct ASTNode **children;
            int count;
            int capacity;
        } block;

        struct {
            char *name;
            struct ASTNode **arguments;
            int count;
            int capacity;
        } funcCall;
    };
} ASTNode;

void syntaxError( const char *message, Token token );

int getASTPosition();

void nextPos();

bool isEnd();

Token currentToken();

ASTNode getAST();

void astParsePrint(ASTNode *parent);

void parserPrintAST(ASTNode *root);

char *astNodeTypeToString(ASTNodeType type);

ASTNode *addASTNode(ASTNode *parent, ASTNode child);

#endif //KRISLANG_PARSER_H