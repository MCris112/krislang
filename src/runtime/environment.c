//
// Created by crisv on 12/24/2025.
//

#include "environment.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "runtime.h"
#include "../debug.h"
#include "../helpers/helper.h"

EnvValue *envValueInt(int v) {
    EnvValue *val = malloc(sizeof(EnvValue));
    val->type = ENV_INT;
    val->number = v;
    return val;
}

EnvValue *envValueString(const char *s) {
    EnvValue *val = malloc(sizeof(EnvValue));
    val->type = ENV_STRING;
    val->text = strdup(s);
    return val;
}

EnvValue *envValueFloat(double v) {
    EnvValue *val = malloc(sizeof(EnvValue));
    val->type = ENV_FLOAT;
    val->decimal = v;
    return val;
}

EnvValue *envValueBoolean(bool v) {
    EnvValue *val = malloc(sizeof(EnvValue));
    val->type = ENV_BOOL;
    val->boolean = v;
    return val;
}

EnvValue *envValueCharacter(char v) {
    EnvValue *val = malloc(sizeof(EnvValue));
    val->type = ENV_CHAR;
    val->character = v;
    return val;
}

EnvValue *envValueNull() {
    EnvValue *val = malloc(sizeof(EnvValue));
    val->type = ENV_NULL;
    return val;
}

EnvValue *envValueVoid() {
    EnvValue *val = malloc(sizeof(EnvValue));
    val->type = ENV_VOID;
    return val;
}

void initSymbolTable(SymbolTable **variableTable) {
    *variableTable = malloc(sizeof(SymbolTable));

    if (!*variableTable) {
        fprintf(stderr, "Out of memory allocating SymbolTable\n");
        exit(EXIT_FAILURE);
    }

    (*variableTable)->parent = NULL;
    (*variableTable)->symbols = NULL;
    (*variableTable)->count = 0;
    (*variableTable)->capacity = 0;
}

SymbolTable *symbolTableFromParent(SymbolTable *parent) {
    SymbolTable *symbolTable = NULL;

    initSymbolTable(&symbolTable);

    symbolTable->parent = parent;

    return symbolTable;
}

Environment *envFind(SymbolTable *table, EnvironmentType type, const char *name) {
    for (int i = 0; i < table->count; i++) {
        Environment *sym = &table->symbols[i];
        if (sym->type == type && strcmp(sym->name, name) == 0) {
            return sym;
        }
    }

    if ( table->parent != NULL ) {
        return envFind(table->parent, type, name);
    }

    return NULL; // SAFE: no error
}


char *envValueToString(EnvValue *v) {
    char buffer[64];

    switch (v->type) {
        case ENV_STRING:
            return strdup(v->text ? v->text : "");

        case ENV_INT:
            snprintf(buffer, sizeof(buffer), "%d", v->number);
            return strdup(buffer);

        case ENV_FLOAT:
            snprintf(buffer, sizeof(buffer), "%.6f", v->decimal);
            return strdup(buffer);

        case ENV_BOOL:
            return strdup(v->boolean ? "TRUE" : "FALSE");

        case ENV_CHAR: {
            char *buf = malloc(2);
            buf[0] = v->character;
            buf[1] = '\0';
            return buf;
        }

        case ENV_NULL:
            return strdup("");

        case ENV_VOID:
            syntaxError("Cannot convert VOID to string", currentToken());
            return strdup("");

        default:
            return strdup("<invalid>");
    }
}

void symbolTableAddChild(SymbolTable *symbolTable, Environment environment) {
    // Grow table if needed
    if (symbolTable->count >= symbolTable->capacity) {
        symbolTable->capacity = symbolTable->capacity ? symbolTable->capacity * 2 : 8;
        symbolTable->symbols = realloc(
            symbolTable->symbols,
            symbolTable->capacity * sizeof(Environment)
        );
    }

    symbolTable->symbols[symbolTable->count++] = environment;
}

void envDeclare(SymbolTable *variableTable, ASTNode *node) {
    if (envFind(variableTable, ENV_TYPE_VARIABLE, node->varDecl.name)) {
        syntaxError("Variable already defined", getTokens()[0]);
        return;
    }

    VarType type = node->varDecl.varType;

    EnvValue *valueNode = runExpression(variableTable, node->varDecl.value);

    // Avoid garbage memory
    EnvValue value = {0};
    value.type = valueNode->type;

    bool abort = false;

    switch (type) {
        case VARIABLE_TYPE_STRING:
            if (valueNode->type != ENV_STRING)
                abort = true;
            else value.text = strdup(valueNode->text);
            break;
        case VARIABLE_TYPE_INT:
            if (valueNode->type != ENV_INT)
                abort = true;
            else value.number = valueNode->number;
            break;
        case VARIABLE_TYPE_FLOAT:
            if (valueNode->type != ENV_FLOAT)
                abort = true;
            else value.decimal = valueNode->decimal;
            break;
        case VARIABLE_TYPE_BOOLEAN:
            printf("\n\n\n======= IN BOOLEAN ======\n\n\n");
            if (valueNode->type != ENV_BOOL)
                abort = true;
            else value.boolean = valueNode->boolean;
            break;
        case VARIABLE_TYPE_CHAR:
            if (valueNode->type != ENV_CHAR)
                abort = true;
            else value.character = valueNode->character;
            break;
        default:
            abort = true;
            break;
    }

    if (abort) {
        fprintf(
            stderr,
            "error: type mismatch in variable assignment\n  %s > expected %s\n  received %s",
            node->varDecl.name,
            parserVarTypeToString(type),
            parseEnvValueTypeToString(valueNode->type)
        );
        exit(EXIT_FAILURE);
    }


    // Assign symbol
    Environment symbol = (Environment){
        .type = ENV_TYPE_VARIABLE,
        .name = strdup(node->varDecl.name),
        .variable = {
            .type = type,
            .value = value
        }
    };

    symbolTableAddChild(variableTable, symbol);
}

void envDeclareFunction(SymbolTable *symbolTable, ASTNode *node) {
    if (node->type != AST_FUNCTION_DEFINITION) {
        syntaxError("EXPECTED FUNCTION DEFINITION", currentToken());
    }

    Environment environment = (Environment){
        .type = ENV_TYPE_FUNCTION,
        .name = node->funcDefinition.name,
        .function = {
            .body = &node->funcDefinition.body,
            .arguments = &node->funcDefinition.arguments,
        }
    };

    symbolTableAddChild(symbolTable, environment);
}

EnvValue *envGetVariableValue(SymbolTable *variableTable, char *name) {
    Environment *value = envFind(variableTable, ENV_TYPE_VARIABLE, name);

    if (value == NULL) {
        syntaxError(strFormat("Error Variable '%s' is UNDEFINIED", name), beforeToken());
        return envValueNull(); // unreachable, but avoids warnings
    }

    return &value->variable.value;
}

EnvFunctionDefinition *envGetFunction(SymbolTable *variableTable, char *name) {
    Environment *environment = envFind(variableTable, ENV_TYPE_FUNCTION, name);

    if (environment == NULL) {
        syntaxError(strFormat("Unknow Function: %s", name), beforeToken());
        return NULL; // Avoid warnings
    }

    return &environment->function;
}

void freeSymbolTable(SymbolTable *table) {
    if (!table) return;

    // Free each symbol
    for (int i = 0; i < table->count; i++) {
        Environment *sym = &table->symbols[i];

        // Free the symbol name
        if (sym->name) {
            free(sym->name);
        }

        // Free variable values
        if (sym->type == ENV_TYPE_VARIABLE) {
            EnvValue *v = &sym->variable.value;

            if (v->type == ENV_STRING && v->text) {
                free(v->text);
            }
        }

        // Functions: do NOT free AST nodes
        // They belong to the parser and live for the whole program
    }

    // Free the symbol array
    if (table->symbols) {
        free(table->symbols);
    }

    // Free the table itself
    free(table);
}
