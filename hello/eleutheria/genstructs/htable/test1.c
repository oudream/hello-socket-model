/*
 * Compile with:
 * gcc test1.c htable.c -o test1 -Wall -W -Wextra -ansi -pedantic
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>    /* for time() in srand() */

#include "htable.h"

/* Function prototypes */
unsigned int myhashf(const void *key);
int mystrcmp(const void *arg1, const void *arg2);
void myprintf(const void *key, const void *data);
void print_elm(void *data);
void get_rand_string(char *str, size_t len);

int main(void)
{
    htable_t htable;
    size_t i;
    char str[20][10+1];    /* +1 for '\0' */

    /* Initialize random number generator */
    srand(time(NULL));

    /* Produce some random strings to put in hash table */
    for (i = 0; i < sizeof str / sizeof *str; i++)
        get_rand_string(str[i], 10);

    /* Initialize table */
    if (htable_init(&htable, 32, 2, myhashf, mystrcmp, myprintf) == HT_NOMEM) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    /* Add random entries in hash table */
    for (i = 0; i < sizeof str / sizeof *str; i++)
        htable_insert(&htable, str[i], str[i]);
    /* htable_print(&htable); */

    /*
     * Traverse all elemets and print each one of them
     * using the print_elm() callback function
     */
    htable_traverse(&htable, print_elm);

    /*
     * Print the length of every chain
     * This gives as a metric on how good or bad our hash function is
     */
    for (i = 0; i < htable_get_size(&htable); i++)
        printf("chain[%lu] = %lu\n",
               (unsigned long)i,
               (unsigned long)htable_stat_get_chain_len(&htable, i));

    /* Print number of automatic resizes */
    printf("Automatic resizes: %lu\n", (unsigned long)htable_stat_get_grows(&htable));

    /* Free memory */
    htable_free(&htable);

    return EXIT_SUCCESS;
}

unsigned int myhashf(const void *key)
{
    unsigned int i, hash = 5381;
    char *str = (char *)key;

    for (i = 0; i < strlen(str); i++)
        hash = ((hash << 5) + hash) + str[i];

    return hash;
}

int mystrcmp(const void *arg1, const void *arg2)
{
    return (strcmp((char *)arg1, (char *)arg2));
}

void myprintf(const void *key, const void *data)
{
    printf("%s(%s) ", (char *)key, (char *)data);
}

void print_elm(void *data)
{
    printf("%s\n", (char *)data);
}

void get_rand_string(char *str, size_t len)
{
    size_t i;

    for (i = 0; i < len; i++)
        str[i] = 65 + 32 + (rand() / (RAND_MAX / 26 + 1));    /* 'a' to 'z' */
    str[i] = '\0';
}
