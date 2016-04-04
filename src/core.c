

#include "core.h"
#include "env.h"
#include "error.h"
#include "debug.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#define throw(format,args...) throw_exception(ERROR_TYPE_CORE, format, ##args);


static Object *apply(Object *proc, Object *args, Env *env);
static Object *eval(Object *exp, Env *env);


static Object *lookup_variable(Object *exp, Env *env)
{
    Object *obj = env_lookup_variable(env, (Unbound*)exp);
    if (obj == NULL) {
        throw("Unbound variable %s", object_to_string(exp));
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
    Object *exp = (Object*)((Pair*)args)->rest;
    Object *obj;

    if (object_get_type(var) == OBJECT_TYPE_LIST) {
        Proc *proc;
        List *list = (List*)var;

        if (list->item == NULL || object_get_type(list->item) != OBJECT_TYPE_VARIABLE) {
            throw("Invalid define pattern  %s", object_to_string((Object*)list));
        }

        proc = (Proc*)object_create(OBJECT_TYPE_PROCEDURE);
        proc->args = (Pair*)list->next;
        proc->body = (Pair*)exp;
        proc->env = env;

        var = list->item;
        obj = (Object*)proc;
    }
    else {
        exp = ((Pair*)exp)->first;
        obj = eval(exp, env);
    }

    if (!env_define_variable(env, (Unbound*)var, obj)) {
        throw("Can't define variable %s", object_to_string(obj));
    }
    return obj;
}

static Object *eval_assignment(Object *args, Env *env)
{
    Object *var = ((Pair*)args)->first;
    Object *exp = ((Pair*)((Pair*)args)->rest)->first;
    Object *obj = eval(exp, env);

    if (!env_set_variable(env, (Unbound*)var, obj)) {
        throw("Can't assign variable %s", object_to_string(obj));
    }
    return obj;
}

static Object *eval_if(Object *args, Env *env)
{
    List *list = (List*)args;

    if (core_get_list_size((Object*)list) != 3) {
        throw("Invalid pattern 'if' in %s", object_to_string(args));
    }

    return core_object_to_bool(eval(list->item, env))
        ? eval(list->next->item, env) : eval(list->next->next->item, env);
}

static Object *eval_cond(Object *args, Env *env)
{
    List *list = (List*)args;
    List *clause;
    Object *pred;
    Object *action;

    do {
        clause = (List*)list->item;
        if (clause == NULL || object_get_type((Object*)clause) != OBJECT_TYPE_LIST) {
            goto error;
        }
        pred = clause->item;
        action = clause->next->item;
        if (pred != NULL && object_get_type((Object*)pred) == OBJECT_TYPE_UNBOUND
            && strcmp(((Unbound*)pred)->cstr, "else") == 0) {

            if (list->next == NULL)
                return eval(action, env);
            else
                goto error;
        }
        else if (core_object_to_bool(eval((Object*)pred, env)) != false) {
            return eval(action, env);
        }
        list = list->next;
    } while (list != NULL);
    return NULL;

error:
    throw("Invalid cond pattern in %s", object_to_string(args));
    return NULL;
}

static Object *eval_sequence(Object *seq, Env *env)
{
    Object *obj;
    List *list = (List*)seq;

    do {
        obj = eval(list->item, env);
        list = list->next;
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
        throw("Invalid lambda expression");
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
            throw("Can't extend environment with variable %s",
                  object_to_string(args->first));
        }
        args = (Pair*)args->rest;
        vals = (Pair*)vals->rest;
    } while (args != NULL && vals != NULL);

    if (args != vals) {
        throw("Wrong number of arguments");
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
        unsigned num_of_args = core_get_list_size(args);
        if ((proc->rst == 0 && num_of_args > proc->req) || num_of_args < proc->req) {
            throw("Invalid args number %u", num_of_args);
        }
        res = proc->native_function(args);
    }
    return res;
}

static Object *eval(Object *exp, Env *env)
{
    Object *obj = NULL;

    if (exp == NULL) {
        obj = NULL;
    }
    else if (exp->type != OBJECT_TYPE_PAIR) {
        obj = exp->type != OBJECT_TYPE_UNBOUND ? exp : lookup_variable(exp, env);
    }
    else {
        Object *operator = ((Pair*)exp)->first;
        Object *operands = ((Pair*)exp)->rest;

        if (operator->type != OBJECT_TYPE_UNBOUND) {
            throw("Invalid type to apply");
        }
        else {
            const char *str = ((Unbound*)operator)->cstr;

            if (strcmp(str, "define") == 0) {
                obj = eval_definition(operands, env);
            }
            else if (strcmp(str, "set!") == 0) {
                obj = eval_assignment(operands, env);
            }
            else if (strcmp(str, "if") == 0) {
                obj = eval_if(operands, env);
            }
            else if (strcmp(str, "cond") == 0) {
                obj = eval_cond(operands, env);
            }
            else if (strcmp(str, "begin") == 0) {
                obj = eval_sequence(operands, env);
            }
            else if (strcmp(str, "lambda") == 0) {
                obj = make_procedure(operands, env);
            }
            else {
                obj = apply(eval(operator, env),
                            list_of_values(operands, env), env);
            }
        }
    }
    return obj;
}

unsigned core_get_list_size(Object *obj)
{
    unsigned res = 0;
    List *list = (List*)obj;
    assert(object_get_type(obj) == OBJECT_TYPE_LIST);

    while (list != NULL) {
        res++;
        list = list->next;
    }
    return res;
}

bool core_object_to_bool(Object *obj)
{
    if (obj != NULL) {
        switch (obj->type) {
        case OBJECT_TYPE_INTEGER:
            return ((Integer*)obj)->value;
        case OBJECT_TYPE_BOOLEAN:
            return ((Boolean*)obj)->value;
        case OBJECT_TYPE_STRING:
            return strlen(((String*)obj)->cstr);
        case OBJECT_TYPE_LIST:
            return core_get_list_size(obj) > 0;
        case OBJECT_TYPE_PROCEDURE:
        case OBJECT_TYPE_NATIVE:
            return obj != NULL;
        default:
            throw("Can't cast %s to bool", object_to_string(obj));
        }
    }
    return false;
}

Object *core_object_to_bool_object(Object *obj)
{
   Boolean *res = (Boolean*)object_create(OBJECT_TYPE_BOOLEAN);
   res->value = core_object_to_bool(obj);
   return (Object*)res;
}

Object *core_eval(Object *exp, Env *env)
{
    assert(env != NULL);
    return eval(exp, env);
}
