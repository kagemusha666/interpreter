

/*
 *    main.c
 */

#include "core.h"
#include "types.h"

#include <stdlib.h>
#include <stdio.h>


int main(int argc, char *argv[])
{
    char output[QUOTE_MAX_LENGTH];
    char *line = NULL;
    size_t size = 0;

    printf("\nREPL ]=>");

    while (!feof(stdin)) {
        ssize_t read = getline(&line, &size, stdin);

        if (is_expression_complete(line, read)) {
            int code = eval(line, read, &output);

            if (code == 0) {
                printf("\nRETURN: %s\n\n", output);
            }
            else {
                printf("\nERROR, code = %d\n\n", code);
            }
        }
    }

    return EXIT_SUCCESS;
}
