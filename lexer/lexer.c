//
// Created by MCris112 on 12/18/2025.
//

#include "lexer.h"

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
                   .text = "(",
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

        pos++;
    }
}