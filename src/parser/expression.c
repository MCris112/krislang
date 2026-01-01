//
// Created by crisv on 12/21/2025.
//

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../debug.h"
#include "../helpers/helper.h"


int getPrecedence(LexerTokenType type) {
    switch (type) {
        case TOK_MULTIPLY:
        case TOK_DIVIDE:
        case TOK_MODULO:
            return 50;

        case TOK_PLUS:
        case TOK_MINUS:
            return 40;

        case TOK_EQUAL_EQUAL:
        case TOK_NOT_EQUAL:
            return 30;

        case TOK_GREATER_THAN:
        case TOK_GREATER_EQUAL:
        case TOK_LESS_THAN:
        case TOK_LESS_EQUAL:
            return 25;   // lower than + -, higher than logical


        case TOK_LOGICAL_AND:
            return 20;

        case TOK_LOGICAL_OR:
            return 10;

        default:
            return 0;
    }
}

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

ASTNode parseFunctionDefinition(FunctionDefinitionVisibility visibility) {
    ASTNode node = (ASTNode){
        .type = AST_FUNCTION_DEFINITION,
        .funcDefinition = {
            .body = NULL,
            .name = strdup(currentToken().text),
            .arguments = (ASTFunctionArguments){0},
            .visibility = visibility,
        }
    };

    nextPos(); //Skip AST_FUNCTION_DEFINITION

    if (currentToken().type != TOK_PARENTHESIS_OPEN) {
        syntaxError("Expected ( after function name", beforeToken());
    }

    nextPos(); // skip (

    while (!isEnd() && currentToken().type != TOK_PARENTHESIS_CLOSE) {
        //nextPos(); // skip coma or first parentesis

        Token token = currentToken(); // for error debug
        ASTNode *arg = parseParameter();

        parserAddFunctionArgument( &node.funcDefinition.arguments, arg);

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
        syntaxError("Expected ')' in function", currentToken());
    }

    nextPos(); // skip ')'

    if ( currentToken().type != TOK_BRACE_OPEN ) {
        syntaxError("Expected { after function definition", beforeToken() );
    }

    nextPos();

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
            printf("CURRENT: %s\n", lexerTokenToString(currentToken().type));
            syntaxError("Expected ',' or ')'", currentToken());
        }
    }

    if (isEnd()) {
        syntaxError("Expected to close or more args", currentToken());
        return;
    }

    if (currentToken().type != TOK_PARENTHESIS_CLOSE) {
        syntaxError("Expected ')' in function to end arguments", currentToken());
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

// ============================
// PRIMARY EXPRESSIONS
// ============================
ASTNode *parsePrimary(int deep) {
    Token token = currentToken();
    ASTNode *node = malloc(sizeof(ASTNode));

    // Some cases the function already do nextPos() so i better decide what need to avoid skipping
    bool needToSkip = true;

    switch (token.type) {
        case TOK_IDENTIFIER:
            if ( nextToken().type == TOK_PARENTHESIS_OPEN ) {
                node = parseFunctionCall();
                needToSkip = false;
            }else {
                node->type = AST_FUNCTION_REFERENCE;
                node->text = strdup(token.text);
            }
            break;
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
            node->text = strdup(token.text);
            break;
        case TOK_CHAR:
            node->type = AST_CHAR;
            node->character = token.text[0];
            break;

        case TOK_NUMBER:
            node->type = AST_NUMBER;
            node->number = token.number;
            break;

        case TOK_NUMBER_DECIMAL:
            node->type = AST_NUMBER_DECIMAL;
            node->decimal = token.decimal;
            break;

        case TOK_LITERAL_BOOLEAN:
            node->type = AST_BOOLEAN;
            node->boolean = token.boolean;
            break;
        case TOK_VARIABLE:
            node->type = AST_VARIABLE_CAST;
            node->text = strdup(currentToken().text);
            break;

        case TOK_PARENTHESIS_OPEN:
            nextPos(); // consume '('
            node = parseExpression(0); // Reset precedence inside parentheses
            if (currentToken().type != TOK_PARENTHESIS_CLOSE) {
                syntaxError("Expected ')' in expression", currentToken());
            }
            break;

        default:
            syntaxError("Expected primary expression", token);
            node->type = AST_ERROR;
            return node;
    }

    if (needToSkip)
        nextPos();

   return node;
}


ASTNode *parseExpression(int deep) {
    if (isEnd()) {
        ASTNode *err = malloc(sizeof(ASTNode));
        err->type = AST_ERROR;

        syntaxError("Expected more code here...", beforeToken() );
        return err;
    }

    ASTNode *node = NULL;
    Token token = currentToken();

    // ============================
    // UNARY (PREFIX) — HIGHEST PRECEDENCE
    // ============================

    if (token.type == TOK_MINUS) {
        node = malloc(sizeof(ASTNode));
        node->type = AST_UNARY;
        node->unary.operator = TOK_MINUS;
        nextPos(); // consume '-'
        node->unary.operand = parseExpression(100);
    }else {
        node = parsePrimary(deep);
    }


    // ============================
    // BINARY OPERATORS (LEFT‑ASSOCIATIVE)
    // ===========================
    if (currentToken().type == TOK_SEMICOLON || currentToken().type == TOK_COMMA || currentToken().type == TOK_PARENTHESIS_CLOSE ) {
        return node;
    }

    while ( !isEnd() ) {
        if (currentToken().type == TOK_PARENTHESIS_CLOSE) break;

        Token operator = currentToken();
        int opPrecedence = getPrecedence(operator.type);

        if (opPrecedence <= deep)
            break;

        nextPos();

        ASTNode *right = parseExpression(opPrecedence);

        ASTNode *unionNode = malloc(sizeof(ASTNode));
        unionNode->binary.left = node;
        unionNode->binary.right = right;


        switch ( operator.type ) {
            case TOK_PLUS: unionNode->type = AST_CONCAT; break;
            case TOK_MINUS: unionNode->type = AST_SUBTRACT; break;
            case TOK_MULTIPLY: unionNode->type = AST_MULTIPLY; break;
            case TOK_DIVIDE: unionNode->type = AST_DIVIDE; break;
            case TOK_MODULO: unionNode->type = AST_MODULO; break;
            case TOK_EQUAL_EQUAL:
            case TOK_LESS_THAN:
            case TOK_LESS_EQUAL:
            case TOK_GREATER_THAN:
            case TOK_GREATER_EQUAL:
            case TOK_NOT_EQUAL:
            {
                ASTNode *compare = malloc(sizeof(ASTNode));
                compare->type = AST_COMPARE;
                compare->compare.operator = operator.type;
                compare->compare.left = node;
                compare->compare.right = right;

                unionNode = compare;
                break;
            }
            default: syntaxError("Unknown operator", operator);
        }

        node = unionNode;
    }

    return node;
}
