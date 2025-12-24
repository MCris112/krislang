//
// Created by crisv on 12/24/2025.
//

#include <stdlib.h>

#include "parser.h"

void parseNodeIf(ASTNode **parent) {
    nextPos(); // Skip TOK_LOGICAL_IF

    ASTNode *nodeIf = malloc(sizeof(ASTNode));
    nodeIf->type = AST_LOGICAL_IF;

    if (currentToken().type != TOK_PARENTHESIS_OPEN) {
        syntaxError("Expected '(' to start if", currentToken());
        return;
    }

    nextPos();

    ASTNode *expression = parseExpression(0);
    nodeIf->logicalIf.conditional = expression;

    if (currentToken().type != TOK_PARENTHESIS_CLOSE) {
        syntaxError("Expected ')' after expression", currentToken());
        return;
    }

    nextPos(); // Skip ')'

    if ( currentToken().type != TOK_BRACE_OPEN ) {
        syntaxError("Expected '{' after expression", currentToken());
        return;
    }

    nextPos(); // Skipp {

    parseBody( &nodeIf );

    nextPos(); // Skipp }

    addASTNode( *parent, *nodeIf);
}
