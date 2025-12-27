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
    ENV_VOID,
    ENV_NULL
} EnvValueType;

typedef enum {
    ENV_TYPE_VARIABLE,
    ENV_TYPE_FUNCTION
} EnvironmentType;

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
    ASTBlock *body;
    ASTFunctionArguments *arguments;
} EnvFunctionDefinition;

typedef struct {
    EnvironmentType type;
    char *name;

    union {
        struct {
            VarType type;
            EnvValue value;
        } variable;

        EnvFunctionDefinition function;
    };

} Environment;


typedef struct SymbolTable {
    struct SymbolTable *parent; // in case exists

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

EnvValue *envValueVoid(char *name);

char* envValueToString(EnvValue *v);

Environment *envFind(SymbolTable *table, EnvironmentType type, const char *name);

bool envIsVariableSameAsType(VarType variable, EnvValueType env);

void symbolTableAddChild(SymbolTable *symbolTable, Environment environment);

void initSymbolTable(SymbolTable **variableTable);

SymbolTable *symbolTableFromParent(SymbolTable *parent);

EnvValue *envGetVariableValue(SymbolTable *variableTable, char *name);

EnvFunctionDefinition *envGetFunction(SymbolTable *variableTable, char *name);

void envDeclare(SymbolTable *variableTable, ASTNode *node);

EnvValue *envVariableAssignment(SymbolTable *table, ASTNode *node);

void envDeclareFunction(SymbolTable *variableTable, ASTNode *node);

void freeSymbolTable(SymbolTable *table);

EnvValue envValueDeepCopy(EnvValue *src);

#endif //KRISLANG_ENVIRONMENT_H
