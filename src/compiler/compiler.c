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



void compileBody(SymbolTable *varTable, ASTNode **children, int childrenLength ) {
    for (int i = 0; i < childrenLength; ++i) {
        ASTNode *child = children[i];

        switch (child->type) {
            case AST_VARIABLE_DEFINITION:
                declareVariableByASTNode(varTable, child);
                break;
            case AST_FUNCTION_CALL:
                compileFunctionCall( varTable, child);
                break;
            case AST_LOGICAL_IF:
                if ( compileExpressionBoolean( varTable, child->logicalIf.conditional ) ) {
                    compileBody( varTable, child->logicalIf.children, child->logicalIf.count );
                }
                break;
            default:
                fprintf(stderr, "Unknown AST node type (%s)\n", astNodeTypeToString(child->type));
                break;
        }
    }
}

void runCompiler() {
    initSymbolTable(&variableTable);
    ASTNode root = getAST();

    // parserPrintAST(&root);

    compileBody( variableTable, root.block.children, root.block.count );

    // printSymbolTable(variableTable);
}
