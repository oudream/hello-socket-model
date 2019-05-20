/*
 * Compile with:
 * gcc prop_dict.c -o prop_dict -lprop -Wall -W -Wextra -ansi -pedantic
 */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <prop/proplib.h>

int main(int argc, char *argv[])
{
    prop_dictionary_t pd;
    prop_string_t ps;
    unsigned int i;

    /* Create a dictionary capable of holding `argc - 1' objects */
    pd = prop_dictionary_create_with_capacity(argc - 1);
    if (pd == NULL)
        err(EXIT_FAILURE, "prop_dictionary_create_with_capacity()");

    /*
     * For every supplied argument, create a <key, value> pair
     * and store it inside the dictionary.
     */
    for (i = 1; i < argc; i++) {
        ps = prop_string_create_cstring_nocopy(argv[i]);
        if (ps == NULL) {
            prop_object_release(pd);
            err(EXIT_FAILURE, "prop_string_create_cstring_nocopy()");
        }

        if (prop_dictionary_set(pd, argv[i], ps) == false) {
            prop_object_release(ps);
            prop_object_release(pd);
            err(EXIT_FAILURE, "prop_dictionary_set()");
        }

        prop_object_release(ps);
    }

    /* Output our property list as an XML file */
    if (prop_dictionary_externalize_to_file(pd, "./data.xml") == false) {
        prop_object_release(pd);
        err(EXIT_FAILURE, "prop_dictionary_externalize_to_file()");
    }

    /* Release dictionary */
    prop_object_release(pd);

    return EXIT_SUCCESS;
}
