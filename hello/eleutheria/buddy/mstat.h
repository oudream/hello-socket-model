#include "mpool.h"

void mpool_stat_get_nodes(const mpool_t *mpool, size_t *avail, size_t *used);
void mpool_stat_get_bytes(const mpool_t *mpool, size_t *avail, size_t *used);
size_t mpool_stat_get_blocks(const mpool_t *mpool);
size_t mpool_stat_get_block_length(const mpool_t *mpool, size_t pos);
#ifdef MPOOL_STATS
size_t mpool_stat_get_splits(const mpool_t *mpool);
size_t mpool_stat_get_merges(const mpool_t *mpool);
#endif

