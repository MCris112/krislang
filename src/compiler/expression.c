//
// Created by crisv on 12/21/2025.
//

#include "expression.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

ASTNode *parseExpression(int deep ) {
    if (isEnd()) {
        ASTNode *err = malloc(sizeof(ASTNode));
        err->type = AST_ERROR;
        return err;
    }

    if (currentToken().type == TOK_TEXT) {
        // TEXT
        ASTNode *text = malloc(sizeof(ASTNode));
        text->type = AST_TEXT;
        text->text = strdup(currentToken().text);

        nextPos();

        return checkConcat(text, deep);
    }

    if (currentToken().type == TOK_CHAR) {
        // TEXT
        ASTNode *text = malloc(sizeof(ASTNode));
        text->type = AST_CHAR;
        text->text = strdup(currentToken().text);

        nextPos();

        return checkConcat(text, deep);
    }

    if (currentToken().type == TOK_NUMBER) {
        // NUMBER
        ASTNode *number = malloc(sizeof(ASTNode));
        number->type = AST_NUMBER;
        number->number = currentToken().number;

        nextPos();

        return checkConcat(number, deep);
    }

    if (currentToken().type == TOK_LITERAL_BOOLEAN) {
        // TEXT
        ASTNode *node = malloc(sizeof(ASTNode));
        node->type = AST_BOOLEAN;
        node->boolean = currentToken().boolean;

        nextPos();

        return checkConcat(node, deep);
    }

    if (currentToken().type == TOK_VARIABLE) {
        ASTNode *var = malloc(sizeof(ASTNode));
        var->type = AST_VARIABLE_CAST;
        var->text = strdup(currentToken().text);

        nextPos();
        return checkConcat(var, deep);
    }

    if ( currentToken().type == TOK_SEMICOLON) {
        // TODO decide if stop parsing by aborting or show errors
        ASTNode *empty = malloc(sizeof(ASTNode));
        empty->type = AST_ERROR;
        return empty;
    }

    ASTNode *err = malloc(sizeof(ASTNode));
    err->type = AST_ERROR;
    nextPos();
    return err;
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
            return getVariableNode( symbolTable , node->text );
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

            if ( left->type == AST_TEXT && right->type == AST_TEXT ) {
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

            if ( left->type == AST_TEXT && right->type == AST_CHAR ) {
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

        default:
            return node;
    }
}
