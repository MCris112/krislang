//
// Created by crisv on 12/21/2025.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../debug.h"

// bool evalVariableDefinition() {
//     ASTNode *node = malloc(sizeof(ASTNode));
//     if (!node) {
//         perror("malloc");
//         exit(EXIT_FAILURE);
//     }
//
//     Token token = currentToken();
//
//     switch (token.type) {
//         case TOK_NUMBER:
//             *node = ( ASTNode){
//                 .type = AST_NUMBER,
//                 .number = token.number
//             };
//             break;
//         case TOK_TEXT:
//             *node = ( ASTNode){
//                 .type = AST_TEXT,
//                 .text = token.text
//             };
//             break;
//         case TOK_NUMBER_DECIMAL:
//             *node = ( ASTNode){
//                 .type = AST_NUMBER_DECIMAL,
//                 .decimal = token.decimal
//             };
//             break;
//         case TOK_CHAR:
//             *node = ( ASTNode){
//                 .type = AST_CHAR,
//                 .text = token.text
//             };
//             break;
//         case TOK_LITERAL_BOOLEAN:
//             *node = ( ASTNode){
//                 .type = AST_BOOLEAN,
//                 .boolean = token.boolean
//             };
//             break;
//         default:
//             // Not a variable type
//             return false;
//     }
//
//     return true;
// }

void parserAddFunctionArgument(ASTFunctionArguments arguments, ASTNode *arg) {

    // Upper memory size for function if is need it
    if ( arguments.count >= arguments.capacity) {
        arguments.capacity = arguments.capacity ? arguments.capacity * 2 : 8;

        arguments.children = realloc(
            arguments.children,
            arguments.capacity * sizeof(ASTNode *)
        );
    }

    arguments.children[arguments.count] = arg;
    arguments.count++;
}

/**
 * Read all token and parse the arguments, also create the ASTNode inside
 * so this can avoid to forget on let empty model and couses crash
 * @param arguments
 */
void parseFunctionArguments( ASTFunctionArguments arguments  ) {
    arguments = (ASTFunctionArguments){
        .children = NULL,
        .count = 0,
        .capacity = 8,
    };

    while (!isEnd() && currentToken().type != TOK_PARENTHESIS_CLOSE) {
        nextPos(); // skip coma or first parentesis

        Token token = currentToken();
        ASTNode *arg = parseExpression(0);

        if (arg == NULL) {
            syntaxError("Unexpected parameter", token);
            break;
        }

        parserAddFunctionArgument(arguments, arg);

        if (currentToken().type != TOK_PARENTHESIS_CLOSE) {
            if (currentToken().type != TOK_COMMA) {
                syntaxError("Expected to close or more args", currentToken());
                break;
            }
        }
    }

    if (isEnd()) {
        syntaxError("Expected to close or more args", currentToken());
        return;
    }

    nextPos(); // skip ')'
}

ASTNode *parseFunctionCall() {
    Token functionCall = currentToken(); // function name token
    nextPos();

    // Expect '('
    if (currentToken().type != TOK_PARENTHESIS_OPEN) {
        syntaxError("Expected '(' after function name", currentToken());
        return NULL;
    }

    ASTNode *func = malloc(sizeof(ASTNode));
    func->type = AST_FUNCTION_CALL;
    func->funcCall.name = strdup(functionCall.text);

   parseFunctionArguments( func->funcCall.arguments );

    if ( currentToken().type != TOK_SEMICOLON ) {
        syntaxError("Expected ';' after function call", beforeToken() );
        return func;
    }

    nextPos();

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
            syntaxError("Expected expression", token);
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

    return node;
}
