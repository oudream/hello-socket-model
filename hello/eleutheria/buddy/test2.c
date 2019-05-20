/*
 * Compile with:
 * gcc test2.c mpool.c -o test2 -Wall -W -Wextra -ansi -pedantic
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>    /* for time() in srand() */

#include "mpool.h"

int main(int argc, char *argv[])
{
    mpool_t *mpool;
    mpret_t mpret;
    int i, j, **array;

    /* Check argument count */
    if (argc != 3) {
        fprintf(stderr, "Usage: %s nrows ncols\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Initialize memory pool with 1024 bytes */
    mpret = mpool_init(&mpool, 10, 5);
    if (mpret == MPOOL_ENOMEM) {
        fprintf(stderr, "mpool: not enough memory\n");
        exit(EXIT_FAILURE);
    }
    else if (mpret == MPOOL_ERANGE) {
        fprintf(stderr, "mpool: out of range in mpool_init()\n");
        exit(EXIT_FAILURE);
    }
    else if (mpret == MPOOL_EBADVAL) {
        fprintf(stderr, "mpool: bad value passed to mpool_init()\n");
        exit(EXIT_FAILURE);
    }

    /* Initialize random number generator */
    srand(time(NULL));

    /* Allocate memory for rows */
    if ((array = mpool_alloc(mpool, atoi(argv[1]) * sizeof *array)) == NULL) {
        fprintf(stderr, "mpool: no available block in pool\n");
        mpool_destroy(mpool);
        exit(EXIT_FAILURE);
    }

    /* Allocate memory for columns */
    for (i = 0; i < atoi(argv[1]); i++) {
        if ((array[i] = mpool_alloc(mpool,
                                    atoi(argv[2]) * sizeof **array)) == NULL) {
            fprintf(stderr, "mpool: no available block in pool\n");
            mpool_destroy(mpool);
            exit(EXIT_FAILURE);
        }
    }

    /* Fill in matrix with random values and print it */
    for (i = 0; i < atoi(argv[1]); i++) {
        for (j = 0; j < atoi(argv[2]); j++) {
            array[i][j] = rand() % 100;
            printf("%2d\t", array[i][j]);
        }
        printf("\n");
    }

    /*
     * Destroy memory pool and free all resources
     *
     * One, normally, would have to explicitly call mpool_free(),
     * in order to liberate the allocated blocks one by one.
     * But since we terminate anyway, just nuke the whole
     * pool with mpool_destroy()
     */
    mpool_destroy(mpool);

    return EXIT_SUCCESS;
}
