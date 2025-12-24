//
// Created by crisv on 12/24/2025.
//

#ifndef KRISLANG_ENVIRONMENT_H
#define KRISLANG_ENVIRONMENT_H
#include "../parser/parser.h"


//-------------------------------------------------
//
// VARIABLES
//
//-------------------------------------------------

typedef enum {
    ENV_STRING,
    ENV_INT,
    ENV_FLOAT,
    ENV_BOOL,
    ENV_CHAR,
    ENV_NULL
} EnvValueType;

typedef struct {
    EnvValueType type;

    union {
        char *text;
        int number;
        double decimal;
        bool boolean;
        char character;
    };
} EnvValue;

typedef struct {
    char *name;
    VarType type;
    EnvValue value;
} Environment;


typedef struct {
    Environment *symbols;
    int count;
    int capacity;
} SymbolTable;

EnvValue *envValueInt(int v);

EnvValue *envValueString(const char *s);

EnvValue *envValueFloat(double v);

EnvValue *envValueBoolean(bool v);

EnvValue *envValueCharacter(char v);

EnvValue *envValueNull(void);

void initSymbolTable(SymbolTable **variableTable);

EnvValue *envGetValue(SymbolTable *variableTable, char *name);

void envDeclare(SymbolTable *variableTable, ASTNode *node);

#endif //KRISLANG_ENVIRONMENT_H
