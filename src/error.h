

/*
 *    errors.h
 */


#ifndef ERRORS_H
#define ERRORS_H

typedef enum error
{
    OK,
    ERROR,
    SYNTAX_ERROR,
    STACK_OVERFLOW,
    INVALID_ARITY
} Error;

const char *error_to_string(Error err);

#endif
