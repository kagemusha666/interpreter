

/*
 *    errors.h
 */


#ifndef ERRORS_H
#define ERRORS_H

#include <setjmp.h>

typedef enum error {
    ERROR_TYPE_NONE,
    ERROR_TYPE_UNKNOWN,
    ERROR_TYPE_PARSER,
    ERROR_TYPE_CORE,
    ERROR_TYPE_LAST
} Error;

extern jmp_buf _env;

const char *error_to_string(Error error);

#define try_and_catch_error() (Error)setjmp(_env)

void throw_exception(Error error, const char *format, ...);

void throw_exception_str(Error error,
                         const char *str, unsigned len,
                         const char *format, ...);

void throw_exception_buf(Error error,
                         const char *begin, const char *end,
                         const char *format, ...);

#endif
