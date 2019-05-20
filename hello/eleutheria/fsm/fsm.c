#include "fsm.h"

#include "states.h"
#include "types.h"

#include <sys/queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

/* Callback funtions prototypes */
static unsigned int fsm_hashf(const void *key);
static int fsm_cmpf(const void *arg1, const void *arg2);
static void fsm_printf(const void *key, const void *data);
static void fsm_pq_lock(const fsm_t *fsm);
static void fsm_pq_unlock(const fsm_t *fsm);

fsmret_t fsm_init(fsm_t **ppfsm, size_t size, unsigned int factor,
                  unsigned int nqueues)
{
    unsigned int i;

    /* FIXME: Validate input  */

    /* Allocate memory for fsm data structure */
    if ((*ppfsm = malloc(sizeof **ppfsm)) == NULL)
        return FSM_ENOMEM;

    /* Allocate memory for fsm's states' hash table */
    if (((*ppfsm)->sttable = malloc(sizeof *(*ppfsm)->sttable)) == NULL) {
        free(*ppfsm);
        return FSM_ENOMEM;
    }

    /* Allocate memory for priority queues */
    if (((*ppfsm)->pqtable = malloc(nqueues * sizeof *(*ppfsm)->pqtable)) == NULL) {
        free((*ppfsm)->sttable);
        free(*ppfsm);
        return FSM_ENOMEM;
    }

    /* Allocate memory for "mutex object" -- machine dependent code */
    if (((*ppfsm)->mobj = malloc(sizeof(pthread_mutex_t))) == NULL) {
        free((*ppfsm)->mobj);
        free((*ppfsm)->sttable);
        free(*ppfsm);
        return FSM_ENOMEM;
    }
    /* Machine dependent code */
    pthread_mutex_init((pthread_mutex_t *)(*ppfsm)->mobj, NULL);

    /* Initialize queues */
    (*ppfsm)->nqueues = nqueues;
    for (i = 0; i < nqueues; i++)
        STAILQ_INIT(&(*ppfsm)->pqtable[i]);

    /* Initialize states' hash table */
    if (htable_init((*ppfsm)->sttable, size, factor,
                    fsm_hashf, fsm_cmpf, fsm_printf) == HT_NOMEM) {
        free((*ppfsm)->pqtable);
        free((*ppfsm)->sttable);
        free(*ppfsm);
        return FSM_ENOMEM;
    }

    return FSM_OK;
}

fsmret_t fsm_add_state(fsm_t *pfsm, unsigned int key, state_t *pstate)
{
    /*
     * There is no need to allocate memory for state's key,
     * since this is done in state_init().
     */
    *pstate->st_key = key;

    /* Insert state to hash table */
    if (htable_insert(pfsm->sttable, pstate->st_key, pstate) == HT_EXISTS)
        return FSM_EEXISTS;

    return FSM_OK;
}

fsmret_t fsm_free(fsm_t *pfsm)
{
    pqhead_t *phead;
    pqnode_t *pnode;
    htable_iterator_t sit;    /* states iterator */
    unsigned int i;

    /* Free states' table */
    htable_iterator_init(&sit);
    while ((sit.pnode = htable_get_next_elm(pfsm->sttable, &sit)) != NULL)
        state_free(htable_iterator_get_data(sit));

    /* Shallow free */
    htable_free_all_obj(pfsm->sttable, HT_FREEKEY);
    htable_free(pfsm->sttable);
    free(pfsm->sttable);

    /* Free queues' elements */
    for (i = 0; i < pfsm->nqueues; i++) {
        phead = &pfsm->pqtable[i];
        while (STAILQ_FIRST(phead) != NULL) {
            pnode = STAILQ_FIRST(phead);
            STAILQ_REMOVE_HEAD(phead, pq_next);
            free(pnode->data);
            free(pnode);
        }
    }

    free(pfsm->pqtable);
    free(pfsm->mobj);
    free(pfsm);

    return FSM_OK;
}

fsmret_t fsm_set_state(fsm_t *pfsm, unsigned int stkey)
{
    state_t *pstate;

    /* Does this state exist in states' hash table ? */
    if ((pstate = htable_search(pfsm->sttable, &stkey)) == NULL)
        return FSM_ENOTFOUND;

    /*
     * Mark state as reachable
     *
     * By calling fsm_mark_reachable_states() we guarantee that
     * states' flags are always in accordance with reality.
     * Also, we use fsm_set_state() to set the _initial_ state,
     * thus rendering it "explicitly reachable".
     */
    STATE_MARK_AS_REACHABLE(pstate);
    fsm_mark_reachable_states(pfsm);

    /* Set fsm to new state */
    pfsm->cstate = pstate;

    return FSM_OK;
}

unsigned int fsm_get_current_state(const fsm_t *pfsm)
{
    return *pfsm->cstate->st_key;
}

fsmret_t fsm_queue_event(fsm_t *pfsm, unsigned int evtkey,
                         void *pdata, size_t size, unsigned int prio)
{
    pqhead_t *phead;
    pqnode_t *pnode;

    /* Validate input */
    if (prio >= pfsm->nqueues)
        return FSM_EPRIO;

    /* Allocate memory for new pending event */
    if ((pnode = malloc(sizeof *pnode)) == NULL)
        return FSM_ENOMEM;

    pnode->evtkey = evtkey;
    pnode->prio = prio;

    /*
     * Allocate memory for data and copy them over.
     * Note that this strategy leads to memory fragmentation,
     * and should be addressed with a custom memory allocator,
     * in due time.
    */
    if ((pnode->data = malloc(size)) == NULL) {
        free(pnode);
        return FSM_ENOMEM;
    }
    memcpy(pnode->data, pdata, size);

    /* Get the head of the queue with the appropriate priority */
    phead = &pfsm->pqtable[prio];

    /* Insert new event in tail (we serve from head) */
    fsm_pq_lock(pfsm);
    STAILQ_INSERT_TAIL(phead, pnode, pq_next);
    fsm_pq_unlock(pfsm);

    return FSM_OK;
}
fsmret_t fsm_dequeue_event(fsm_t *pfsm)
{
    pqhead_t *phead;
    pqnode_t *pnode;
    unsigned int i;

    /* Scan queues starting from the one with the biggest priority */
    i = pfsm->nqueues - 1;
    do {
        phead = &pfsm->pqtable[i];
        if ((pnode = STAILQ_FIRST(phead)) != NULL) {
            if (fsm_process_event(pfsm, pnode->evtkey, pnode->data) == FSM_ENOTFOUND) {
                /*
                 * XXX: Should the event should stay in queue, waiting for fsm
                 * to go into a state that can handle it ? We haven't though
                 * implemented such a sticky bit in event's structure yet.
                 */
            }

            /* Delete event */
            pnode = STAILQ_FIRST(phead);
            STAILQ_REMOVE_HEAD(phead, pq_next);
            free(pnode->data);    /* Argh, memory fragmentation */
            free(pnode);
            return FSM_OK;
        }
    } while (i-- != 0);

    return FSM_EMPTY;
}

size_t fsm_get_queued_events(const fsm_t *pfsm)
{
    const pqhead_t *phead;
    const pqnode_t *pnode;
    size_t i, total;

    total = 0;
    fsm_pq_lock(pfsm);
    for (i = 0; i < pfsm->nqueues; i++) {
        phead = &pfsm->pqtable[i];
        STAILQ_FOREACH(pnode, phead, pq_next)
            total++;
    }
    fsm_pq_unlock(pfsm);

    return total;
}

fsmret_t fsm_process_event(fsm_t *pfsm, unsigned int evtkey, void *data)
{
    event_t *pevt;

    /* Can the current state handle the incoming event ? */
    if ((pevt = htable_search(pfsm->cstate->evttable, &evtkey)) == NULL)
        return FSM_ENOTFOUND;

    /* Execute appropriate action */
    if (pevt->evt_actionf != NULL)
        pevt->evt_actionf(data);

    /* Is the transition made to an existent state ? */
    if ((pevt->evt_newstate == NULL)
        || (htable_search(pfsm->sttable, pevt->evt_newstate->st_key) == NULL))
        return FSM_ENOTFOUND;

    /* Set new state */
    pfsm->cstate = pevt->evt_newstate;

    return FSM_OK;
}

fsmret_t fsm_validate(const fsm_t *pfsm)
{
    /* Is FSM empty of states ? */
    if (htable_get_used(pfsm->sttable) == 0)
        return FSM_EMPTY;

    return FSM_CLEAN;
}

void fsm_export_to_dot(const fsm_t *pfsm, FILE *fp)
{
    const state_t *pstate;
    const event_t *pevt;
    htable_iterator_t sit;    /* state iterator */
    htable_iterator_t eit;    /* events iterator */

    fprintf(fp, "digraph {\n");

    /* Traverse all states of FSM */
    htable_iterator_init(&sit);
    while ((sit.pnode = htable_get_next_elm(pfsm->sttable, &sit)) != NULL) {
        /* Traverse all events associated with the current state */
        pstate = htable_iterator_get_data(sit);
        htable_iterator_init(&eit);
        while ((eit.pnode = htable_get_next_elm(pstate->evttable, &eit)) != NULL) {
            pevt = htable_iterator_get_data(eit);
            printf("S%u -> S%u [label=\"E%u\"]\n",
                   *(unsigned int *) htable_iterator_get_key(sit),
                   *(unsigned int *) (pevt->evt_newstate->st_key),
                   *(unsigned int *) htable_iterator_get_key(eit));
        }
    }

    fprintf(fp, "}\n");
}

void fsm_print_states(const fsm_t *pfsm, FILE *fp)
{
    const state_t *pstate;
    htable_iterator_t sit;    /* states iterator */

    /* Traverse all states of FSM */
    htable_iterator_init(&sit);
    while ((sit.pnode = htable_get_next_elm(pfsm->sttable, &sit)) != NULL) {
        pstate = htable_iterator_get_data(sit);
        fprintf(fp, "state [key = %u, reachable = %c]\n",
                *(unsigned int *) (pstate->st_key),
                STATE_IS_REACHABLE(pstate) ? 'T' : 'F');
        state_print_evts(pstate, fp);
    }
}

void fsm_mark_reachable_states(fsm_t *pfsm)
{
    const state_t *pstate;
    const event_t *pevt;
    htable_iterator_t sit;    /* states iterator */
    htable_iterator_t eit;    /* events iterator */

    /* For all states */
    htable_iterator_init(&sit);
    while ((sit.pnode = htable_get_next_elm(pfsm->sttable, &sit)) != NULL) {
        pstate = htable_iterator_get_data(sit);
        htable_iterator_init(&eit);

        /*
         * We mark a state as reachable, if and only if there exist transitions
         * to this state from other _reachable_ states.
         */
        while ((eit.pnode = htable_get_next_elm(pstate->evttable, &eit)) != NULL) {
            pevt = htable_iterator_get_data(eit);
            if (STATE_IS_REACHABLE(pstate))
                STATE_MARK_AS_REACHABLE(pevt->evt_newstate);
        }
    }
}

void fsm_remove_unreachable_state(fsm_t *pfsm, const state_t *pstate)
{
}

void fsm_minimize(fsm_t *pfsm)
{
    const state_t *pstate;
    htable_iterator_t sit;    /* states iterator */

    /* Remove unreachable states */
    htable_iterator_init(&sit);
    while ((sit.pnode = htable_get_next_elm(pfsm->sttable, &sit)) != NULL) {
        pstate = htable_iterator_get_data(sit);

        if (!STATE_IS_REACHABLE(pstate))
            fsm_remove_unreachable_state(pfsm, pstate);
    }
}

/* Callback funtions */
static unsigned int fsm_hashf(const void *pkey)
{
    return *(const unsigned int *) pkey;
}

static int fsm_cmpf(const void *parg1, const void *parg2)
{
    unsigned int a = *(const unsigned int *) parg1;
    unsigned int b = *(const unsigned int *) parg2;

    if (a > b)
        return -1;
    else if (a == b)
        return 0;
    else
        return 1;
}

static void fsm_printf(const void *pkey, const void *pdata)
{
    printf("key: %u ", *(const unsigned int *) pkey);
}

static void fsm_pq_lock(const fsm_t *pfsm)
{
    /* Machine dependent code */
    pthread_mutex_lock((pthread_mutex_t *) pfsm->mobj);
}

static void fsm_pq_unlock(const fsm_t *pfsm)
{
    /* Machine dependent code */
    pthread_mutex_unlock((pthread_mutex_t *) pfsm->mobj);
}

