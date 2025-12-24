//
// Created by MCris112 on 12/18/2025.
//

#include "parser.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../debug.h"
#include "../compiler/expression.h"

int syntax_error_count = 0;

void syntaxError(const char *message, Token token) {
    fprintf(stderr,
            "Syntax error at %d:%d - %s\n",
            token.line, token.column, message
    );
    syntax_error_count++;
    exit(EXIT_FAILURE);
}

char *astNodeTypeToString(ASTNodeType type) {
    switch (type) {
        case AST_PROGRAM:
            return "AST_PROGRAM";

        case AST_PRINT_STMT:
            return "AST_PRINT_STMT";

        case AST_TEXT:
            return "AST_TEXT";

        case AST_NUMBER:
            return "AST_NUMBER";

        case AST_ERROR:
            return "AST_ERROR";

        case AST_VARIABLE_DEFINITION:
            return "AST_VARIABLE_DEFINITION";

        case AST_VARIABLE_CAST:
            return "AST_VARIABLE_CAST";

        case AST_CONCAT:
            return "AST_CONCAT";
        case AST_FUNCTION_CALL:
            return "AST_FUNCTION_CALL";
        case AST_LOGICAL_IF:
            return "AST_LOGICAL_IF";
        default:
            return "AST_UNKNOWN";
    }
}

int current = 0;

int getASTPosition() {
    return current;
}

void nextPos() {
    current++;
}

bool isEnd() {
    return current >= getTokensCount();
}

Token currentToken() {
    return getTokens()[current];
}

ASTNode *addASTNode(ASTNode *parent, ASTNode child) {

    if ( parent->type == AST_LOGICAL_IF ) {

        // Grow children array if needed
        if (parent->logicalIf.count >= parent->logicalIf.capacity) {
            parent->logicalIf.capacity = parent->logicalIf.capacity
                                         ? parent->logicalIf.capacity * 2
                                         : 4;

            parent->logicalIf.children = realloc(
                parent->logicalIf.children,
                parent->logicalIf.capacity * sizeof(ASTNode *)
            );

            if (!parent->logicalIf.children) {
                perror("realloc");
                exit(EXIT_FAILURE);
            }
        }

        // Allocate new node
        ASTNode *node = malloc(sizeof(ASTNode));
        if (!node) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        *node = child;
        parent->logicalIf.children[ parent->logicalIf.count++ ] = node;
        return parent;
    }

    // is not block type
    // if (parent->type != AST_BLOCK && parent->type != AST_PROGRAM)
    //     abort();

    // Grow children array if needed
    if (parent->block.count >= parent->block.capacity) {
        parent->block.capacity = parent->block.capacity
                                     ? parent->block.capacity * 2
                                     : 4;

        parent->block.children = realloc(
            parent->block.children,
            parent->block.capacity * sizeof(ASTNode *)
        );

        if (!parent->block.children) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
    }

    // Allocate new node
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    *node = child;

    parent->block.children[parent->block.count++] = node;
    return node;
}

bool evalExpectedToken(Token token, TokenType expected, char *message) {
    if (token.type != expected) {
        if (!message)
            message = "Sintaxis unexpected, please check the code";

        syntaxError(message, token);
        return false;
    }

    return true;
}

ASTNode *evalVariableDefinitionValue(Token token) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    switch (token.type) {
        case TOK_NUMBER:
            *node = ( ASTNode){
                .type = AST_NUMBER,
                .number = token.number
            };
            break;
        case TOK_TEXT:
            *node = ( ASTNode){
                .type = AST_TEXT,
                .text = token.text
            };
            break;
        case TOK_NUMBER_DECIMAL:
            *node = ( ASTNode){
                .type = AST_NUMBER_DECIMAL,
                .decimal = token.decimal
            };
            break;
        case TOK_CHAR:
            *node = ( ASTNode){
                .type = AST_CHAR,
                .text = token.text
            };
            break;
        case TOK_LITERAL_BOOLEAN:
            *node = ( ASTNode){
                .type = AST_BOOLEAN,
                .boolean = token.boolean
            };
            break;
        default:
            syntaxError("The value of the var was not expected", token);
            free(node);
            return NULL;
    }

    return node;
}

bool isVariableDefinition() {
    TokenType t = currentToken().type;
    return t == TOK_VARIABLE_TYPE_INT ||
           t == TOK_VARIABLE_TYPE_STRING ||
           t == TOK_VARIABLE_TYPE_BOOLEAN ||
           t == TOK_VARIABLE_TYPE_FLOAT ||
           t == TOK_VARIABLE_TYPE_CHAR ||
           t == TOK_VARIABLE_TYPE_VOID;
}


bool evalVariableDefinition(ASTNode *parent, TokenType type, VarType varType) {
    Token tok = currentToken();
    printf("==== SET VARIABLE ON: %s \n", lexerTokenToString(tok.type) );
    if (tok.type != type) {
        printf("NO VALID! \n");
        return false;
    }
        printf("IS VALID! +++++ \n");

    nextPos(); // skip TYPE

    Token varName = currentToken();
    if (varName.type != TOK_PARENTHESIS_OPEN && varName.type != TOK_VARIABLE) {
        syntaxError("Variable name/size expected", varName);
        nextPos();
        return false;
    }

    int size = -1;

    // Optional (size)

    if (varName.type == TOK_PARENTHESIS_OPEN) {
        nextPos(); // SKIP '('

        Token sizeTok = currentToken();
        if (!evalExpectedToken(sizeTok, TOK_NUMBER, "Expected size inside STRING(...)"))
            return false;

        size = sizeTok.number;
        nextPos();

        if (!evalExpectedToken(currentToken(), TOK_PARENTHESIS_CLOSE, "Expected ')' after size"))
            return false;

        nextPos(); // skip ')'
    }

    // Now expect variable name
    varName = currentToken();
    nextPos();
    if (!evalExpectedToken(varName, TOK_VARIABLE, "Variable name expected"))
        return false;

    // Expect '='
    Token equals = currentToken();
    nextPos();
    if (!evalExpectedToken(equals, TOK_EQUALS, "Expected '='"))
        return false;

    // Parse value
    // Token varValue = currentToken(); nextPos();
    ASTNode *valueNode = parseExpression(0);
    printf("[VAR][VALUE_NODE] DEFINITION: %s \n", astNodeTypeToString(valueNode->type) );
    // TODO check well the tokens, cuz if u pass like "String" this will throw like a normal function call instead of var definition

    printf("[VAR] After value expresed: %s\n", lexerTokenToString(currentToken().type));

    if (valueNode->type == AST_ERROR) {
        return false;
    }

    printf("CURRENCT TOKEN: %s \n", lexerTokenToString(currentToken().type));
    if (currentToken().type != TOK_SEMICOLON) {
        parserPrintASTNode(valueNode, 0);

        syntaxError("Semicolon expected", currentToken());
        return false;
    }

    nextPos();

    ASTNode definition = (ASTNode){
        .type = AST_VARIABLE_DEFINITION,
        .varDecl = {
            .varType = varType,
            .name = varName.text,
            .value = valueNode,
            .size = size
        }
    };

    addASTNode(parent, definition);
    return true;
}


// void parseFunctionCall2(ASTNode *parent) {
//     Token functionCall = currentToken(); // function name token
//     nextPos();
//
//     // Expect '('
//     if (currentToken().type != TOK_PARENTHESIS_OPEN) {
//         syntaxError("Expected '(' after function name", currentToken());
//         return;
//     }
//
//     nextPos(); // skip '('
//
//     // Parse argument (for now only one)
//     // TODO accept comas as parameters, for now only has one
//     ASTNode *arg = parseExpression(0);
//
//     if (currentToken().type != TOK_PARENTHESIS_CLOSE) {
//         syntaxError("Function not closed", currentToken());
//         return;
//     }
//
//     nextPos(); // skip ')'
//
//     ASTNode *node = malloc(sizeof(ASTNode));
//     node->type = AST_FUNCTION_CALL;
//     node->funcCall.name = strdup(functionCall.text);
//
//     // Allocate children array
//     node->funcCall.count = 1;
//     node->funcCall.capacity = 1;
//     node->funcCall.arguments = malloc(sizeof(ASTNode *));
//     node->funcCall.arguments[0] = arg;
//
//     addASTNode(parent, *node);
// }

void *parseBody(ASTNode **parent) {
    printf("----------------NEW BODY-------------\n\n");
    while (!isEnd() && currentToken().type != TOK_BRACE_CLOSE ) {

        if (currentToken().type == TOK_LOGICAL_IF) {
            printf("IS TOK_LOGICAL_IF!!!..\n");

            nextPos();
            ASTNode *nodeIf = malloc(sizeof(ASTNode));
            nodeIf->type = AST_LOGICAL_IF;

            printf("Current token: %s on line: %d \n", lexerTokenToString(currentToken().type), currentToken().line);
            if (currentToken().type != TOK_PARENTHESIS_OPEN) {
                syntaxError("Expected '(' to start if", currentToken());
                continue;
            }

            nextPos();

            ASTNode *expression = parseExpression(0);
            nodeIf->logicalIf.conditional = expression;

            printf("Current token: %s on line: %d \n", lexerTokenToString(currentToken().type), currentToken().line);
            if (currentToken().type != TOK_PARENTHESIS_CLOSE) {
                syntaxError("Expected ')' after expression", currentToken());
                continue;
            }

            nextPos(); // Skip ')'

            if ( currentToken().type != TOK_BRACE_OPEN ) {
                syntaxError("Expected '{' after expression", currentToken());
                continue;
            }

            nextPos(); // Skipp {

            printf("====== IN BODY?......\n");
            parseBody( &nodeIf );

            nextPos(); // Skipp }

            addASTNode( *parent, *nodeIf);
            continue;
        }

        // 2. VARIABLE DEFINITIONS
        if (isVariableDefinition()) {
            if (evalVariableDefinition(*parent, TOK_VARIABLE_TYPE_STRING, VARIABLE_TYPE_STRING)) continue;
            if (evalVariableDefinition(*parent, TOK_VARIABLE_TYPE_INT, VARIABLE_TYPE_INT)) continue;
            if (evalVariableDefinition(*parent, TOK_VARIABLE_TYPE_BOOLEAN, VARIABLE_TYPE_BOOLEAN)) continue;
            if (evalVariableDefinition(*parent, TOK_VARIABLE_TYPE_FLOAT, VARIABLE_TYPE_FLOAT)) continue;
            if (evalVariableDefinition(*parent, TOK_VARIABLE_TYPE_CHAR, VARIABLE_TYPE_CHAR)) continue;
        }

        if (currentToken().type == TOK_FUNCTION_CALL) {
            printf("IS TOK_FUNCTION CALL: TOKEN: %s  \n \n", lexerTokenToString(currentToken().type));
            ASTNode *func = parseFunctionCall();
            addASTNode( *parent, *func);
            continue;
        }


        nextPos();
    }

    printf("\n\n----------------END BODY-------------\n\n");
}

ASTNode getAST() {
    ASTNode *parent = malloc( sizeof(ASTNode) );
    parent->type = AST_PROGRAM;
    parent->block.children = NULL;
    parent->block.capacity = 0;
    parent->block.count = 0;

    parseBody( &parent );

    parserPrintAST( parent );

    if (syntax_error_count > 0) {
        fprintf(stderr,
                "\nCompilation failed with %d syntax error(s).\n",
                syntax_error_count
        );
        exit(EXIT_FAILURE);
    }

    return *parent;
}
