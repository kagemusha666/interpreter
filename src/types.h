

/*
 *    types.h
 */


#define STRING_MAX_LENGTH 256


typedef enum type
{
    NUMERIC,
    STRING,
    PAIR,
    VARIABLE
} Type;

typedef struct object
{
    Type type;
    struct object *next; /* next allocated object */
    char marked;
    int (*from_string)(struct object*, const char*);
    const char *(*to_string)(struct object*);
    void (*finalize)(struct bject*);
} Object;

typedef struct numeric
{
    Object object;
    int value;
} Numeric;

typedef struct string
{
    Object object;
    char *text;
} String;

typedef struct pair
{
    Object object;
    Object *first;
    Object *rest;
} Pair;

typedef struct variable
{
    Object object;
    char *text_representation;
} Variable;

Type object_get_type(Object *obj);

void object_mark(Object *obj);

int object_from_string(Object *obj, const char *str);

const char *object_to_string(Object *obj);

Object *object_create(Object *prev, Type type);

Object *object_create_from_string(Object *prev, const char *str);

void object_finalize(Object *obj);
