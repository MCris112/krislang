//
// Created by crisv on 12/24/2025.
//

#ifndef KRISLANG_ENVIRONMENT_H
#define KRISLANG_ENVIRONMENT_H
#include "../parser/parser.h"

// Count bits needed for a number
static int count_bits(int n) {
    if (n == 0) return 1;
    if (n < 0) n = -n;  // Handle negative numbers

    int bits = 0;
    while (n > 0) {
        bits++;
        n >>= 1;
    }
    return bits;
}

#define BITS(n) count_bits(n)

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
    ENV_TYPE_FUNCTION,
    ENV_TYPE_CLASS
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
    ASTBlock functions;
} EnvClassDefinition;

typedef struct {
    EnvironmentType type;
    char *name;

    union {
        struct {
            VarType type;
            EnvValue value;
            int size; //-1 autosize;
        } variable;

        EnvFunctionDefinition function;

        EnvClassDefinition class;
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

EnvValue *envDeclare(SymbolTable *variableTable, ASTNodeVariableDeclaration *node);

EnvValue *envVariableAssignment(SymbolTable *table, ASTNode *node);

void envDeclareFunction(SymbolTable *variableTable, ASTNode *node);

void freeSymbolTable(SymbolTable *table);

EnvValue envValueDeepCopy(EnvValue *src);

void envDeclareClass(SymbolTable *table, ASTClass classNode );

ASTNode *envGetClassMethod(EnvClassDefinition *cls, char *methodName);

#endif //KRISLANG_ENVIRONMENT_H
