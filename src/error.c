

/*
 *    error.c
 */


#include "error.h"
#include "types.h"
#include "debug.h"

#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>


jmp_buf _env;

static char _msg[STRING_MAX_LENGTH];

static const char *_messages[] = {
    "none",
    "unknown",
    "parser",
    "core"
};

const char *error_to_string(Error error)
{
    assert(error > 0);
    assert(error < ERROR_TYPE_LAST);
    return _messages[error];
}

void throw_exception(Error error, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
    longjmp(_env, (int)error);
}

void throw_exception_str(Error error,
                         const char *str, unsigned len,
                         const char *format, ...)
{
    va_list args;

    len  = len + 1 > STRING_MAX_LENGTH ? STRING_MAX_LENGTH : len;
    memcpy(_msg, str, len);
    _msg[len] = 0;

    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");

    printf("Invalid string: '%s'\n", _msg);
    longjmp(_env, (int)error);
}

void throw_exception_buf(Error error,
                         const char *begin, const char *end,
                         const char *format, ...)
{
    va_list args;
    unsigned len = end - begin;

    len  = len + 1 > STRING_MAX_LENGTH ? STRING_MAX_LENGTH : len;
    memcpy(_msg, begin, len);
    _msg[len] = 0;

    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");

    printf("Invalid string: '%s'\n", _msg);
    longjmp(_env, (int)error);
}
