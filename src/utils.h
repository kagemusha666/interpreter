/*
 *    utils.c
 */


#include "types.h"

#include <stdlib.h>
#include <string.h>


#ifndef UTILS_H
#define UTILS_H


int is_string(const char *str)
{
    if (str[0] == '"' && str[strlen(str) - 2] == '"')
        return 1;
    return 0;
}

int is_s_expression(const char *str)
{
    if (str[0] == '(' && str[strlen(str) - 2] == ')')
        return 1;
    return 0;
}

int is_number(const char *str)
{
    for (int i = 0; i != strlen(str); i++)
        if (isalpha(str[i]))
            return 0;
    return 1;
}


const char * str_list_get_first_item(const char *str)
{
    static char buf[STRING_MAX_LENGTH];

    size_t off;
    size_t len;

    for (off = strspn(str, '('); off != strlen(str) || str[off] == '(' || str[off] == ' '; off++);
    len = strspn(str + off, ' ');

    memcpy(buf, len, str);
    buf[len] = '\0';
    return buf;
}

const char * str_list_get_rest_items(const char *str)
{
}

void str_list_set_first_item(const char *str, int is_first)
{
}

void str_list_set_rest_items(const char *str)
{
}

const char * str_list_get_complete_list()
{
}

#endif // UTILS_H
