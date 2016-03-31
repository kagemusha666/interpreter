

/*
 *    types.h
 */


#ifndef TYPES_H
#define TYPES_H

#define STRING_MAX_LENGTH 256

typedef enum type
{
    OBJECT_TYPE_NONE,
    OBJECT_TYPE_INTEGER,
    OBJECT_TYPE_STRING,
    OBJECT_TYPE_PAIR,
    OBJECT_TYPE_LIST = OBJECT_TYPE_PAIR,
    OBJECT_TYPE_UNBOUND,
    OBJECT_TYPE_VARIABLE = OBJECT_TYPE_UNBOUND,
    OBJECT_TYPE_ENVIRONMENT,
    OBJECT_TYPE_PROCEDURE,
    OBJECT_TYPE_NATIVE,
    OBJECT_TYPE_LAST
} Type;

typedef struct object
{
    Type type;
    struct gc_attributes {
        struct object *next; /* next allocated object */
        char marked;
    } gc;
    const char *(*to_string)(struct object*);
    void (*dump)(struct object*);
    void (*mark)(struct object*);
    void (*finalize)(struct object*);
} Object;

typedef Object *(*NativeFunction)(Object *);

typedef struct frame {
    const char *cstr;
    Object *object;
    struct frame *next;
} Frame;

typedef struct integer
{
    Object object;
    int value;
} Integer;

typedef struct string
{
    Object object;
    char *cstr;
} String;

typedef struct pair
{
    Object object;
    Object *first;
    Object *rest;
} Pair;

typedef struct list
{
    Object object;
    Object *item;
    struct list *next;
} List;

typedef struct unbound
{
    Object object;
    char *cstr;
} Unbound;

typedef struct unbound Variable;

typedef struct environment
{
    Object object;
    Frame *frame;
    struct environment *next;
} Env;

typedef struct procedure
{
    Object object;
    char *cstr;
    Pair *args;
    Pair *body;
    Env *env;
} Proc;

typedef struct native
{
    Object object;
    char *cstr;
    unsigned req;
    unsigned rst;
    NativeFunction native_function;
} Native;


Object *object_create(Type type);

void object_delete(Object *obj);

Type object_get_type(Object *obj);

const char* object_to_string(Object *obj);

void object_dump(Object *obj);

void object_mark(Object *obj);

#endif
