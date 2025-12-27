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
    AST_FUNCTION_DEFINITION,
    AST_FUNCTION_PARAMETER,
    AST_FUNCTION_REFERENCE,
    AST_LOGICAL_IF,
    AST_RETURN,

    AST_TYPE_LITERAL,
    AST_TEXT,
    AST_NUMBER,
    AST_NUMBER_DECIMAL,
    AST_BOOLEAN,
    AST_CHAR,
    AST_VOID,
    AST_NULL,
    AST_UNKNOWN,
    AST_ERROR,

    AST_CONCAT, // CONCAT CERTAIN VALUES
    AST_SUBTRACT,
    AST_COMPARE,
    AST_UNARY,

    AST_VARIABLE_DEFINITION, // Define a new var $varname = "something"
    AST_VARIABLE_CAST, // call a var like: echo $varname;

    AST_EOF // DECLARE END;
} ASTNodeType;

typedef enum {
    VARIABLE_TYPE_STRING,
    VARIABLE_TYPE_INT,
    VARIABLE_TYPE_FLOAT,
    VARIABLE_TYPE_BOOLEAN,
    VARIABLE_TYPE_CHAR,
    VARIABLE_TYPE_VOID,

    // Avoiding for now, for some future
    VARIABLE_TYPE_UNKNOWN,
    VARIABLE_TYPE_NEVER
} VarType;



typedef struct {
    struct ASTNode **children;
    int count;
    int capacity;
} ASTBlock;

typedef struct {
    TokenType operator;   // TOKEN_MINUS, TOKEN_PLUS, etc.
    struct ASTNode *operand;
} ASTUnary;

typedef struct {
    struct ASTNode **children;
    int count;
    int capacity;
} ASTFunctionArguments;


typedef struct ASTNode {
    ASTNodeType type;

    union {
        /* -------------------------
         * Literals
         * ------------------------- */
        char *text;
        char character;
        int number;
        double decimal;
        bool boolean;

        struct ASTNode *child;

        struct {
            ASTNodeType type;
            int size; //-1 means auto memory size
        } literal;

        /* -------------------------
         * Binary operations
         * ------------------------- */
        struct {
            struct ASTNode *left;
            struct ASTNode *right;
        } binary;

        /* -------------------------
        * Unary declaration
        * when detect an operator before, like - / +
        * ------------------------- */
        ASTUnary unary;

        /* -------------------------
         * Variable declaration
         * ------------------------- */
        struct {
            VarType varType;
            char *name;
            struct ASTNode *value;
            int size; /* // -1 means autosize */
        } varDecl;

        /* -------------------------
         * Block of statements
         * ------------------------- */
        ASTBlock block;

        /* -------------------------
         * Function call
         * ------------------------- */
        struct {
            char *name;
            ASTFunctionArguments arguments;
        } funcCall;

        /* -------------------------
        * Function Definition
        * ------------------------- */
        struct {
            char *name;
            ASTFunctionArguments arguments;
            ASTBlock body;
        } funcDefinition;

        /* -------------------------
         * If / Else
         * ------------------------- */
        struct {
            struct ASTNode *conditional;

            // body of if
            ASTBlock bodyBlock;

            // else-block (optional)
            ASTBlock elseBlock;
        } logicalIf;
    };
} ASTNode;


void syntaxError( const char *message, Token token );

int getASTPosition();

void nextPos();

bool isEnd();

bool isVariableDefinition();

VarType parseTokToVarType();

Token currentToken();

Token beforeToken();

ASTNode getAST();

void astParsePrint(ASTNode *parent);

void parserPrintAST(ASTNode *root);

char *astNodeTypeToString(ASTNodeType type);

ASTNode *addASTNode(ASTBlock *parent, ASTNode child);

ASTNode *parseASTConcat();
void parseNodeIf(ASTBlock *parent);

void *parseBody(ASTBlock *parent);

ASTNode parseTypeLiteral();

ASTNode *parseExpression(int deep );

ASTNode parseFunctionDefinition();

void parseFunctionArguments( ASTFunctionArguments *arguments  );

ASTNode *parseFunctionCall();

#endif //KRISLANG_PARSER_H