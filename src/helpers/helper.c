//
// Created by crisv on 12/25/2025.
//

#include "helper.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

char* strFormat(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    int size = vsnprintf(NULL, 0, fmt, args) + 1;
    va_end(args);

    char *buffer = malloc(size);

    va_start(args, fmt);
    vsnprintf(buffer, size, fmt, args);
    va_end(args);

    return buffer;
}
