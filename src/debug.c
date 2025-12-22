//
// Created by crisv on 12/21/2025.
//

#include "debug.h"

#include <stdio.h>

//------------------------------------
//
// LEXER
//
//------------------------------------
const char *lexerTokenToString(TokenType type) {
    switch (type) {
        // VARIABLES
        case TOK_VARIABLE_TYPE_INT: return "TOK_VARIABLE_TYPE_INT";
        case TOK_VARIABLE_TYPE_STRING: return "TOK_VARIABLE_TYPE_STRING";
        case TOK_VARIABLE_TYPE_BOOLEAN: return "TOK_VARIABLE_TYPE_BOOLEAN";
        case TOK_VARIABLE_TYPE_FLOAT: return "TOK_VARIABLE_TYPE_FLOAT";
        case TOK_VARIABLE_TYPE_CHAR: return "TOK_VARIABLE_TYPE_CHAR";
        case TOK_VARIABLE_TYPE_VOID: return "TOK_VARIABLE_TYPE_VOID";
        case TOK_VARIABLE: return "TOK_VARIABLE";

        // FUNCTIONS
        case TOK_FUNCTION_CALL: return "TOK_FUNCTION_CALL";
        case TOK_FUNCTION_DEFINITION: return "TOK_FUNCTION_DEFINITION";

        // COMPARISON
        case TOK_LESS_THAN: return "TOK_LESS_THAN";
        case TOK_GREATER_THAN: return "TOK_GREATER_THAN";
        case TOK_LESS_EQUAL: return "TOK_LESS_EQUAL";
        case TOK_GREATER_EQUAL: return "TOK_GREATER_EQUAL";
        case TOK_EQUAL_EQUAL: return "TOK_EQUAL_EQUAL";
        case TOK_NOT_EQUAL: return "TOK_NOT_EQUAL";

        // LOGICAL
        case TOK_LOGICAL_AND: return "TOK_LOGICAL_AND";
        case TOK_LOGICAL_OR: return "TOK_LOGICAL_OR";
        case TOK_LOGICAL_NOT: return "TOK_LOGICAL_NOT";

        // ASSIGNMENT
        case TOK_EQUALS: return "TOK_EQUALS";
        case TOK_PLUS_ASSIGN: return "TOK_PLUS_ASSIGN";
        case TOK_MINUS_ASSIGN: return "TOK_MINUS_ASSIGN";
        case TOK_MUL_ASSIGN: return "TOK_MUL_ASSIGN";
        case TOK_DIV_ASSIGN: return "TOK_DIV_ASSIGN";

        // ARITHMETIC
        case TOK_PLUS: return "TOK_PLUS";
        case TOK_MINUS: return "TOK_MINUS";
        case TOK_MULTIPLY: return "TOK_MULTIPLY";
        case TOK_DIVIDE: return "TOK_DIVIDE";
        case TOK_MODULO: return "TOK_MODULO";
        case TOK_INCREMENT: return "TOK_INCREMENT";
        case TOK_DECREMENT: return "TOK_DECREMENT";

        // DELIMITERS
        case TOK_PARENTHESIS_OPEN: return "TOK_PARENTHESIS_OPEN";
        case TOK_PARENTHESIS_CLOSE: return "TOK_PARENTHESIS_CLOSE";
        case TOK_BRACE_OPEN: return "TOK_BRACE_OPEN";
        case TOK_BRACE_CLOSE: return "TOK_BRACE_CLOSE";
        case TOK_BRACKET_OPEN: return "TOK_BRACKET_OPEN";
        case TOK_BRACKET_CLOSE: return "TOK_BRACKET_CLOSE";
        case TOK_COMMA: return "TOK_COMMA";
        case TOK_SEMICOLON: return "TOK_SEMICOLON";
        case TOK_COLON: return "TOK_COLON";

        // ACCESS
        case TOK_ARROW: return "TOK_ARROW";
        case TOK_DOT: return "TOK_DOT";

        // LITERALS
        case TOK_TEXT: return "TOK_TEXT";
        case TOK_NUMBER: return "TOK_NUMBER";
        case TOK_NUMBER_DECIMAL: return "TOK_NUMBER_DECIMAL";

        case TOK_WHITESPACE: return "TOK_WHITESPACE";
        case TOK_COMMENT_NORMAL: return "TOK_COMMENT_NORMAL";

        default: return "TOK_UNKNOWN";
    }
}

void lexerPrintSingleToken(const Token *tok) {
    printf("TOKEN %-20s", lexerTokenToString(tok->type));

    printf("\n");
}

void lexerPrintTokens(const Token *tokens, int count) {
    printf("\n=== TOKEN STREAM ===\n");

    for (int i = 0; i < count; i++) {
        printf("[%03d] ", i);
        lexerPrintSingleToken(&tokens[i]);
    }

    printf("====================\n");
}


//------------------------------------
//
// PARSER
//
//------------------------------------
void parserPrintASTNode(ASTNode *node, int indent) {
    // Print indentation
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }

    // Print node type
    printf("%s", astNodeTypeToString(node->type));

    // Extra info
    if (node->type == AST_TEXT && node->text) {
        printf(" (\"%s\")", node->text);
    }

    if (node->type == AST_NUMBER) {
        printf(" (%d)", node->number);
    }

    if (node->type == AST_VARIABLE_DEFINITION) {
        printf(" (%s)", node->varDecl.name);
    }

    printf("\n");

    // Recurse depending on node type
    switch (node->type) {

        case AST_PROGRAM:
        case AST_PRINT_STMT:
            for (int i = 0; i < node->block.count; i++) {
                parserPrintASTNode(node->block.children[i], indent + 1);
            }
            break;

        case AST_VARIABLE_DEFINITION:
            if (node->varDecl.value) {
                parserPrintASTNode(node->varDecl.value, indent + 1);
            }
            break;
        case AST_CONCAT:
            // Print left child
            if (node->binary.left) {
                parserPrintASTNode(node->binary.left, indent + 1);
            }
            // Print right child
            if (node->binary.right) {
                parserPrintASTNode(node->binary.right, indent + 1);
            }
            break;
        case AST_FUNCTION_CALL:
            printf(" (%s)\n", node->funcCall.name);
            for (int i = 0; i < node->funcCall.count; i++) {
                parserPrintASTNode(node->funcCall.arguments[i], indent + 1);
            }
            break;
        default:
            // Leaf nodes: nothing to recurse into
            break;
    }
}


void parserPrintAST(ASTNode *root) {
    printf("=== AST ===\n");
    parserPrintASTNode(root, 0);
    printf("===========\n");
}


char *parserVarTypeToString(VarType type) {
    switch (type) {
        case VARIABLE_TYPE_STRING:
            return "string";
        case VARIABLE_TYPE_INT:
            return "int";
        case VARIABLE_TYPE_FLOAT:
            return "float";
        case VARIABLE_TYPE_BOOLEAN:
            return "boolean";
        case VARIABLE_TYPE_CHAR:
            return "char";
        default: return "UNKNOWN";
    }
}

//------------------------------------
//
// VARIABLES
//
//------------------------------------
void printSymbolTable(SymbolTable *variableTable) {
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
            case VARIABLE_TYPE_FLOAT:
                printf("FLOAT    ");
                break;
            case VARIABLE_TYPE_BOOLEAN:
                printf("BOOLEAN  ");
                break;
            case VARIABLE_TYPE_CHAR:
                printf("CHAR     ");
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
                printf("%d", sym->value.number);
                break;
            default:
                printf("<invalid>");
                break;
        }

        printf("\n");
    }

    printf("====================\n");
}

