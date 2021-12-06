#ifndef ASSERT_H
#define ASSERT_H

#include <stdio.h>
#include <stdlib.h>

#define assert(x)                                                              \
    {                                                                          \
        if(!(x)) {                                                             \
            printf("Assertion failed: line %d, file %s (function %s)\n",       \
                   __LINE__, __FILE__, __func__);                              \
            exit(1);                                                           \
        }                                                                      \
    }

#endif // ASSERT_H
