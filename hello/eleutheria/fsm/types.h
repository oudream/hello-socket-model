#ifndef TYPES_H
#define TYPES_H

#include "../genstructs/htable/htable.h"

#define MAX_EVT_DESC 64

typedef struct event {
    char evt_desc[MAX_EVT_DESC];
    void (*evt_actionf)(void *data);
    struct state *evt_newstate;
} event_t;

typedef struct state {
    htable_t *evttable;
    unsigned int *st_key;
    unsigned char flag;
} state_t;

#define STATE_REACHABLE (1 << 0)
#define STATE_IS_REACHABLE(pstate) ((pstate)->flag & STATE_REACHABLE)
#define STATE_MARK_AS_REACHABLE(pstate) (pstate)->flag |= STATE_REACHABLE;
#define STATE_MARK_AS_UNREACHABLE(pstate) (pstate)->flag &= ~STATE_REACHABLE;

typedef enum {
    ST_OK,
    ST_EXISTS,
    ST_NOMEM,
    ST_NOTFOUND
} stret_t;

typedef struct pqnode {
    void *data;
    unsigned int evtkey;
    unsigned int prio;
    STAILQ_ENTRY(pqnode) pq_next;
} pqnode_t;

typedef struct fsm {
    htable_t *sttable;       /* hash table for states */
    state_t *cstate;         /* current state of fsm  */
    unsigned int nqueues;    /* number of priority queues */
    void *mobj;              /* mutual exclusion object */
    void (*fsm_pq_lock)(const struct fsm *);
    void (*fsm_pq_unlock)(const struct fsm *);
    STAILQ_HEAD(pqhead, pqnode) *pqtable;
} fsm_t;

typedef struct pqhead pqhead_t;

typedef enum {
    FSM_CLEAN,
    FSM_DIRTY,
    FSM_EMPTY,
    FSM_EPRIO,
    FSM_EEXISTS,
    FSM_ENOMEM,
    FSM_ENOTFOUND,
    FSM_OK
} fsmret_t;

#endif    /* TYPES_H */
