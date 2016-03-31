

/*
 *    main.c
 */


#include "parser.h"
#include "core.h"
#include "env.h"
#include "debug.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define INPUT_BUFFER_MAX_SIZE 256

static Object *cons(Object *obj)
{
    List *list = (List*)obj;
    Pair *pair = (Pair*)object_create(OBJECT_TYPE_PAIR);
    pair->first = list->item;
    pair->rest = list->next->item;
    return (Object*)pair;
}

static size_t read_buffer(char *buf, size_t len)
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

static int is_expression_complete(const char *line, size_t len)
{
    long counter = 0;
    size_t i;

    for (i = 0; i < len; i++) {
        if (line[i] == '(')
            counter++;
        else if (line[i] == ')')
            counter--;
    }

    return counter == 0;
}

int main(int argc, char *argv[])
{
    size_t size;
    ssize_t read;
    Object *object;

    char *buffer = (char*)malloc(INPUT_BUFFER_MAX_SIZE);
    Env *env = env_extend(NULL);

    env_add_native_function(env, "cons", 2, 0, cons);

    while (!feof(stdin)) {
        memset(buffer, 0, INPUT_BUFFER_MAX_SIZE);
        size = INPUT_BUFFER_MAX_SIZE;
        read = 0;

        printf("\nREPL ]=>");

        while (!feof(stdin) && size > 0) {
            read += read_buffer(buffer + read, size - read);

            if (is_expression_complete(buffer, read)) {
                object = parser_create_object_from_string(buffer);
                if (object != NULL) {
                    object = core_eval(object, env);
                    if (object != NULL) {
                        object_dump(object);
                    }
                }
                break;
            }
        }
    }

    free(buffer);

    return EXIT_SUCCESS;
}
