//
// Created by MCris112 on 12/18/2025.
//

#include "compiler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/lexer/lexer.h"
#include "../parser/parser.h"


SymbolTable *variableTable;

void initSymbolTable(void) {
    variableTable = malloc(sizeof(SymbolTable));
    variableTable->symbols = NULL;
    variableTable->count = 0;
    variableTable->capacity = 0;
}


void declareVariable( const char *name, VarType type, VarValue value  ) {
    // Type check
    if ((type == VARIABLE_TYPE_STRING && value.type != AST_TEXT) ||
        (type == VARIABLE_TYPE_INT && value.type != AST_NUMBER)) {
        fprintf(stderr, "Type mismatch for variable %s\n", name);
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

    Symbol *sym = &variableTable->symbols[variableTable->count++];
    sym->name = strdup(name);
    sym->type = type;
    sym->value = value;
}

char *getVariableValue( char *name ) {
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
                snprintf(buf, sizeof(buf), "%d", sym->value.integer);
                return strdup(buf);
            }
            return strdup("");
        }
    }

    // Not found
    return strdup("");  // or NULL, but then handle NULL in compileExpr
}

void printSymbolTable() {
    printf("\n=== SYMBOL TABLE ===\n");

    if (!variableTable || variableTable->count == 0) {
        printf("(empty)\n");
        printf("====================\n");
        return;
    }

    for (int i = 0; i < variableTable->count; i++) {
        Symbol *sym = &variableTable->symbols[i];

        printf("name: %s   type: ", sym->name);

        // Print declared type
        switch (sym->type) {
            case VARIABLE_TYPE_STRING:
                printf("STRING   ");
                break;
            case VARIABLE_TYPE_INT:
                printf("INT      ");
                break;
            default:
                printf("UNKNOWN  ");
                break;
        }

        printf("value: ");

        // Print runtime value
        switch (sym->value.type) {
            case AST_TEXT:
                printf("\"%s\"", sym->value.text);
                break;
            case AST_NUMBER:
                printf("%d", sym->value.integer);
                break;
            default:
                printf("<invalid>");
                break;
        }

        printf("\n");
    }

    printf("====================\n");
}



static char *join( const char *left, const char *right ) {
    if (!left) left = strdup("");
    if (!right) right = strdup("");

    size_t na = strlen(left), nb = strlen(right);
    char *out = malloc(na + nb + 1);
    if (!out) { perror("malloc"); exit(EXIT_FAILURE); }

    memcpy(out, left, na);
    memcpy(out + na, right, nb + 1);

    free(left);
    free(right);
    return out;
}

char *compileExpr(ASTNode *node) {

    switch (node->type) {
        case AST_TEXT:
            return strdup(node->text ? node->text : "");

        case AST_NUMBER: {
            char buf[32];
            snprintf(buf, sizeof(buf), "%d", node->number);
            return strdup(buf);
        }

        case AST_CONCAT: {
            char *left = compileExpr(node->binary.left);
            char *right = compileExpr(node->binary.right);
            return join(left, right);
        }

        case AST_VARIABLE_CAST: {
            return getVariableValue( node->text );
        }
        default:
            return strdup("");
    }
}


void debugToken(const Token *tok) {
    printf("TOKEN %-20s", tokenTypeToString(tok->type));

    printf("\n");
}

void debugTokenStream(const Token *tokens, int count) {
    printf("\n=== TOKEN STREAM ===\n");

    for (int i = 0; i < count; i++) {
        printf("[%03d] ", i);
        debugToken(&tokens[i]);
    }

    printf("====================\n");
}


void runCompiler() {
    initSymbolTable();
    debugTokenStream( getTokens(), getTokensCount() );
    ASTNode root = getAST();

    showAST( &root );

    for (int i = 0; i < root.block.count; ++i) {
        ASTNode *child = root.block.children[i];

        switch ( child->type ) {
            case AST_VARIABLE_DEFINITION:

                // if ( child->varDecl.value->type == AST_TEXT ) {
                //     declareVariable( child->varDecl.name, child->varDecl.varType, (VarValue){
                //         .type =  child->varDecl.value->type,
                //         .text =  child->varDecl.value->text,
                //     } );
                // }
                //
                // if ( child->varDecl.value->type == AST_NUMBER ) {
                //     declareVariable( child->varDecl.name, child->varDecl.varType, (VarValue){
                //         .type =  child->varDecl.value->type,
                //         .integer =  child->varDecl.value->number,
                //     } );
                // }
                break;
            case AST_PRINT_STMT:
                // char *s = compileExpr( child->block.children[0] );
                // printf("%s\n", s);
                // free(s);

                break;
            default:
                fprintf(stderr, "Unknown AST node type (%s)\n", astNodeTypeToString(child->type));
                break;
        }
    }

    printSymbolTable();

}
