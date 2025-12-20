//
// Created by MCris112 on 12/18/2025.
//

#include "content.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ASTNode *parseExpression( int deep ) {

    if ( isEnd() ) {
        ASTNode *err = malloc(sizeof(ASTNode));
        err->type = AST_ERROR;
        return err;
    }

    if ( currentToken().type == TOK_PARENTESIS_OPEN) {
        deep++;
        nextPos();
    }

    if ( currentToken().type == TOK_PARENTESIS_CLOSE) {
        deep--;
        nextPos();
    }

    if ( currentToken().type == TOK_TEXT ) {

        // TEXT
        ASTNode *text = malloc(sizeof(ASTNode));
        text->type = AST_TEXT;
        text->text = strdup(currentToken().text);

        printf( "%s \n", text->text );
        nextPos();

        // check next is sum for concat
        if ( currentToken().type == TOK_SUM ) {
            nextPos(); // consume '+'

            ASTNode *right = parseExpression( deep );

            ASTNode *concat = malloc(sizeof(ASTNode));
            concat->type = AST_CONCAT;
            concat->binary.left = text;
            concat->binary.right = right;

            return concat;
        }

        return text;
    }

    ASTNode *err = malloc(sizeof(ASTNode));
    err->type = AST_ERROR;
    nextPos();
    return err;
}

