//
// Created by MCris112 on 12/18/2025.
//

#include "../src/lexer/lexer.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


Token *tokens = NULL;
int tokenCount = 0;
int capacity = 0;

void addToken( const Token token ) {
    if ( tokenCount >= capacity ) {
        capacity = capacity == 0 ? 8 : capacity*2;
        tokens = realloc(tokens, capacity*sizeof(Token));

        // In case fail realloc to set new size
        if (!tokens ) {
            perror("realloc");
            exit(1);
        }
    }

    tokens[tokenCount++] = token;
}

Token *getTokens() {
    return tokens;
}

int getTokensCount()
{
    return tokenCount;
}

void parseLexer(const char *input) {

    // Array of all character witout change the main
    const char *pos = input;

    while ( *pos ) {



        if ( *pos == '"') {
            pos++; // skip opening quote

            const char *start = pos;

            while (*pos && *pos != '"') {
                pos++;
            }

            if (*pos != '"') {
                fprintf(stderr, "Unterminated string literal\n");
                exit(1);
            }

            size_t length = pos - start;

            char *text = malloc(length + 1);
            if (!text) {
                perror("malloc");
                exit(1);
            }

            memcpy(text, start, length);
            text[length] = '\0';

            addToken((Token){
                .type = TOK_TEXT,
                .text = text,
                .number = 0
            });

            pos++; // skip closing quote
            continue;
        }


        if ( *pos == ';') {
            addToken( (Token){
                .type = TOK_SEMICOLON,
                .text = ";"
            });
            pos++;
            continue;
        }

        // maybe a variable
        if ( *pos == '$') {
            const char *start = pos;
            pos++; // skip '$'

            // Variable name must start with letter or underscore
            if (!isalpha(*pos) && *pos != '_') {
                fprintf(stderr, "Invalid variable name\n");
                exit(EXIT_FAILURE);
            }

            // Allow letters and digits only (no underscores after start)
            while (isalnum(*pos)) {
                pos++;
            }

            size_t length = pos - start;

            char *text = malloc(length + 1);
            if (!text) {
                perror("malloc");
                exit(1);
            }

            memcpy(text, start, length);
            text[length] = '\0';

            addToken((Token){
                .type = TOK_VARIABLE,
                .text = text,
                .number = 0
            });
            continue;

        }

        if ( *pos == '=') {
            addToken( (Token){
                .type = TOK_EQUALS,
                .text = "=",
            } );
            pos++;
            continue;
        }

        if ( isdigit(*pos) ) {
            const char *start = pos;

            while ( isdigit(*pos) ) {
                pos++;
            }

            size_t length = pos - start;
            char *text = malloc(length + 1);
            if (!text) {
                perror("malloc");
                exit(EXIT_FAILURE);
            }

            memcpy(text, start, length);
            text[length] = '\0';

            char *endptr;
            errno = 0;
            long value = strtol(text, &endptr, 10);

            if (errno != 0 || *endptr != '\0') {
                fprintf(stderr, "Invalid integer literal: %s\n", text);
                exit(EXIT_FAILURE);
            }

            addToken((Token){
                .type = TOK_NUMBER,
                .number = (int) value
            });
            free(text);
            continue;
        }

        /*
         * 1. check if letter actually exists
         * 2. Check if is the word print
         */

        if ( *pos && strncmp(pos, "print", 5) == 0 && !isalnum((unsigned char)pos[5])) {
            addToken(
                (Token) {
                    .type = TOK_PRINT,
                    .text = "print",
                    .number = 0
                }
                );
            pos += 5;
            continue;
        }

        if ( *pos == '+') {
            addToken(
               (Token) {
                   .type = TOK_SUM,
                   .text = "+",
                   .number = 0
               }
               );
            pos++;
        }
        if ( *pos == '(') {
            addToken(
                (Token) {
                    .type = TOK_PARENTESIS_OPEN,
                    .text = "(",
                    .number = 0
                }
                );
            pos ++;
            continue;
        }

        if ( *pos == ')') {
            addToken(
                (Token) {
                    .type = TOK_PARENTESIS_CLOSE,
                    .text = "(",
                    .number = 0
                }
                );
            pos ++;
            continue;
        }


        if ( *pos && strncmp(pos, "INT", 3) == 0 ) {
            pos += 3;

            addToken( (Token){
                .type = TOK_VARIABLE_TYPE_INT
            } );
            continue;
        }
        pos++;
    }
}


const char *tokenTypeToString(TokenType type) {
    switch (type) {
        case TOK_PRINT:             return "TOK_PRINT";
        case TOK_PARENTESIS_OPEN:   return "TOK_PARENTESIS_OPEN";
        case TOK_PARENTESIS_CLOSE:  return "TOK_PARENTESIS_CLOSE";

        case TOK_TEXT:              return "TOK_TEXT";
        case TOK_NUMBER:            return "TOK_NUMBER";

        case TOK_SUM:               return "TOK_SUM";

        case TOK_VARIABLE:          return "TOK_VARIABLE";
        case TOK_SEMICOLON:         return "TOK_SEMICOLON";
        case TOK_EQUALS:            return "TOK_EQUALS";

        default:                    return "TOK_UNKNOWN";
    }
}
