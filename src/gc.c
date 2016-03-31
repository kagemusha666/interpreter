

#include "gc.h"

#include "debug.h"
#include "error.h"

#include <stdlib.h>

/*
void gc_mark_all_objects(void)
{
    for (int i = 0; i < vm->stack_size; i++)
        object_mark(vm->stack[i]);
}

void gc_sweep_unreached_objects(void)
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
*/
void gc_collect_garbage(void)
{
/*
    vm->object_max_number = vm->object_number * 2;

    vm_mark_all_objects(vm);
    vm_sweep_unreached_objects(vm);
*/
}
/*
void gc_add_object_to_common_list(Object *obj)
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
*/
