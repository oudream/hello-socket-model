/*
 * Compile with:
 * gcc bitmap.c -o bitmap -Wall -W -Wextra -ansi -pedantic
 */

#include <limits.h>    /* for CHAR_BIT */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>    /* for memset() */

/* #define DEBUG */
#ifdef DEBUG
#define DPRINTF(x) printf x
#else
#define DPRINTF(x)
#endif

/* Function prototypes */
int is_blk_free(const unsigned char *array, unsigned int nblk);
void print_blks(const unsigned char *array, unsigned int nblks);
void diep(const char *s);

int main(int argc, char *argv[])
{
    unsigned char *array;
    unsigned int i, narr, nblks, u;

    /* Check argument count */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s nblocks\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Get number of blocks */
    nblks = (unsigned int) strtoul(argv[1], NULL, 10);
    u = nblks - CHAR_BIT * (nblks / CHAR_BIT);
    if (u == 0)
        narr = nblks / CHAR_BIT;
    else
        narr = nblks / CHAR_BIT + 1;
    printf("Requested nblks = %u\tAllocating narr = %u chars\n", nblks, narr);

    /* Allocate memory for char bitmap */
    array = malloc(narr);
    if (array == NULL)
        diep("malloc");

    /* Initialize all bits to 0 representing free blocks */
    memset(array, 0, narr);

    /* Print blocks' status */
    print_blks(array, nblks);

    /* Flag all blocks as used */
    for (i = 0; i < narr; i++)
        array[i] = 0xFF;
    print_blks(array, nblks);

    /* Free bitmap */
    free(array);

    return EXIT_SUCCESS;
}

int is_blk_free(const unsigned char *array, unsigned int nblk)
{
    unsigned int idx, u;

    idx = nblk / CHAR_BIT;
    u = nblk - CHAR_BIT * idx;
    DPRINTF(("blk %u is in array %u and bitpos %u\n",
             nblk, idx, u));

    return ((array[idx] & (1 << u)) == 0);
}

void print_blks(const unsigned char *array, unsigned int nblks)
{
    unsigned int i;

    for (i = 0; i < nblks; i++)
        printf("blk[%04u] = %s\n",
               i, is_blk_free(array, i) ? "FREE" : "USED");
}

void diep(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}
