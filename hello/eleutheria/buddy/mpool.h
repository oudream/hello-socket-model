#ifndef MPOOL_H_
#define MPOOL_H_

#include <stdlib.h>
#include <sys/queue.h>

/*#define MPOOL_DEBUG*/
/*#define MPOOL_OPT_FOR_SECURITY*/
#define MPOOL_STATS

#ifdef MPOOL_DEBUG
#define DPRINTF(a) printf a
#else
#define DPRINTF(a)
#endif

/* `flags' bits in blknode data structure */
#define MPOOL_NODE_AVAIL  (1 << 0)    /* If not set, node is reserved, else available */
#define MPOOL_NODE_LR     (1 << 1)    /* If not set, node is left buddy, else right buddy */
#define MPOOL_NODE_PARENT (1 << 2)    /* If not set, parent is left buddy, else right buddy */

#define MPOOL_BLOCK_USED   0    /* Block is used */
#define MPOOL_BLOCK_AVAIL  1    /* Block is available */
#define MPOOL_BLOCK_LEFT   2    /* Block is left buddy */
#define MPOOL_BLOCK_RIGHT  3    /* Block is right buddy */
#define MPOOL_BLOCK_PARENT 4    /* Block's parent is right buddy */

/* Macro definitions */
#define MPOOL_MARK_AVAIL(pnode) pnode->flags |= MPOOL_NODE_AVAIL
#define MPOOL_MARK_USED(pnode) pnode->flags &= ~MPOOL_NODE_AVAIL
#define MPOOL_MARK_LEFT(pnode) pnode->flags &= ~MPOOL_NODE_LR
#define MPOOL_MARK_RIGHT(pnode) pnode->flags |= MPOOL_NODE_LR
#define MPOOL_MARK_PARENT(pnode) pnode->flags |= MPOOL_NODE_PARENT
#define MPOOL_MARK_NOTPARENT(pnode) pnode->flags &= ~MPOOL_NODE_PARENT

#define MPOOL_IS_AVAIL(pnode) ((pnode->flags & MPOOL_NODE_AVAIL) != 0)
#define MPOOL_IS_USED(pnode)  ((pnode->flags & MPOOL_NODE_AVAIL) == 0)
#define MPOOL_IS_LEFT(pnode)  ((pnode->flags & MPOOL_NODE_LR) == 0)
#define MPOOL_IS_RIGHT(pnode) ((pnode->flags & MPOOL_NODE_LR) != 0)
#define MPOOL_IS_PARENT(pnode) ((pnode->flags & MPOOL_NODE_PARENT) != 0)

#define MPOOL_GET_LEFT_BUDDY_OF(pnode) \
    ((blknode_t *)((char *)pnode - (1 << pnode->logsize)))
#define MPOOL_GET_RIGHT_BUDDY_OF(pnode) \
    ((blknode_t *)((char *)pnode + (1 << pnode->logsize)))

/* This macro is provided for easy initialization of a blknode structure */
#define MPOOL_BLOCK_INIT(_node, _base, _ptr, _avail, _lr, _parent, _logsize) \
    do {                                                                \
        _node = _base;                                                  \
        _node->ptr = _ptr;                                              \
        _node->logsize = _logsize;                                      \
                                                                        \
        /* Availability */                                              \
        if ((_avail) == MPOOL_BLOCK_AVAIL)                              \
            MPOOL_MARK_AVAIL(_node);                                    \
        else                                                            \
            MPOOL_MARK_USED(_node);                                     \
                                                                        \
        /* Left-Right relationship */                                   \
        if ((_lr) == MPOOL_BLOCK_RIGHT)                                 \
            MPOOL_MARK_RIGHT(_node);                                    \
        else                                                            \
            MPOOL_MARK_LEFT(_node);                                     \
                                                                        \
        /* Parent L-R relationship */                                   \
        if ((_parent) == MPOOL_BLOCK_PARENT)                            \
            MPOOL_MARK_PARENT(_node);                                   \
        else                                                            \
            MPOOL_MARK_NOTPARENT(_node);                                \
    } while(0)

typedef struct blknode {
    unsigned char flags;    /* availability, left-right buddiness, parentship */
    size_t logsize;         /* logarithm of size with base 2 */
    void *ptr;              /* what mpool_alloc() returns */
    LIST_ENTRY(blknode) next_chunk;
} blknode_t;

typedef struct mpool {
    void *mem;
    size_t nblocks;       /* nblocks = maxlogsize - minlogsize + 1 */
    size_t maxlogsize;    /* logarithm of maximum size of chunk with base 2 */
    size_t minlogsize;    /* logarithm of minimum size of chunk with base 2 */
#ifdef MPOOL_STATS
    size_t nsplits;       /* number of splits made */
    size_t nmerges;       /* number of merges made */
#endif
    LIST_HEAD(blkhead, blknode) *blktable;
} mpool_t;

typedef struct blkhead blkhead_t;

typedef enum {
    MPOOL_OK,
    MPOOL_EBADVAL,
    MPOOL_ENOMEM,
    MPOOL_ERANGE
} mpret_t;

/* Function prototypes */
mpret_t mpool_init(mpool_t **mpool, size_t maxlogsize, size_t minlogsize);
void *mpool_alloc(mpool_t *mpool, size_t blksize);
void mpool_free(mpool_t *mpool, void *ptr);
void mpool_destroy(mpool_t *mpool);

#endif    /* MPOOL_H_ */
