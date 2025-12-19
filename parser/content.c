//
// Created by MCris112 on 12/18/2025.
//

#include "content.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parseContent( ASTNode *parent, Token *children, int numChild ) {

    // if (numChild == 0) {
    //     addASTNode(parent, (ASTNode){
    //         .type = AST_ERROR
    //     });
    //     return;
    // }
    //
    // // Start with the first value
    // ASTNode *left = NULL;
    //
    // // Parse first token
    // if (children[0].type == TOK_TEXT) {
    //     left = addASTNode(parent, (ASTNode){
    //         .type = AST_TEXT,
    //         .text = strdup(children[0].text)
    //     });
    // }
    //
    // // Now parse the rest
    // for (int i = 1; i < numChild; i += 2) {
    //
    //     if (children[i].type != TOK_SUM) {
    //         printf("Unexpected token in expression\n");
    //         exit(1);
    //     }
    //
    //     // Right operand
    //     ASTNode right = {0};
    //
    //     if (children[i+1].type == TOK_TEXT) {
    //         right = (ASTNode){
    //             .type = AST_TEXT,
    //             .text = strdup(children[i+1].text)
    //         };
    //     }
    //
    //     // Build CONCAT node
    //     ASTNode *concat = addASTNode(parent, (ASTNode){
    //         .type = AST_CONCAT
    //     });
    //
    //     // Add left and right children
    //     addASTNode(concat, *left);
    //     addASTNode(concat, right);
    //
    //     // The result becomes the new left
    //     left = concat;
    // }
}

