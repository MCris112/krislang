//
// Created by MCris112 on 12/18/2025.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "content.h"
#include "parser.h"

void astParsePrint(ASTNode *parent) {
    nextPos();

    if ( isEnd() )
        return;

    if ( currentToken().type != TOK_PARENTESIS_OPEN ) {
        printf("Unexpected token to start: %d", currentToken().type);
        exit(EXIT_FAILURE);
    }
    nextPos();

    // parseContent( parent );

    Token *tokens = (Token *)malloc(sizeof(Token));
    int numTokens = 0;

    while (!isEnd() && currentToken().type != TOK_PARENTESIS_CLOSE) {
        tokens[numTokens++] = currentToken();
        nextPos();
    }

    if ( isEnd() ) {
        printf("Unexpected token to end");
        exit(EXIT_FAILURE);
    }

    parseContent( parent, tokens, numTokens );
    nextPos();
}
