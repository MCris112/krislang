//
// Created by MCris112 on 12/18/2025.
//

#ifndef KRISLANG_LEXER_H
#define KRISLANG_LEXER_H
#include <stdbool.h>

typedef enum {
    TOK_ERROR,

    // VARIABLES
    TOK_VARIABLE_TYPE_INT,
    TOK_VARIABLE_TYPE_STRING,
    TOK_VARIABLE_TYPE_BOOLEAN,
    TOK_VARIABLE_TYPE_FLOAT,
    TOK_VARIABLE_TYPE_CHAR,
    TOK_VARIABLE_TYPE_VOID,
    TOK_VARIABLE,

    // FUNCTION
    TOK_FUNCTION_CALL,
    TOK_FUNCTION_DEFINITION,

    //---------------------------
    // Comparison operators
    //---------------------------
    TOK_LESS_THAN,
    TOK_GREATER_THAN,
    TOK_LESS_EQUAL,
    TOK_GREATER_EQUAL,
    TOK_EQUAL_EQUAL,
    TOK_NOT_EQUAL,

    //---------------------------
    // Logical operators
    //---------------------------
    TOK_LOGICAL_AND,
    TOK_LOGICAL_OR,
    TOK_LOGICAL_NOT,

    //---------------------------
    // Assignment operators
    //---------------------------
    TOK_EQUALS,
    TOK_PLUS_ASSIGN,
    TOK_MINUS_ASSIGN,
    TOK_MUL_ASSIGN,
    TOK_DIV_ASSIGN,

    //---------------------------
    // Arithmetic operators
    //---------------------------
    TOK_PLUS, // +
    TOK_MINUS, // -
    TOK_MULTIPLY, // *
    TOK_DIVIDE, // /
    TOK_MODULO,// %
    TOK_INCREMENT, // ++
    TOK_DECREMENT, // --

    //---------------------------
    // Delimiters & punctuation
    //---------------------------
    TOK_PARENTHESIS_OPEN,
    TOK_PARENTHESIS_CLOSE,
    TOK_BRACE_OPEN,
    TOK_BRACE_CLOSE,
    TOK_BRACKET_OPEN,
    TOK_BRACKET_CLOSE,
    TOK_COMMA,
    TOK_SEMICOLON,
    TOK_COLON,

    //---------------------------
    // Arrow / Access operators
    //---------------------------
    TOK_ARROW,
    TOK_DOT,

    //---------------------------
    // LITERALS
    //---------------------------
    TOK_TEXT,
    TOK_CHAR,
    TOK_NUMBER,
    TOK_NUMBER_DECIMAL,
    TOK_LITERAL_BOOLEAN,

    TOK_WHITESPACE,
    TOK_COMMENT_NORMAL // -> //
} TokenType;

typedef struct {
    TokenType type;
    int line;
    int column;

    // Have to be string or number
    union {
        char *text;   // used for STRING (and keyword literals), NULL for numbers unless you want text
        int number;   // used for NUMBER
        double decimal;
        bool boolean;
    };
} Token;

void parseLexer(const char *input);

Token *getTokens();

int getTokensCount();

#endif //KRISLANG_LEXER_H