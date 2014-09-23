

/*
 *    vm.c
 */


#include "debug.h"


void VM_mark_all_objects(VM *vm)
{
    for (int i = 0; i < vm->stack_size; i++)
        object_mark(stack[i]);
}

void VM_sweep_unreached_objects(VM *vm)
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

void VM_collect_garbage(VM *vm)
{
    vm->object_max_number = vm->object_number * 2;

    VM_mark_all_objects(vm);
    VM_sweep_unreached_objects(vm);
}

Object *VM_create_object(VM *vm, Type type)
{
    if (vm->object_number == vm->object_max_nubmer)
        VM_collect_garbage(vm);

    Object *obj = create_object(vm->last_object, type);
    vm->last_object = obj;
    vm->object_number++;
}

Object *VM_create_object_from_string(VM *vm, const char *str)
{
    if (vm->object_number == vm->object_max_nubmer)
        VM_collect_garbage(vm);

    Object *obj = create_object(vm->last_object, str);
    vm->last_object = obj;
    vm->object_number++;
}

unsigned VM_get_arg_count(VM *vm)
{
    return (vm->stack_size - vm->last_stack_size);
}

Object *VM_get_arg(VM *vm, unsigned num)
{
    return vm->stack[vm->last_stack_size + num];
}

int VM_print(VM *vm)
{
    int code = OK;
    if (VM_get_arg_count(vm) != 1)
        return INVALID_ARITY;
    printf("%s", object_to_string(VM_get_arg(0)));
    return code;
}

VM *VM_create()
{
    VM *vm = (VM*)malloc(sizeof(VM));
    vm->stack_size = 0;
    vm->object_number = 0;
    vm->object_max_number = OBJECT_MAX_NUMBER;
    vm->last_object = 0;
}

void VM_finalize(VM *vm)
{
    free(vm);
}

int VM_call(VM *vm, Object *obj, int (*proc)(VM*))
{
    int code = OK;

    vm->last_stack_size = vm->stack_size;

    code = VM_push(vm, obj);
    if (code != OK)
        return code;

    code = proc(vm);

    assert(VM_pop(vm) != NULL);
    assert(vm->last_stack_size == vm->stack_size);
    return code;
}

int VM_eval_exp(VM *vm, const char *exp)
{
    Object *obj = VM_create_object_from_string(vm, exp);
    if (obj == NULL)
        return SYNTAX_ERROR;
    return VM_call(vm, obj, &VM_print);
}

int VM_push(VM *vm, Object *obj)
{
    if (vm->stack_size == STACK_MAX_SIZE)
        return STACK_OVERFLOW;

    vm->stack[vm->stack_size++] = obj;
}

Object *VM_pop(VM *vm)
{
    if(vm->stack_size == 0)
        return NULL;

    return vm->stack[--vm->stack_size];
}
