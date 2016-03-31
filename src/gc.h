

/*
 *    vm.h
 */


#include "types.h"

#ifndef GC_H
#define GC_H

#define GC_STACK_MAX_SIZE 256
#define GC_OBJECT_MAX_NUMBER 64

void gc_collect_garbage(void);

#endif // GC_H
