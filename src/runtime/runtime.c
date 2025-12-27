//
// Created by crisv on 12/24/2025.
//

#include "runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "environment.h"
#include "../debug.h"
#include "../helpers/helper.h"


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
            printf("<%s>", parseEnvValueTypeToString(value->type));
            break;
    }

    printf("\n");
}

/**
 *
 *
 * @param table
 * @param node
 * @return
 */
EnvValue *runFunctionCall(SymbolTable *table, ASTNode *node) {
    printSymbolTable( table );

    char *name = node->funcCall.name;
    int argCount = node->funcCall.arguments.count;
    ASTNode **arguments = node->funcCall.arguments.children;

    // Check if is calling a variable as a function
    if ( strncmp(name, "$", 1) == 0) {
        printf("INSIDE VARIABLE CALLING \n");
        EnvValue *variable = envGetVariableValue(table, name);

        if ( variable->type != ENV_VOID ) {
            syntaxError(strFormat("Only variables VOID can be called on variable '%s'", name), beforeToken() );
        }

        printf("VARIABLE VALUE %s \n", variable->text);
        name = variable->text;
    }
    // ---------------------------
    // Built-in: print
    // ---------------------------
    if (strcmp(name, "print") == 0) {
        if (argCount < 1)
            syntaxError("print() expects at least 1 argument", beforeToken());

        printLiteral(runExpression(table, node->funcCall.arguments.children[0]));
        return NULL; // print never returns a value
    }

    // ---------------------------
    // Built-in: input
    // ---------------------------
    if (strcmp(name, "input") == 0) {
        // Optional prompt
        if (argCount > 0) {
            EnvValue *message = runExpression(table, node->funcCall.arguments.children[0]);
            if (message) printLiteral(message); // If dev set some text on it, show a print
        }
        // First args to show if it has shown on console


        // TODO check correct buffer
        char buffer[256];
        fflush(stdout);

        if (!fgets(buffer, sizeof(buffer), stdin)) {
            syntaxError("Someting happended on input", beforeToken());
        }

        buffer[strcspn(buffer, "\n")] = '\0'; // remove newline

        if (argCount > 1) {
            EnvValue *argType = runExpression(table, node->funcCall.arguments.children[1]);

            // TODO fix the error on console, this is not syntaxis error is input error or something like it
            switch (argType->type) {
                case ENV_STRING: return envValueString(buffer);
                case ENV_INT: {
                    char *end;
                    errno = 0;
                    long v = strtol(buffer, &end, 10);
                    if (end == buffer) { syntaxError("Expected integer input", currentToken()); }
                    if (errno == ERANGE) { syntaxError("Integer value out of range", currentToken()); }
                    if (*end != '\0') { syntaxError("Invalid characters after number", currentToken()); }
                    return envValueInt((int) v);
                }
                case ENV_FLOAT: {
                    char *end;
                    errno = 0;
                    double value = strtod(buffer, &end);
                    if (end == buffer) { syntaxError("Expected a floating-point number", currentToken()); }
                    if (errno == ERANGE) { syntaxError("Floating-point value out of range", currentToken()); }
                    if (*end != '\0') { syntaxError("Invalid characters after number", currentToken()); }
                    return envValueFloat(value);
                }
                case ENV_BOOL:
                    // 1. Direct boolean strings
                    if (strcmp(buffer, "1") == 0)
                        return envValueBoolean(true);

                    if (strcmp(buffer, "0") == 0)
                        return envValueBoolean(false);

                    if (strcmp(buffer, "true") == 0)
                        return envValueBoolean(true);

                    if (strcmp(buffer, "false") == 0)
                        return envValueBoolean(false);

                    // if is negative is false
                    char *end;
                    errno = 0;
                    long num = strtol(buffer, &end, 10);

                    if (end != buffer && *end == '\0' && errno == 0) {
                        // It's a valid integer
                        if (num < 0) return envValueBoolean(false); // negative → false
                        if (num > 0) return envValueBoolean(true); // positive → true

                        return envValueBoolean(false); // zero → false
                    }

                    // 3. Fallback: any non-empty string is true
                    if (buffer[0] != '\0' && buffer[0] != ' ')
                        return envValueBoolean(true);

                    return envValueBoolean(false);
                case ENV_CHAR:
                    // Empty input is not allower
                    if (buffer[0] == '\0') syntaxError("Expected a character, got empty input", beforeToken());

                    // more than one character is not allowed
                    if (buffer[1] != '\0')
                        syntaxError("Expected a character", beforeToken());
                    return envValueCharacter(buffer[0]);
                case ENV_NULL:
                    syntaxError("Expected a non-null argument", beforeToken());
                    break;
                case ENV_VOID:
                    syntaxError("Expected a non-void argument", beforeToken());
                    break;
                default: return envValueString(buffer);
            }
        }

        return envValueString(buffer);
    }

    // ---------------------------
    // User-defined function
    // ---------------------------
    EnvFunctionDefinition *definition = envGetFunction(table, name);

    int expected = definition->arguments->count;
    if (expected > argCount) {
        syntaxError(
            strFormat(
                "Function '%s' expects %d argment(s)",
                name,
                expected
            ),
            beforeToken());
    }

    if (expected < argCount) {
        syntaxError(strFormat("You're giving %d argment(s) more than need Function '%s' ",
                              argCount - expected, name),
                    beforeToken());
    }

    // Create function symbol table
    SymbolTable *funcTable = symbolTableFromParent(table);

    // Bind parameters
    for (int i = 0; i < expected; i++) {
        ASTNode *param = definition->arguments->children[i]; // AST_FUNCTION_PARAMETER
        ASTNode *argExpr = node->funcCall.arguments.children[i]; // expression

        // Evaluate argument expression
        EnvValue *value = runExpression(table, argExpr);

        if (!envIsVariableSameAsType(param->varDecl.varType, value->type)) {
            syntaxError(strFormat("Argument type mismatch for parameter '%s' in function '%s'", param->varDecl.name,
                                  name), beforeToken());
        }

        // Create a variable in the function scope
        Environment env = {
            .type = ENV_TYPE_VARIABLE,
            .name = strdup(param->varDecl.name),
            .variable = {
                .type = param->varDecl.varType,
                .value = envValueDeepCopy(value) // Safe to shallow‑copy, in nested functions
            }
        };

        symbolTableAddChild(funcTable, env);
    }

    // Execute function body
    EnvValue *ret = runBody(funcTable, definition->body, true);

    freeSymbolTable(funcTable); // forget the table after function
    return ret;
}

EnvValue *runExpression(SymbolTable *symbolTable, ASTNode *node) {
    if (!node) return envValueNull();

    switch (node->type) {
        // ============================
        // LITERALS
        // ============================
        case AST_TEXT: return envValueString(node->text);
        case AST_CHAR: return envValueCharacter(node->character);
        case AST_NUMBER: return envValueInt(node->number);
        case AST_NUMBER_DECIMAL: return envValueFloat(node->decimal);
        case AST_BOOLEAN: return envValueBoolean(node->boolean);
        case AST_FUNCTION_REFERENCE:
            Environment *value = envFind( symbolTable, ENV_TYPE_FUNCTION, node->text );

            if ( value == NULL ) {
                syntaxError(strFormat("The function '%s' is not definied", node->text), beforeToken() );
            }

            return envValueVoid(node->text);
        // ============================
        // VARIABLES
        // ============================
        case AST_VARIABLE_CAST:
            return envGetVariableValue(symbolTable, node->text);

        // ============================
        // UNARY
        // ============================
        case AST_UNARY: {
            EnvValue *value = runExpression(symbolTable, node->unary.operand);
            if (node->unary.operator == TOK_MINUS) {
                switch (value->type) {
                    case ENV_INT: return envValueInt(-value->number);
                    case ENV_FLOAT: return envValueFloat(-value->decimal);
                    // TODO SUPPORT ( ... )
                    default: syntaxError("Unary '-' can only be applied to numbers", currentToken());
                        return envValueNull();
                }
            }
            syntaxError("Unsupported unary operator", currentToken());
            return envValueNull();
        }

        // ============================
        // CONCAT (+)
        // ============================
        case AST_CONCAT: {
            EnvValue *left = runExpression(symbolTable, node->binary.left);
            EnvValue *right = runExpression(symbolTable, node->binary.right);

            // Numeric addition
            if ((left->type == ENV_INT || left->type == ENV_FLOAT) && (
                    right->type == ENV_INT || right->type == ENV_FLOAT)) {
                double a = (left->type == ENV_INT) ? left->number : left->decimal;
                double b = (right->type == ENV_INT) ? right->number : right->decimal;
                // If both are INT, return INT
                if (left->type == ENV_INT && right->type == ENV_INT)
                    return envValueInt((int) (a + b));
                // Otherwise return FLOAT
                return envValueFloat(a + b);
            }

            // String concatenation (convert both sides to string)
            char *A = envValueToString(left);
            char *B = envValueToString(right);
            size_t lenA = strlen(A);
            size_t lenB = strlen(B);
            char *buf = malloc(lenA + lenB + 1);
            memcpy(buf, A, lenA);
            memcpy(buf + lenA, B, lenB + 1);
            free(A);
            free(B);

            return envValueString(buf);
        }
        case AST_SUBTRACT: {
            EnvValue *left = runExpression(symbolTable, node->binary.left);
            EnvValue *right = runExpression(symbolTable, node->binary.right);

            if (!left || !right) {
                syntaxError("Invalid operands for subtraction", beforeToken());
            }

            // INT - INT
            if (left->type == ENV_INT && right->type == ENV_INT) {
                return envValueInt(left->number - right->number);
            }

            // FLOAT - FLOAT
            if (left->type == ENV_FLOAT && right->type == ENV_FLOAT) {
                return envValueFloat(left->decimal - right->decimal);
            }

            // INT - FLOAT
            if (left->type == ENV_INT && right->type == ENV_FLOAT) {
                return envValueFloat((double) left->number - right->decimal);
            }

            // FLOAT - INT
            if (left->type == ENV_FLOAT && right->type == ENV_INT) {
                return envValueFloat(left->decimal - (double) right->number);
            }

            syntaxError("Unsupported types for subtraction", beforeToken());
            return envValueNull();
        }

        case AST_FUNCTION_CALL:
            return runFunctionCall(symbolTable, node);

        // ============================
        // COMPARE (==)
        // ============================
        case AST_COMPARE:
            EnvValue *left = runExpression(symbolTable, node->binary.left);
            EnvValue *right = runExpression(symbolTable, node->binary.right);

            bool boolean = false;
            switch (left->type) {
                // -----------------------------
                // STRING == STRING
                // -----------------------------
                case ENV_STRING:
                    if (right->type == ENV_STRING) {
                        boolean = strcmp(left->text, right->text) == 0;
                        break;
                    }
                    syntaxError(strFormat("Cant compare STRING with %s", parseEnvValueTypeToString(right->type)),
                                currentToken());
                    break;

                // -----------------------------
                // INT == INT or INT == FLOAT
                // -----------------------------
                case ENV_INT: if (right->type == ENV_INT) {
                        boolean = left->number == right->number;
                        break;
                    }
                    if (right->type == ENV_FLOAT) {
                        boolean = (double) left->number == right->decimal;
                        break;
                    }
                    syntaxError(strFormat("Cant compare INT with %s", parseEnvValueTypeToString(right->type)),
                                currentToken());
                    break;

                // -----------------------------
                // FLOAT == FLOAT or FLOAT == INT
                // -----------------------------
                case ENV_FLOAT:
                    if (right->type == ENV_FLOAT) {
                        boolean = left->decimal == right->decimal;
                        break;
                    }
                    if (right->type == ENV_INT) {
                        boolean = left->decimal == (double) right->number;
                        break;
                    }
                    syntaxError(strFormat("Cant compare FLOAT with %s", parseEnvValueTypeToString(right->type)),
                                currentToken());
                    break;
                // -----------------------------
                //  BOOL == BOOL
                // -----------------------------
                case ENV_BOOL: if (right->type == ENV_BOOL) {
                        boolean = left->boolean == right->boolean;
                        break;
                    }
                    syntaxError(strFormat("Cant compare BOOLEAN with %s", parseEnvValueTypeToString(right->type)),
                                currentToken());
                    break;

                // -----------------------------
                // CHAR == CHAR
                // -----------------------------
                case ENV_CHAR: if (right->type == ENV_CHAR) {
                        boolean = left->character == right->character;
                        break;
                    }
                    syntaxError(strFormat("Cant compare CHAR with %s", parseEnvValueTypeToString(right->type)),
                                currentToken());
                    break;

                // -----------------------------
                // NULL comparisons (optional)
                // -----------------------------
                case ENV_NULL:
                    boolean = (right->type == ENV_NULL);
                    break;
                default:
                    syntaxError("Unsupported comparison type", currentToken());
                    break;
            }

            return envValueBoolean(boolean);
        case AST_TYPE_LITERAL:
            switch (node->literal.type) {
                // TODO more literals
                case AST_NUMBER: return envValueInt(0);
                case AST_NUMBER_DECIMAL: return envValueFloat(0);
                case AST_BOOLEAN: return envValueBoolean(false);
                case AST_CHAR: return envValueCharacter(0);
                case AST_NULL: return envValueNull();
                case AST_VOID: return envValueVoid("");
                case AST_UNKNOWN: syntaxError("Unknown Expression", currentToken());
                default: return envValueString("");
            }
            break;
        default:
            //TODO RETURN ERROR
            return envValueNull();
    }
}

bool runExpressionBoolean(SymbolTable *symbolTable, ASTNode *node) {
    EnvValue *value = runExpression(symbolTable, node);

    switch (value->type) {
        case ENV_STRING: return value->text != NULL && value->text[0] != '\0';
        case ENV_INT: return value->number != 0;
        case ENV_FLOAT: return value->decimal > 0; // Only floats are true if are over 0, anything negative is false
        case ENV_BOOL: return value->boolean;
        case ENV_CHAR: return value->character != '\0';
        case ENV_VOID: syntaxError("Is not possible to compare with a VOID", currentToken());
        default: return false; // ENV_NULL is also false
    }
}

EnvValue *runBody(SymbolTable *varTable, ASTBlock *block, bool insideFunction) {
    for (int i = 0; i < block->count; i++) {
        ASTNode *child = block->children[i];

        switch (child->type) {
            case AST_VARIABLE_DEFINITION:
                envDeclare(varTable, child);
                break;

            case AST_FUNCTION_DEFINITION:
                envDeclareFunction(varTable, child);
                break;

            case AST_FUNCTION_CALL: {
                EnvValue *v = runFunctionCall(varTable, child);
                if (insideFunction && v != NULL) return v;
                break;
            }

            case AST_RETURN:
                if (insideFunction) {
                    if (child->child == NULL) return NULL;
                    return runExpression(varTable, child->child);
                }

                // Top-level RETURN should NOT stop the program
                // Just ignore it
                return NULL;
            case AST_LOGICAL_IF: {
                if (runExpressionBoolean(varTable, child->logicalIf.conditional)) {
                    EnvValue *v = runBody(varTable, &child->logicalIf.bodyBlock, insideFunction);
                    if (insideFunction && v != NULL) return v;
                } else if (child->logicalIf.elseBlock.count > 0) {
                    EnvValue *v = runBody(varTable, &child->logicalIf.elseBlock, insideFunction);
                    if (insideFunction && v != NULL) return v;
                }
                break;
            }

            default:
                fprintf(stderr, "Unknown AST node type (%s)\n",
                        astNodeTypeToString(child->type));
                break;
        }
    }

    return NULL;
}


void runtime() {
    SymbolTable *variableTable;

    initSymbolTable(&variableTable);
    ASTNode root = getAST();


    parserPrintAST(&root);

    printf("\n\n\n\n\n");

    runBody(variableTable, &root.block, false);

    printSymbolTable(variableTable);
}
