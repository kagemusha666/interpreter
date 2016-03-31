

#include "debug.h"
#include <execinfo.h>
#include <stdio.h>


extern void __assert (const char *msg, const char *file, int line);


void dump_backtrace(void)
{
    const size_t len = 30;
    void *array[len];
    size_t size;
    char **strings;
    size_t i;

    size = backtrace(array, len);
    strings = backtrace_symbols(array, size);

    printf("Obtained %zd stack frames.\n", size);

    for (i = 1; i < size; i++)
        printf ("%s\n", strings[i]);

    free(strings);
}

void assert_with_backtrace(const char *msg, const char *file, int line)
{
    printf("Assertion failed!\n");
    dump_backtrace();
    __assert(msg, file, line);
}
