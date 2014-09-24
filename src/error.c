

/*
 *    error.c
 */


#include "error.h"


const char *messages[] = {
    "ok", // OK
    "Unknown error", // ERROR
    "Syntax error",  // SYNTAX_ERROR
    "Stack overflow!", // STACK_OVERFLOW
    "Invalid arity", // INVALID_ARITY
};

const char *error_to_string(Error err)
{
    return messages[err];
}
