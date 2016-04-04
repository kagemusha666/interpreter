

/*
 *    main.c
 */


#include "parser.h"
#include "core.h"
#include "env.h"
#include "error.h"
#include "debug.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define INPUT_BUFFER_MAX_SIZE 256

#define throw(format,args...) throw_exception(ERROR_TYPE_CORE, format, ##args);

static Object *cons(Object *obj)
{
    List *list = (List*)obj;
    Pair *pair = (Pair*)object_create(OBJECT_TYPE_PAIR);
    pair->first = list->item;
    pair->rest = list->next->item;
    return (Object*)pair;
}

static Object *car(Object *obj)
{
    Object *res;
    List *list = (List*)obj;

    if (list->next != NULL) {
        throw("Wrong number of arguments: expected one argument");
    }
    else if (object_get_type(list->item) != OBJECT_TYPE_PAIR) {
        throw("Wrong type of argument: expected pair");
    }
    else {
        res = ((Pair*)list->item)->first;
    }
    return res;
}

static Object *cdr(Object *obj)
{
    Object *res;
    List *list = (List*)obj;

    if (list->next != NULL) {
        throw("Wrong number of arguments: expected one argument");
    }
    else if (object_get_type(list->item) != OBJECT_TYPE_PAIR) {
        throw("Wrong type of argument: expected pair");
    }
    else {
        res = ((Pair*)list->item)->rest;
    }
    return res;

}

static Object *plus(Object *obj)
{
    Integer *ans;
    List *list = (List*)obj;
    ans = (Integer*)object_create(OBJECT_TYPE_INTEGER);

    while (list != NULL) {
        if (object_get_type(list->item) != OBJECT_TYPE_INTEGER) {
            throw("Wrong type of argument %s: expected numeric",
                  object_to_string(list->item));            
        }
        ans->value += ((Integer*)list->item)->value;
        list = list->next;
    }
    return (Object*)ans;
}

static Object *equal(Object *obj)
{
    Boolean *ans = (Boolean*)object_create(OBJECT_TYPE_BOOLEAN);
    List *list = (List*)obj;
    int prevVar;
    bool isPrevVarInit = false;

    ans->value = true;

    while (list != NULL) {
        if (object_get_type(list->item) != OBJECT_TYPE_INTEGER) {
            throw("Wrong type of argument %s: expected numeric",
                  object_to_string(list->item));            
            return NULL;
        }
        if (!isPrevVarInit) {
            prevVar = ((Integer*)list->item)->value;
            isPrevVarInit = true;
        }
        else if (prevVar != ((Integer*)list->item)->value) {
            ans->value = false;
            break;
        }
        list = list->next;
    }
    return (Object*)ans;
}

static Object *display(Object *obj)
{
    List *list = (List*)obj;
    while (list != NULL) {
        object_dump(list->item);
        list = list->next;
    }
    printf("\n");
    return NULL;
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
    Error error;

    char *buffer = (char*)malloc(INPUT_BUFFER_MAX_SIZE);
    Env *env = env_extend(NULL);

    env_add_native_function(env, "cons", 2, 0, cons);
    env_add_native_function(env, "car", 1, 0, car);
    env_add_native_function(env, "cdr", 1, 0, cdr);
    env_add_native_function(env, "+", 1, 1, plus);
    env_add_native_function(env, "=", 1, 1, equal);
    env_add_native_function(env, "display", 1, 1, display);

    while (!feof(stdin)) {
        memset(buffer, 0, INPUT_BUFFER_MAX_SIZE);
        size = INPUT_BUFFER_MAX_SIZE;
        read = 0;

        printf("\nREPL ]=>");

        while (!feof(stdin) && size > 0) {
            read += read_buffer(buffer + read, size - read);

            if (is_expression_complete(buffer, read)) {

                error = try_and_catch_error();
                if (error != ERROR_TYPE_NONE) {
                    printf("Catched error in component %s.\n", error_to_string(error));
                }
                else {
                    object_dump(core_eval(parser_create_object_from_string(buffer), env));
//                  object_dump(parser_create_object_from_string(buffer));
                }
                break;
            }
        }
    }

    free(buffer);

    return EXIT_SUCCESS;
}
