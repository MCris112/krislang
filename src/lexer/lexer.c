//
// Created by MCris112 on 12/18/2025.
//

#include "lexer.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../debug.h"

// For error lines
int currentLine = 0;
int currentColumn = 0;

Token *tokens = NULL;
int tokenCount = 0;
int capacity = 0;

void addToken(const Token token) {
    if (tokenCount >= capacity) {
        capacity = capacity == 0 ? 8 : capacity * 2;
        tokens = realloc(tokens, capacity * sizeof(Token));

        // In case fail realloc to set new size
        if (!tokens) {
            perror("realloc");
            exit(1);
        }
    }

    tokens[tokenCount++] = token;
}

Token *getTokens() {
    return tokens;
}


int getTokensCount() {
    return tokenCount;
}

bool evalTokenVariable(char *typeName, const char **reference, TokenType type) {
    int startColumn = currentColumn;
    const char *pos = *reference;

    if (!*pos)
        return false;

    size_t len = strlen(typeName);

    // Check if actual type name
    if (strncmp(pos, typeName, len) != 0)
        return false;

    // Allow: TYPE(8), TYPE $var
    /*
     if (pos[len] != ' ' && pos[len] != '(') {

        return false;
    }*/

    addToken((Token){
        .type = type,
        .text = typeName,
        .line = currentLine,
        .column = startColumn
    });

    *reference += len; // Advance caller's pointer
    currentColumn += len;

    return true;
}

char *evalTokenFunction(const char **reference) {
    const char *pos = *reference;

    if (!*pos)
        return NULL;

    // Function name must start with letter or _
    if (!isalpha(*pos) && *pos != '_')
        return NULL;

    const char *start = pos;

    while (isalnum(*pos) || *pos == '_')
        pos++;

    // Must be followed by '('
    if (*pos != '(') {
        *reference = start;
        return NULL;
    }

    size_t length = pos - start;
    char *text = malloc(length + 1);
    if (!text) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    memcpy(text, start, length);
    text[length] = '\0';

    *reference = pos;
    currentColumn += length;
    return text;
}

bool evalTokenText(const char **reference, char *content, const TokenType type) {
    int startColumn = currentColumn;
    const char *pos = *reference;

    if (!*pos)
        return false;

    size_t len = strlen(content);

    if (strncmp(*reference, content, len) != 0)
        return false;

    addToken(
        (Token){
            .type = type,
            .text = content,
            .line = currentLine,
            .column = startColumn
        }
    );

    *reference += len;
    currentColumn += len;
    return true;
}

const char *code;

const char *getCode() {
    return code;
}

/**
 * Main important component of compiler, take actual text into TOKENS
 * @param input
 */
void parseLexer(const char *input) {
    code = input;
    // Array of all character without change the main
    const char *pos = input;

    while (*pos) {
        if (*pos == '\n') {
            currentLine++;
            currentColumn = 0;
            pos++;
            continue;
        }

        // Explicitly skip whitespace:
        if (*pos == ' ' || *pos == '\t' || *pos == '\r') {
            pos++;
            currentColumn++;
            continue;
        }


        //---------------------------
        // Parse Comments
        //---------------------------

        if (*pos && strncmp(pos, "//", 2) == 0) {
            int startColumn = currentColumn;
            const char *start = pos;

            while (*pos && *pos != '\n') {
                pos++;
                currentColumn++;
            }

            size_t length = pos - start;
            char *text = malloc(length + 1);
            if (!text) {
                perror("malloc");
                exit(EXIT_FAILURE);
            }

            memcpy(text, start, length);
            text[length] = '\0';

            addToken((Token){
                .type = TOK_COMMENT_NORMAL,
                .text = text,
                .line = currentLine,
                .column = startColumn
            });

            continue;
        }

        //---------------------------
        // Parse String
        //---------------------------
        if (*pos == '"') {
            int startColumn = currentColumn;
            pos++; // skip opening quote
            currentColumn++;

            const char *start = pos;

            while (*pos && *pos != '"') {
                pos++;
                currentColumn++;
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
                .line = currentLine,
                .column = startColumn
            });

            pos++; // skip closing quote
            continue;
        }

        //---------------------------
        // Parse Char
        //---------------------------
        if (*pos == '\'') {
            int startColumn = currentColumn;
            pos++;
            currentColumn++; // skip opening '

            if (*pos == '\0') {
                addToken((Token){
                    .type = TOK_ERROR,
                    .text = "Unterminated char literal",
                    .line = currentLine,
                    .column = startColumn
                });
                return;
            }

            char value;

            // Handle escape sequences
            if (*pos == '\\') {
                pos++;
                currentColumn++; // skip '\'

                switch (*pos) {
                    case '0': value = '\0';
                        break;
                    case 'n': value = '\n';
                        break;
                    case 't': value = '\t';
                        break;
                    case '\\': value = '\\';
                        break;
                    case '\'': value = '\'';
                        break;
                    default: addToken((Token){
                            .type = TOK_ERROR, .text = "Invalid escape sequence in char literal", .line = currentLine,
                            .column = startColumn
                        });
                        continue;
                }
            } else {
                value = *pos;
            }

            pos++;
            currentColumn++; // consume character

            // Expect closing '
            if (*pos != '\'') {
                addToken((Token){
                    .type = TOK_ERROR, .text = "Expected closing ' for char literal", .line = currentLine,
                    .column = startColumn
                });
                continue;
            }

            pos++;
            currentColumn++; // skip closing '

            // Store char as string length 1

            char *text = malloc(2);
            text[0] = value;
            text[1] = '\0';
            addToken((Token){.type = TOK_CHAR, .text = text, .line = currentLine, .column = startColumn});
            continue;
        }

        //---------------------------
        // Parse TRUE | FALSE
        //---------------------------
        if (strncmp(pos, "TRUE", 4) == 0 && !isalnum(pos[4])) {
            addToken((Token){
                .type = TOK_LITERAL_BOOLEAN,
                .boolean = true,
                .line = currentLine,
                .column = currentColumn
            });

            pos += 4;
            currentColumn += 4;
            continue;
        }

        if (strncmp(pos, "FALSE", 5) == 0 && !isalnum(pos[5])) {
            addToken((Token){
                .type = TOK_LITERAL_BOOLEAN,
                .boolean = false,
                .line = currentLine,
                .column = currentColumn
            });

            pos += 5;
            currentColumn += 5;
            continue;
        }

        if (*pos == ';') {
            addToken((Token){
                .type = TOK_SEMICOLON,
                .text = ";",
                .line = currentLine,
                .column = currentColumn
            });
            pos++;
            currentColumn++;
            continue;
        }

        //---------------------------
        // Parse Variables
        //---------------------------
        if (*pos == '$') {
            int startColumn = currentColumn;
            const char *start = pos;
            pos++; // skip '$'
            currentColumn++;

            // Variable name must start with letter or underscore
            if (!isalpha(*pos) && *pos != '_') {
                fprintf(stderr, "Invalid variable name\n");
                exit(EXIT_FAILURE);
            }

            // Allow letters and digits only (no underscores after start)
            while (isalnum(*pos)) {
                pos++;
                currentColumn++;
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
                .line = currentLine,
                .column = startColumn
            });

            continue;
        }

        //---------------------------
        // Parse Variables Types
        //---------------------------
        if (evalTokenVariable("INT", &pos, TOK_VARIABLE_TYPE_INT)) {
            continue;
        }

        if (evalTokenVariable("STRING", &pos, TOK_VARIABLE_TYPE_STRING)) {
            continue;
        }

        if (evalTokenVariable("CHAR", &pos, TOK_VARIABLE_TYPE_CHAR)) {
            continue;
        }

        // TODO allow custom size on left.rigth FLOAT<2,3> = 32.123
        if (evalTokenVariable("FLOAT", &pos, TOK_VARIABLE_TYPE_FLOAT)) {
            continue;
        }

        if (evalTokenVariable("BOOLEAN", &pos, TOK_VARIABLE_TYPE_BOOLEAN)) {
            continue;
        }
        if (evalTokenVariable("VOID", &pos, TOK_VARIABLE_TYPE_VOID)) {
            continue;
        }

        //---------------------------
        // Parse Function
        //---------------------------
        int startColumn = currentColumn;
        char *funcName = evalTokenFunction(&pos);
        if (funcName) {
            addToken((Token){
                .type = TOK_FUNCTION_CALL,
                .text = funcName,
                .line = currentLine,
                .column = startColumn
            });
            currentColumn += strlen(funcName);
            continue;
        }

        if ( strncmp(pos, "RETURN", 6) == 0 ) {
            addToken((Token){
                .type = TOK_RETURN,
                .line = currentLine,
                .column = startColumn
            });

            pos +=6;
            currentColumn += 6;
            continue;
        }

        //---------------------------
        // Comparison operators
        //---------------------------
        if (evalTokenText(&pos, "<=", TOK_LESS_EQUAL)) continue;
        if (evalTokenText(&pos, ">=", TOK_GREATER_EQUAL)) continue;
        if (evalTokenText(&pos, "==", TOK_EQUAL_EQUAL)) continue;
        if (evalTokenText(&pos, "!=", TOK_NOT_EQUAL)) continue;

        if (evalTokenText(&pos, "<", TOK_LESS_THAN)) continue;
        if (evalTokenText(&pos, ">", TOK_GREATER_THAN)) continue;

        //---------------------------
        // Logical
        //---------------------------
        if (strncmp(pos, "IF", 2) == 0) {
            addToken((Token){
                .type = TOK_LOGICAL_IF,
                .line = currentLine,
                .column = startColumn
            });

            pos += 2;
            continue;
        }

        if (strncmp(pos, "ELSE", 4) == 0) {
            addToken((Token){
                .type = TOK_LOGICAL_ELSE,
                .line = currentLine,
                .column = startColumn
            });

            pos += 4;
            continue;
        }
        if (evalTokenText(&pos, "&&", TOK_LOGICAL_AND)) continue;
        if (evalTokenText(&pos, "||", TOK_LOGICAL_OR)) continue;
        if (evalTokenText(&pos, "!", TOK_LOGICAL_NOT)) continue;


        //---------------------------
        // Assignment operators
        //---------------------------
        if (evalTokenText(&pos, "+=", TOK_PLUS_ASSIGN)) continue;
        if (evalTokenText(&pos, "-=", TOK_MINUS_ASSIGN)) continue;
        if (evalTokenText(&pos, "*=", TOK_MUL_ASSIGN)) continue;
        if (evalTokenText(&pos, "/=", TOK_DIV_ASSIGN)) continue;

        if (evalTokenText(&pos, "=", TOK_EQUALS)) continue;


        //---------------------------
        // Arithmetic operators
        //---------------------------
        if (evalTokenText(&pos, "++", TOK_INCREMENT)) continue;
        if (evalTokenText(&pos, "--", TOK_DECREMENT)) continue;

        if (evalTokenText(&pos, "+", TOK_PLUS)) continue;
        if (evalTokenText(&pos, "-", TOK_MINUS)) continue;
        if (evalTokenText(&pos, "*", TOK_MULTIPLY)) continue;
        if (evalTokenText(&pos, "/", TOK_DIVIDE)) continue;
        if (evalTokenText(&pos, "%", TOK_MODULO)) continue;

        //---------------------------
        // Delimiters & punctuation
        //---------------------------
        if (evalTokenText(&pos, "(", TOK_PARENTHESIS_OPEN)) continue;
        if (evalTokenText(&pos, ")", TOK_PARENTHESIS_CLOSE)) continue;
        if (evalTokenText(&pos, "{", TOK_BRACE_OPEN)) continue;
        if (evalTokenText(&pos, "}", TOK_BRACE_CLOSE)) continue;
        if (evalTokenText(&pos, "[", TOK_BRACKET_OPEN)) continue;
        if (evalTokenText(&pos, "]", TOK_BRACKET_CLOSE)) continue;

        if (evalTokenText(&pos, ",", TOK_COMMA)) continue;
        if (evalTokenText(&pos, ";", TOK_SEMICOLON)) continue;
        if (evalTokenText(&pos, ":", TOK_COLON)) continue;

        //---------------------------
        // Arrow / Access operators
        //---------------------------
        if (evalTokenText(&pos, "->", TOK_ARROW)) continue;
        if (evalTokenText(&pos, ".", TOK_DOT)) continue;

        //---------------------------
        // Parse Number (integer / decimal)
        // TODO support: numeric suffixes (1f, 1.0d), hex / binary literals, scientific notation (1e-3)
        //---------------------------
        if (isdigit(*pos)) {
            startColumn = currentColumn;
            TokenType type = TOK_NUMBER;
            const char *start = pos;

            while (isdigit(*pos)) {
                pos++;
                currentColumn++;
            }

            if (*pos == '.') {
                type = TOK_NUMBER_DECIMAL;
                pos++;
                currentColumn++;

                while (isdigit(*pos)) {
                    pos++;
                    currentColumn++;
                }
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

            if (type == TOK_NUMBER_DECIMAL) {
                double value = strtod(text, &endptr);

                if (errno != 0 || *endptr != '\0') {
                    fprintf(stderr, "Invalid decimal literal: %s\n", text);
                    exit(EXIT_FAILURE);
                }

                addToken((Token){
                    .type = TOK_NUMBER_DECIMAL,
                    .decimal = value,
                    .line = currentLine,
                    .column = startColumn
                });
            } else {
                long value = strtol(text, &endptr, 10);

                if (errno != 0 || *endptr != '\0') {
                    fprintf(stderr, "Invalid integer literal: %s\n", text);
                    exit(EXIT_FAILURE);
                }

                addToken((Token){
                    .type = TOK_NUMBER,
                    .number = (int) value,
                    .line = currentLine,
                    .column = startColumn
                });
            }

            free(text);
            continue;
        }

        if ( isalpha(*pos) || *pos == '_' ) {
            const char *start = pos;

            while (isalnum(*pos) || *pos == '_')
                pos++;

            size_t length = pos - start;
            char *text = malloc(length + 1);
            if (!text) {
                perror("malloc");
                exit(EXIT_FAILURE);
            }

            memcpy(text, start, length);
            text[length] = '\0';

            addToken( (Token){
                .type = TOK_IDENTIFIER,
                .text = strdup(text),
            } );
            continue;
        }

        pos++;
        currentColumn++;
    }

    addToken((Token){ .type = TOK_EOF, .text = NULL });

    lexerPrintTokens(tokens, tokenCount);
}
