

/*
 *    vm_priv.c
 *    Private methods of VM class.
 */


#include "vm_priv.h"
#include "debug.h"
#include "error.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>


Object *vm_create_object(VM *vm, Type type)
{
    if (vm->object_number == vm->object_max_number)
        vm_collect_garbage(vm);

    Object *obj = object_create(vm->last_object, type);
    vm->last_object = obj;
    vm->object_number++;
}

int vm_push(VM *vm, Object *obj)
{
    if (vm->stack_size == STACK_MAX_SIZE)
        return STACK_OVERFLOW;

    vm->stack[vm->stack_size++] = obj;
    return OK;
}

Object *vm_pop(VM *vm)
{
    if(vm->stack_size == 0)
        return NULL;

    return vm->stack[--vm->stack_size];
}

int vm_call(VM *vm, Object *obj, int (*function)(VM*))
{
    int code = OK;

    vm->last_stack_size = vm->stack_size;

    code = vm_push(vm, obj);
    if (code != OK)
        return code;

    vm->return_value = NULL;

    code = function(vm);

    assert(vm_pop(vm) != NULL);
    assert(vm->last_stack_size == vm->stack_size);
    return code;
}

unsigned vm_get_arg_count(VM *vm)
{
    return (vm->stack_size - vm->last_stack_size);
}

Object *vm_get_arg(VM *vm, unsigned num)
{
    if (vm->last_stack_size + num >= vm->stack_size)
      return NULL;

    return vm->stack[vm->last_stack_size + num];
}

int vm_print(VM *vm)
{
    int code = OK;
    if (vm_get_arg_count(vm) != 1)
        return INVALID_ARITY;

    printf("%s", object_to_string(vm_get_arg(vm, 0)));
    return code;
}
