//
// Created by MCris112 on 12/18/2025.
//

#ifndef KRISLANG_LEXER_H
#define KRISLANG_LEXER_H

typedef enum {

    TOK_PRINT,
    TOK_PARENTESIS_OPEN,
    TOK_PARENTESIS_CLOSE,

    TOK_TEXT,

    TOK_SUM, // +

} TokenType;

typedef struct {
    TokenType type;
    char *text;   // used for STRING (and keyword literals), NULL for numbers unless you want text
    int number;   // used for NUMBER
} Token;

void parseLexer(const char *input);

Token *getTokens();

int getTokensCount();

#endif //KRISLANG_LEXER_H