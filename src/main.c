

/*
 *    main.c
 */

#include "vm.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#define INPUT_BUFFER_MAX_SIZE 256


size_t read_buffer(char *buf, size_t len)
{
    char *line = NULL;
    size_t size = 0;
    ssize_t read;

    read = getline(&line, &size, stdin);
    memcpy(buf, line, len);
    size -= read;

    free(line);
    return read < len ? read : len;
}

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

    return (len > 2 && counter == 0);
}

int main(int argc, char *argv[])
{
    VM *vm;
    char *buffer;
    size_t size;
    ssize_t read;
    int code;

    vm = VM_create();
    buffer = (char*)malloc(INPUT_BUFFER_MAX_SIZE);

    while (!feof(stdin)) {
        memset(buffer, 0, INPUT_BUFFER_MAX_SIZE);
        size = INPUT_BUFFER_MAX_SIZE;
        read = 0;

        printf("\nREPL ]=>");

        while (!feof(stdin) && size > 0) {
            read += read_buffer(buffer + read, size - read);

            if (is_expression_complete(buffer, read)) {
                code = vm_eval_exp(vm, buffer);

                if (code == OK) {
                    // printf("\nRETURN: %s\n", output);
                }
                else {
                    printf("\nERROR, code = %d\n", code);
                }

                break;
            }
        }
    }

    free(buffer);
    VM_finalize(vm);

    return EXIT_SUCCESS;
}
