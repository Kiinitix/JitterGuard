#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define DIE(msg)                           \
    do {                                    \
        perror(msg);                        \
        exit(EXIT_FAILURE);                 \
    } while (0)

#endif
