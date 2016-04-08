

#include "core.h"
#include "env.h"
#include "gc.h"
#include "error.h"
#include "debug.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#define throw(format,args...) throw_exception(ERROR_TYPE_CORE, format, ##args);

typedef enum context {
    CONTEXT_UNKNOWN,
    CONTEXT_APPLICATION,
    CONTEXT_EVALUATION,
    CONTEXT_RETURN,
    CONTEXT_LAST
} Context;

static Object *apply(Object *proc, Object *args, Env *env);
static Object *eval(Object *exp, Env *env, Context ctx);


static Object *lookup_variable(Object *exp, Env *env)
{
    GC_BEGIN;
    GC_PUSH2(exp, env);

    Object *obj = env_lookup_variable(env, (Unbound*)exp);
    if (obj == NULL) {
        throw("Unbound variable %s", object_to_string(exp));
    }

    GC_END;
    return obj;
}

static Object *list_of_values(Object *args, Env *env, Context ctx)
{
    List *res;
    GC_BEGIN;
    GC_PUSH2(args, env);

    if (args != NULL) {
        assert(object_get_type(args) == OBJECT_TYPE_LIST);

        List *list = (List*)args;
        List **ptr = &res;
        do {
            *ptr = (List*)object_create(OBJECT_TYPE_LIST);
            GC_PUSH1(*ptr);
            (*ptr)->item = eval(list->item, env, CONTEXT_EVALUATION);
            GC_PUSH1((*ptr)->item);

            list = list->next;
            ptr = &(*ptr)->next;
        } while (list != NULL);
    }
    else {
        res = (List*)object_create(OBJECT_TYPE_LIST);
    }
    GC_END;
    return (Object*)res;
}

static Object *eval_definition(Object *args, Env *env, Context ctx)
{
    Object *var = ((Pair*)args)->first;
    Object *exp = (Object*)((Pair*)args)->rest;
    Object *obj;
    GC_BEGIN;
    GC_PUSH2(args, env);

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
        obj = eval(exp, env, CONTEXT_EVALUATION);
    }
    GC_PUSH1(obj);

    if (!env_define_variable(env, (Unbound*)var, obj)) {
        throw("Can't define variable %s", object_to_string(obj));
    }
    return obj;
}

static Object *eval_assignment(Object *args, Env *env, Context ctx)
{
    Object *var = ((Pair*)args)->first;
    Object *exp = ((Pair*)((Pair*)args)->rest)->first;
    Object *obj = eval(exp, env, CONTEXT_EVALUATION);
    GC_BEGIN;
    GC_PUSH2(args, env);

    if (!env_set_variable(env, (Unbound*)var, obj)) {
        throw("Can't assign variable %s", object_to_string(obj));
    }
    GC_END;
    return obj;
}

static Object *eval_if(Object *args, Env *env, Context ctx)
{
    List *list = (List*)args;
    Object *obj;
    GC_BEGIN;
    GC_PUSH2(args, env);

    if (core_get_list_size((Object*)list) != 3) {
        throw("Invalid pattern 'if' in %s", object_to_string(args));
    }

    obj = eval(list->item, env, CONTEXT_EVALUATION);
    GC_PUSH1(obj);

    if (core_object_to_bool(obj)) {
        obj = eval(list->next->item, env, ctx);
    }
    else {
        obj = eval(list->next->next->item, env, ctx);
    }
    GC_END;
    return obj;
}

static Object *eval_cond(Object *args, Env *env, Context ctx)
{
    List *list = (List*)args;
    List *clause;
    Object *pred;
    Object *action;
    Object *res;
    GC_BEGIN;
    GC_PUSH2(args, env);

    do {
        clause = (List*)list->item;
        if (clause == NULL || object_get_type((Object*)clause) != OBJECT_TYPE_LIST) {
            throw("Invalid cond pattern in %s", object_to_string(args));
            res = NULL;
            break;
        }
        pred = clause->item;
        action = clause->next->item;
        // If it is the last clause:
        if (pred != NULL && object_get_type((Object*)pred) == OBJECT_TYPE_UNBOUND
            && strcmp(((Unbound*)pred)->cstr, "else") == 0) {

            if (list->next == NULL) {
                res = eval(action, env, ctx);
                break;
            }
            else {
                throw("Invalid cond pattern in %s", object_to_string(args));
                res = NULL;
                break;
            }
        }
        else  {
            pred = eval((Object*)pred, env, CONTEXT_EVALUATION);
            GC_PUSH1(pred);
            if (core_object_to_bool(pred) != false) {
                res = eval(action, env, ctx);
                break;
            }
        }
        list = list->next;
    } while (list != NULL);
    GC_END;
    return res;
}

static void change_environment(Env *env, Pair *args, Pair *vals)
{
    GC_BEGIN;
    GC_PUSH3(env, args, vals);

    do {
        if (!env_set_variable(env, (Unbound*)args->first, vals->first)) {
            throw("Can't change environment with variable %s",
                  object_to_string(args->first));
        }
        args = (Pair*)args->rest;
        vals = (Pair*)vals->rest;
    } while (args != NULL && vals != NULL);
    assert(args == vals);
    GC_END;
}

static Object *eval_sequence(Object *seq, Env *env, Context ctx, Proc *proc)
{
    Object *obj;
    List *list = (List*)seq;
    GC_BEGIN;
begin:
    GC_PUSH3(seq, env, proc);

    do {

        if (list->next != NULL || ctx != CONTEXT_APPLICATION) {
            obj = eval(list->item, env, ctx);
            GC_PUSH1(obj);
        }
        else { // Tail recursive optimization
            ctx = CONTEXT_RETURN;
            obj = eval(list->item, env, ctx);
            GC_PUSH1(obj);

            // If obj is a function call
            ctx = CONTEXT_APPLICATION;
            if (obj != NULL && obj->type == OBJECT_TYPE_PAIR
                && ((Pair*)obj)->first->type == OBJECT_TYPE_UNBOUND) {
                Object *proc_next = eval(((Pair*)obj)->first, env, ctx);
                GC_PUSH1(proc_next);
                Object *args = list_of_values(((Pair*)obj)->rest, env, ctx);
                GC_PUSH1(args);

                if (proc_next != (Object*)proc) {
                    obj = apply(proc_next, args, env);
                }
                else {
                    change_environment(env, proc->args, (Pair*)args);
                    list = (List*)seq;
                    GC_END;
                    goto begin;
                }
            }
        }
        list = list->next;
    } while (list != NULL);

    GC_END;
    return obj;
}

static Object *make_procedure(Object *exp, Env *env)
{
    Proc *proc = NULL;
    List *list = (List*)exp;
    Object *args = list->item;
    Object *body = (Object*)list->next;
    GC_BEGIN;
    GC_PUSH2(exp, env);

    if (object_get_type(args) != OBJECT_TYPE_PAIR
        || object_get_type(body) != OBJECT_TYPE_PAIR) {
        throw("Invalid lambda expression");
    }

    proc = (Proc*)object_create(OBJECT_TYPE_PROCEDURE);
    proc->args = (Pair*)args;
    proc->body = (Pair*)body;
    proc->env = env;

    GC_END;
    return (Object*)proc;
}

static Env *extend_environment(Pair *args, Pair *vals, Env *env)
{
    GC_BEGIN;
    GC_PUSH3(args, vals, env);

    env = env_extend(env);
    GC_PUSH1(env);

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

    GC_END;
    return env;
}

static Object *apply(Object *operator, Object *args, Env *env)
{
    Object *res = NULL;
    GC_BEGIN;
    GC_PUSH3(operator, args, env);

    if (operator->type == OBJECT_TYPE_PROCEDURE) {
        Proc *proc = (Proc*)operator;
        res = eval_sequence((Object*)proc->body,
                            extend_environment(proc->args, (Pair*)args, env),
                            CONTEXT_APPLICATION, proc);
    }
    else if (operator->type == OBJECT_TYPE_NATIVE) {
        Native *proc = (Native*)operator;
        unsigned num_of_args = core_get_list_size(args);
        if ((proc->rst == 0 && num_of_args > proc->req) || num_of_args < proc->req) {
            throw("Invalid args number %u", num_of_args);
        }
        res = proc->native_function(args);
    }
    GC_END;
    return res;
}

static Object *eval(Object *exp, Env *env, Context ctx)
{
    Object *obj = NULL;
    GC_BEGIN;
    GC_PUSH2(exp, env);

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
                obj = eval_definition(operands, env, ctx);
            }
            else if (strcmp(str, "set!") == 0) {
                obj = eval_assignment(operands, env, ctx);
            }
            else if (strcmp(str, "if") == 0) {
                obj = eval_if(operands, env, ctx);
            }
            else if (strcmp(str, "cond") == 0) {
                obj = eval_cond(operands, env, ctx);
            }
            else if (strcmp(str, "begin") == 0) {
                obj = eval_sequence(operands, env, ctx, NULL);
            }
            else if (strcmp(str, "lambda") == 0) {
                obj = make_procedure(operands, env);
            }
            else {
                obj = (ctx == CONTEXT_RETURN) ? exp
                    : apply(eval(operator, env, ctx),
                            list_of_values(operands, env, ctx), env);
            }
        }
    }
    GC_END;
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
    Boolean *res;
    GC_BEGIN;
    GC_PUSH1(obj);
    res = (Boolean*)object_create(OBJECT_TYPE_BOOLEAN);
    res->value = core_object_to_bool(obj);
    GC_END;
    return (Object*)res;
}

Object *core_eval(Object *exp, Env *env)
{
    Object *obj;
    assert(env != NULL);
    GC_BEGIN;
    GC_PUSH2(exp, env);
    gc_start();
    obj =  eval(exp, env, CONTEXT_EVALUATION);
    GC_PUSH1(obj);
    gc_stop();
    GC_END;
    return obj;
}
