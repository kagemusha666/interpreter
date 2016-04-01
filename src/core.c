

#include "core.h"
#include "env.h"
#include "debug.h"

#include <setjmp.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


typedef enum error {
    ERROR_NONE,
    ERROR_UNKNOWN,
    ERROR_UNBOUND,
    ERROR_INVALID_ARGS,
    ERROR_LAST
} Error;

static jmp_buf _coreEnv;

static Object *apply(Object *proc, Object *args, Env *env);
static Object *eval(Object *exp, Env *env);


static void throw_exception(Error error, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    longjmp(_coreEnv, (int)error);
}

static unsigned get_list_size(Object *obj)
{
    unsigned res = 0;
    List *list = (List*)obj;
    assert(object_get_type(obj) == OBJECT_TYPE_LIST);

    while (list != NULL) {
        if (list->item != NULL)
            res++;
        list = list->next;
    }
    return res;
}

static Object *lookup_variable(Object *exp, Env *env)
{
    Object *obj = env_lookup_variable(env, (Unbound*)exp);
    if (obj == NULL) {
        throw_exception(ERROR_UNBOUND, "Unbound variable %s", object_to_string(exp));
    }
    return obj;
}

static Object *list_of_values(Object *args, Env *env)
{
    List *res;

    if (args != NULL) {
        assert(object_get_type(args) == OBJECT_TYPE_LIST);

        List *list = (List*)args;
        List **ptr = &res;
        do {
            *ptr = (List*)object_create(OBJECT_TYPE_LIST);
            (*ptr)->item = eval(list->item, env);
            list = list->next;
            ptr = &(*ptr)->next;
        } while (list != NULL);
    }
    else {
        res = (List*)object_create(OBJECT_TYPE_LIST);
    }
    return (Object*)res;
}

static Object *eval_definition(Object *args, Env *env)
{
    Object *var = ((Pair*)args)->first;
    Object *exp = ((Pair*)((Pair*)args)->rest)->first;
    Object *obj = eval(exp, env);

    if (!env_define_variable(env, (Unbound*)var, obj)) {
        throw_exception(ERROR_UNKNOWN, "Can't define variable %s", object_to_string(obj));
    }
    return obj;
}

static Object *eval_sequence(Object *seq, Env *env)
{
    Object *obj;
    Pair *list = (Pair*)seq;

    do {
        obj = eval(list->first, env);
        list = (Pair*)list->rest;
    } while (list != NULL);
    return obj;
}

static Object *make_procedure(Object *exp, Env *env)
{
    Proc *proc = NULL;
    List *list = (List*)exp;
    Object *args = list->item;
    Object *body = (Object*)list->next;

    if (object_get_type(args) != OBJECT_TYPE_PAIR
        || object_get_type(body) != OBJECT_TYPE_PAIR) {
        throw_exception(ERROR_INVALID_ARGS, "Invalid lambda expression");
    }

    proc = (Proc*)object_create(OBJECT_TYPE_PROCEDURE);
    proc->args = (Pair*)args;
    proc->body = (Pair*)body;
    proc->env = env;

    return (Object*)proc;
}

static Env *extend_environment(Pair *args, Pair *vals, Env *env)
{
    env = env_extend(env);

    do {
        if (!env_define_variable(env, (Unbound*)args->first, vals->first)) {
            throw_exception(ERROR_UNKNOWN, "Can't extend environment with variable %s",
                            object_to_string(args->first));
        }
        args = (Pair*)args->rest;
        vals = (Pair*)vals->rest;
    } while (args != NULL && vals != NULL);

    if (args != vals) {
        throw_exception(ERROR_INVALID_ARGS, "Wrong number of arguments");
    }

    return env;
}

static Object *apply(Object *operator, Object *args, Env *env)
{
    Object *res = NULL;

    if (operator->type == OBJECT_TYPE_PROCEDURE) {
        Proc *proc = (Proc*)operator;
        res = eval_sequence((Object*)proc->body,
                            extend_environment(proc->args, (Pair*)args, env));
    }
    else if (operator->type == OBJECT_TYPE_NATIVE) {
        Native *proc = (Native*)operator;
        unsigned num_of_args = get_list_size(args);
        if ((proc->rst == 0 && num_of_args > proc->req) || num_of_args < proc->req) {
            throw_exception(ERROR_INVALID_ARGS,
                            "Invalid args number %u", num_of_args);
        }
        res = proc->native_function(args);
    }
    if (res == NULL) {
        throw_exception(ERROR_INVALID_ARGS,
                        "Function call %s failed", object_to_string(operator));
    }
    return res;
}

static Object *eval(Object *exp, Env *env)
{
    Object *obj = NULL;

    if (exp->type != OBJECT_TYPE_PAIR) {
        obj = exp->type != OBJECT_TYPE_UNBOUND ? exp : lookup_variable(exp, env);
    }
    else {
        Object *operator = ((Pair*)exp)->first;
        Pair *operands = (Pair*)((Pair*)exp)->rest;

        if (operator->type != OBJECT_TYPE_UNBOUND) {
            throw_exception(ERROR_INVALID_ARGS, "Invalid type to apply");
        }
        else {
            const char *str = ((Unbound*)operator)->cstr;

            if (strcmp(str, "define") == 0) {
                obj = eval_definition((Object*)operands, env);
            }
            else if (strcmp(str, "begin") == 0) {
                obj = eval_sequence((Object*)operands, env);
            }
            else if (strcmp(str, "lambda") == 0) {
                obj = make_procedure((Object*)operands, env);
            }
            else {
                obj = apply(eval(operator, env),
                            list_of_values((Object*)operands, env), env);
            }
        }
    }
    return obj;
}

Object *core_eval(Object *exp, Env *env)
{
    int errorCode;

    assert(exp != NULL);
    assert(env != NULL);

    errorCode = (Error)setjmp(_coreEnv);
    if (errorCode != ERROR_NONE) {
        return NULL;
    }
    return eval(exp, env);
}
