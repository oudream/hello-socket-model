/*
 * This program executes du(1) via pipe(2), then parses
 * the resulted output and populates a dictionary which
 * looks like this:
 *
 * [root dictionary]
 *     [child dictionary]
 *         [path]
 *         [size]
 *         [type]
 *     [child dictionary]
 *         [path]
 *         [size]
 *         [type]
 *     ...
 *
 * It then gets a reference to the child dictionary
 * pertaining to `.' path and extracts all <key, value>
 * pairs before it prints them to stdout.
 *
 * Compile with:
 * gcc prop_parse_du.c -o prop_parse_du -lprop -Wall -W -Wextra -ansi
 */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <prop/proplib.h>
#include <sys/stat.h>    /* for lstat(2) */

#define INIT_ROOT_CAPACITY 100    /* root's dict initial capacity */
#define INIT_CHILD_CAPACITY 3     /* child's dict initial capacity */
#define MAX_STR 100
#define MAX_TOKENS 3

int main(void)
{
    char str[MAX_STR];
    struct stat sb;               /* for lstat(2) */
    prop_dictionary_t prd;        /* root dictionary */
    prop_dictionary_t pcd;        /* child dictionary */
    prop_string_t ps;             /* path name */
    prop_number_t pn;             /* size in bytes */
    prop_bool_t pb;               /* true = dir */
    prop_object_t po;
    char *tokens[MAX_TOKENS];     /* for du(1) output parse */
    char *last, *p;
    FILE *fp;
    int i;

    /*
     * Initiate pipe stream to du(1)
     * -a flag: Display an entry for each file in the file hierarchy.
     * -P flag: No symbolic links are followed.
     */
    fp = popen("du -a -P", "r");
    if (fp == NULL)
        err(EXIT_FAILURE, "popen()");

    /* Create root dictionary */
    prd = prop_dictionary_create_with_capacity(INIT_ROOT_CAPACITY);
    if (prd == NULL)
        err(EXIT_FAILURE, "prop_dictionary_create_with_capacity()");

    /* Read from stream */
    while (fgets(str, MAX_STR, fp) != NULL) {
        /* Parse output of du(1) command */
        i = 0;
        p = strtok_r(str, "\t", &last);
        while (p && i < MAX_TOKENS - 1) {
            tokens[i++] = p;
            p = strtok_r(NULL, "\t", &last);
        }
        tokens[i] = NULL;

        /* Create child dictionary */
        pcd = prop_dictionary_create_with_capacity(INIT_CHILD_CAPACITY);
        if (pcd == NULL)
            err(EXIT_FAILURE, "prop_dictionary_create_with_capacity()");

        /*
         * tokens[0] holds the size in bytes
         *
         * We use a signed prop_number_t object, so that
         * when externalized it will be represented as decimal
         * (unsigned numbers are externalized in base-16).
         *
         * Note: atoi(3) does not detect errors, but we trust
         * du(1) to provide us with valid input. Otherwise,
         * we should use strtol(3) or sscanf(3).
         */
        pn = prop_number_create_integer(atoi(tokens[0]));
        if (pn == NULL)
            err(EXIT_FAILURE, "prop_number_create_integer()");

        /* tokens[1] holds the path (trim '\n') */
        (tokens[1])[strlen(tokens[1]) - 1] = '\0';
        ps = prop_string_create_cstring(tokens[1]);
        if (ps == NULL)
            err(EXIT_FAILURE, "prop_string_create_cstring()");

        /* Is it a directory ? Find out with lstat(2) */
        if (lstat(tokens[1], &sb) == -1)
            err(EXIT_FAILURE, "lstat()");

        pb = prop_bool_create(sb.st_mode & S_IFDIR ? true : false);
        if (pb == NULL)
            err(EXIT_FAILURE, "prop_bool_create()");

        /* Add path, size and type to child dictionary */
        if ((prop_dictionary_set(pcd, "path", ps) == false)
            || (prop_dictionary_set(pcd, "size in bytes", pn) == false)
            || (prop_dictionary_set(pcd, "is it dir?", pb) == false))
            err(EXIT_FAILURE, "prop_dictionary_set()");

        /* Add child dictionary to root dictionary */
        if (prop_dictionary_set(prd, tokens[1], pcd) == false)
            err(EXIT_FAILURE, "prop_dictionary_set()");

        /* Release all objects except for the root dictionary */
        prop_object_release(pn);
        prop_object_release(ps);
        prop_object_release(pb);
        prop_object_release(pcd);
    }

    /* Externalize root dictionary to file in XML representation */
    if (prop_dictionary_externalize_to_file(prd, "./data.xml") == false)
        err(EXIT_FAILURE, "prop_dictionary_externalize_to_file()");

    /* Get child dictionary pertaining to `.' path */
    po = prop_dictionary_get(prd, ".");
    if (po == NULL)
        err(EXIT_FAILURE, "prop_dictionary_get()");

    /* Extract all <key, value> pairs and print them to stdout */
    printf("Path: %s\nSize in bytes: %lld\nIs it dir?: %s\n",
           prop_string_cstring(
               prop_dictionary_get(po, "path")),
           prop_number_integer_value(
               prop_dictionary_get(po, "size in bytes")),
           prop_bool_true(
               prop_dictionary_get(po, "is it dir?")) == true ?
           "true" : "false");

    /* Release root dictionary */
    prop_object_release(prd);

    /* Close pipe stream */
    if  (pclose(fp) == -1)
        err(EXIT_FAILURE, "pclose()");

    return EXIT_SUCCESS;
}
