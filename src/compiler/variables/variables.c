//
// Created by crisv on 12/21/2025.
//

#include "variables.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void initSymbolTable(SymbolTable *variableTable) {
    variableTable = malloc(sizeof(SymbolTable));
    variableTable->symbols = NULL;
    variableTable->count = 0;
    variableTable->capacity = 0;
}

void declareVariableByASTNode(SymbolTable *variableTable, ASTNode *node) {
    VarType type = node->varDecl.varType;
    ASTNode *valueNode = node->varDecl.value;

    VarValue value = {.type = valueNode->type};
    bool abort = false;

    switch (type) {
        case VARIABLE_TYPE_STRING:
            if (valueNode->type != AST_TEXT)
                abort = true;
            else value.text = strdup(valueNode->text);
            break;
        case VARIABLE_TYPE_INT:
            if (valueNode->type != AST_NUMBER)
                abort = true;
            else value.number = valueNode->number;
            break;
        case VARIABLE_TYPE_FLOAT:
            if (valueNode->type != AST_NUMBER_DECIMAL)
                abort = true;
            else value.decimal = valueNode->decimal;
            break;
        case VARIABLE_TYPE_BOOLEAN:
            if (valueNode->type != AST_BOOLEAN)
                abort = true;
            else value.boolean = valueNode->boolean;
            break;
        case VARIABLE_TYPE_CHAR:
            if (valueNode->type != AST_CHAR)
                abort = true;
            else value.text = strdup(valueNode->text);
            break;
        default:
            abort = true;
            break;
    }

    if (abort) {
        fprintf(stderr, "Type mismatch for variable %s\n", node->varDecl.name);
        exit(EXIT_FAILURE);
    }

    // Grow table if needed
    if (variableTable->count >= variableTable->capacity) {
        variableTable->capacity = variableTable->capacity ? variableTable->capacity * 2 : 8;
        variableTable->symbols = realloc(
            variableTable->symbols,
            variableTable->capacity * sizeof(Symbol)
        );
    }

    // Assign symbol
    Symbol symbol = {0};
    symbol.name = strdup(node->varDecl.name);
    symbol.type = type;
    symbol.value = value;

    variableTable->symbols[variableTable->count++] = symbol;
}
//
// void declareVariable(const char *name, VarType type, VarValue value) {
//     // Type check
//     bool abort = false;
//
//     switch (type) {
//         case VARIABLE_TYPE_STRING:
//             if (value.type != AST_TEXT)
//                 abort = true;
//             break;
//         case VARIABLE_TYPE_INT:
//             if (value.type != AST_NUMBER)
//                 abort = true;
//             break;
//         case VARIABLE_TYPE_FLOAT:
//             if (value.type != AST_NUMBER_DECIMAL)
//                 abort = true;
//             break;
//         case VARIABLE_TYPE_BOOLEAN:
//             if (value.type != AST_BOOLEAN)
//                 abort = true;
//             break;
//         case VARIABLE_TYPE_CHAR:
//             if (value.type != AST_CHAR)
//                 abort = true;
//             break;
//         default:
//             break;
//     }
//
//     if (abort) {
//         fprintf(stderr, "Type mismatch for variable %s\n", name);
//         exit(EXIT_FAILURE);
//         return;
//     }
//
//     // Grow table if needed
//     if (variableTable->count >= variableTable->capacity) {
//         variableTable->capacity = variableTable->capacity ? variableTable->capacity * 2 : 8;
//         variableTable->symbols = realloc(
//             variableTable->symbols,
//             variableTable->capacity * sizeof(Symbol)
//         );
//     }
//
//     Symbol *sym = &variableTable->symbols[variableTable->count++];
//     sym->name = strdup(name);
//     sym->type = type;
//     sym->value = value;
// }

char *getVariableValue(SymbolTable *variableTable, char *name) {
    if (variableTable->count == 0) {
        return NULL;
    }

    for (int i = 0; i < variableTable->count; i++) {
        Symbol *sym = &variableTable->symbols[i];

        if (strcmp(sym->name, name) == 0) {
            // Convert VarValue → heap string, like compileExpr does
            if (sym->value.type == AST_TEXT) {
                return strdup(sym->value.text ? sym->value.text : "");
            }
            if (sym->value.type == AST_NUMBER) {
                char buf[32];
                snprintf(buf, sizeof(buf), "%d", sym->value.number);
                return strdup(buf);
            }
            return strdup("");
        }
    }

    // Not found
    return strdup(""); // or NULL, but then handle NULL in compileExpr
}
