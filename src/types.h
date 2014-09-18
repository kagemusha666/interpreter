

/*
 *    types.h
 */


#define QUOTE_MAX_LENGTH 256


enum Types
{
  OBJECT,
  NUMERIC,
  QUOTE,
  PAIR
};

struct Object
{
    enum Types type;
};

struct Numeric
{
    struct Object object;
    int val;
};

struct Quote
{
    struct Object object;
    size_t len;
    char *buf;
};

struct Pair
{
    struct Object object;
    struct Object *first;
    struct Object *rest;
};
