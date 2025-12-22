//
// Created by MCris112 on 12/18/2025.
//

#include "compiler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "expression.h"
#include "../debug.h"
#include "../parser/parser.h"
#include "variables/variables.h"


SymbolTable *variableTable;

static char *join(const char *left, const char *right) {
    if (!left) left = strdup("");
    if (!right) right = strdup("");

    size_t na = strlen(left), nb = strlen(right);
    char *out = malloc(na + nb + 1);
    if (!out) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    memcpy(out, left, na);
    memcpy(out + na, right, nb + 1);

    free(left);
    free(right);
    return out;
}

// char *compileExpr(ASTNode *node) {
//     switch (node->type) {
//         case AST_TEXT:
//             return strdup(node->text ? node->text : "");
//
//         case AST_NUMBER: {
//             char buf[32];
//             snprintf(buf, sizeof(buf), "%d", node->number);
//             return strdup(buf);
//         }
//
//         case AST_CONCAT: {
//             char *left = compileExpr(node->binary.left);
//             char *right = compileExpr(node->binary.right);
//             return join(left, right);
//         }
//
//         case AST_VARIABLE_CAST: {
//             return getVariableValue(variableTable, node->text);
//         }
//         default:
//             return strdup("");
//     }
// }

void printLiteral(ASTNode *node) {
    switch (node->type) {

        case AST_TEXT:
            printf("%s", node->text);
            break;

        case AST_CHAR:
            printf("%c", node->text ? node->text[0] : '?');
            break;

        case AST_NUMBER:
            printf("%d", node->number);
            break;

        case AST_NUMBER_DECIMAL:
            printf("%f", node->decimal);
            break;

        case AST_BOOLEAN:
            printf(node->boolean ? "TRUE" : "FALSE");
            break;

        default:
            printf("<invalid>");
            break;
    }

    printf("\n");
}


void compileFunctionCall( SymbolTable *table, ASTNode *node ) {

    if ( strcmp( node->funcCall.name, "print") == 0) {
        ASTNode *result = compileExpression( table, node->funcCall.arguments[0] );

        printLiteral(result);
        return;
    }

    printf("Unknown function: %s\n", node->funcCall.name);
    // for (int i = 0; i < node->funcCall.count; ++i) {
    //
    // }
}


void runCompiler() {
    initSymbolTable(&variableTable);
    ASTNode root = getAST();

    parserPrintAST(&root);

    for (int i = 0; i < root.block.count; ++i) {
        ASTNode *child = root.block.children[i];

        switch (child->type) {
            case AST_VARIABLE_DEFINITION:
                declareVariableByASTNode(variableTable, child);
                break;
            case AST_FUNCTION_CALL:
                compileFunctionCall( variableTable, child);
                break;
            default:
                fprintf(stderr, "Unknown AST node type (%s)\n", astNodeTypeToString(child->type));
                break;
        }
    }

    printSymbolTable(variableTable);
}
