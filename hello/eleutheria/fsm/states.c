#include "types.h"
#include "states.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Callback functions prototypes */
static unsigned int state_hashf(const void *pkey);
static int state_cmpf(const void *parg1, const void *parg2);
static void state_printf(const void *pkey, const void *pdata);

stret_t state_init(state_t **ppstate, size_t size, unsigned int factor)
{
    /* Allocate memory state's event table */
    if ((*ppstate = malloc(sizeof **ppstate)) == NULL)
        return ST_NOMEM;

    if (((*ppstate)->evttable = malloc(sizeof *(*ppstate)->evttable)) == NULL) {
        free(*ppstate);
        return ST_NOMEM;
    }

    /* Initialize hash table that stores the events the state can process */
    if (htable_init((*ppstate)->evttable, size, factor,
                    state_hashf, state_cmpf, state_printf) == HT_NOMEM) {
        free((*ppstate)->evttable);
        free(*ppstate);
        return ST_NOMEM;
    }

    /* Allocate memory for state's key */
    if (((*ppstate)->st_key = malloc(sizeof *(*ppstate)->st_key)) == NULL) {
        free((*ppstate)->evttable);
        free(*ppstate);
        return ST_NOMEM;
    }

    /* Initialize flags */
    STATE_MARK_AS_UNREACHABLE(*ppstate);

    return ST_OK;
}

stret_t state_add_evt(state_t *pstate, unsigned int key, const char *pdesc,
                      void (*pactionf)(void *pdata), state_t *pnewstate)
{
    event_t *pevt;
    unsigned int *pkey;

    /* Allocate memory for new key */
    if ((pkey = malloc(sizeof *pkey)) == NULL)
        return ST_NOMEM;

    *pkey = key;

    /* Allocate memory for new event */
    if ((pevt = malloc(sizeof *pevt)) == NULL) {
        free(pkey);
        return ST_NOMEM;
    }

    /* Fill in structure's members */
    strncpy(pevt->evt_desc, pdesc, MAX_EVT_DESC);
    pevt->evt_actionf = pactionf;
    pevt->evt_newstate = pnewstate;

    /* Insert event to hash table */
    if (htable_insert(pstate->evttable, pkey, pevt) == HT_EXISTS) {
        free(pkey);
        free(pevt);
        return ST_EXISTS;
    }

    return ST_OK;
}

stret_t state_rem_evt(state_t *pstate, unsigned int key)
{
    if (htable_free_obj(pstate->evttable, &key, HT_FREEKEY | HT_FREEDATA) == HT_NOTFOUND)
        return ST_NOTFOUND;

    return ST_OK;
}

unsigned int state_get_key(state_t *pstate)
{
    return *pstate->st_key;
}

stret_t state_free(state_t *pstate)
{
    htable_free_all_obj(pstate->evttable, HT_FREEKEY | HT_FREEDATA);
    htable_free(pstate->evttable);
    free(pstate->evttable);
    free(pstate);

    return ST_OK;
}

void state_print_evts(const state_t *pstate, FILE *fp)
{
    htable_print(pstate->evttable, fp);
}

/* Callback funtions */
static unsigned int state_hashf(const void *pkey)
{
    return *(const unsigned int *) pkey;
}

static int state_cmpf(const void *parg1, const void *parg2)
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

static void state_printf(const void *pkey, const void *pdata)
{
    const event_t *pevt;

    pevt = (const event_t *) pdata;
    printf("evtkey: %u\tdesc: %s\tnewstate: %u",
           *(const unsigned int *) pkey,
           pevt->evt_desc,
           *(const unsigned int *) pevt->evt_newstate->st_key);
}
