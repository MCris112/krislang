//
// Created by crisv on 12/27/2025.
//

#include <string.h>

#include "environment.h"
#include "runtime.h"
#include "../debug.h"
#include "../helpers/helper.h"

bool runCompareEquals(EnvValue *left, EnvValue *right) {
    switch (left->type) {
        // -----------------------------
        // STRING == STRING
        // -----------------------------
        case ENV_STRING:
            if (right->type == ENV_STRING) {
                return strcmp(left->text, right->text) == 0;
            }
            syntaxError(strFormat("Cant compare STRING with %s", parseEnvValueTypeToString(right->type)),
                        currentToken());
            break;

        // -----------------------------
        // INT == INT or INT == FLOAT
        // -----------------------------
        case ENV_INT: if (right->type == ENV_INT) {
                return left->number == right->number;
            }
            if (right->type == ENV_FLOAT) {
                return (double) left->number == right->decimal;
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
                return left->decimal == right->decimal;
                break;
            }
            if (right->type == ENV_INT) {
                return left->decimal == (double) right->number;
                break;
            }
            syntaxError(strFormat("Cant compare FLOAT with %s", parseEnvValueTypeToString(right->type)),
                        currentToken());
            break;
        // -----------------------------
        //  BOOL == BOOL
        // -----------------------------
        case ENV_BOOL: if (right->type == ENV_BOOL) {
                return left->boolean == right->boolean;
                break;
            }
            syntaxError(strFormat("Cant compare BOOLEAN with %s", parseEnvValueTypeToString(right->type)),
                        currentToken());
            break;

        // -----------------------------
        // CHAR == CHAR
        // -----------------------------
        case ENV_CHAR: if (right->type == ENV_CHAR) {
                return left->character == right->character;
                break;
            }

            // TODO Thinking if i let it by default or not... for now i like it
            if (right->type == ENV_INT) {
                return left->character == right->number;
            }

            if (right->type == ENV_FLOAT) {
                return (double) left->character == right->decimal;
            }
            syntaxError(strFormat("Cant compare CHAR with %s", parseEnvValueTypeToString(right->type)),
                        currentToken());
            break;

        // -----------------------------
        // NULL comparisons (optional)
        // -----------------------------
        case ENV_NULL:
            return (right->type == ENV_NULL);
        default:
            syntaxError("Unsupported comparison type", currentToken());
            return false;
    }

    return false; // unreachable avoid warnings
}

bool runCompareLessThan(EnvValue *left, EnvValue *right) {
    // STRING < STRING uses lexicographical comparison
    if (left->type == ENV_STRING && right->type == ENV_STRING) {
        return strcmp(left->text, right->text) < 0;
    }

    // Numeric mixed comparisons
    if (left->type == ENV_INT && right->type == ENV_FLOAT) {
        return (double) left->number < right->decimal;
    }

    if (left->type == ENV_FLOAT && right->type == ENV_INT) {
        return left->decimal < (double) right->number;
    }

    // Numeric same-type comparisons
    if (left->type == ENV_INT && right->type == ENV_INT) {
        return left->number < right->number;
    }

    if (left->type == ENV_FLOAT && right->type == ENV_FLOAT) {
        return left->decimal < right->decimal;
    }

    // CHAR < CHAR
    if (left->type == ENV_CHAR && right->type == ENV_CHAR)
        return left->character < right->character;

    // BOOL < BOOL (TRUE > FALSE)
    if (left->type == ENV_BOOL && right->type == ENV_BOOL)
        return left->boolean < right->boolean;

    if (left->type == ENV_BOOL && right->type != ENV_BOOL)
        syntaxError("Cannot compare BOOLEAN with non-BOOLEAN", currentToken());

    if (left->type == ENV_CHAR && right->type == ENV_INT)
        return left->character < right->number;

    if (left->type == ENV_CHAR && right->type == ENV_FLOAT)
        return left->character < right->decimal;


    syntaxError("Unsupported comparison type", currentToken());
    return false;
}

EnvValue *runCompare(SymbolTable *symbolTable, ASTCompare node) {
    EnvValue *left = runExpression(symbolTable, node.left);
    EnvValue *right = runExpression(symbolTable, node.right);

    bool boolean = false;

    switch (node.operator) {
        case TOK_LESS_THAN:
            boolean = runCompareLessThan(left, right);
            break;
        case TOK_LESS_EQUAL:
            if (runCompareLessThan(left, right))
                boolean = true;
            else
                boolean = runCompareEquals(left, right);
            break;
        case TOK_GREATER_THAN:
            boolean = runCompareLessThan(right, left);
            break;
        case TOK_GREATER_EQUAL:
            if (runCompareLessThan(right, left))
                boolean = true;
            else
                boolean = runCompareEquals( right, left );
            break;
        case TOK_EQUAL_EQUAL:
            boolean = runCompareEquals(left, right);
            break;
        case TOK_NOT_EQUAL:
            boolean = !runCompareEquals(left, right);
            break;
        default:
            syntaxError("No valid comparator", beforeToken());
            break;
    }

    return envValueBoolean(boolean);
}
