

/*
 *    vm_priv.h
 *    Private methods of VM class.
 */


#include "vm.h"

#ifndef VM_PRIV_H
#define VM_PRIV_H

Object *vm_create_object(VM *vm, Type type);

int vm_push(VM *vm, Object *obj);

Object *vm_pop(VM *vm);

int vm_call(VM *vm, Object *obj, int (*function)(VM*));

int vm_print(VM *vm);

#endif
