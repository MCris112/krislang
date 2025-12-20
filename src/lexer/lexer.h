//
// Created by MCris112 on 12/18/2025.
//

#ifndef KRISLANG_LEXER_H
#define KRISLANG_LEXER_H

typedef enum {

    // VARIABLES
    TOK_VARIABLE_TYPE_INT,
    TOK_VARIABLE_TYPE_STRING,
    TOK_VARIABLE_TYPE_BOOLEAN,
    TOK_VARIABLE_TYPE_FLOAT,
    TOK_VARIABLE_VOID,
    TOK_VARIABLE,


    TOK_PRINT,
    TOK_PARENTHESIS_OPEN,
    TOK_PARENTHESIS_CLOSE,

    TOK_TEXT,
    TOK_NUMBER,
    TOK_NUMBER_DECIMAL,

    TOK_SUM, // +
    TOK_MINUS, // -
    TOK_DOT,
    TOK_EQUALS,

    TOK_WHITESPACE,
    TOK_SEMICOLON,
} TokenType;

typedef struct {
    TokenType type;
    char *text;   // used for STRING (and keyword literals), NULL for numbers unless you want text
    int number;   // used for NUMBER
} Token;

void parseLexer(const char *input);

Token *getTokens();

int getTokensCount();

const char *tokenTypeToString(TokenType type);

#endif //KRISLANG_LEXER_H