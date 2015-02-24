

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
        if (!(*obj)->vm.marked) {
            Object *unreached = *obj;
            *obj = unreached->vm.next;
            object_finalize(unreached);
            vm->object_number--;
        }
        else {
            (*obj)->vm.marked = 0;
            obj = &(*obj)->vm.next;
        }
    }
}

void vm_collect_garbage(VM *vm)
{
    vm->object_max_number = vm->object_number * 2;

    vm_mark_all_objects(vm);
    vm_sweep_unreached_objects(vm);
}

void vm_add_object_to_common_list(VM *vm, Object *obj)
{
    if (obj != NULL) {
        obj->vm.next = vm->last_object;
        obj->vm.marked = 0;
        vm->last_object = obj;
        vm->object_number++;

        if (object_get_type(obj) == PAIR) {
            Pair *pair = (Pair*)obj;

            vm_add_object_to_common_list(vm, pair->first);
            vm_add_object_to_common_list(vm, pair->rest);
        }
    }
}

Object *vm_create_object_from_string(VM *vm, const char *str)
{
    if (vm->object_number == vm->object_max_number)
        vm_collect_garbage(vm);

    Object *obj = object_create_from_string(str);

    vm_add_object_to_common_list(vm, obj);

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
        obj = &(*obj)->vm.next;
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
