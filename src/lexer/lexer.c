//
// Created by MCris112 on 12/18/2025.
//

#include "lexer.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int currentLine = 1;
int currentColumn = 1;

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
    if (pos[len] != ' ' && pos[len] != '(')
        return false;

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

void parseLexer(const char *input) {
    // Array of all character without change the main
    const char *pos = input;

    while (*pos) {
        if (*pos == '\n') {
            currentLine++;
            currentColumn = 1;
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
        if ( *pos == '\'') {
            int startColumn = currentColumn;
            pos++; currentColumn++; // skip opening '

            if (*pos == '\0') {
                addToken((Token){
                    .type = TOK_ERROR,
                    .text = "Unterminated char literal",
                    .line = currentLine,
                    .column = startColumn
                });
                return;
            }

            char value = *pos; // read the character
            pos++; currentColumn++;

            // Expect closing '
            if (*pos != '\'') {
                addToken((Token){
                    .type = TOK_ERROR,
                    .text = "Expected closing ' for char literal",
                    .line = currentLine,
                    .column = startColumn
                });
                continue;
            }

            pos++; currentColumn++; // skip closing '

            // Store char as a string of length 1
            char *text = malloc(2);
            text[0] = value;
            text[1] = '\0';

            addToken((Token){
                .type = TOK_CHAR,
                .text = text,
                .line = currentLine,
                .column = startColumn
            });

            continue;
        }

        //---------------------------
        // Parse TRUE | FALSE
        //---------------------------
        if ( strncmp( pos, "TRUE", 4) == 0 && !isalnum(pos[4] ) ) {

            addToken( (Token){
                .type = TOK_LITERAL_BOOLEAN,
                .boolean = true,
                .line = currentLine,
                .column = currentColumn
            } );

            pos +=4;
            currentColumn +=4;
            continue;
        }

        if ( strncmp( pos, "FALSE", 5) == 0 && !isalnum(pos[5]) ) {

            addToken( (Token){
                .type = TOK_LITERAL_BOOLEAN,
                .boolean = false,
                .line = currentLine,
                .column = currentColumn
            } );

            pos +=5;
            currentColumn +=5;
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
        // Logical operators
        //---------------------------
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

        pos++;
        currentColumn++;
    }
}


const char *tokenTypeToString(TokenType type) {
    switch (type) {
        // VARIABLES
        case TOK_VARIABLE_TYPE_INT: return "TOK_VARIABLE_TYPE_INT";
        case TOK_VARIABLE_TYPE_STRING: return "TOK_VARIABLE_TYPE_STRING";
        case TOK_VARIABLE_TYPE_BOOLEAN: return "TOK_VARIABLE_TYPE_BOOLEAN";
        case TOK_VARIABLE_TYPE_FLOAT: return "TOK_VARIABLE_TYPE_FLOAT";
        case TOK_VARIABLE_TYPE_CHAR: return "TOK_VARIABLE_TYPE_CHAR";
        case TOK_VARIABLE_TYPE_VOID: return "TOK_VARIABLE_TYPE_VOID";
        case TOK_VARIABLE: return "TOK_VARIABLE";

        // FUNCTIONS
        case TOK_FUNCTION_CALL: return "TOK_FUNCTION_CALL";
        case TOK_FUNCTION_DEFINITION: return "TOK_FUNCTION_DEFINITION";

        // COMPARISON
        case TOK_LESS_THAN: return "TOK_LESS_THAN";
        case TOK_GREATER_THAN: return "TOK_GREATER_THAN";
        case TOK_LESS_EQUAL: return "TOK_LESS_EQUAL";
        case TOK_GREATER_EQUAL: return "TOK_GREATER_EQUAL";
        case TOK_EQUAL_EQUAL: return "TOK_EQUAL_EQUAL";
        case TOK_NOT_EQUAL: return "TOK_NOT_EQUAL";

        // LOGICAL
        case TOK_LOGICAL_AND: return "TOK_LOGICAL_AND";
        case TOK_LOGICAL_OR: return "TOK_LOGICAL_OR";
        case TOK_LOGICAL_NOT: return "TOK_LOGICAL_NOT";

        // ASSIGNMENT
        case TOK_EQUALS: return "TOK_EQUALS";
        case TOK_PLUS_ASSIGN: return "TOK_PLUS_ASSIGN";
        case TOK_MINUS_ASSIGN: return "TOK_MINUS_ASSIGN";
        case TOK_MUL_ASSIGN: return "TOK_MUL_ASSIGN";
        case TOK_DIV_ASSIGN: return "TOK_DIV_ASSIGN";

        // ARITHMETIC
        case TOK_PLUS: return "TOK_PLUS";
        case TOK_MINUS: return "TOK_MINUS";
        case TOK_MULTIPLY: return "TOK_MULTIPLY";
        case TOK_DIVIDE: return "TOK_DIVIDE";
        case TOK_MODULO: return "TOK_MODULO";
        case TOK_INCREMENT: return "TOK_INCREMENT";
        case TOK_DECREMENT: return "TOK_DECREMENT";

        // DELIMITERS
        case TOK_PARENTHESIS_OPEN: return "TOK_PARENTHESIS_OPEN";
        case TOK_PARENTHESIS_CLOSE: return "TOK_PARENTHESIS_CLOSE";
        case TOK_BRACE_OPEN: return "TOK_BRACE_OPEN";
        case TOK_BRACE_CLOSE: return "TOK_BRACE_CLOSE";
        case TOK_BRACKET_OPEN: return "TOK_BRACKET_OPEN";
        case TOK_BRACKET_CLOSE: return "TOK_BRACKET_CLOSE";
        case TOK_COMMA: return "TOK_COMMA";
        case TOK_SEMICOLON: return "TOK_SEMICOLON";
        case TOK_COLON: return "TOK_COLON";

        // ACCESS
        case TOK_ARROW: return "TOK_ARROW";
        case TOK_DOT: return "TOK_DOT";

        // LITERALS
        case TOK_TEXT: return "TOK_TEXT";
        case TOK_NUMBER: return "TOK_NUMBER";
        case TOK_NUMBER_DECIMAL: return "TOK_NUMBER_DECIMAL";

        case TOK_WHITESPACE: return "TOK_WHITESPACE";
        case TOK_COMMENT_NORMAL: return "TOK_COMMENT_NORMAL";

        default: return "TOK_UNKNOWN";
    }
}
