/*
 * Compile with:
 * gcc test1.c mpool.c -o test1 -Wall -W -Wextra -ansi -pedantic
 */

#include <stdio.h>
#include <stdlib.h>

#include "mpool.h"

int main(void)
{
    mpool_t *mpool;
    mpret_t mpret;
    char *buffer;

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

    /* Allocate 32 bytes for buffer */
    if ((buffer = mpool_alloc(mpool, 5)) == NULL) {
        fprintf(stderr, "mpool: no available block in pool\n");
        mpool_destroy(mpool);
        exit(EXIT_FAILURE);
    }

    /* Read input from standard input stream */
    fgets(buffer, 1<<5, stdin);

    /* Print buffer's contents */
    printf("Buffer: %s", buffer);

    /*
     * Free buffer --
     * could be omitted, since we call mpool_destroy() afterwards
    */
    mpool_free(mpool, buffer);

    /* Destroy memory pool and free all resources */
    mpool_destroy(mpool);

    return EXIT_SUCCESS;
}
