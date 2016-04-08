

#include "gc.h"
#include "error.h"
#include "debug.h"

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>


typedef struct stack {
    Object *item;
    struct stack *next;
} Stack;

static Stack *_stack;
static Object *_heap[GC_OBJECT_MAX_NUMBER];
static unsigned _objects = 0;
static bool _started = false;

void gc_start()
{
    _started = true;
    gc_clean();
}

void gc_stop()
{
    gc_clean();
    _started = false;
}

void gc_add(Object *obj)
{
    if (!(_objects < GC_OBJECT_MAX_NUMBER)) {
        ERROR("Not enough memory! Force GC!");
        gc_force();
    }
    else {
        gc_clean();
    }
    _heap[_objects] = obj;
    _objects += 1;
}

void gc_clean(void)
{
    if (_started && _objects > (GC_OBJECT_MAX_NUMBER * 2 / 3)) {
        gc_force();
    }
}

void gc_force(void)
{
    const unsigned num = _objects;
    Stack *stack = _stack;
    unsigned i;
    Object *tmp[GC_OBJECT_MAX_NUMBER];

    while (stack != NULL) {
        object_mark(stack->item);
        stack = stack->next;
    }

    _objects = 0;
    for (i = 0; i < num; i++) {
        assert(_heap[i] != NULL);

        if (_heap[i]->marked) {
            _heap[i]->marked = false;
            tmp[_objects] = _heap[i];
            _objects += 1;
        }
        else {
            object_delete(_heap[i]);
        }
    }
    memcpy(_heap, tmp, _objects * sizeof(Object*));
}

void gc_push(Object *obj)
{
    Stack *stack = malloc(sizeof(Stack));
    stack->item = obj;
    stack->next = _stack;
    _stack = stack;
}

Object *gc_pop(void)
{
    assert(_stack != NULL);
    Object *obj = NULL;
    Stack *stack = _stack;
    obj = stack->item;
    _stack = stack->next;
    free(stack);
    return obj;
}
