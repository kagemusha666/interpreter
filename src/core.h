

/*
 *    core.h
 */


#include <stdlib.h>


int is_expression_complete(const char *line, size_t len);

int eval(const char *line, size_t len, char *output);
