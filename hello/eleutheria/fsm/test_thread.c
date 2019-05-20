#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "fsm.h"
#include "states.h"
#include "types.h"

#define NUM_THREADS 3

/* Function prototypes */
void *thread_alpha_gen(void *arg);
void *thread_num_gen(void *arg);
void *thread_consumer(void *arg);
void dief(const char *s);
void diep(const char *s);
void print_char(void *data);

struct mythread {
    void *(*th_pfunc)(void *);
    pthread_t th_id;
} thtbl[] = {
    { thread_alpha_gen, 0 },
    { thread_num_gen, 0 },
    { thread_consumer, 0 }
};

int main(void)
{
    fsm_t *fsm;
    state_t *steadystate;
    int c, i;

    /* Initialize fsm */
    fsm_init(&fsm, 2<<10, 5, 2);

    /* Initialize state */
    if (state_init(&steadystate, 2<<10, 2) == ST_NOMEM) {
        fsm_free(fsm);
        dief("state_init(): ST_NOMEM");
    }

    /* Add events to state
     *
     * We have only one state named "steady state",
     * which handles all events from 'A' to 'Z'
     * and '0' to '9'.
     */
    for (c = 'A'; c < 'Z'; c++) {
        if (state_add_evt(steadystate, c, "", print_char, steadystate) == ST_NOMEM) {
            dief("state_add_evt(): ST_NOMEM");
            fsm_free(fsm);
        }
    }
    for (c = '0'; c < '9'; c++) {
        if (state_add_evt(steadystate, c, "", print_char, steadystate) == ST_NOMEM) {
            dief("state_add_evt(): ST_NOMEM");
            fsm_free(fsm);
        }
    }

    /* Add steady state to fsm */
    fsm_add_state(fsm, 0, steadystate);

    /* Set initial state */
    fsm_set_state(fsm, 0);

    /* Create threads (alpha generator, num generator and consumer) */
    for (i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&thtbl[i].th_id, NULL, thtbl[i].th_pfunc, (void *)fsm))
            diep("pthread_create");
    }

    /* Loop forever */
    for (;;)
        ;

    /* Free memory */
    fsm_free(fsm);

    return EXIT_SUCCESS;
}

void *thread_alpha_gen(void *arg)
{
    fsm_t *fsm;
    size_t c;

    /* Get a pointer to the fsm we are interested in */
    fsm = (fsm_t *)arg;

    /* Initialize random number generator */
    srand(time(NULL));

    /* Broadcast events */
    for (;;) {
        c = 'A' + rand() % 26;    /* from 'A' to 'Z' */
        fsm_queue_event(fsm, c, &c, 1, 0);
    }

    pthread_exit(NULL);
}

void *thread_num_gen(void *arg)
{
    fsm_t *fsm;
    size_t c;

    /* Get a pointer to the fsm we are interested in */
    fsm = (fsm_t *)arg;

    /* Initialize random number generator */
    srand(time(NULL));

    /* Broadcast events */
    for (;;) {
        c = '0' + rand() % 10;    /* from '0' to '9' */
        fsm_queue_event(fsm, c, &c, 1, 0);
    }

    pthread_exit(NULL);
}

void *thread_consumer(void *arg)
{
    fsm_t *fsm;

    /* Get a pointer to the fsm we are interested in */
    fsm = (fsm_t *)arg;

    /* Thread acts as event consumer */
    for (;;)
        fsm_dequeue_event(fsm);

    pthread_exit(NULL);
}

void dief(const char *s)
{
    fprintf(stderr, "error: %s\n", s);
    exit(EXIT_FAILURE);
}

void diep(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}

void print_char(void *data)
{
    printf("%c", *(char *)data);
}
