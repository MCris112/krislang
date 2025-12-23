//
// Created by crisv on 12/21/2025.
//

#include "expression.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../debug.h"
#include "variables/variables.h"

// bool evalVariableDefinition() {
//     ASTNode *node = malloc(sizeof(ASTNode));
//     if (!node) {
//         perror("malloc");
//         exit(EXIT_FAILURE);
//     }
//
//     Token token = currentToken();
//
//     switch (token.type) {
//         case TOK_NUMBER:
//             *node = ( ASTNode){
//                 .type = AST_NUMBER,
//                 .number = token.number
//             };
//             break;
//         case TOK_TEXT:
//             *node = ( ASTNode){
//                 .type = AST_TEXT,
//                 .text = token.text
//             };
//             break;
//         case TOK_NUMBER_DECIMAL:
//             *node = ( ASTNode){
//                 .type = AST_NUMBER_DECIMAL,
//                 .decimal = token.decimal
//             };
//             break;
//         case TOK_CHAR:
//             *node = ( ASTNode){
//                 .type = AST_CHAR,
//                 .text = token.text
//             };
//             break;
//         case TOK_LITERAL_BOOLEAN:
//             *node = ( ASTNode){
//                 .type = AST_BOOLEAN,
//                 .boolean = token.boolean
//             };
//             break;
//         default:
//             // Not a variable type
//             return false;
//     }
//
//     return true;
// }

ASTNode *checkConcat(ASTNode *node, int deep) {
    // check next is sum for concat
    if (currentToken().type == TOK_PLUS) {
        printf("[EXPRESSION] CONCATING...: %s\n", lexerTokenToString(currentToken().type));

        nextPos(); // consume '+'

        ASTNode *right = parseExpression(deep);

        ASTNode *concat = malloc(sizeof(ASTNode));
        concat->type = AST_CONCAT;
        concat->binary.left = node;
        concat->binary.right = right;

        return concat;
    }

    return node;
}

void parserAddFunctionArgument(ASTNode **funcNode, ASTNode *arg) {
    // Upper memory size for function if is need it
    if ((*funcNode)->funcCall.count >= (*funcNode)->funcCall.capacity) {
        (*funcNode)->funcCall.capacity = (*funcNode)->funcCall.capacity ? (*funcNode)->funcCall.capacity * 2 : 8;

        (*funcNode)->funcCall.arguments = realloc(
            (*funcNode)->funcCall.arguments,
            (*funcNode)->funcCall.capacity * sizeof(ASTNode *)
        );
    }

    (*funcNode)->funcCall.arguments[(*funcNode)->funcCall.count] = arg;
    (*funcNode)->funcCall.count++;
}

ASTNode *parseFunctionCall() {
    Token functionCall = currentToken(); // function name token
    nextPos();

    // Expect '('
    if (currentToken().type != TOK_PARENTHESIS_OPEN) {
        syntaxError("Expected '(' after function name", currentToken());
        return NULL;
    }

    ASTNode *func = malloc(sizeof(ASTNode));
    func->type = AST_FUNCTION_CALL;
    func->funcCall.name = strdup(functionCall.text);
    func->funcCall.arguments = NULL;
    func->funcCall.capacity = 0;
    func->funcCall.count = 0;

    while (!isEnd() && currentToken().type != TOK_PARENTHESIS_CLOSE) {
        nextPos(); // skip coma or first parentesis

        Token token = currentToken();
        ASTNode *arg = parseExpression(0);

        if (arg == NULL) {
            syntaxError("Unexpected parameter", token);
            break;
        }

        parserAddFunctionArgument(&func, arg);

        if (currentToken().type != TOK_PARENTHESIS_CLOSE) {
            if (currentToken().type != TOK_COMMA) {
                syntaxError("Expected to close or more args", currentToken());
                break;
            }
        }
    }

    if (isEnd()) {
        syntaxError("Expected to close or more args", currentToken());
        return NULL;
    }

    nextPos(); // skip ')'

    return func;
}

ASTNode *parseExpression(int deep) {
    if (isEnd()) {
        ASTNode *err = malloc(sizeof(ASTNode));
        err->type = AST_ERROR;
        return err;
    }

    ASTNode *node = malloc(sizeof(ASTNode));

    if ( currentToken().type == TOK_SEMICOLON) {
        node->type = AST_EOF;
        return node;
    }


    // if (currentToken().type == TOK_SEMICOLON) {
    //     // TODO decide if stop parsing by aborting or show errors
    //     ASTNode *empty = malloc(sizeof(ASTNode));
    //     empty->type = AST_ERROR;
    //     return empty;
    // }

    switch ( currentToken().type ) {
        case  TOK_TEXT:
            node->type = AST_TEXT;
            node->text = strdup(currentToken().text);
            break;
        case  TOK_CHAR:
            node->type = AST_CHAR;
            node->text = strdup(currentToken().text);
            break;
        case TOK_NUMBER:
            node->type = AST_NUMBER;
            node->number = currentToken().number;
            break;
        case TOK_NUMBER_DECIMAL:
            node->type = AST_NUMBER_DECIMAL;
            node->decimal = currentToken().decimal;
            break;
        case TOK_LITERAL_BOOLEAN:
            node->type = AST_BOOLEAN;
            node->boolean = currentToken().boolean;
            break;
        case TOK_VARIABLE:
            node->type = AST_VARIABLE_CAST;
            node->text = strdup(currentToken().text);
            break;
        case TOK_FUNCTION_CALL:
            node = parseFunctionCall();
            break;
        default:
            node->type = AST_ERROR;
            break;
    }

    nextPos();

    printf("[EXPRESSION] SKIPPING: %s\n", lexerTokenToString(currentToken().type));

    // Case have error, return the error, dont do more
    if ( node->type == AST_ERROR ) {
        return node;
    }

    node = checkConcat(node, deep);

    if ( currentToken().type == TOK_EQUAL_EQUAL ) {
        nextPos(); // consume '=='

        ASTNode *right = parseExpression(deep);

        ASTNode *compare = malloc(sizeof(ASTNode));
        compare->type = AST_COMPARE;
        compare->binary.left = node;
        compare->binary.right = right;

        node = compare;
    }

    printf("[EXPRESSION] RETURNING: %s\n", lexerTokenToString(currentToken().type));
    return node;
}


//-----------------------------------
// Compile
//-----------------------------------
void printLiteral(ASTNode *node) {
    switch (node->type) {
        case AST_TEXT:
            printf("%s", node->text);
            break;

        case AST_CHAR:
            printf("%c", node->text ? node->text[0] : '?');
            break;

        case AST_NUMBER:
            printf("%d", node->number);
            break;

        case AST_NUMBER_DECIMAL:
            printf("%f", node->decimal);
            break;

        case AST_BOOLEAN:
            printf(node->boolean ? "TRUE" : "FALSE");
            break;

        default:
            printf("<invalid>");
            break;
    }

    printf("\n");
}

ASTNode *compileFunctionCall(SymbolTable *table, ASTNode *node) {
    if (strcmp(node->funcCall.name, "print") == 0) {
        ASTNode *result = compileExpression(table, node->funcCall.arguments[0]);

        printLiteral(result);
        return NULL;
    }

    if (strcmp(node->funcCall.name, "input") == 0) {
        ASTNode *result = compileExpression(table, node->funcCall.arguments[0]);

        // TODO check correct buffer
        char buffer[256];

        // If dev set some text on it, show a print
        if ( result != NULL )
            printLiteral( result );

        fflush(stdout);
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; // remove newline

        ASTNode *input = malloc(sizeof(ASTNode));
        input->type = AST_TEXT;
        input->text = strdup(buffer);
        return input;
    }

    printf("Unknown function: %s\n", node->funcCall.name);
    // for (int i = 0; i < node->funcCall.count; ++i) {
    //
    // }
}

ASTNode *compileExpression(SymbolTable *symbolTable, ASTNode *node) {
    if (!node) return NULL;

    switch (node->type) {
        case AST_TEXT:
        case AST_CHAR:
        case AST_NUMBER:
        case AST_NUMBER_DECIMAL:
        case AST_BOOLEAN:
            return node; // literal

        case AST_VARIABLE_CAST:
            return getVariableNode(symbolTable, node->text);
            break;
        case AST_CONCAT: {
            ASTNode *left = compileExpression(symbolTable, node->binary.left);
            ASTNode *right = compileExpression(symbolTable, node->binary.right);

            ASTNode *result = malloc(sizeof(ASTNode));
            memset(result, 0, sizeof(ASTNode));

            if (left->type == AST_NUMBER && right->type == AST_NUMBER) {
                result->type = AST_NUMBER;
                result->number = left->number + right->number;
                return result;
            }

            if (left->type == AST_TEXT && right->type == AST_NUMBER) {
                result->type = AST_TEXT;
                char buffer[64];
                snprintf(buffer, sizeof(buffer), "%s%d", left->text, right->number);
                result->text = strdup(buffer);
                return result;
            }

            if (left->type == AST_TEXT && right->type == AST_TEXT) {
                result->type = AST_TEXT;

                // Getting size of each one
                size_t lenA = strlen(left->text);
                size_t lenB = strlen(right->text);

                // Creating a buffer to combine
                char *buf = malloc(lenA + lenB + 1);
                memcpy(buf, left->text, lenA);
                memcpy(buf + lenA, right->text, lenB + 1);

                result->text = buf;

                return result;
            }

            if (left->type == AST_TEXT && right->type == AST_CHAR) {
                result->type = AST_TEXT;

                // Getting size of each one
                size_t lenA = strlen(left->text);
                size_t lenB = strlen(right->text);

                // Creating a buffer to combine
                char *buf = malloc(lenA + lenB + 1);
                memcpy(buf, left->text, lenA);
                memcpy(buf + lenA, right->text, lenB + 1);

                result->text = buf;

                return result;
            }


            if (left->type == AST_NUMBER && right->type == AST_TEXT) {
                result->type = AST_TEXT;
                char buffer[64];
                snprintf(buffer, sizeof(buffer), "%d%s", left->number, right->text);
                result->text = strdup(buffer);
                return result;
            }
            // TODO: handle string concat, variable concat, etc.

            // Unsupported combination
            result->type = AST_ERROR;
            return result;
        }

        case AST_FUNCTION_CALL:
            return compileFunctionCall(symbolTable, node);
            break;
        default:
            return node;
    }
}
