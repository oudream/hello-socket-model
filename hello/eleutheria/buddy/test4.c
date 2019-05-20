/*
 * Compile with:
 * gcc test4.c mpool.c mstat.c -o test4 -Wall -W -Wextra -ansi -pedantic
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>  /* for memset() */
#include <time.h>    /* for time() in srand() */
#include <sys/queue.h>

#include "mpool.h"
#include "mstat.h"

#define MAX_EPOCHS    20000   /* Maximum number of epochs of simulation */
#define MAX_LIFETIME   1000   /* Maximum lifetime of a reserved block */
#define MAX_LOGSIZE      5    /* Maximum logarithm of block's size */
#define TI 5                  /* Every `TI' steps dump statistics */

typedef struct simnode {
    void *ptr;
    unsigned int lifetime;
    LIST_ENTRY(simnode) next_node;
} simnode_t;

LIST_HEAD(simhead, simnode);
typedef struct simhead simhead_t;

/* Function prototypes */
void sim_add_to_list(simhead_t *simhead, simnode_t *simnode);
void sim_free_from_list(mpool_t *mpool, simhead_t *simhead, unsigned int t);
void sim_print_stats(const mpool_t *mpool, unsigned int t, FILE *fp);

int main(void)
{
    simnode_t simnode[MAX_EPOCHS];
    mpool_t *mpool;
    mpret_t mpret;
    simhead_t simhead;
    size_t t, sz, lt;

    /* Initialize memory pool */
    mpret = mpool_init(&mpool, 25, 5);
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

    /* Initialize simlist */
    LIST_INIT(&simhead);

    /* Run simulation */
    for (t = 0; t < MAX_EPOCHS; t++) {
        /* Is it time to dump statistics ? */
        if (t % TI == 0)
            sim_print_stats(mpool, t, stdout);

        /* Free all blocks that lived their life */
        sim_free_from_list(mpool, &simhead, t);

        /*
         * Calculate a random size `sz' and a random lifetime `lt',
         * similar to the way Moirae defined peoples' lives in Greek Mythology.
         * (One could use other distributions than the uniform we use here)
         */
        sz = 1 << rand() % (1 + MAX_LOGSIZE);
        if (t < (MAX_EPOCHS - MAX_LIFETIME))
            lt = 1 + rand() % MAX_LIFETIME;
        else
            lt = 1 + rand() % (MAX_EPOCHS - t);
        /*printf("t = %u\tsz = %u\tlt = %u\n", t, sz, lt);*/

        /* Allocate a block of size `sz' and make it last `lt' time intervals */
        if ((simnode[t].ptr = mpool_alloc(mpool, sz)) == NULL) {
            fprintf(stderr, "mpool: no available block\n");
            mpool_destroy(mpool);
            exit(EXIT_FAILURE);
        }
        simnode[t].lifetime = t + lt;

        /* Add block to list and let it find its correct position in it */
        sim_add_to_list(&simhead, &simnode[t]);
    }

    /* Free the last of Mohicans */
    sim_free_from_list(mpool, &simhead, t);

    /* Dump statistics */
    sim_print_stats(mpool, t, stdout);

    /* Destroy memory pool and free all resources */
    mpool_destroy(mpool);

    return EXIT_SUCCESS;
}

void sim_add_to_list(simhead_t *simhead, simnode_t *simnode)
{
    simnode_t *pnode;

    /*
     * LIST_FOREACH(pnode, simhead, next_node)
     *     printf("%u -> ", pnode->lifetime);
     * printf("\n");
    */

    /* Make sure that we put `simnode' in the right position */
    LIST_FOREACH(pnode, simhead, next_node) {
        if (simnode->lifetime < pnode->lifetime) {
            LIST_INSERT_BEFORE(pnode, simnode, next_node);
            return;
        }
        else if (LIST_NEXT(pnode, next_node) == NULL) {
            LIST_INSERT_AFTER(pnode, simnode, next_node);
            return;
        }
   }

    /*
     * First element goes here.
     * This is called only when the list is empty.
     */
    LIST_INSERT_HEAD(simhead, simnode, next_node);
}

void sim_free_from_list(mpool_t *mpool, simhead_t *simhead, unsigned int t)
{
    simnode_t *pnode;

    /*
     * Blocks with the same lifetime are placed together,
     * e.g. ... -> 5 -> 5 -> 7 -> 7 -> 7 -> 7 -> 8 -> 8 -> 9 -> 9 -> ...
     * That said, if the continuity breaks in one node,
     * we are done and we should return.
     */
    LIST_FOREACH(pnode, simhead, next_node) {
        if (t == pnode->lifetime) {
            /*printf("freeing %u\tptr = %p\n", t, pnode->ptr);*/
            mpool_free(mpool, pnode->ptr);
            LIST_REMOVE(pnode, next_node);
        }
        else
            return;
    }
}

void sim_print_stats(const mpool_t *mpool, unsigned int t, FILE *fp)
{
    size_t an, un;    /* nodes */
    size_t ab, ub;    /* blocks */
    size_t me = 1, sp = 1;    /* merges, splits */
    /*size_t i;*/

    mpool_stat_get_nodes(mpool, &an, &un);
    mpool_stat_get_bytes(mpool, &ab, &ub);
    me = mpool_stat_get_merges(mpool);
    sp = mpool_stat_get_splits(mpool);

    fprintf(fp, "%u\t%u\t%u\t%.2f\t%u\t%u\t%.2f\t%u\t%u\t%.2f\n", t,
            an, un, 100.0 * an / (an + un),
            ab, ub, 100.0 * ab / (ab + ub),
            sp, me, 1.0 * sp/me);

    /* Print length of every block
     * for (i = 0; i < mpool_stat_get_blocks(mpool); i++)
     *     fprintf(fp, "%u\n", mpool_stat_get_block_length(mpool, i));
     */
}
