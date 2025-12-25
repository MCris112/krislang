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
    (*variableTable)->symbols = NULL;
    (*variableTable)->count = 0;
    (*variableTable)->capacity = 0;
}

char* envValueToString(EnvValue *v) {
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


void envDeclare(SymbolTable *variableTable, ASTNode *node) {
    VarType type = node->varDecl.varType;

    EnvValue *valueNode = runExpression( variableTable, node->varDecl.value);

    EnvValue value = {.type = valueNode->type};
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
            if (valueNode->type != ENV_BOOL)
                abort = true;
            else value.boolean = valueNode->boolean;
            break;
        case VARIABLE_TYPE_CHAR:
            if (valueNode->type != ENV_CHAR)
                abort = true;
            else value.text = strdup(valueNode->text);
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
            parseEnvValueTypeToString( valueNode->type )
            );
        exit(EXIT_FAILURE);
    }

    // Grow table if needed
    if (variableTable->count >= variableTable->capacity) {
        variableTable->capacity = variableTable->capacity ? variableTable->capacity * 2 : 8;
        variableTable->symbols = realloc(
            variableTable->symbols,
            variableTable->capacity * sizeof(Environment)
        );
    }

    // Assign symbol
    Environment symbol = {0};
    symbol.name = strdup(node->varDecl.name);
    symbol.type = type;
    symbol.value = value;

    variableTable->symbols[variableTable->count++] = symbol;
}


EnvValue *envGetValue(SymbolTable *variableTable, char *name) {
    for (int i = 0; i < variableTable->count; i++) {
        Environment *sym = &variableTable->symbols[i];
        if (strcmp(sym->name, name) == 0) {
            return &sym->value;
        }
    }
    return NULL;
}