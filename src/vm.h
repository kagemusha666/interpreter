

/*
 *    vm.h
 */


#include "types.h"

#ifndef VM_H
#define VM_H

#define STACK_MAX_SIZE 256
#define OBJECT_MAX_NUMBER 64


typedef struct virtual_machine {
    Object *stack[STACK_MAX_SIZE];
    unsigned stack_size;
    unsigned object_number;
    unsigned object_max_number;
    Object *last_object;
    unsigned last_stack_size;
    Object *return_value;
} VM;

VM *vm_create();

void vm_finalize(VM *vm);

int vm_eval_str(VM *vm, const char *str);

void vm_collect_garbage(VM *vm);

#endif
