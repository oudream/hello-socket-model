#include "mpool.h"
#include "mstat.h"

void mpool_stat_get_nodes(const mpool_t *mpool, size_t *avail, size_t *used)
{
    const blkhead_t *phead;
    const blknode_t *pnode;
    size_t i;

    *avail = 0;
    *used = 0;
    for (i = 0; i < mpool->nblocks; i++) {
        phead = &mpool->blktable[i];
        LIST_FOREACH(pnode, phead, next_chunk) {
            if (MPOOL_IS_AVAIL(pnode))
                (*avail)++;
            else
                (*used)++;
        }
    }
}

void mpool_stat_get_bytes(const mpool_t *mpool, size_t *avail, size_t *used)
{
    const blkhead_t *phead;
    const blknode_t *pnode;
    size_t i;

    *avail = 0;
    *used = 0;
    for (i = 0; i < mpool->nblocks; i++) {
        phead = &mpool->blktable[i];
        LIST_FOREACH(pnode, phead, next_chunk) {
            if (MPOOL_IS_AVAIL(pnode))
                *avail += 1 << pnode->logsize;
            else
                *used += 1 << pnode->logsize;
        }
    }
}

size_t mpool_stat_get_blocks(const mpool_t *mpool)
{
    return mpool->nblocks;
}

size_t mpool_stat_get_block_length(const mpool_t *mpool, size_t pos)
{
    const blknode_t *pnode;
    size_t length;

    if (pos >= mpool->nblocks)
        return 0;    /* FIXME: Better error handling */

    length = 0;
    LIST_FOREACH(pnode, &mpool->blktable[pos], next_chunk)
        length++;

    return length;
}

#ifdef MPOOL_STATS
size_t mpool_stat_get_splits(const mpool_t *mpool)
{
    return mpool->nsplits;
}

size_t mpool_stat_get_merges(const mpool_t *mpool)
{
    return mpool->nmerges;
}
#endif

