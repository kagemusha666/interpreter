

/*
 *    vm.h
 */


#include "type.h"


#define STACK_MAX_SIZE 256
#define OBJECT_MAX_NUMBER 64


typedef struct virtual_machine {
    Object *stack[STACK_MAX_SIZE]
    unsigned stack_size;
    unsinged object_number;
    unsigned object_max_number;
    Object *last_object;
    unsigned last_stack_size;
} VM;

VM *VM_create();

void VM_finalize(VM *vm);

int VM_eval_exp(VM *vm, const char *exp);

void VM_collect_garbage(VM *vm);

int VM_push(VM *vm, Object *obj);

Object *VM_pop(VM *vm);
