#ifndef STATES_H
#define STATES_H

#include "types.h"

/* Function prototypes */
stret_t state_init(state_t **ppstate, size_t size, unsigned int factor);
stret_t state_add_evt(state_t *pstate, unsigned int key, const char *pdesc,
                      void (*pactionf)(void *pdata), state_t *pnewstate);
stret_t state_rem_evt(state_t *pstate, unsigned int key);
unsigned int state_get_key(state_t *pstate);
stret_t state_free(state_t *pstate);
void state_print_evts(const state_t *pstate, FILE *fp);

#endif    /* STATES_H */
