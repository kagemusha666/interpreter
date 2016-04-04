

/*
 *    types.c
 */


#include "types.h"
#include "error.h"
#include "debug.h"

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


static char string[STRING_MAX_LENGTH];

static void mark(Object *obj)
{
    obj->gc.marked = 1;
}

static void finalize(Object *obj)
{
    /* Empty */
}

static const char *integer_to_string(Object *obj)
{
    sprintf(string, "%d", ((Integer*)obj)->value);
    return string;
}

static void integer_dump(Object *obj)
{
    printf("%d", ((Integer*)obj)->value);
}

static Integer *integer_initialize()
{
    Integer *obj = malloc(sizeof(Integer));
    obj->object.to_string = &integer_to_string;
    obj->object.dump = &integer_dump;
    obj->object.mark = &mark;
    obj->object.finalize = &finalize;
    obj->value = 0;
    return obj;
}

static const char *boolean_to_string(Object *obj)
{
    sprintf(string, "%s", ((Boolean*)obj)->value ? "#true" : "#false");
    return string;
}

static void boolean_dump(Object *obj)
{
    printf("%s", ((Boolean*)obj)->value ? "#true" : "#false");
}

static Integer *boolean_initialize()
{
    Integer *obj = malloc(sizeof(Integer));
    obj->object.to_string = &boolean_to_string;
    obj->object.dump = &boolean_dump;
    obj->object.mark = &mark;
    obj->object.finalize = &finalize;
    obj->value = false;
    return obj;
}

static const char *string_to_string(Object *obj)
{
    const char *str = ((String*)obj)->cstr;
    if (str) {
        if (strstr(str, " ") != NULL) {
            sprintf(string, "\"%s\"", str);
        }
        else {
            sprintf(string, "\'%s", str);
        }
    }
    else {
        sprintf(string, "\"INVALID STRING\"");
    }
    return string;
}

static void string_dump(Object *obj)
{
    const char *str = ((String*)obj)->cstr;
    if (str) {
        if (strstr(str, " ") != NULL) {
            printf("\"%s\"", str);
        }
        else {
            printf("\'%s", str);
        }
    }
    else {
        printf("INVALID STRING");
    }
}

static void string_finalize(Object *obj)
{
    free((void*)((String*)obj)->cstr);
}

static String *string_initialize()
{
    String *obj = (String*)malloc(sizeof(String));
    obj->object.to_string = &string_to_string;
    obj->object.dump = &string_dump;
    obj->object.mark = &mark;
    obj->object.finalize = &string_finalize;
    obj->cstr = NULL;
    return obj;
}

static const char *unbound_to_string(Object *obj)
{
    const char *str = ((Unbound*)obj)->cstr;
    if (str != NULL) {
        sprintf(string, "%s", str);
    }
    else {
        sprintf(string, "*unknown*");
    }
    return string;
}

static void unbound_dump(Object *obj)
{
    const char *str = ((Unbound*)obj)->cstr;
    if (str != NULL) {
        printf("%s", str);
    }
    else {
        printf("*unknown*");
    }
}

static void unbound_finalize(Object *obj)
{
    free((void*)((Unbound*)obj)->cstr);
}

Unbound *unbound_initialize()
{
    Unbound *obj = (Unbound*)malloc(sizeof(Unbound));
    obj->object.to_string = &unbound_to_string;
    obj->object.dump = &unbound_dump;
    obj->object.mark = &mark;
    obj->object.finalize = &unbound_finalize;
    obj->cstr = NULL;
    return obj;
}

static const char *env_to_string(Object *obj)
{
    Frame *frame = ((Env*)obj)->frame;
    if (frame != NULL) {
        sprintf(string, "[Frame:@]");
    }
    else {
        sprintf(string, "[Frame:NULL]");
    }
    return string;
}

static void env_dump(Object *obj)
{
    Frame *frame = ((Env*)obj)->frame;
    printf("[Frame:");
    while (frame != NULL) {
        printf("%s=%s,", frame->cstr, object_to_string(frame->object));
        frame = frame->next;
    }
    printf("]");
}

static void env_mark(Object *obj)
{
    Env *env = (Env*)obj;
    while (env != NULL) {
        if (!env->object.gc.marked) {
            Frame *frame = env->frame;
            while (frame != NULL) {            
                object_mark((Object*)frame->next);
                frame = frame->next;
            }
            env->object.gc.marked = 1;
            env = env->next;
        }
        else break;
    }
}

static void env_finalize(Object *obj)
{
    Frame *frame = ((Env*)obj)->frame;
    Frame *tmp;
    while (frame != NULL) {
        tmp = frame->next;
        free((void*)frame->cstr);
        free(frame);
        frame = tmp;
    }
}

Env *env_initialize()
{
    Env *obj = (Env*)malloc(sizeof(Env));
    obj->object.to_string = &env_to_string;
    obj->object.dump = &env_dump;
    obj->object.mark = &env_mark;
    obj->object.finalize = &env_finalize;
    obj->frame = NULL;
    obj->next = NULL;
    return obj;
}

static const char *pair_to_string(Object *obj)
{
    sprintf(string, "*list*");
    return string;
}

static void pair_dump(Object *obj)
{
    Object *first = ((Pair*)obj)->first;
    Object *rest = ((Pair*)obj)->rest;

    if (first != NULL) {
        if (first->type == OBJECT_TYPE_PAIR) {
            printf("(");
        }
        first->dump(first);
    }
    if (rest != NULL) {
        if (first != NULL && first->type != OBJECT_TYPE_PAIR) {
            printf(" ");
        }
        if (rest->type != OBJECT_TYPE_PAIR) {
            printf(". ");
        }
        rest->dump(rest);
    }

    if (rest == NULL || (rest != NULL && rest->type != OBJECT_TYPE_PAIR)) {
        printf(")");
    }
}

static void pair_mark(Object *obj)
{
    if (!obj->gc.marked) {
        Pair *pair = (Pair*)obj;
        obj->gc.marked = 1;

        if (pair->first != NULL)
            pair->first->mark(pair->first);
        if (pair->rest != NULL)
            pair->rest->mark(pair->rest);
    }
}

static Pair *pair_initialize()
{
    Pair *obj = (Pair*)malloc(sizeof(Pair));
    obj->object.to_string = &pair_to_string;
    obj->object.dump = &pair_dump;
    obj->object.mark = &pair_mark;
    obj->object.finalize = &finalize;
    obj->first = NULL;
    obj->rest = NULL;
    return obj;
}

static const char *procedure_to_string(Object *obj)
{
    const char *str = ((Proc*)obj)->cstr;
    if (str != NULL) {
        sprintf(string, "<procedure %s>", str);
    }
    else {
        sprintf(string, "<procedure *unknown*>");
    }
    return string;
}

static void procedure_dump(Object *obj)
{
    const char *str = ((Proc*)obj)->cstr;
    if (str != NULL) {
        printf("<procedure %s>", str);
    }
    else {
        printf("<procedure *unknown*>");
    }
}

static void procedure_mark(Object *obj)
{
    if (!obj->gc.marked) {
        Proc *proc = (Proc*)obj;
        obj->gc.marked = 1;

        if (proc->args != NULL)
            object_mark((Object*)proc->args);
        if (proc->body != NULL)
            object_mark((Object*)proc->body);
        if (proc->env != NULL)
            object_mark((Object*)proc->env);
    }
}

static void procedure_finalize(Object *obj)
{
    free((void*)((Proc*)obj)->cstr);
}

Proc *procedure_initialize()
{
    Proc *obj = (Proc*)malloc(sizeof(Proc));
    obj->object.to_string = &procedure_to_string;
    obj->object.dump = &procedure_dump;
    obj->object.mark = &procedure_mark;
    obj->object.finalize = &procedure_finalize;
    obj->cstr = NULL;
    obj->args = NULL;
    obj->body = NULL;
    obj->env = NULL;
    return obj;
}

static const char *native_to_string(Object *obj)
{
    const char *str = ((Native*)obj)->cstr;
    if (str != NULL) {
        sprintf(string, "<native function %s>", str);
    }
    else {
        sprintf(string, "<native function *unknown*>");
    }
    return string;
}

static void native_dump(Object *obj)
{
    const char *str = ((Native*)obj)->cstr;
    if (str != NULL) {
        printf("<native function %s>", str);
    }
    else {
        printf("<native function *unknown*>");
    }
}

static void native_finalize(Object *obj)
{
    free((void*)((Native*)obj)->cstr);
}

Native *native_initialize()
{
    Native *obj = (Native*)malloc(sizeof(Native));
    obj->object.to_string = &native_to_string;
    obj->object.dump = &native_dump;
    obj->object.mark = &mark;
    obj->object.finalize = &native_finalize;
    obj->cstr = NULL;
    obj->req = 0;
    obj->rst = 0;
    obj->native_function = NULL;
    return obj;
}


Object *object_create(Type type)
{
    Object *obj = NULL;

    switch (type) {
    case OBJECT_TYPE_INTEGER:
        obj = (Object*)integer_initialize();
        break;
    case OBJECT_TYPE_BOOLEAN:
        obj = (Object*)boolean_initialize();
        break;
    case OBJECT_TYPE_STRING:
        obj = (Object*)string_initialize();
        break;
    case OBJECT_TYPE_PAIR:
        obj = (Object*)pair_initialize();
        break;
    case OBJECT_TYPE_UNBOUND:
        obj = (Object*)unbound_initialize();
        break;
    case OBJECT_TYPE_ENVIRONMENT:
        obj = (Object*)env_initialize();
        break;
    case OBJECT_TYPE_PROCEDURE:
        obj = (Object*)procedure_initialize();
        break;
    case OBJECT_TYPE_NATIVE:
        obj = (Object*)native_initialize();
        break;
    default:
        FATAL("Invalid object type");
    }

    obj->type = type;
    return obj;
}

void object_delete(Object *obj)
{
    if (obj != NULL) {
        obj->finalize(obj);
        free((void*)obj);
    }
}

Type object_get_type(Object *obj)
{
    assert(obj != NULL);
    return obj->type;
}

const char* object_to_string(Object *obj)
{
    assert(obj != NULL);
    return obj->to_string(obj);
}

void object_dump(Object *obj)
{
    if (obj != NULL) {
        if (obj->type == OBJECT_TYPE_PAIR) {
            printf("(");
        }
        obj->dump(obj);
    }
}

void object_mark(Object *obj)
{
    assert(obj != NULL);
    obj->mark(obj);
}
