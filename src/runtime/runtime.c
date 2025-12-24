//
// Created by crisv on 12/24/2025.
//

#include "runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "environment.h"


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
        EnvValue *result = runExpression(table, node->funcCall.arguments[0]);

        // TODO check correct buffer
        char buffer[256];

        // If dev set some text on it, show a print
        if (result != NULL)
            printLiteral(result);

        fflush(stdout);
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; // remove newline

        return envValueString( buffer );
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
                return envValueInt( left->number + right->number );
            }

            if (left->type == ENV_STRING && right->type == ENV_INT) {
                char buffer[64];
                snprintf(buffer, sizeof(buffer), "%s%d", left->text, right->number);
                return envValueString( buffer );
            }

            if (left->type == ENV_STRING && right->type == ENV_STRING) {

                // Getting size of each one
                size_t lenA = strlen(left->text);
                size_t lenB = strlen(right->text);

                // Creating a buffer to combine
                char *buf = malloc(lenA + lenB + 1);
                memcpy(buf, left->text, lenA);
                memcpy(buf + lenA, right->text, lenB + 1);

                return envValueString( buf );
            }

            if (left->type == ENV_STRING && right->type == ENV_CHAR) {

                // Getting size of each one
                size_t lenA = strlen(left->text);
                size_t lenB = strlen(right->text);

                // Creating a buffer to combine
                char *buf = malloc(lenA + lenB + 1);
                memcpy(buf, left->text, lenA);
                memcpy(buf + lenA, right->text, lenB + 1);

                return envValueString( buf);
            }


            if (left->type == ENV_INT && right->type == ENV_STRING) {
                char buffer[64];
                snprintf(buffer, sizeof(buffer), "%d%s", left->number, right->text);
                return envValueString( buffer );
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
                case AST_TEXT:
                    if (right->type == ENV_STRING ){
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
        case ENV_STRING:
            break;
        case ENV_BOOL:
            return node->boolean;
        default:
            return false;
    }

    return true;
}


void runBody(SymbolTable *varTable, ASTNode **children, int childrenLength) {
    for (int i = 0; i < childrenLength; ++i) {
        ASTNode *child = children[i];

        switch (child->type) {
            case AST_VARIABLE_DEFINITION:
                envDeclare(varTable, child);
                break;
            case AST_FUNCTION_CALL:
                runFunctionCall(varTable, child);
                break;
            case AST_LOGICAL_IF:
                if (runExpressionBoolean(varTable, child->logicalIf.conditional)) {
                    runBody(varTable, child->logicalIf.children, child->logicalIf.count);
                }
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

    runBody(variableTable, root.block.children, root.block.count);

    // printSymbolTable(variableTable);
}
