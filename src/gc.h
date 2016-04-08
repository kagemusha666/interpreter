

/*
 *    vm.h
 */


#include "types.h"

#ifndef GC_H
#define GC_H

#define GC_OBJECT_MAX_NUMBER 128

void gc_start();

void gc_stop();

void gc_add(Object *obj);

void gc_clean(void);

void gc_force(void);

void gc_push(Object *obj);

Object *gc_pop(void);


#define GC_BEGIN unsigned _pushed_ = 0

#define GC_PUSH1(OBJ) do { gc_push((Object*)OBJ);\
                           _pushed_ += 1; } while(0)

#define GC_PUSH2(OBJ1,OBJ2) do { gc_push((Object*)OBJ1);\
                                 gc_push((Object*)OBJ2);\
                                 _pushed_ += 2; } while(0)

#define GC_PUSH3(OBJ1,OBJ2,OBJ3) do { gc_push((Object*)OBJ1);       \
                                      gc_push((Object*)OBJ2);\
                                      gc_push((Object*)OBJ3);\
                                      _pushed_ += 3; } while(0)

#define GC_END while (_pushed_ > 0) { gc_pop(); _pushed_--; }

#endif // GC_H
