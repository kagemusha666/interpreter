


/*
 *    str-list.c, for testing s-expression parsing functionality
 */


#include "../src/utils.h"

#include <stdlib.h>
#include <stdio.h>


int main(int argc, char *argv[])
{
    const char *example = "  ( define ( foo x y ) (bar (x y)))";
    char *result;

    printf("Parsing expression '%s'...", example);
    result = str_list_get_first_item(example);
    printf("First list item is %s", result);

    return EXIT_SUCCESS;
}
