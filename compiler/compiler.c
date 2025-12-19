//
// Created by MCris112 on 12/18/2025.
//

#include "compiler.h"

#include <stdio.h>

#include "../lexer/lexer.h"
#include "../parser/parser.h"

void runCompiler() {
    ASTNode root = getAST();

    showAST( &root );

    for (int i = 0; i < root.childCount; ++i) {
        ASTNode *child = &root.children[i];

        switch ( child->type ) {
            case AST_PRINT_STMT:
                printf("%s\n", child->children[0].text);
                break;
            default:
                fprintf(stderr, "Unknown AST node type (%s)\n", astNodeTypeToString(child->type));
                break;
        }
    }
}
