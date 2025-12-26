//
// Created by MCris112 on 12/18/2025.
//

#include "parser.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../debug.h"
#include "../helpers/helper.h"

int syntax_error_count = 0;

void syntaxError(const char *message, Token token) {
    printf("LINE: %d - COLUMN: %d \n", token.line, token.column);
    const char *code = getCode();
    const char *lineStart = code;
    const char *pos = code;
    int currentLine = 0;
    // Find start of the error line
    while (*pos && currentLine < token.line) {
        if (*pos == '\n') {
            currentLine++;
            lineStart = pos + 1;
        }
        pos++;
    }
    // Find end of the line
    const char *lineEnd = lineStart;
    while (*lineEnd && *lineEnd != '\n') { lineEnd++; }

    // Print error header
    fprintf(stderr,
            "Error: Syntax error at\n --> *.kris %d:%d\n\n",
            token.line + 1, token.column + 1
    );

    fwrite(lineStart, 1, lineEnd - lineStart, stderr);
    fprintf(stderr, "\n");

    for (int i = 0; i < token.column; i++) { fputc(' ', stderr); }
    fprintf(stderr, "^\n");

    fprintf(stderr, "= %s\n", message);
    syntax_error_count++;
    exit(EXIT_FAILURE);
}

char *astNodeTypeToString(ASTNodeType type) {
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

        case AST_FUNCTION_DEFINITION:
            return "AST_FUNCTION_DEFINITION";

        case AST_TYPE_LITERAL:
            return "AST_TYPE_LITERAL";
        case AST_VARIABLE_CAST:
            return "AST_VARIABLE_CAST";
        case AST_FUNCTION_PARAMETER:
            return "AST_FUNCTION_PARAMETER";
        case AST_CONCAT:
            return "AST_CONCAT";
        case AST_COMPARE:
            return "AST_COMPARE";
        case AST_SUBTRACT:
            return "AST_SUBTRACT";
        case AST_FUNCTION_CALL:
            return "AST_FUNCTION_CALL";
        case AST_LOGICAL_IF:
            return "AST_LOGICAL_IF";
        case AST_RETURN:
            return "AST_RETURN";
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
    if ( isEnd() )
        syntaxError("Expected more code here", beforeToken() );

    return getTokens()[current];
}

Token beforeToken() {
    return getTokens()[current - 1];
}

ASTNode *addASTNode(ASTBlock *parent, ASTNode child) {
    // Grow children array if needed
    if (parent->count >= parent->capacity) {
        parent->capacity = parent->capacity
                               ? parent->capacity * 2
                               : 4;

        parent->children = realloc(
            parent->children,
            parent->capacity * sizeof(ASTNode *)
        );

        if (!parent->children) {
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

    parent->children[parent->count++] = node;
    return node;
}

bool evalExpectedToken(Token token, TokenType expected, char *message) {
    if (token.type != expected) {
        if (!message)
            message = "Sintaxis unexpected, please check the code";

        syntaxError(message, token);
        return false;
    }

    return true;
}

ASTNode *evalVariableDefinitionValue(Token token) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (!node) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    switch (token.type) {
        case TOK_NUMBER:
            *node = ( ASTNode){
                .type = AST_NUMBER,
                .number = token.number
            };
            break;
        case TOK_TEXT:
            *node = ( ASTNode){
                .type = AST_TEXT,
                .text = token.text
            };
            break;
        case TOK_NUMBER_DECIMAL:
            *node = ( ASTNode){
                .type = AST_NUMBER_DECIMAL,
                .decimal = token.decimal
            };
            break;
        case TOK_CHAR:
            *node = ( ASTNode){
                .type = AST_CHAR,
                .text = token.text
            };
            break;
        case TOK_LITERAL_BOOLEAN:
            *node = ( ASTNode){
                .type = AST_BOOLEAN,
                .boolean = token.boolean
            };
            break;
        default:
            syntaxError("The value of the var was not expected", token);
            free(node);
            return NULL;
    }

    return node;
}

bool isVariableDefinition() {
    TokenType t = currentToken().type;
    return t == TOK_VARIABLE_TYPE_INT ||
           t == TOK_VARIABLE_TYPE_STRING ||
           t == TOK_VARIABLE_TYPE_BOOLEAN ||
           t == TOK_VARIABLE_TYPE_FLOAT ||
           t == TOK_VARIABLE_TYPE_CHAR ||
           t == TOK_VARIABLE_TYPE_VOID;
}

ASTNodeType fromTokVariableTypeToASTNodeType(TokenType type) {
    switch (type) {
        case TOK_VARIABLE_TYPE_INT: return AST_NUMBER;
        case TOK_VARIABLE_TYPE_STRING: return AST_TEXT;
        case TOK_VARIABLE_TYPE_BOOLEAN: return AST_BOOLEAN;
        case TOK_VARIABLE_TYPE_FLOAT : return AST_NUMBER_DECIMAL;
        case TOK_VARIABLE_TYPE_CHAR: return AST_CHAR;
        case TOK_VARIABLE_TYPE_VOID: return AST_VOID;
        default: return AST_UNKNOWN;
    }
}

VarType parseTokToVarType() {
    switch (currentToken().type) {
        case TOK_VARIABLE_TYPE_STRING: return VARIABLE_TYPE_STRING;
        case TOK_VARIABLE_TYPE_INT: return VARIABLE_TYPE_INT;
        case TOK_VARIABLE_TYPE_FLOAT : return VARIABLE_TYPE_FLOAT;
        case TOK_VARIABLE_TYPE_BOOLEAN: return VARIABLE_TYPE_BOOLEAN;
        case TOK_VARIABLE_TYPE_CHAR: return VARIABLE_TYPE_CHAR;
        case TOK_VARIABLE_TYPE_VOID: return VARIABLE_TYPE_NEVER;
        default:
            syntaxError("The variable type was not expected", beforeToken() );
            return VARIABLE_TYPE_UNKNOWN;
    }
}

ASTNode parseTypeLiteral() {
    Token type = currentToken();
    VarType varType = parseTokToVarType();

    printf("\n\n\n(1) - CURRENT TOKEN: %s\n", lexerTokenToString(currentToken().type));


    Token varName = currentToken();
    int size = -1;

    if ( varName.type == TOK_PARENTHESIS_OPEN ) {
        nextPos(); // SKIP '('

        Token sizeTok = currentToken();
        evalExpectedToken(sizeTok, TOK_NUMBER, "Expected size inside TYPE(...)");

        size = sizeTok.number;
        nextPos();

        evalExpectedToken(currentToken(), TOK_PARENTHESIS_CLOSE, "Expected ')' after size");

        nextPos(); // skip ')'
    }

    nextPos();

    printf("(2) - CURRENT TOKEN: %s\n\n\n", lexerTokenToString(currentToken().type));

    // Verify if is function definition
    if ( currentToken().type == TOK_FUNCTION_CALL ) {
        return parseFunctionDefinition();
    }

    printf( "[PARSERTYPELITERAL] Current: %s \n", lexerTokenToString(currentToken().type));

    if ( currentToken().type != TOK_VARIABLE) {

        ASTNodeType literal = fromTokVariableTypeToASTNodeType(type.type);

        if ( literal ==  AST_UNKNOWN ) {
            syntaxError( "Something went wrong..., expected a variable type, but UNKNOW returned", type );
        }

        return (ASTNode){
            .type = AST_TYPE_LITERAL,
            .literal = {
                .type = literal,
                .size = size
            }
        };
    }



    // Now expect variable name
    varName = currentToken();
    nextPos();
    //if (!evalExpectedToken(varName, TOK_VARIABLE, "Variable name expected"))
    //    return false;

    // Expect '='
    Token equals = currentToken();
    evalExpectedToken(equals, TOK_EQUALS, "Expected '='");
    nextPos();

    // Parse value
    // Token varValue = currentToken(); nextPos();
    ASTNode *valueNode = parseExpression(0);
    printf("[VAR][VALUE_NODE] DEFINITION: %s \n", astNodeTypeToString(valueNode->type));
    // TODO check well the tokens, cuz if u pass like "String" this will throw like a normal function call instead of var definition

    printf("CURRENCT TOKEN: %s \n", lexerTokenToString(currentToken().type));
    if (currentToken().type != TOK_SEMICOLON) {
        syntaxError(strFormat("Semicolon expected - Current: %s", lexerTokenToString(currentToken().type) ), beforeToken());
    }

    nextPos();

    ASTNode definition = (ASTNode){
        .type = AST_VARIABLE_DEFINITION,
        .varDecl = {
            .varType = varType,
            .name = varName.text,
            .value = valueNode,
            .size = size
        }
    };

    printf("LAST TOKEN ON PARSE TYPE LITERAL: %s \n", lexerTokenToString(currentToken().type));
    return definition;
}

void *parseBody(ASTBlock *parent) {
    printf("----------------NEW BODY-------------\n\n");
    while (!isEnd() && currentToken().type != TOK_BRACE_CLOSE) {
        printf("\nIN WHILE BODY \n\n");

        if (currentToken().type == TOK_LOGICAL_IF) {
            parseNodeIf(parent);
            continue;
        }

        // 2. VARIABLE DEFINITIONS
        if (isVariableDefinition()) {
            addASTNode( parent, parseTypeLiteral() );
            continue;
        }

        if (currentToken().type == TOK_FUNCTION_CALL) {
            printf("IS TOK_FUNCTION CALL: TOKEN: %s  \n \n", lexerTokenToString(currentToken().type));
            ASTNode *func = parseFunctionCall();

            if (currentToken().type != TOK_SEMICOLON) {
                syntaxError("Expected ';' after function call", beforeToken());
                return func;
            }

            nextPos(); // Skip ;

            addASTNode(parent, *func);
            printf("FINISHED PARSING TOK_FUNCTION_CALL \n");
            continue;
        }

        if ( currentToken().type == TOK_RETURN ) {
            nextPos(); // Skip TOK_RETURN

            ASTNode node = (ASTNode){
                .type = AST_RETURN,
                .child =  NULL
            };

            if ( currentToken().type != TOK_SEMICOLON) {
                node.child = parseExpression(0);

                if ( currentToken().type != TOK_SEMICOLON) {
                    syntaxError("Expected ';' after return", beforeToken());
                }
            }

            nextPos(); // Skip ;

            addASTNode( parent, node);
            continue;
        }


        nextPos();
    }

    printf("\n\n----------------END BODY-------------\n\n");
}

ASTNode getAST() {
    ASTNode *parent = malloc(sizeof(ASTNode));
    parent->type = AST_PROGRAM;
    parent->block.children = NULL;
    parent->block.capacity = 0;
    parent->block.count = 0;

    parseBody(&parent->block);

    parserPrintAST(parent);

    if (syntax_error_count > 0) {
        fprintf(stderr,
                "\nCompilation failed with %d syntax error(s).\n",
                syntax_error_count
        );
        exit(EXIT_FAILURE);
    }

    return *parent;
}
