

/*
 *    types.c
 */


#include "types.h"
#include "utils.h"
#include "debug.h"
#include "error.h"

#include <ctype.h>
#include <string.h>
#include <assert.h>


int numeric_from_string(Object *obj, const char *str)
{
    if (strchr(str, ' ') != NULL)
      return SYNTAX_ERROR;

    ((Numeric*)obj)->value = atoi(str);
    return OK;
}

const char *numeric_to_string(Object *obj)
{
    static char str[STRING_MAX_LENGTH];
    sprintf(str, "%d", ((Numeric*)obj)->value);
    return str;
}

void numeric_finalize(Object *obj)
{
    /* Empty */
}

Numeric *numeric_initialize()
{
    Numeric *obj = (Numeric*)malloc(sizeof(Numeric));
    obj->object.from_string = &numeric_from_string;
    obj->object.to_string = &numeric_to_string;
    obj->object.finalize = &numeric_finalize;
    obj->value = 0;
}

int string_from_string(Object *obj, const char *str)
{
    size_t len = strlen(str);
    char **text = &((String*)obj)->text;

    if (len < 2
        || len > STRING_MAX_LENGTH
        || str[0] != '"'
        || str[len - 2] != '"')
        return ERROR;

    if (*text != NULL)
        free(*text);

    len -= 2;
    *text = (char*)malloc(len);
    memcpy(*text, str + 1, len);
    (*text)[len - 1] = '\0';
    return OK;
}

const char *string_to_string(Object *obj)
{
    static char str[STRING_MAX_LENGTH];
    sprintf(str, "\"%s\"", ((String*)obj)->text);
    return str;
}

void string_finalize(Object *obj)
{
    free((void*)((String*)obj)->text);
}

String *string_initialize()
{
    String *obj = (String*)malloc(sizeof(String));
    obj->object.from_string = &string_from_string;
    obj->object.to_string = &string_to_string;
    obj->object.finalize = &string_finalize;
    obj->text = NULL;
    return obj;
}

int variable_from_string(Object *obj, const char *str)
{
    size_t len = strlen(str);
    char **text = &((Variable*)obj)->text_representation;

    if (strchr(str, ' ') != NULL)
      return SYNTAX_ERROR;

    if (*text != NULL)
        free(*text);
    *text = (char*)malloc(len + 1);
    memcpy(*text, str, len - 1);
    return OK;
}

const char *variable_to_string(Object *obj)
{
    static char str[STRING_MAX_LENGTH];
    sprintf(str, "%s", ((Variable*)obj)->text_representation);
    return str;
}

void variable_finalize(Object *obj)
{
    free((void*)((Variable*)obj)->text_representation);
}

Variable *variable_initialize()
{
    Variable *obj = (Variable*)malloc(sizeof(Variable));
    obj->object.from_string = &variable_from_string;
    obj->object.to_string = &variable_to_string;
    obj->object.finalize = &variable_finalize;
    obj->text_representation = NULL;
    return obj;
}

int pair_from_string(Object *obj, const char *str);

const char *pair_to_string(Object *obj);

void pair_finalize(Object *obj)
{
    object_finalize(((Pair*)obj)->first);
    object_finalize(((Pair*)obj)->rest);
}

Pair *pair_initialize()
{
    Pair *obj = (Pair*)malloc(sizeof(Pair));
    obj->object.from_string = &pair_from_string;
    obj->object.to_string = &pair_to_string;
    obj->object.finalize = &pair_finalize;
    obj->first = NULL;
    obj->rest = NULL;
    return obj;
}

Type object_get_type(Object *obj)
{
    assert(obj != NULL);
    return obj->type;
}

void object_mark(Object *obj)
{
    assert(obj != NULL);

    if (obj->marked)
        return;

    obj->marked = 1;

    if (obj->type == PAIR) {
        Pair *pair = (Pair*)obj;

        object_mark(pair->first);
        object_mark(pair->rest);
    }
}

int object_from_string(Object *obj, const char *str)
{
    assert(obj != NULL);
    return obj->from_string(obj, str);
}

const char *object_to_string(Object *obj)
{
    assert(obj != NULL);
    return obj->to_string(obj);
}

Object *object_create(Type type)
{
    Object *obj = NULL;

    switch (type) {
    case NUMERIC:
        obj = (Object*)numeric_initialize();
        break;
    case STRING:
        obj = (Object*)string_initialize();
        break;
    case VARIABLE:
        obj = (Object*)variable_initialize();
        break;
    case PAIR:
        obj = (Object*)pair_initialize();
        break;
    default:
        FATAL("Invalid object type");
    }

    obj->type = type;
    return obj;
}

Type get_type_of_str(const char *str)
{
    if (is_string(str))
        return STRING;
    else if (is_s_expression(str))
        return PAIR;
    else if (is_number(str))
        return NUMERIC;
    return VARIABLE;
}

Object *object_create_from_string(const char *str)
{
    Object *obj = object_create(get_type_of_str(str));
    if (object_from_string(obj, str) == OK) {
        return obj;
    }
    else {
        ERROR("Error parsing: %s", str);
        object_finalize(obj);
        return NULL;
    }
}

void object_finalize(Object *obj)
{
    if (obj != NULL) {
        obj->finalize(obj);
        free((void*)obj);
    }
}


int pair_from_string(Object *obj, const char *str)
{
    /*
    Pair *pair = (Pair*)obj;

    if (str_list_get_first_item(str) != NULL) {
        pair->first = object_create_from_string(str_list_get_first_item(str));
        if (pair->first == NULL)
            return SYNTAX_ERROR;

        if (str_list_get_rest_items(str) != NULL) {
            pair->rest = object_create(PAIR);
            return pair_from_string(pair->rest, str_list_get_rest_items(str));
        }
    }
    return OK;
    */
}

const char *pair_to_string(Object *obj)
{
    /*
    Pair *pair = (Pair*)obj;

    if (pair->first != NULL)
        str_list_set_first_item(object_to_string(pair->first), pair->first == PAIR);
    else
        str_list_set_first_item(NULL, 0);

    if (pair->rest != NULL)
        str_list_set_rest_items(object_to_string(pair->rest));
    else
        str_list_set_rest_items(NULL);

    return str_list_get_complete_list();
    */
}
