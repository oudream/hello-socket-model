/*
 * Compile with:
 * gcc test3.c mpool.c mstat.c -o test3 -Wall -W -Wextra -ansi -pedantic
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>  /* for memset() */
#include <time.h>    /* for time() in srand() */

#include "mpool.h"
#include "mstat.h"

#define MAX_BLOCKS 1000

int main(void)
{
    char *pblk[MAX_BLOCKS];
    mpool_t *mpool;
    mpret_t mpret;
    size_t an, un;    /* nodes */
    size_t ab, ub;    /* blocks */
    size_t me, sp;    /* merges, splits */
    size_t i, s;

    /* Initialize memory pool with 1048576 bytes */
    mpret = mpool_init(&mpool, 20, 5);
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

    /* Allocate blocks */
    for (i = 0; i < MAX_BLOCKS; i++) {
        if ((pblk[i] = mpool_alloc(mpool, s = 1 << (5 + rand() % 7))) == NULL)
            break;

        /*
         * Zero out the allocated block
         *
         * By doing this, there is a chance that we catch
         * allocator's bugs, i.e. by writing over the block
         * and corrupting next block's header.
         */
        memset(pblk[i], 0, s);

        /* Every now and then free a block to create holes and invoke merges */
        if (rand() % 3 == 0) {
            mpool_free(mpool, pblk[i]);
            pblk[i] = NULL;
        }
    }

    /* Free the rest of the blocks */
    for (i = 0; i < MAX_BLOCKS; i++)
        if (pblk[i] != NULL)
            mpool_free(mpool, pblk[i]);

    /* Dump statistics */
    mpool_stat_get_nodes(mpool, &an, &un);
    mpool_stat_get_bytes(mpool, &ab, &ub);
    me = mpool_stat_get_merges(mpool);
    sp = mpool_stat_get_splits(mpool);

    printf("avail nodes = %-5u\tused nodes = %-5u\tfree(%%) = %.2f\n",
           an, un, 100.0 * an / (an + un));
    printf("avail bytes = %-5u\tused bytes = %-5u\tfree(%%) = %.2f\n",
           ab, ub, 100.0 * ab / (ab + ub));
    printf("splits      = %-5u\tmerges     = %-5u\n",
           sp, me);

    /* Destroy memory pool and free all resources */
    mpool_destroy(mpool);

    return EXIT_SUCCESS;
}
