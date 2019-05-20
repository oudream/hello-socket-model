#include <stdio.h>
#include <stdlib.h>
#include <time.h>    /* for time() in srand() */

#include "fsm.h"
#include "states.h"
#include "types.h"

#define NSTATES 20
#define NEVENTS 20
#define NEPOCHS 100000

/* Function prototypes */
void foo1(void *data);
void foo2(void *data);

void foo1(void *data)
{
}

void foo2(void *data)
{
}

int main(int argc, char *argv[])
{
    state_t *state[NSTATES];
    void (*pf[])(void *) = { foo1, foo2, NULL };
    fsm_t *fsm;
    unsigned int i, j, k;

    /* Initialize fsm */
    printf("Initializing fsm\n");
    fsm_init(&fsm, 2<<11, 5, 0);

    /* Initialize random number generator */
    srand(time(NULL));

    /* Initialize states */
    printf("Initializing states (%u)\n", NSTATES);
    for (i = 0; i < NSTATES; i++) {
        if (state_init(&state[i], 2<<12, 2) == ST_NOMEM) {
            fprintf(stderr, "error: state_init(): ST_NOMEM\n");
            for (j = 0; j < i; j++)
                state_free(state[j]);
            fsm_free(fsm);
            exit(EXIT_FAILURE);
        }
    }

    /* Populate states with events they can handle */
    printf("Populating states with events (events/state = %u)\n", NEVENTS);
    for (i = 0; i < NSTATES; i++) {
        for (j = 0; j < NEVENTS; j++) {
            if (state_add_evt(state[i], j, "", pf[rand() % 3], state[i]) == ST_NOMEM) {
                fprintf(stderr, "error: state_add_evt(): ST_NOMEM\n");
                for (k = 0; k < i; k++)
                    state_free(state[k]);
                fsm_free(fsm);
                exit(EXIT_FAILURE);
            }
        }
    }

    /* Add states to fsm */
    printf("Adding states to fsm (%u)\n", NSTATES);
    for (i = 0; i < NSTATES; i++)
        fsm_add_state(fsm, i, state[i]);

    /* Set initial state */
    fsm_set_state(fsm, 0);

    /* Process events */
    printf("Simulating (events = %u)\n", NEPOCHS);
    for (i = 0; i < NEPOCHS; i++)
        fsm_process_event(fsm, rand() % NEVENTS, NULL);

    /* Free memory */
    printf("Destroying FSM\n");
    fsm_free(fsm);

    return EXIT_SUCCESS;
}

