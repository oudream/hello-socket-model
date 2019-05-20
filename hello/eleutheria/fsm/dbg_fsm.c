#include <assert.h>    /* Arg, ISO C99 only */
#include <stdio.h>
#include <stdlib.h>

#include "fsm.h"
#include "states.h"
#include "types.h"

int main(int argc, char *argv[])
{
    state_t *state1;
    state_t *state2;
    state_t *state3;
    state_t *state4;
    fsm_t *fsm;

    /* Initialize fsm */
    fsm_init(&fsm, 2<<8, 5, 0);

    /* Initialize states */
    assert(state_init(&state1, 2<<5, 2) != ST_NOMEM);
    assert(state_init(&state2, 2<<5, 2) != ST_NOMEM);
    assert(state_init(&state3, 2<<5, 2) != ST_NOMEM);
    assert(state_init(&state4, 2<<5, 2) != ST_NOMEM);

    /* Construct state transition table */
    assert(state_add_evt(state1, 0, "e0", NULL, state1) != ST_NOMEM);
    assert(state_add_evt(state1, 1, "e1", NULL, state2) != ST_NOMEM);
    assert(state_add_evt(state2, 0, "e0", NULL, state2) != ST_NOMEM);
    assert(state_add_evt(state2, 1, "e1", NULL, state1) != ST_NOMEM);
    assert(state_add_evt(state3, 0, "e0", NULL, state4) != ST_NOMEM);

    /* Add states */
    fsm_add_state(fsm, 1, state1);
    fsm_add_state(fsm, 2, state2);
    fsm_add_state(fsm, 3, state3);
    fsm_add_state(fsm, 4, state4);

    /* Set initial state */
    fsm_set_state(fsm, 1);

    /* Scan graph and mark reachable states */
    fsm_mark_reachable_states(fsm);

    /* Print state transition table */
    fsm_print_states(fsm, stdout);

    /* Free memory */
    fsm_free(fsm);

    return EXIT_SUCCESS;
}

