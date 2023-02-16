#ifndef ERROR_H
#define ERROR_H

#include "Arduino.h"

// Custom assert
#undef assert
#define assert(expr) \
    if (expr) {      \
    } else           \
        aFailed((uint8_t *)__FILE__, __LINE__)

// Foward declaration
void aFailed(uint8_t *file, uint32_t line);

#endif