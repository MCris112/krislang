//
// Created by MCris112 on 12/18/2025.
//

#include "parser.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../lexer/lexer.h"

char *astNodeTypeToString(ASTNodeType type){
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
    if (parent->childCount >= parent->childCapacity) {
        parent->childCapacity = parent->childCapacity ? parent->childCapacity * 2 : 4;
        parent->children = realloc(parent->children,
                                   parent->childCapacity * sizeof(ASTNode));
        if (!parent->children) {
            perror("realloc");
            exit(1);
        }
    }

    int index = parent->childCount++;
    parent->children[index] = child;
    return &parent->children[index];
}



ASTNode getAST() {

    ASTNode parent = {
        .type = AST_PROGRAM,
        .number = 0,
        .text = NULL,
        .children = NULL,
        .childCount = 0,
        .childCapacity = 0
    };

    const Token *tokens = getTokens();
    int count = getTokensCount();

    for (int i = 0; i < count; i++) {

        if (tokens[i].type == TOK_PRINT) {

            ASTNode *printNode = addASTNode(&parent, (ASTNode){
              .type = AST_PRINT_STMT,
              .number = tokens[i].number,
              .text = tokens[i].text,
              .children = NULL,
              .childCount = 0,
              .childCapacity = 0
      });

            astParsePrint(printNode);
        }

    }

    return parent;
}

void showASTNode(ASTNode *node, int indent) {
    // Print indentation
    for (int i = 0; i < indent; i++) {
        printf("  "); // two spaces per level
    }

    // Print node type
    printf("%s", astNodeTypeToString(node->type));

    // Print extra info depending on node type
    if (node->type == AST_TEXT && node->text) {
        printf(" (\"%s\")", node->text);
    }

    if (node->type == AST_NUMBER) {
        printf(" (%d)", node->number);
    }

    printf("\n");

    // Recursively print children
    for (int i = 0; i < node->childCount; i++) {
        showASTNode(&node->children[i], indent + 1);
    }
}

void showAST(ASTNode *root) {
    printf("=== AST ===\n");
    showASTNode(root, 0);
    printf("===========\n");
}
