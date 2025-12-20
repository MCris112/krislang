#include <stdio.h>
#include <stdlib.h>

#include "src/lexer/lexer.h"
#include "compiler/compiler.h"


int main(int argc, char *argv[]) {
    FILE *input = stdin;

    if (argc > 1) {
        input = fopen(argv[1], "rb");
        if (!input) {
            perror("fopen");
            return 1;
        }
    }

    // Read file into buffer
    char *buffer = NULL;
    size_t size = 0;
    size_t capacity = 0;

    int ch;
    while ((ch = fgetc(input)) != EOF) {
        if (size + 1 >= capacity) {
            capacity = capacity ? capacity * 2 : 1024;
            buffer = realloc(buffer, capacity);
            if (!buffer) {
                perror("realloc");
                return 1;
            }
        }
        buffer[size++] = (char)ch;
    }

    buffer[size] = '\0';

    if (input != stdin) {
        fclose(input);
    }

    // Now you have the full source code
    parseLexer(buffer);
    runCompiler();

    free(buffer);
    return 0;
}
