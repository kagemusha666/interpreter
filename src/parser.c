

#include "parser.h"
#include "error.h"
#include "debug.h"

#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define throw(format,args...) throw_exception(ERROR_TYPE_PARSER, format, ##args);

#define throw_str(str, len, format, args...) throw_exception_str(ERROR_TYPE_PARSER, \
                                                                str, len, format, ##args);

#define throw_buf(beg, end, format, args...) throw_exception_buf(ERROR_TYPE_PARSER, \
                                                                beg, end, format, ##args);


typedef enum element {
    ELEMENT_INVALID,
    ELEMENT_BRACE,
    ELEMENT_QUOTE,
    ELEMENT_DOT,
    ELEMENT_NUMBER,
    ELEMENT_SPECIAL,
    ELEMENT_TEXT,
    ELEMENT_LAST
} Element;

static int isbrace(int c)
{
    return ((char)c) == '(' || ((char)c) == ')' ? 1 : 0;
}

static unsigned get_braces_element_size(const char *begin, const char *end);
static unsigned get_quote_element_size(const char *begin, const char *end);
static unsigned get_number_element_size(const char *begin, const char *end);
static unsigned get_text_element_size(const char *begin, const char *end);
static char *find_element(
    const char *begin, const char *end, Element *el, unsigned *size);

static Object *create_object_from_element(Element el, const char *str, unsigned len);
static Object *create_object_pair_from_string(const char *str, unsigned len);
static Object *create_object_string_from_string(const char *str, unsigned len);
static Object *create_object_integer_from_string(const char *str, unsigned len);
static Object *create_object_unbound_from_string(const char *str, unsigned len);


static unsigned get_braces_element_size(const char *begin, const char *end)
{
    unsigned size = 0;
    char *it = (char*)begin;
    unsigned count = 0;

    do {
        if (*it == '(')
            count++;
        else if (*it == ')')
            count--;
        size++;
        it++;
    } while (count > 0 && it != end);
    if (count != 0) {
        throw_buf(begin, end, "invalid braces count");
    }
    return size;
}

static unsigned get_quote_element_size(const char *begin, const char *end)
{
    unsigned size = 1;
    char *it = (char*)begin;

    if (*it == '\'') {
        do {
            size++;                
            it++;
        } while (!isspace(*it) && it != end);
        size--;
    }
    else {
        do {
            size++;                
            it++;
        } while (*it != '\"' && it != end);
        if (*it != '\"') {
            throw_buf(begin, end, "unescaped quote");
        }
    }
    return size;
}

static unsigned get_number_element_size(const char *begin, const char *end)
{
    unsigned size = 0;
    char *it = (char*)begin;

    do {
        if (!isdigit(*it)) {
            throw_buf(begin, end, "invalid number representation");
        }
        size++;                
        it++;
    } while (!isbrace(*it) && !isspace(*it) && it != end);

    return size;
}

static unsigned get_text_element_size(const char *begin, const char *end)
{
    unsigned size = 0;
    char *it = (char*)begin;

    do {
        size++;                
        it++;
    } while (!isspace(*it) && it != end);

    return size;
}

static char *find_element(const char *begin, const char *end, Element *el, unsigned *size)
{
    char *it;
    assert(begin != NULL);
    assert(end != NULL);
    assert(el != NULL);
    assert(size != NULL);
    *el = ELEMENT_INVALID;
    *size = 0;

    for (it = (char*)begin; it != (char*)end; it++) {
        if (isspace(*it)) {
            continue;
        }
        else if (!isprint(*it)) {
            throw_buf(begin, end, "invalid character");
        }
        else if (*it == '(') {
            *el = ELEMENT_BRACE;
            *size = get_braces_element_size(it, end);
            break;
        }
        else if (*it == '\'' || *it == '"') {
            *el = ELEMENT_QUOTE;
            *size = get_quote_element_size(it, end);
            break;
        }
        else if (*it == '.') {
            *el = ELEMENT_DOT;
            *size = 1;
            break;
        }
        else if (isdigit(*it)) {
            *el = ELEMENT_NUMBER;
            *size = get_number_element_size(it, end);
            break;
        }
        else if (*it == '#') {
            *el = ELEMENT_SPECIAL;
            *size = get_text_element_size(it + 1, end) + 1;
            break;
        }
        else {
            *el = ELEMENT_TEXT;
            *size = get_text_element_size(it, end);
            break;
        }
    }
    return it;
}

static Object *create_object_string_from_string(const char *str, unsigned size)
{
    Object *obj;
    char **text;

    if (size < 2
        || size > STRING_MAX_LENGTH
        || (str[0] != '\'' && str[0] != '"' && str[size - 2] != '"'))
        throw_str(str, size, "unexpected quote position");

    obj = object_create(OBJECT_TYPE_STRING);
    text = &((String*)obj)->cstr;

    size = (str[0] != '\'') ? size - 2 : size - 1;

    *text = (char*)malloc(size + 1);
    memcpy(*text, str + 1, size);
    (*text)[size] = 0;
    return obj;
}

static Object *create_object_integer_from_string(const char *str, unsigned size)
{
    Object *obj;
    char buf[size + 1];
    memcpy(buf, str, size);
    buf[size] = 0;

    obj = object_create(OBJECT_TYPE_INTEGER);
    ((Integer*)obj)->value = atoi(buf);
    return obj;
}

static Object *create_object_unbound_from_string(const char *str, unsigned size)
{
    Object *obj;
    char **text;

    obj = object_create(OBJECT_TYPE_UNBOUND);
    text = &((Unbound*)obj)->cstr;

    *text = (char*)malloc(size + 1);
    memcpy(*text, str, size);
    (*text)[size] = 0;
    return obj;
}

static Object *create_object_from_special_string(const char *str, unsigned size)
{
    if (strncmp(str, "#nil", size) == 0) {
        return NULL;
    }
    else if (strncmp(str, "#true", size) == 0) {
        Boolean *obj = (Boolean*)object_create(OBJECT_TYPE_BOOLEAN);
        obj->value = true;
        return (Object*)obj;
    }
    else if (strncmp(str, "#false", size) == 0) {
        Boolean *obj = (Boolean*)object_create(OBJECT_TYPE_BOOLEAN);
        obj->value = true;
        return (Object*)obj;
    }
    else {
        throw_str(str, size, "unknown special symbol");
    }
    return NULL;
}

static Object *create_object_pair_from_string(const char *str, unsigned size)
{
    const char *begin = str + 1;
    const char *end = str + size - 1;
    Object *obj = NULL;
    Object *head = NULL;
    char *it;
    Element el;

    it = find_element(begin, end, &el, &size);

    if (el != ELEMENT_INVALID) {
        obj = object_create(OBJECT_TYPE_PAIR);
        head = obj;

        do {
            Pair *pair = (Pair*)obj;
            if (pair->first == NULL) {
                pair->first = create_object_from_element(el, it, size);
            }
            else {
                pair->rest = create_object_from_element(el, it, size);
            }
            it = find_element(it + size, end, &el, &size);
            if (el != ELEMENT_INVALID) {
                if (pair->rest != NULL) {
                    throw_buf(it + size, end, "unexpected in line");
                }
                if (el != ELEMENT_DOT) {
                    obj = object_create(OBJECT_TYPE_PAIR);
                    pair->rest = obj;
                }
                else {
                    it = find_element(it + size, end, &el, &size);
                }
            }
        } while (it != end);
    }
    else {
        throw_buf(begin, end, "unexpected in line");
    }

    return head;
}

static Object *create_object_from_element(Element el, const char *str, unsigned size)
{
    switch (el) {
    case ELEMENT_BRACE:
        return create_object_pair_from_string(str, size);
    case ELEMENT_QUOTE:
        return create_object_string_from_string(str, size);
    case ELEMENT_NUMBER:
        return create_object_integer_from_string(str, size);
    case ELEMENT_SPECIAL:
        return create_object_from_special_string(str, size);
    case ELEMENT_TEXT:
        return create_object_unbound_from_string(str, size);
    case ELEMENT_INVALID:
    default:
        throw_str(str, size, "unexpected element");
    }
    return NULL;
}

Object *parser_create_object_from_string(const char *str)
{
    Object *obj = NULL;
    unsigned len = strlen(str);
    char *it;
    Element el;
    unsigned size;

    it = find_element(str, str + len, &el, &size);
    obj = create_object_from_element(el, it, size);

    if (find_element(it + size, str + len, &el, &size) != (str + len)) {
        throw_str(it, size, "Multiple expressions not supported");
    }

    return obj;
}
