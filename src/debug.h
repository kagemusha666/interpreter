

/*
 *    debug.h
 */


#ifndef DEBUG_H
#define DEBUG_H

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

void dump_backtrace(void);
void assert_with_backtrace(const char *msg, const char *file, int line);

#define assert(EX) (void)((EX) || (assert_with_backtrace (#EX, __FILE__, __LINE__),0))

#define OBJECT_DUMP(OBJ) do { printf("%s:%d:%s: ", __FILE__, __LINE__, __func__);\
                              object_dump((Object*)OBJ); printf("\n"); } while (0)

#endif // DEBUG_H
