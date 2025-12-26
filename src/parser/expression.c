//
// Created by crisv on 12/21/2025.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../debug.h"
#include "../helpers/helper.h"

ASTNode *parseParameter() {
    // Expect a type
    if (!isVariableDefinition() ){
        syntaxError("Expected parameter type", currentToken());
    }

    VarType type = parseTokToVarType(); // STRING, INT, FLOAT, etc.
    nextPos(); // skip type token

    // Expect a variable name
    if (currentToken().type != TOK_VARIABLE) {
        syntaxError("Expected parameter name", currentToken());
    }

    char *name = strdup(currentToken().text);
    nextPos(); // skip $nombre

    // Build AST node
    ASTNode *param = malloc(sizeof(ASTNode));
    param->type = AST_FUNCTION_PARAMETER;
    param->varDecl.varType = type;
    param->varDecl.name = name;
    param->varDecl.value = NULL; // parameters have no initial value

    printf("[parseParameter] Name: %s\n", name);
    return param;
}


void parserAddFunctionArgument(ASTFunctionArguments *arguments, ASTNode *arg) {
    if (arguments->children == NULL) {
        arguments->capacity = 8;
        arguments->count = 0;
        arguments->children = malloc(arguments->capacity * sizeof(ASTNode *));
        if (!arguments->children) {
            fprintf(stderr, "Out of memory allocating function arguments\n");
            exit(EXIT_FAILURE);
        }
    }
    // Upper memory size for function if is need it
    if (arguments->count >= arguments->capacity) {
        arguments->capacity *= 2;
        ASTNode **tmp = realloc(arguments->children, arguments->capacity * sizeof(ASTNode *));
        if (!tmp) {
            fprintf(stderr, "Out of memory growing function arguments\n");
            exit(EXIT_FAILURE);
        }
        arguments->children = tmp;
    }

    arguments->children[arguments->count++] = arg;
}

ASTNode parseFunctionDefinition() {
    ASTNode node = (ASTNode){
        .type = AST_FUNCTION_DEFINITION,
        .funcDefinition = {
            .body = NULL,
            .name = strdup(currentToken().text),
            .arguments = (ASTFunctionArguments){0}
        }
    };

    nextPos(); //Skip AST_FUNCTION_DEFINITION

    if (currentToken().type != TOK_PARENTHESIS_OPEN) {
        syntaxError("Expected ( after function name", beforeToken());
    }

    nextPos(); // skip (

    while (!isEnd() && currentToken().type != TOK_PARENTHESIS_CLOSE) {
        //nextPos(); // skip coma or first parentesis

        printf("[FUNC_DEF] INSIDE WHILE PARSE FUNCTION ARGS, Current: %s \n", lexerTokenToString(currentToken().type));
        Token token = currentToken(); // for error debug
        ASTNode *arg = parseParameter();

        parserAddFunctionArgument( &node.funcDefinition.arguments, arg);
        printf("[FUNC_DEF] ARG ADDED! now: %d: %s \n", node.funcDefinition.arguments.count, lexerTokenToString(currentToken().type));

        if (currentToken().type == TOK_COMMA) {
            nextPos(); // skip comma
        } else if (currentToken().type != TOK_PARENTHESIS_CLOSE) {
            syntaxError("Expected ',' or ')'", currentToken());
        }
    }

    if (isEnd()) {
        syntaxError("Expected to close or more args", currentToken());
    }

    if (currentToken().type != TOK_PARENTHESIS_CLOSE) {
        syntaxError("Expected ')'", currentToken());
    }

    nextPos(); // skip ')'

    if ( currentToken().type != TOK_BRACE_OPEN ) {
        syntaxError("Expected { after function definition", beforeToken() );
    }

    nextPos();
    printf("(4) - CURRENT TOKEN: %s\n\n\n", lexerTokenToString(currentToken().type));
    parseBody( &node.funcDefinition.body );

    if (currentToken().type != TOK_BRACE_CLOSE) {
        syntaxError("Expected '}' after body content", currentToken());
    }

    nextPos(); // Skipp }

    return node;
}


/**
 * Read all token and parse the arguments, also create the ASTNode inside
 * so this can avoid to forget on let empty model and couses crash
 * @param arguments
 */
void parseFunctionArguments(ASTFunctionArguments *arguments) {
    *arguments = (ASTFunctionArguments){0}; // CRITICAL

    nextPos(); //Skip TOK_FUNCTION_CALL

    if (currentToken().type != TOK_PARENTHESIS_OPEN) {
        syntaxError("Expected ( after function name", beforeToken());
    }

    nextPos();

    while (!isEnd() && currentToken().type != TOK_PARENTHESIS_CLOSE) {
        //nextPos(); // skip coma or first parentesis

        Token token = currentToken(); // for error debug
        ASTNode *arg = parseExpression(0);

        if (arg == NULL) {
            syntaxError("Unexpected parameter", token);
            break;
        }

        parserAddFunctionArgument(arguments, arg);

        if (currentToken().type == TOK_COMMA) {
            nextPos(); // skip comma
        } else if (currentToken().type != TOK_PARENTHESIS_CLOSE) {
            syntaxError("Expected ',' or ')'", currentToken());
        }
    }

    if (isEnd()) {
        syntaxError("Expected to close or more args", currentToken());
        return;
    }

    if (currentToken().type != TOK_PARENTHESIS_CLOSE) {
        syntaxError("Expected ')'", currentToken());
    }

    nextPos(); // skip ')'
}

ASTNode *parseFunctionCall() {
    Token functionCall = currentToken(); // function name token

    ASTNode *func = malloc(sizeof(ASTNode));
    func->type = AST_FUNCTION_CALL;
    func->funcCall.name = strdup(functionCall.text);

    parseFunctionArguments(&func->funcCall.arguments);

    return func;
}


ASTNode *parseExpression(int deep) {
    if (isEnd()) {
        ASTNode *err = malloc(sizeof(ASTNode));
        err->type = AST_ERROR;
        return err;
    }

    Token token = currentToken();

    // ============================
    // UNARY (PREFIX) — HIGHEST PRECEDENCE
    // ============================

    if (token.type == TOK_MINUS) {
        ASTNode *unary = malloc(sizeof(ASTNode));
        unary->type = AST_UNARY;
        unary->unary.operator = TOK_MINUS;
        nextPos(); // consume '-'
        unary->unary.operand = parseExpression(deep + 1);
        return unary; // IMPORTANT: stop here
    }

    // ============================
    // PRIMARY EXPRESSIONS
    // ============================

    ASTNode *node = malloc(sizeof(ASTNode));
    // Some cases the function already do nextPos() so i better decide what need to avoid skipping
    bool needToSkip = true;

    switch (token.type) {
        case TOK_VARIABLE_TYPE_INT:
        case TOK_VARIABLE_TYPE_STRING:
        case TOK_VARIABLE_TYPE_BOOLEAN:
        case TOK_VARIABLE_TYPE_FLOAT:
        case TOK_VARIABLE_TYPE_CHAR:
        case TOK_VARIABLE_TYPE_VOID:
            ASTNode literal = parseTypeLiteral();
            node->type = literal.type;
            node->literal = literal.literal;
            needToSkip = false;
            break;
        case TOK_TEXT:
            node->type = AST_TEXT;
            node->text = strdup(currentToken().text);
            break;
        case TOK_CHAR:
            node->type = AST_CHAR;
            node->character = currentToken().text[0];
            break;
        case TOK_NUMBER:
            node->type = AST_NUMBER;
            node->number = currentToken().number;
            break;
        case TOK_NUMBER_DECIMAL:
            node->type = AST_NUMBER_DECIMAL;
            node->decimal = currentToken().decimal;
            break;
        case TOK_LITERAL_BOOLEAN:
            node->type = AST_BOOLEAN;
            node->boolean = currentToken().boolean;
            break;
        case TOK_VARIABLE:
            node->type = AST_VARIABLE_CAST;
            node->text = strdup(currentToken().text);
            break;
        case TOK_FUNCTION_CALL:
            node = parseFunctionCall();

            // Avoid debug, cuz in parseFunctionCall() already skip one that is the parentesis, so
            // The program can skip again or will skip the TOK_SEMICOLON
            needToSkip = false;
            break;
        default:
            syntaxError(strFormat("Expected expression, - Current: %s", lexerTokenToString(token.type)), token);
            ASTNode *err = malloc(sizeof(ASTNode));
            err->type = AST_ERROR;
            return err;
    }

    if (needToSkip)
        nextPos();

    // ============================
    // BINARY OPERATORS (LEFT‑ASSOCIATIVE)
    // ===========================

    if (currentToken().type == TOK_SEMICOLON) {
        printf("[EXPRESSION SKYKING ON : %s ]\n", lexerTokenToString(currentToken().type));
        return node;
    }

    // CONCAT (+)
    if (currentToken().type == TOK_PLUS) {
        nextPos(); // consume '+'

        ASTNode *right = parseExpression(deep);

        ASTNode *concat = malloc(sizeof(ASTNode));
        concat->type = AST_CONCAT;
        concat->binary.left = node;
        concat->binary.right = right;

        node = concat;
    }

    // SUBTRACT (-)
    if (currentToken().type == TOK_MINUS) {
        nextPos(); // consume '-'

        ASTNode *right = parseExpression(deep);

        ASTNode *sub = malloc(sizeof(ASTNode));
        sub->type = AST_SUBTRACT;
        sub->binary.left = node;
        sub->binary.right = right;

        node = sub;
    }


    // COMPARE (==)
    if (currentToken().type == TOK_EQUAL_EQUAL) {
        nextPos(); // consume '=='

        ASTNode *right = parseExpression(deep);

        ASTNode *compare = malloc(sizeof(ASTNode));
        compare->type = AST_COMPARE;
        compare->binary.left = node;
        compare->binary.right = right;

        node = compare;
    }

    printf("[EXPRESSION][FINISHED] Current: %s \n", lexerTokenToString(currentToken().type));

    return node;
}
