#ifndef HTABLE_H
#define HTABLE_H

#include <sys/queue.h>
#include <stddef.h>    /* for size_t type */
#include <stdio.h>    /* for FILE */

#define HTABLE_STATS

typedef struct hnode {
    void *hn_key;
    void *hn_data;
    TAILQ_ENTRY(hnode) hn_next;
} hnode_t;

/* Type definitions for use in callback functions */
typedef size_t hashf_t(const void *key);
typedef int cmpf_t(const void *arg1, const void *arg2);
typedef void printf_t(const void *key, const void *data);

typedef struct htable {
    #ifdef HTABLE_STATS
    size_t ht_grows;        /* number of automatic resizes */
    #endif
    size_t ht_size;         /* size must be a power of 2 */
    size_t ht_used;         /* number of hash table entries */
    size_t ht_factor;
    size_t ht_limit;        /* limit = factor * size */
    hashf_t *ht_hashf;      /* pointer to hash function */
    cmpf_t *ht_cmpf;        /* pointer to compare function */
    printf_t *ht_printf;    /* pointer to printf function */
    TAILQ_HEAD(htablehead, hnode) *ht_table;
} htable_t;

typedef struct htablehead hhead_t;

typedef struct htable_iterator {
    size_t pos;
    const hnode_t *pnode;
} htable_iterator_t;

typedef enum {
    HT_OK,
    HT_EXISTS,
    HT_NOMEM,
    HT_NOTFOUND
} htret_t;

typedef enum {
    HT_FREEKEY = 1,
    HT_FREEDATA = 2
} htfree_t;

/* Function prototypes */
htret_t htable_init(htable_t *htable, size_t size, size_t factor,
                    hashf_t *myhashf,
                    cmpf_t *mycmpf,
                    printf_t *myprintf);
void htable_free(htable_t *htable);
htret_t htable_free_obj(htable_t *htable, void *key, htfree_t htfree);
void htable_free_all_obj(htable_t *htable, htfree_t htfree);
htret_t htable_grow(htable_t *htable);
htret_t htable_insert(htable_t *htable, void *key, void *data);
htret_t htable_remove(htable_t *htable, const void *key);
void *htable_search(const htable_t *htable, const void *key);
void htable_print(const htable_t *htable, FILE *fp);
size_t htable_get_size(const htable_t *htable);
size_t htable_get_used(const htable_t *htable);
void htable_traverse(const htable_t *htable, void (*pfunc)(void *data));
void htable_iterator_init(htable_iterator_t *it);
void *htable_iterator_get_data(const htable_iterator_t it);
void *htable_iterator_get_key(const htable_iterator_t it);
const hnode_t *htable_get_next_elm(const htable_t *htable, htable_iterator_t *it);

#ifdef HTABLE_STATS
size_t htable_stat_get_grows(const htable_t *htable);
size_t htable_stat_get_chain_len(const htable_t *htable, size_t pos);
#endif

#endif    /* HTABLE_H */
