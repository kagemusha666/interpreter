

#include "env.h"
#include "debug.h"

#include <string.h>
#include <stdlib.h>


static char *create_equal_string(const char *str)
{
    const unsigned len = strlen(str);
    char *ptr = malloc(len + 1);
    ptr[len] = 0;
    strcpy(ptr, str);
    return ptr;
}

static Object *create_native(const char *name, unsigned req, unsigned rst, NativeFunction function)
{
    Native *obj = (Native*)object_create(OBJECT_TYPE_NATIVE);
    obj->cstr = create_equal_string(name);
    obj->req = req;
    obj->rst = rst;
    obj->native_function = function;
    return (Object*)obj;
}

Env *env_extend(Env *env)
{
    Env *newEnv = (Env*)object_create(OBJECT_TYPE_ENVIRONMENT);
    newEnv->next = env;
    return newEnv;
}

bool env_add_native_function(
    Env *env, const char *name, unsigned req, unsigned rst, NativeFunction function)
{
    Frame *frame;

    if (env_lookup_variable_str(env, name) == NULL) {
        frame = malloc(sizeof(Frame));
        frame->cstr = create_equal_string(name);
        frame->object = create_native(name, req, rst, function);
        frame->next = env->frame;
        env->frame = frame;
        return true;
    }
    return false;
}

Object *env_lookup_variable_str(Env *env, const char *str)
{
    while (env != NULL) {
        Frame *frame = env->frame;

        while (frame != NULL) {
            if (strcmp(str, frame->cstr) == 0) {
                return frame->object;
            }
            frame = frame->next;
        }
        env = env->next;
    }
    return NULL;
}

bool env_define_variable_str(Env *env, const char *str, Object *val)
{
    Frame *frame = env->frame;
    while (frame != NULL) {
        if (strcmp(str, frame->cstr) == 0) {
            return false;
        }
        frame = frame->next;
    }
    frame = malloc(sizeof(Frame));
    frame->cstr = create_equal_string(str);
    frame->object = val;
    frame->next = env->frame;
    env->frame = frame;
    return true;
}

bool env_set_variable_str(Env *env, const char *str, Object *val)
{
    Frame *frame = env->frame;
    while (frame != NULL) {
        if (strcmp(str, frame->cstr) == 0) {
            frame->object = val;
            return true;
        }
        frame = frame->next;
    }
    return false;
}

Object *env_lookup_variable(Env *env, Unbound *var)
{
    assert(env != NULL);
    assert(object_get_type((Object*)var) == OBJECT_TYPE_UNBOUND);
    return env_lookup_variable_str(env, var->cstr);
}

bool env_define_variable(Env *env, Unbound *var, Object *val)
{
    assert(env != NULL);
    assert(object_get_type((Object*)var) == OBJECT_TYPE_UNBOUND);
    assert(val != NULL);
    return env_define_variable_str(env, var->cstr, val);
}

bool env_set_variable(Env *env, Unbound *var, Object *val)
{
    assert(env != NULL);
    assert(object_get_type((Object*)var) == OBJECT_TYPE_UNBOUND);
    assert(val != NULL);
    return env_set_variable_str(env, var->cstr, val);
}
