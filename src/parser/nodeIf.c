//
// Created by crisv on 12/24/2025.
//

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "../debug.h"

void parseNodeIf(ASTBlock *parent) {
    nextPos(); // Skip TOK_LOGICAL_IF

    ASTNode nodeIf = {0};
    nodeIf.type = AST_LOGICAL_IF;
    // ALWAYS initialize both blocks
    nodeIf.logicalIf.bodyBlock = (ASTBlock){.children = NULL, .count = 0, .capacity = 0};
    nodeIf.logicalIf.elseBlock = (ASTBlock){.children = NULL, .count = 0, .capacity = 0};

    if (currentToken().type != TOK_PARENTHESIS_OPEN) {
        syntaxError("Expected '(' to start if", beforeToken() );
    }

    nextPos();

    ASTNode *expression = parseExpression(0);

    nodeIf.logicalIf.conditional = expression;

    if (currentToken().type != TOK_PARENTHESIS_CLOSE) {
        syntaxError("Expected ')' after expression", currentToken());
    }

    nextPos(); // Skip ')'

    if (currentToken().type != TOK_BRACE_OPEN) {
        syntaxError("Expected '{' after expression", currentToken());
    }

    nextPos(); // Skipp {

    parseBody(&nodeIf.logicalIf.bodyBlock);

    if (currentToken().type != TOK_BRACE_CLOSE) {
        syntaxError("Expected '}' after body content", currentToken());
    }

    nextPos(); // Skipp }

    if (currentToken().type == TOK_LOGICAL_ELSE) {
        nextPos();

        if ( currentToken().type == TOK_LOGICAL_IF ) {
            parseNodeIf( &nodeIf.logicalIf.elseBlock );
            addASTNode(parent, nodeIf);
            return;
        }
        if (currentToken().type != TOK_BRACE_OPEN) {
            syntaxError("Expected '{' after expression", currentToken());
        }

        nextPos(); // Skipp {

        parseBody(&nodeIf.logicalIf.elseBlock);

        if (currentToken().type != TOK_BRACE_CLOSE) {
            syntaxError("Expected '}' after body content", currentToken());
        }

        nextPos(); // Skipp }

    }

    addASTNode(parent, nodeIf);
}
