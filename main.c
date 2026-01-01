#include <stdio.h>
#include <stdlib.h>

#include "src/lexer/lexer.h"
#include "src/runtime/runtime.h"


// Helper function to read entire file
char *readFile(FILE *input) {
    size_t capacity = 4096;  // Start with reasonable size
    size_t size = 0;
    char *buffer = malloc(capacity);

    if (!buffer) {
        perror("malloc");
        return NULL;
    }

    int ch;
    while ((ch = fgetc(input)) != EOF) {
        // Need space for character + null terminator
        if (size + 2 > capacity) {
            capacity *= 2;
            char *new_buffer = realloc(buffer, capacity);
            if (!new_buffer) {
                perror("realloc");
                free(buffer);
                return NULL;
            }
            buffer = new_buffer;
        }
        buffer[size++] = (char)ch;
    }

    // Check for read errors
    if (ferror(input)) {
        perror("fgetc");
        free(buffer);
        return NULL;
    }

    buffer[size] = '\0';
    return buffer;
}

int main(int argc, char *argv[]) {
    FILE *input = stdin;

    // Open file if provided
    if (argc > 1) {
        input = fopen(argv[1], "r");  // Use "r" not "rb" for text files
        if (!input) {
            fprintf(stderr, "Error: Could not open file '%s': %s\n",
                    argv[1], strerror(errno));
            return 1;
        }
    }

    // Read entire file into buffer
    char *buffer = readFile(input);

    if (input != stdin) {
        fclose(input);
    }

    if (!buffer) {
        fprintf(stderr, "Error: Failed to read input\n");
        return 1;
    }

    // Empty file check
    if (*buffer == '\0') {
        fprintf(stderr, "Warning: Empty input\n");
        free(buffer);
        return 0;
    }

    // Now you have the full source code
    parseLexer(buffer);

    runtime();

    free(buffer);
    return 0;
}
