

#ifndef CORE_H
#define CORE_H

#include "types.h"

#include <stdbool.h>


unsigned core_get_list_size(Object *obj);

bool core_object_to_bool(Object *obj);

Object *core_object_to_object(Object *obj);

Object *core_eval(Object *exp, Env *env);

#endif
