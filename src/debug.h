

/*
 *    debug.h
 */


#include <stdio.h>
#include <stdlib.h>


#define DEBUG(...) \
    do {                          \
        fprintf(stderr, "%s:%d:%s: ", __FILE__, __LINE__, __func__);    \
        fprintf(stderr, __VA_ARGS__);                                   \
        fprintf(stderr, "\n");                                          \
    } while (0)

#define WARNING(...) \
    do {                          \
        fprintf(stderr, "%s:%d:%s: ", __FILE__, __LINE__, __func__);    \
        fprintf(stderr, __VA_ARGS__);                                   \
        fprintf(stderr, "\n");                                          \
    } while (0)

#define ERROR(...) \
    do {                          \
        fprintf(stderr, "%s:%d:%s: ", __FILE__, __LINE__, __func__);    \
        fprintf(stderr, __VA_ARGS__);                                   \
        fprintf(stderr, "\n");                                          \
    } while (0)

#define FATAL(...) \
    do {                          \
        fprintf(stderr, "*** FATAL: %s:%d:%s: ", __FILE__, __LINE__, __func__);    \
        fprintf(stderr, __VA_ARGS__);                                   \
        fprintf(stderr, "\n");                                          \
        abort();                                                        \
    } while (0)
