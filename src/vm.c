

/*
 *    vm.c
 */


#include "vm.h"
#include "vm_priv.h"
#include "debug.h"
#include "error.h"

#include <stdlib.h>


void vm_mark_all_objects(VM *vm)
{
    for (int i = 0; i < vm->stack_size; i++)
        object_mark(vm->stack[i]);
}

void vm_sweep_unreached_objects(VM *vm)
{
    Object **obj = &vm->last_object;

    while (*obj) {
        if (!(*obj)->marked) {
            Object *unreached = *obj;
            *obj = unreached->next;
            object_finalize(unreached);
            vm->object_number--;
        }
        else {
            (*obj)->marked = 0;
            obj = &(*obj)->next;
        }
    }
}

void vm_collect_garbage(VM *vm)
{
    vm->object_max_number = vm->object_number * 2;

    vm_mark_all_objects(vm);
    vm_sweep_unreached_objects(vm);
}

Object *vm_create_object_from_string(VM *vm, const char *str)
{
    if (vm->object_number == vm->object_max_number)
        vm_collect_garbage(vm);

    Object *obj = object_create_from_string(vm->last_object, str);
    vm->last_object = obj;
    vm->object_number++;
    return obj;
}

VM *vm_create()
{
    VM *vm = (VM*)malloc(sizeof(VM));
    vm->stack_size = 0;
    vm->object_number = 0;
    vm->object_max_number = OBJECT_MAX_NUMBER;
    vm->last_object = 0;
}

void vm_finalize(VM *vm)
{
    Object **obj = &vm->last_object;
    while (*obj) {
        Object *tmp = *obj;
        obj = &(*obj)->next;
        free(tmp);
    }
    free(vm);
}

int vm_eval_str(VM *vm, const char *str)
{
    Object *obj = vm_create_object_from_string(vm, str);
    if (obj == NULL)
        return SYNTAX_ERROR;

    return vm_call(vm, obj, &vm_print)
           && vm_call(vm, vm->return_value, &vm_print);
}
