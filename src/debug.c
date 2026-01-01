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
const char *lexerTokenToString(LexerTokenType type) {
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


// Helper function to convert token type to string
const char* tokenTypeToString(LexerTokenType type) {
    switch (type) {
        case TOK_EQUAL_EQUAL: return "==";
        case TOK_NOT_EQUAL: return "!=";
        case TOK_LESS_THAN: return "<";
        case TOK_LESS_EQUAL: return "<=";
        case TOK_GREATER_THAN: return ">";
        case TOK_GREATER_EQUAL: return ">=";
        case TOK_PLUS: return "+";
        case TOK_MINUS: return "-";
        case TOK_MULTIPLY: return "*";
        case TOK_DIVIDE: return "/";
        case TOK_MODULO: return "%";
        default: return "?";
    }
}

void printIndent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

void printNodeHeader(ASTNode *node, int indent) {
    printIndent(indent);

    switch (node->type) {
        case AST_PROGRAM:
            printf("┌─ PROGRAM\n");
            break;
        case AST_BLOCK:
            printf("┌─ BLOCK (%d statements)\n", node->block.count);
            break;
        case AST_VARIABLE_DEFINITION:
            printf("┌─ VAR_DEF: %s %s (size: %d)\n",
                   parserVarTypeToString(node->varDecl.varType),
                   node->varDecl.name,
                   node->varDecl.size);
            break;
        case AST_VARIABLE_ASSIGNMENT:
            printf("┌─ VAR_ASSIGN: %s\n", node->variableAssignment.name);
            break;
        case AST_VARIABLE_CAST:
            printf("└─ VAR_CAST: %s\n", node->text);
            break;
        case AST_RETURN:
            printf("┌─ RETURN\n");
            break;
        case AST_CONCAT:
            printf("┌─ CONCAT (+)\n");
            break;
        case AST_SUBTRACT:
            printf("┌─ SUBTRACT (-)\n");
            break;
        case AST_MULTIPLY:
            printf("┌─ MULTIPLY (*)\n");
            break;
        case AST_DIVIDE:
            printf("┌─ DIVIDE (/)\n");
            break;
        case AST_MODULO:
            printf("┌─ MODULO (%%)\n");
            break;
        case AST_COMPARE:
            printf("┌─ COMPARE (%s)\n", tokenTypeToString(node->compare.operator));
            break;
        case AST_UNARY:
            printf("┌─ UNARY (%s)\n", tokenTypeToString(node->unary.operator));
            break;
        case AST_TYPE_LITERAL:
            printf("└─ TYPE: %s (size: %d)\n",
                   astNodeTypeToString(node->literal.type),
                   node->literal.size);
            break;
        case AST_NUMBER:
            printf("└─ INT: %d\n", node->number);
            break;
        case AST_NUMBER_DECIMAL:
            printf("└─ FLOAT: %.2f\n", node->decimal);
            break;
        case AST_TEXT:
            printf("└─ STRING: \"%s\"\n", node->text);
            break;
        case AST_CHAR:
            printf("└─ CHAR: '%c'\n", node->character);
            break;
        case AST_BOOLEAN:
            printf("└─ BOOL: %s\n", node->boolean ? "TRUE" : "FALSE");
            break;
        case AST_VOID:
            printf("└─ VOID\n");
            break;
        case AST_FUNCTION_CALL:
            printf("┌─ FUNC_CALL: %s (%d args)\n",
                   node->funcCall.name,
                   node->funcCall.arguments.count);
            break;
        case AST_FUNCTION_DEFINITION:
            printf("┌─ FUNC_DEF: %s (%d params, %d statements)\n",
                   node->funcDefinition.name,
                   node->funcDefinition.arguments.count,
                   node->funcDefinition.body.count);
            break;
        case AST_FUNCTION_PARAMETER:
            printf("└─ PARAM: %s %s (size: %d)\n",
                   parserVarTypeToString(node->varDecl.varType),
                   node->varDecl.name,
                   node->varDecl.size);
            break;
        case AST_LOGICAL_IF:
            printf("┌─ IF (%d body, %d else)\n",
                   node->logicalIf.bodyBlock.count,
                   node->logicalIf.elseBlock.count);
            break;
        case AST_LOOP_WHILE:
            printf("┌─ WHILE (%d statements)\n", node->loopWhile.body.count);
            break;
        case AST_LOOP_FOR:
            printf("┌─ FOR: $%s (%d statements)\n",
                   node->loopFor.param.name,
                   node->loopFor.body.count);
            break;
        case AST_CLASS:
            printf("┌─ CLASS: %s (%d functions)\n",
                   node->class.name,
                   node->class.functions.count);
            break;
        default:
            printf("┌─ UNKNOWN (%d) [%s]\n", node->type, astNodeTypeToString(node->type));
            break;
    }
}

void parserPrintASTNode(ASTNode *node, int indent) {
    if (!node) {
        printIndent(indent);
        printf("└─ <NULL>\n");
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
        case AST_FUNCTION_PARAMETER:
            // Function parameters may not have values
            if (node->varDecl.value) {
                printIndent(indent + 1);
                printf("└─ VALUE:\n");
                parserPrintASTNode(node->varDecl.value, indent + 2);
            }
            break;

        case AST_VARIABLE_ASSIGNMENT:
            printIndent(indent + 1);
            printf("└─ VALUE:\n");
            parserPrintASTNode(node->variableAssignment.value, indent + 2);
            break;

        case AST_RETURN:
            if (node->child) {
                printIndent(indent + 1);
                printf("└─ VALUE:\n");
                parserPrintASTNode(node->child, indent + 2);
            }
            break;

        case AST_CONCAT:
        case AST_SUBTRACT:
        case AST_MULTIPLY:
        case AST_DIVIDE:
        case AST_MODULO:
            printIndent(indent + 1);
            printf("├─ LEFT:\n");
            parserPrintASTNode(node->binary.left, indent + 2);
            printIndent(indent + 1);
            printf("└─ RIGHT:\n");
            parserPrintASTNode(node->binary.right, indent + 2);
            break;

        case AST_COMPARE:
            printIndent(indent + 1);
            printf("├─ LEFT:\n");
            parserPrintASTNode(node->compare.left, indent + 2);
            printIndent(indent + 1);
            printf("└─ RIGHT:\n");
            parserPrintASTNode(node->compare.right, indent + 2);
            break;

        case AST_UNARY:
            printIndent(indent + 1);
            printf("└─ OPERAND:\n");
            parserPrintASTNode(node->unary.operand, indent + 2);
            break;

        case AST_FUNCTION_CALL:
            if (node->funcCall.arguments.count > 0) {
                printIndent(indent + 1);
                printf("└─ ARGS:\n");
                for (int i = 0; i < node->funcCall.arguments.count; i++) {
                    printIndent(indent + 2);
                    printf("[%d]:\n", i);
                    parserPrintASTNode(node->funcCall.arguments.children[i], indent + 3);
                }
            }
            break;

        case AST_FUNCTION_DEFINITION:
            if (node->funcDefinition.arguments.count > 0) {
                printIndent(indent + 1);
                printf("├─ PARAMS:\n");
                for (int i = 0; i < node->funcDefinition.arguments.count; i++) {
                    printIndent(indent + 2);
                    printf("[%d]:\n", i);
                    parserPrintASTNode(node->funcDefinition.arguments.children[i], indent + 3);
                }
            }

            printIndent(indent + 1);
            printf("└─ BODY:\n");
            for (int i = 0; i < node->funcDefinition.body.count; i++) {
                parserPrintASTNode(node->funcDefinition.body.children[i], indent + 2);
            }
            break;

        case AST_LOGICAL_IF:
            printIndent(indent + 1);
            printf("├─ CONDITION:\n");
            parserPrintASTNode(node->logicalIf.conditional, indent + 2);

            printIndent(indent + 1);
            printf("├─ THEN:\n");
            for (int i = 0; i < node->logicalIf.bodyBlock.count; i++) {
                parserPrintASTNode(node->logicalIf.bodyBlock.children[i], indent + 2);
            }

            if (node->logicalIf.elseBlock.count > 0) {
                printIndent(indent + 1);
                printf("└─ ELSE:\n");
                for (int i = 0; i < node->logicalIf.elseBlock.count; i++) {
                    parserPrintASTNode(node->logicalIf.elseBlock.children[i], indent + 2);
                }
            }
            break;

        case AST_LOOP_WHILE:
            printIndent(indent + 1);
            printf("├─ CONDITION:\n");
            parserPrintASTNode(node->loopWhile.condition, indent + 2);

            printIndent(indent + 1);
            printf("└─ BODY:\n");
            for (int i = 0; i < node->loopWhile.body.count; i++) {
                parserPrintASTNode(node->loopWhile.body.children[i], indent + 2);
            }
            break;

        case AST_LOOP_FOR:
            printIndent(indent + 1);
            printf("├─ INIT: $%s\n", node->loopFor.param.name);

            printIndent(indent + 1);
            printf("├─ CONDITION:\n");
            parserPrintASTNode(node->loopFor.condition, indent + 2);

            printIndent(indent + 1);
            printf("└─ BODY:\n");
            for (int i = 0; i < node->loopFor.body.count; i++) {
                parserPrintASTNode(node->loopFor.body.children[i], indent + 2);
            }
            break;

        case AST_CLASS:
            printIndent(indent + 1);
            printf("└─ FUNCTIONS:\n");
            for (int i = 0; i < node->class.functions.count; i++) {
                printIndent(indent + 2);
                printf("[%d]:\n", i);
                parserPrintASTNode(node->class.functions.children[i], indent + 3);
            }
            break;

        // Leaf nodes - no children to traverse
        case AST_TYPE_LITERAL:
        case AST_VARIABLE_CAST:
        case AST_NUMBER:
        case AST_NUMBER_DECIMAL:
        case AST_TEXT:
        case AST_CHAR:
        case AST_BOOLEAN:
        case AST_VOID:
            break;

        default:
            printIndent(indent + 1);
            printf("└─ <unhandled type: %d [%s]>\n", node->type, astNodeTypeToString(node->type));
            break;
    }
}

void parserPrintAST(ASTNode *root) {
    printf("\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("║          ABSTRACT SYNTAX TREE          ║\n");
    printf("╚════════════════════════════════════════╝\n");
    printf("\n");
    parserPrintASTNode(root, 0);
    printf("\n");
    printf("════════════════════════════════════════\n");
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
// ASTNode
//
//------------------------------------

char *astNodeTypeToString(ASTNodeType type) {
    switch (type) {
        case AST_PROGRAM: return "AST_PROGRAM";
        case AST_BLOCK: return "AST_BLOCK";
        case AST_PRINT_STMT: return "AST_PRINT_STMT";

        case AST_FUNCTION_CALL: return "AST_FUNCTION_CALL";
        case AST_FUNCTION_DEFINITION: return "AST_FUNCTION_DEFINITION";
        case AST_FUNCTION_PARAMETER: return "AST_FUNCTION_PARAMETER";
        case AST_FUNCTION_REFERENCE: return "AST_FUNCTION_REFERENCE";

        case AST_LOGICAL_IF: return "AST_LOGICAL_IF";
        case AST_RETURN: return "AST_RETURN";
        case AST_LOOP_WHILE: return "AST_LOOP_WHILE";
        case AST_LOOP_FOR: return "AST_LOOP_FOR";

        case AST_TYPE_LITERAL: return "AST_TYPE_LITERAL";
        case AST_TEXT: return "AST_TEXT";
        case AST_NUMBER: return "AST_NUMBER";
        case AST_NUMBER_DECIMAL: return "AST_NUMBER_DECIMAL";
        case AST_BOOLEAN: return "AST_BOOLEAN";
        case AST_CHAR: return "AST_CHAR";
        case AST_VOID: return "AST_VOID";
        case AST_NULL: return "AST_NULL";
        case AST_UNKNOWN: return "AST_UNKNOWN";
        case AST_ERROR: return "AST_ERROR";

        case AST_CONCAT: return "AST_CONCAT";
        case AST_SUBTRACT: return "AST_SUBTRACT";
        case AST_COMPARE: return "AST_COMPARE";
        case AST_UNARY: return "AST_UNARY";
        case AST_MULTIPLY: return "AST_MULTIPLY";
        case AST_DIVIDE: return "AST_DIVIDE";
        case AST_MODULO: return "AST_MODULO";

        case AST_CLASS: return "AST_CLASS";

        case AST_VARIABLE_DEFINITION: return "AST_VARIABLE_DEFINITION";
        case AST_VARIABLE_CAST: return "AST_VARIABLE_CAST";
        case AST_VARIABLE_ASSIGNMENT: return "AST_VARIABLE_ASSIGNMENT";

        case AST_EOF: return "AST_EOF";

        default: return "AST_<INVALID>";
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

        else if ( sym->type == ENV_TYPE_CLASS ) {
            printf("kind: CLASS  functions: %d  body: %d",
                  sym->class.functions.count,
                  sym->class.functions.count);
        }
        printf("\n");
    }

    printf("====================\n");
}
