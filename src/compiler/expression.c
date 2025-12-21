//
// Created by crisv on 12/21/2025.
//

#include "expression.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

        // check next is sum for concat
        if (currentToken().type == TOK_PLUS) {
            nextPos(); // consume '+'

            ASTNode *right = parseExpression(deep);

            ASTNode *concat = malloc(sizeof(ASTNode));
            concat->type = AST_CONCAT;
            concat->binary.left = text;
            concat->binary.right = right;

            return concat;
        }

        return text;
    }

    if (currentToken().type == TOK_NUMBER) {
        // NUMBER
        ASTNode *number = malloc(sizeof(ASTNode));
        number->type = AST_NUMBER;
        number->number = currentToken().number;

        nextPos();

        // check next is sum for concat
        if (currentToken().type == TOK_PLUS) {
            nextPos(); // consume '+'

            ASTNode *right = parseExpression(deep);

            ASTNode *concat = malloc(sizeof(ASTNode));
            concat->type = AST_CONCAT;
            concat->binary.left = number;
            concat->binary.right = right;

            return concat;
        }

        return number;
    }

    if (currentToken().type == TOK_VARIABLE) {
        ASTNode *var = malloc(sizeof(ASTNode));
        var->type = AST_VARIABLE_CAST;
        var->text = strdup(currentToken().text);

        nextPos();
        return var;
    }

    if ( currentToken().type == TOK_SEMICOLON) {
        return NULL;
    }

    ASTNode *err = malloc(sizeof(ASTNode));
    err->type = AST_ERROR;
    nextPos();
    return err;
}
