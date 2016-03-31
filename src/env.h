

#ifndef ENV_H
#define ENV_H

#include "types.h"

#include <stdbool.h>

Env *env_extend(Env *env);

bool env_add_native_function(
    Env *env, const char *name, unsigned req, unsigned rst, NativeFunction function);

Object *env_lookup_variable(Env *env, Unbound *var);

bool env_define_variable(Env *env, Unbound *var, Object *val);

bool env_set_variable(Env *env, Unbound *var, Object *val);

Object *env_lookup_variable_str(Env *env, const char *str);

bool env_define_variable_str(Env *env, const char *str, Object *val);

bool env_set_variable_str(Env *env, const char *str, Object *val);

#endif // ENV_H
