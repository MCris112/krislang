//
// Created by crisv on 12/24/2025.
//

#include "runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "environment.h"
#include "../debug.h"


//-----------------------------------
// Compile
//-----------------------------------
void printLiteral(EnvValue *value) {
    switch (value->type) {
        case ENV_STRING:
            printf("%s", value->text);
            break;

        case ENV_CHAR:
            printf("%c", value->text ? value->text[0] : '?');
            break;

        case ENV_INT:
            printf("%d", value->number);
            break;

        case ENV_FLOAT:
            printf("%f", value->decimal);
            break;

        case ENV_BOOL:
            printf(value->boolean ? "TRUE" : "FALSE");
            break;

        default:
            printf("<INVALID>");
            break;
    }

    printf("\n");
}

EnvValue *runFunctionCall(SymbolTable *table, ASTNode *node) {
    if (strcmp(node->funcCall.name, "print") == 0) {
        EnvValue *result = runExpression(table, node->funcCall.arguments[0]);

        printLiteral(result);
        return envValueNull();
    }

    if (strcmp(node->funcCall.name, "input") == 0) {
        // First args to show if has show on console
        EnvValue *result = runExpression(table, node->funcCall.arguments[0]);

        // TODO check correct buffer
        char buffer[256];

        // If dev set some text on it, show a print
        if (result != NULL)
            printLiteral(result);

        fflush(stdout);
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; // remove newline

        if ( node->funcCall.count > 1 ) {
            EnvValue *argType = runExpression( table, node->funcCall.arguments[1]);

            switch ( argType->type ) {
                case ENV_STRING:
                    break;
                case ENV_INT:
                    return envValueInt( buffer );
            }
        }
        return envValueString(buffer);
    }

    printf("Unknown function: %s\n", node->funcCall.name);

    return envValueNull();
}

EnvValue *runExpression(SymbolTable *symbolTable, ASTNode *node) {
    if (!node) return envValueNull();

    switch (node->type) {
        case AST_TEXT: return envValueString(node->text);
        case AST_CHAR: return envValueCharacter(node->text[0]);
        case AST_NUMBER: return envValueInt(node->number);
        case AST_NUMBER_DECIMAL: return envValueFloat(node->decimal);
        case AST_BOOLEAN: return envValueBoolean(node->boolean);

        case AST_VARIABLE_CAST:
            return envGetValue(symbolTable, node->text);
        case AST_CONCAT: {
            EnvValue *left = runExpression(symbolTable, node->binary.left);
            EnvValue *right = runExpression(symbolTable, node->binary.right);

            if (left->type == ENV_INT && right->type == ENV_INT) {
                return envValueInt(left->number + right->number);
            }

            if (left->type == ENV_STRING && right->type == ENV_INT) {
                char buffer[64];
                snprintf(buffer, sizeof(buffer), "%s%d", left->text, right->number);
                return envValueString(buffer);
            }

            if (left->type == ENV_STRING && right->type == ENV_STRING) {
                // Getting size of each one
                size_t lenA = strlen(left->text);
                size_t lenB = strlen(right->text);

                // Creating a buffer to combine
                char *buf = malloc(lenA + lenB + 1);
                memcpy(buf, left->text, lenA);
                memcpy(buf + lenA, right->text, lenB + 1);

                return envValueString(buf);
            }

            if (left->type == ENV_STRING && right->type == ENV_CHAR) {
                // Getting size of each one
                size_t lenA = strlen(left->text);
                size_t lenB = strlen(right->text);

                // Creating a buffer to combine
                char *buf = malloc(lenA + lenB + 1);
                memcpy(buf, left->text, lenA);
                memcpy(buf + lenA, right->text, lenB + 1);

                return envValueString(buf);
            }


            if (left->type == ENV_INT && right->type == ENV_STRING) {
                char buffer[64];
                snprintf(buffer, sizeof(buffer), "%d%s", left->number, right->text);
                return envValueString(buffer);
            }
            // TODO: handle string concat, variable concat, etc.

            // Unsupported combination
            return envValueNull();
        }

        case AST_FUNCTION_CALL:
            return runFunctionCall(symbolTable, node);
        case AST_COMPARE:
            EnvValue *left = runExpression(symbolTable, node->binary.left);
            EnvValue *right = runExpression(symbolTable, node->binary.right);

            bool boolean = false;
            switch (left->type) {
                case ENV_STRING:
                    if (right->type == ENV_STRING) {
                        boolean = strcmp(left->text, right->text) == 0;
                    }
                    break;
                default:
                    break;
            }

            return envValueBoolean(boolean);
        default:
            //TODO RETURN ERRO
            return envValueNull();
    }
}

bool runExpressionBoolean(SymbolTable *symbolTable, ASTNode *node) {
    EnvValue *value = runExpression(symbolTable, node);

    switch (value->type) {
        case ENV_BOOL: return value->boolean;
        case ENV_INT: return value->number != 0;
        case ENV_STRING: return value->text && value->text[0] != '\0';
        default: return false;
    }
}


void runBody(SymbolTable *varTable, ASTBlock *block) {
    for (int i = 0; i < block->count; ++i) {
        ASTNode *child = block->children[i];

        switch (child->type) {
            case AST_VARIABLE_DEFINITION:
                envDeclare(varTable, child);
                break;
            case AST_FUNCTION_CALL:
                runFunctionCall(varTable, child);
                break;
            case AST_LOGICAL_IF:
                if (runExpressionBoolean(varTable, child->logicalIf.conditional)) {
                    runBody(varTable, &child->logicalIf.bodyBlock);
                } else if ( child->logicalIf.elseBlock.count > 0 ) { runBody(varTable, &child->logicalIf.elseBlock); }
                break;
            default:
                fprintf(stderr, "Unknown AST node type (%s)\n", astNodeTypeToString(child->type));
                break;
        }
    }
}

void runtime() {
    SymbolTable *variableTable;

    initSymbolTable(&variableTable);
    ASTNode root = getAST();

    // parserPrintAST(&root);

    printf("\n\n\n\n\n");

    runBody(variableTable, &root.block);

    printSymbolTable(variableTable);
}
