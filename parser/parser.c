//
// Created by MCris112 on 12/18/2025.
//

#include "parser.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/lexer/lexer.h"

int syntax_error_count = 0;

void syntaxError(const char *message, int line, int column) {
    fprintf(stderr,
        "Syntax error at %d:%d - %s\n",
        line, column, message
    );
    syntax_error_count++;
}

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

    // is not block type
    // if (parent->type != AST_BLOCK && parent->type != AST_PROGRAM)
    //     abort();

    // Grow children array if needed
    if (parent->block.count >= parent->block.capacity) {
        parent->block.capacity = parent->block.capacity
            ? parent->block.capacity * 2
            : 4;

        parent->block.children = realloc(
            parent->block.children,
            parent->block.capacity * sizeof(ASTNode *)
        );

        if (!parent->block.children) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
    }

    // Allocate new node
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    *node = child;

    parent->block.children[parent->block.count++] = node;
    return node;
}


ASTNode getAST() {

    ASTNode parent = {
        .type = AST_PROGRAM,
        .block = {
            .children = NULL,
            .count = 0,
            .capacity = 0
        }
    };

    const Token *tokens = getTokens();
    int count = getTokensCount();

    while (!isEnd()) {
        Token tok = currentToken();

        // if (tok.type == TOK_PRINT) {
        //
        //     ASTNode *printNode = addASTNode(&parent, (ASTNode){
        //       .type = AST_PRINT_STMT,
        //     });
        //
        //     astParsePrint(printNode);
        // }

        if ( tok.type == TOK_VARIABLE_TYPE_INT ||   tok.type == TOK_VARIABLE_TYPE_STRING) {

            Token varType  = currentToken(); nextPos();
            Token varName  = currentToken(); nextPos();
            Token equals   = currentToken(); nextPos();
            Token varValue = currentToken(); nextPos();

            if (varName.type != TOK_VARIABLE || equals.type != TOK_EQUALS) {
                syntaxError( "Variable name expected", varName.line, varName.column );
                continue;
            }

            ASTNode *valueNode = malloc(sizeof(ASTNode));
            if (!valueNode) { perror("malloc"); exit(EXIT_FAILURE); }

            if (varType.type == TOK_VARIABLE_TYPE_STRING) {
                *valueNode = (ASTNode){
                    .type = AST_TEXT,
                    .text = varValue.text
                };
            } else {
                *valueNode = (ASTNode){
                    .type = AST_NUMBER,
                    .number = varValue.number
                };
            }

            ASTNode definition = {
                .type = AST_VARIABLE_DEFINITION,
                .varDecl = {
                    .varType = (varType.type == TOK_VARIABLE_TYPE_STRING)
                                 ? VARIABLE_TYPE_STRING
                                 : VARIABLE_TYPE_INT,
                    .name = varName.text,
                    .value = valueNode
                }
            };

            addASTNode(&parent, definition);
            continue;
            // if ( tokens[getASTPosition()+1].type != TOK_VARIABLE ) {
            //     perror("Is type and?");
            //     abort();
            // }
            //
            // if ( tokens[getASTPosition()+2].type != TOK_EQUALS ) {
            //     perror("Variable is not set");
            //     abort();
            // }
            //
            //
            // Token varType = tokens[getASTPosition()];
            // Token varName = tokens[getASTPosition()+1];
            // nextPos();
            // Token varValue = tokens[getASTPosition()+3];
            // nextPos();
            //
            // ASTNode *valueNode = malloc(sizeof(ASTNode));
            //
            // if (varType.type == TOK_VARIABLE_TYPE_STRING) {
            //     *valueNode = (ASTNode){
            //         .type = AST_TEXT,
            //         .text = varValue.text
            //     };
            // } else {
            //     *valueNode = (ASTNode){
            //         .type = AST_NUMBER,
            //         .number = varValue.number
            //     };
            // }
            //
            //
            // ASTNode definition = {
            //     .type = AST_VARIABLE_DEFINITION,
            //     .varDecl = {
            //         .varType = (varType.type == TOK_VARIABLE_TYPE_STRING)
            //                      ? VARIABLE_TYPE_STRING
            //                      : VARIABLE_TYPE_INT,
            //         .name = varName.text,
            //         .value = valueNode   // ✅ ASTNode*
            //     }
            // };
            //
            //
            // addASTNode(&parent, definition);

            // VarValue *content;
            //
            // if ( varType.type == TOK_VARIABLE_TYPE_STRING ) {
            //     content = malloc( sizeof(VarValue) );
            //     content->type = AST_TEXT;
            //     content->text = varvalue.text;
            //
            //     declareVariable( varname.text, VARIABLE_TYPE_STRING, *content );
            // }
            //
            // if ( varType.type == TOK_VARIABLE_TYPE_INT ) {
            //     content = malloc( sizeof(VarValue) );
            //     content->type = AST_NUMBER;
            //     content->integer = varvalue.number;
            //
            //     declareVariable( varname.text, VARIABLE_TYPE_INT, *content );
            //
            // }
            //
            // if ( !content ) {
            //     perror("Variable is not definied");
            //     abort();
            // }


            continue;
        }

        nextPos();
    }

    if (syntax_error_count > 0) {
        fprintf(stderr,
            "\nCompilation failed with %d syntax error(s).\n",
            syntax_error_count
        );
        exit(EXIT_FAILURE);
    }

    return parent;
}
void showASTNode(ASTNode *node, int indent) {
    // Print indentation
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }

    // Print node type
    printf("%s", astNodeTypeToString(node->type));

    // Extra info
    if (node->type == AST_TEXT && node->text) {
        printf(" (\"%s\")", node->text);
    }

    if (node->type == AST_NUMBER) {
        printf(" (%d)", node->number);
    }

    if (node->type == AST_VARIABLE_DEFINITION) {
        printf(" (%s)", node->varDecl.name);
    }

    printf("\n");

    // Recurse depending on node type
    switch (node->type) {

        case AST_PROGRAM:
        case AST_PRINT_STMT:
            for (int i = 0; i < node->block.count; i++) {
                showASTNode(node->block.children[i], indent + 1);
            }
            break;

        case AST_VARIABLE_DEFINITION:
            if (node->varDecl.value) {
                showASTNode(node->varDecl.value, indent + 1);
            }
            break;

        default:
            // Leaf nodes: nothing to recurse into
            break;
    }
}


void showAST(ASTNode *root) {
    printf("=== AST ===\n");
    showASTNode(root, 0);
    printf("===========\n");
}
