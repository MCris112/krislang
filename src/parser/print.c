//
// Created by MCris112 on 12/18/2025.
//

#include <stdio.h>
#include <stdlib.h>
#include "parser.h"


void astParsePrint(ASTNode *parent) {
    nextPos(); // consume 'print'

    if ( isEnd() )
        return;

    if ( currentToken().type != TOK_PARENTHESIS_OPEN ) {
        exit(EXIT_FAILURE);
    }

    ASTNode *expr = parseExpression(0);

    addASTNode( parent, *expr);

    if (isEnd() || currentToken().type != TOK_PARENTHESIS_CLOSE) {
        if ( isEnd() )
            printf("NOT_ENOUFH_CODE: ");

        printf("Unexpected token to end\n");
        exit(EXIT_FAILURE);
    }

    nextPos(); // consume ')'
};