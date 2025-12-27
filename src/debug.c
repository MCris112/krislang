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
        case TOK_EOF: return "TOK_EOF";
        case TOK_ERROR: return "TOK_ERROR";

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
        case TOK_IDENTIFIER: return "TOK_IDENTIFIER";
        case TOK_LOOP_WHILE: return "TOK_LOOP_WHILE";

        // COMPARISON
        case TOK_LESS_THAN: return "TOK_LESS_THAN";
        case TOK_GREATER_THAN: return "TOK_GREATER_THAN";
        case TOK_LESS_EQUAL: return "TOK_LESS_EQUAL";
        case TOK_GREATER_EQUAL: return "TOK_GREATER_EQUAL";
        case TOK_EQUAL_EQUAL: return "TOK_EQUAL_EQUAL";
        case TOK_NOT_EQUAL: return "TOK_NOT_EQUAL";

        // LOGICAL
        case TOK_LOGICAL_IF: return "TOK_LOGICAL_IF";
        case TOK_LOGICAL_ELSE: return "TOK_LOGICAL_ELSE";
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
        case TOK_CHAR: return "TOK_CHAR";
        case TOK_NUMBER: return "TOK_NUMBER";
        case TOK_NUMBER_DECIMAL: return "TOK_NUMBER_DECIMAL";
        case TOK_LITERAL_BOOLEAN: return "TOK_LITERAL_BOOLEAN";

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
// AST PRETTY PRINTER
//
//------------------------------------

static void printIndent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

static void printNodeHeader(ASTNode *node, int indent) {
    printIndent(indent);
    printf("%s", astNodeTypeToString(node->type));

    switch (node->type) {
        case AST_TEXT:
            printf(" \"%s\"", node->text);
            break;

        case AST_NUMBER:
            printf(" %d", node->number);
            break;

        case AST_BOOLEAN:
            printf(" %s", node->boolean ? "TRUE" : "FALSE");
            break;

        case AST_VARIABLE_DEFINITION:
            printf(" %s", node->varDecl.name);
            break;

        case AST_VARIABLE_ASSIGNMENT:
            printf(" %s", node->variableAssignment.name);
            break;

        case AST_VARIABLE_CAST:
            printf(" %s", node->text);
            break;

        case AST_FUNCTION_CALL:
            printf(" %s()", node->funcCall.name);
            break;

        case AST_FUNCTION_DEFINITION:
            printf(" %s()", node->funcDefinition.name);
            break;

        case AST_FUNCTION_PARAMETER:
            printf(" %s %s",
                   parserVarTypeToString(node->varDecl.varType),
                   node->varDecl.name);
            break;

        case AST_TYPE_LITERAL:
            printf(" %s (size=%d)",
                   astNodeTypeToString(node->literal.type),
                   node->literal.size);
            break;

        case AST_LOGICAL_IF:
            printf(" [body=%d else=%d]",
                   node->logicalIf.bodyBlock.count,
                   node->logicalIf.elseBlock.count);
            break;

        case AST_LOOP_WHILE:
            printf(" [body=%d]", node->loopWhile.body.count);
            break;

        default:
            break;
    }

    printf("\n");
}

void parserPrintASTNode(ASTNode *node, int indent) {
    if (!node) {
        printIndent(indent);
        printf("<NULL>\n");
        return;
    }

    printNodeHeader(node, indent);

    switch (node->type) {

        case AST_PROGRAM:
        case AST_BLOCK:
            for (int i = 0; i < node->block.count; i++) {
                parserPrintASTNode(node->block.children[i], indent + 1);
            }
            break;

        case AST_VARIABLE_DEFINITION:
            parserPrintASTNode(node->varDecl.value, indent + 1);
            break;

        case AST_VARIABLE_ASSIGNMENT:
            parserPrintASTNode(node->variableAssignment.value, indent + 1);
            break;

        case AST_RETURN:
            parserPrintASTNode(node->child, indent + 1);
            break;

        case AST_CONCAT:
        case AST_SUBTRACT:
        case AST_COMPARE:
            parserPrintASTNode(node->binary.left, indent + 1);
            parserPrintASTNode(node->binary.right, indent + 1);
            break;

        case AST_FUNCTION_CALL:
            for (int i = 0; i < node->funcCall.arguments.count; i++) {
                parserPrintASTNode(node->funcCall.arguments.children[i], indent + 1);
            }
            break;

        case AST_FUNCTION_DEFINITION:
            printIndent(indent + 1);
            printf("ARGS:\n");
            for (int i = 0; i < node->funcDefinition.arguments.count; i++) {
                parserPrintASTNode(node->funcDefinition.arguments.children[i], indent + 2);
            }

            printIndent(indent + 1);
            printf("BODY:\n");
            for (int i = 0; i < node->funcDefinition.body.count; i++) {
                parserPrintASTNode(node->funcDefinition.body.children[i], indent + 2);
            }
            break;

        case AST_LOGICAL_IF:
            printIndent(indent + 1);
            printf("CONDITION:\n");
            parserPrintASTNode(node->logicalIf.conditional, indent + 2);

            printIndent(indent + 1);
            printf("BODY:\n");
            for (int i = 0; i < node->logicalIf.bodyBlock.count; i++) {
                parserPrintASTNode(node->logicalIf.bodyBlock.children[i], indent + 2);
            }

            if (node->logicalIf.elseBlock.count > 0) {
                printIndent(indent + 1);
                printf("ELSE:\n");
                for (int i = 0; i < node->logicalIf.elseBlock.count; i++) {
                    parserPrintASTNode(node->logicalIf.elseBlock.children[i], indent + 2);
                }
            }
            break;

        case AST_LOOP_WHILE:
            printIndent(indent + 1);
            printf("CONDITION:\n");
            parserPrintASTNode(node->loopWhile.condition, indent + 2);

            printIndent(indent + 1);
            printf("BODY:\n");
            for (int i = 0; i < node->loopWhile.body.count; i++) {
                parserPrintASTNode(node->loopWhile.body.children[i], indent + 2);
            }
            break;

        default:
            break;
    }
}

void parserPrintAST(ASTNode *root) {
    printf("\n=== AST ===\n");
    parserPrintASTNode(root, 0);
    printf("===========\n");
}



char *parserVarTypeToString(VarType type) {
    switch (type) {
        case VARIABLE_TYPE_STRING:
            return "STRING";
        case VARIABLE_TYPE_INT:
            return "INT";
        case VARIABLE_TYPE_FLOAT:
            return "FLOAT";
        case VARIABLE_TYPE_BOOLEAN:
            return "BOOLEAN";
        case VARIABLE_TYPE_CHAR:
            return "CHAR";
        case VARIABLE_TYPE_NEVER:
            return "NEVER";
        case VARIABLE_TYPE_VOID:
            return "VOID";
        default: return "UNKNOWN";
    }
}

char *parseEnvValueTypeToString(EnvValueType type) {
    switch (type) {
        case ENV_STRING:
            return "STRING";
        case ENV_INT:
            return "INT";
        case ENV_FLOAT:
            return "FLOAT";
        case ENV_BOOL:
            return "BOOLEAN";
        case ENV_CHAR:
            return "CHAR";
        case ENV_VOID:
            return "VOID";
        case ENV_NULL:
            return "NULL";
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
        Environment *sym = &variableTable->symbols[i];

        printf("name: %-10s ", sym->name);

        // -----------------------------
        // VARIABLE
        // -----------------------------
        if (sym->type == ENV_TYPE_VARIABLE) {
            printf("kind: VARIABLE  declared-type: ");

            switch (sym->variable.type) {
                case VARIABLE_TYPE_STRING:  printf("STRING   "); break;
                case VARIABLE_TYPE_INT:     printf("INT      "); break;
                case VARIABLE_TYPE_FLOAT:   printf("FLOAT    "); break;
                case VARIABLE_TYPE_BOOLEAN: printf("BOOLEAN  "); break;
                case VARIABLE_TYPE_CHAR:    printf("CHAR     "); break;
                case VARIABLE_TYPE_VOID:    printf("VOID     "); break;
                default:                    printf("UNKNOWN  "); break;
            }

            printf(" value: ");

            switch (sym->variable.value.type) {
                case ENV_STRING:
                    if (sym->variable.value.text)
                        printf("\"%s\"", sym->variable.value.text);
                    else
                        printf("NULL_STRING");
                    break;

                case ENV_INT:
                    printf("%d", sym->variable.value.number);
                    break;

                case ENV_FLOAT:
                    printf("%f", sym->variable.value.decimal);
                    break;

                case ENV_BOOL:
                    printf(sym->variable.value.boolean ? "TRUE" : "FALSE");
                    break;

                case ENV_CHAR:
                    if (sym->variable.value.character == '\0') printf("'\\0'");
                    else if (sym->variable.value.character == '\n') printf("'\\n'");
                    else if (sym->variable.value.character == '\t') printf("'\\t'");
                    else printf("'%c'", sym->variable.value.character);
                    break;

                case ENV_VOID:
                    printf("VOID(%s)", sym->variable.value.text);
                    break;

                case ENV_NULL:
                    printf("NULL");
                    break;

                default:
                    printf("<UNKNOWN>");
                    break;
            }
        }

        // -----------------------------
        // FUNCTION
        // -----------------------------
        else if (sym->type == ENV_TYPE_FUNCTION) {
            printf("kind: FUNCTION  args: %d  body: %d",
                   sym->function.arguments ? sym->function.arguments->count : 0,
                   sym->function.body->count);
        }

        printf("\n");
    }

    printf("====================\n");
}
