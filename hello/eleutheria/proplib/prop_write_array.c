/*
 * Compile with:
 * gcc prop_write_array.c -o prop_write_array -lprop -Wall -W -Wextra -ansi -pedantic
 */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <prop/proplib.h>

#define INIT_CAPACITY 10

int main(int argc, char *argv[])
{
    /*
     * Declare a pointer to a prop_array object
     * Note that prop_array_t is a pointer being
     * hidden inside a typedef, i.e.
     * typedef struct _prop_array *prop_array_t;
     */
    prop_array_t pa;
    prop_string_t ps;
    int i;

    /* No effect in NetBSD, but increases portability */
    setprogname(argv[0]);

    /* Check argument count */
    if (argc < 2) {
        fprintf(stderr, "Usage: %s data.xml [arguments]\n", getprogname());
        exit(EXIT_FAILURE);
    }

    /*
     * Create array object with initial capacity set to `INIT_CAPACITY'.
     * Note that the array will expand on demand by the prop_array_add()
     * with `EXPAND_STEP' step as defined in libprop/prop_array.c
     */
    pa = prop_array_create_with_capacity(INIT_CAPACITY);
    if (pa == NULL)
        err(EXIT_FAILURE, "prop_array_create_with_capacity()");

    /*
     * For every argument, create a prop_string_t object
     * that references it and store it in the array
     */
    for (i = 0; i < argc; i++) {
        ps = prop_string_create_cstring_nocopy(argv[i]);
        if (ps == NULL) {
            prop_object_release(pa);
            err(EXIT_FAILURE, "prop_string_create_cstring_nocopy()");
        }

        if (prop_array_add(pa, ps) == false) {
            prop_object_release(pa);
            prop_object_release(ps);
            err(EXIT_FAILURE, "prop_array_add()");
        }

        prop_object_release(ps);
    }

    /* Export array contents to file as XML */
    if (prop_array_externalize_to_file(pa, argv[1]) == false) {
        prop_object_release(pa);
        err(EXIT_FAILURE, "prop_array_externalize_to_file()");
    }

    /* Release array object */
    prop_object_release(pa);

    return EXIT_SUCCESS;
}
