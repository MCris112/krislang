//
// Created by MCris112 on 12/18/2025.
//

#include "parser.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/lexer/lexer.h"

int syntax_error_count = 0;

void syntaxError(const char *message, Token token) {
    fprintf(stderr,
        "Syntax error at %d:%d - %s\n",
        token.line, token.column, message
    );
    syntax_error_count++;
}

char *astNodeTypeToString(ASTNodeType type){
    switch (type) {
        case AST_PROGRAM:
            return "AST_PROGRAM";

        case AST_PRINT_STMT:
            return "AST_PRINT_STMT";

        case AST_TEXT:
            return "AST_TEXT";

        case AST_NUMBER:
            return "AST_NUMBER";

        case AST_ERROR:
            return "AST_ERROR";

        case AST_VARIABLE_DEFINITION:
            return "AST_VARIABLE_DEFINITION";

        case AST_VARIABLE_CAST:
            return "AST_VARIABLE_CAST";

        case AST_CONCAT:
            return "AST_CONCAT";

        default:
            return "AST_UNKNOWN";
    }
}

int current = 0;

int getASTPosition() {
    return current;
}

void nextPos() {
    current++;
}

bool isEnd() {
    return current >= getTokensCount();
}

Token currentToken() {
    return getTokens()[current];
}

ASTNode *addASTNode(ASTNode *parent, ASTNode child) {

    // is not block type
    // if (parent->type != AST_BLOCK && parent->type != AST_PROGRAM)
    //     abort();

    // Grow children array if needed
    if (parent->block.count >= parent->block.capacity) {
        parent->block.capacity = parent->block.capacity
            ? parent->block.capacity * 2
            : 4;

        parent->block.children = realloc(
            parent->block.children,
            parent->block.capacity * sizeof(ASTNode *)
        );

        if (!parent->block.children) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
    }

    // Allocate new node
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    *node = child;

    parent->block.children[parent->block.count++] = node;
    return node;
}

bool evalExpectedToken(Token token, TokenType expected, char *message ) {
    if ( token.type != expected) {
        if ( !message )
            message = "Sintaxis unexpected, please check the code";

        syntaxError( message, token );
        return false;
    }

    return true;
}

ASTNode *evalVariableDefinitionValue(Token token ) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) { perror("malloc"); exit(EXIT_FAILURE); }

    switch (token.type) {
        case TOK_NUMBER:
            *node = ( ASTNode ){
                .type = AST_NUMBER,
                .number = token.number
            };
            break;
        case TOK_TEXT:
            *node = ( ASTNode ){
                .type = AST_TEXT,
                .text =  token.text
            };
            break;
        case TOK_NUMBER_DECIMAL:
            *node = ( ASTNode ){
                .type = AST_NUMBER_DECIMAL,
                .decimal =  token.decimal
            };
            break;
        case TOK_CHAR:
            *node = ( ASTNode ){
                .type = AST_CHAR,
                .text =  token.text
            };
            break;
        case TOK_LITERAL_BOOLEAN:
            *node = ( ASTNode ){
                .type = AST_BOOLEAN,
                .boolean =  token.boolean
            };
            break;
        default:
            syntaxError( "The value of the var was not expected", token );
            free(node);
            return NULL;
    }

    return node;
}

bool evalVariableDefinition( ASTNode *parent, TokenType type) {
    Token tok = currentToken();
    if ( tok.type != type) {
        return false;
    }

    printf("CURRENT TOKEN: %s \n", tokenTypeToString(tok.type));

     nextPos(); // skip TYPE

    Token varName  = currentToken();
    if ( varName.type != TOK_PARENTHESIS_OPEN  && varName.type != TOK_VARIABLE ) {
        syntaxError( "Variable name/size expected", varName );
        nextPos();
        return false;
    }

    int size = -1;

    // Optional (size)

    if (varName.type == TOK_PARENTHESIS_OPEN) {
        nextPos(); // SKIP '('

        Token sizeTok = currentToken();
        if (!evalExpectedToken(sizeTok, TOK_NUMBER, "Expected size inside STRING(...)"))
            return false;

        size = sizeTok.number;
        nextPos();

        if (!evalExpectedToken(currentToken(), TOK_PARENTHESIS_CLOSE, "Expected ')' after size"))
            return false;

        nextPos(); // skip ')'
    }

    // Now expect variable name
    varName  = currentToken(); nextPos();
    if (  !evalExpectedToken(varName, TOK_VARIABLE, "Variable name expected") )
        return false;

    // Expect '='
    Token equals   = currentToken(); nextPos();
    if ( !evalExpectedToken(equals, TOK_EQUALS, "Expected '='") )
        return false;

    // Parse value
    Token varValue = currentToken(); nextPos();
    ASTNode *valueNode = evalVariableDefinitionValue(varValue);
    if (!valueNode) {
        return false;
    }

    if ( currentToken().type != TOK_SEMICOLON) {
        syntaxError( "Semicolon expected", currentToken() );
        return false;
    }

    nextPos();

    ASTNode definition = (ASTNode) {
        .type = AST_VARIABLE_DEFINITION,
        .varDecl = {
            .varType = VARIABLE_TYPE_STRING,
            .name =varName.text,
            .value =  valueNode,
            .size = size
        }
    };

    addASTNode(parent, definition);
    return true;
}
ASTNode getAST() {

    ASTNode parent = {
        .type = AST_PROGRAM,
        .block = {
            .children = NULL,
            .count = 0,
            .capacity = 0
        }
    };

    const Token *tokens = getTokens();
    int count = getTokensCount();

    while (!isEnd()) {
        Token tok = currentToken();

        if(evalVariableDefinition( &parent, TOK_VARIABLE_TYPE_STRING )) {
            continue;
        }

        if(evalVariableDefinition( &parent, TOK_VARIABLE_TYPE_INT )) {
            continue;
        }

        if(evalVariableDefinition( &parent, TOK_VARIABLE_TYPE_BOOLEAN )) {
            continue;
        }

        if(evalVariableDefinition( &parent, TOK_VARIABLE_TYPE_FLOAT )) {
            continue;
        }

        if(evalVariableDefinition( &parent, TOK_VARIABLE_TYPE_CHAR )) {
            continue;
        }

        nextPos();
    }

    if (syntax_error_count > 0) {
        fprintf(stderr,
            "\nCompilation failed with %d syntax error(s).\n",
            syntax_error_count
        );
        exit(EXIT_FAILURE);
    }

    return parent;
}
void showASTNode(ASTNode *node, int indent) {
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
                showASTNode(node->block.children[i], indent + 1);
            }
            break;

        case AST_VARIABLE_DEFINITION:
            if (node->varDecl.value) {
                showASTNode(node->varDecl.value, indent + 1);
            }
            break;

        default:
            // Leaf nodes: nothing to recurse into
            break;
    }
}


void showAST(ASTNode *root) {
    printf("=== AST ===\n");
    showASTNode(root, 0);
    printf("===========\n");
}
