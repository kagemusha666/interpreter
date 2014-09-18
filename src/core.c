

/*
 *    core.c
 */


#include <stdio.h>


int is_expression_complete(const char *line, size_t len)
{
    long counter = 0;
    size_t i;

    for (i = 0; i < len; i++) {
        if (line[i] == '(')
            counter++;
        else if (line[i] == ')')
            counter--;
    }

    return (counter == 0);
}

int eval(const char *line, size_t len, char *output)
{
    sprintf(output, "nil", (unsigned)len);
    return 0;
}
