#ifndef FSM_H
#define FSM_H

#include "types.h"
#include <stdio.h>    /* for FILE type */

/* Function prototypes */
fsmret_t fsm_init(fsm_t **ppfsm, size_t size, unsigned int factor,
                  unsigned int nqueues);
fsmret_t fsm_add_state(fsm_t *pfsm, unsigned int key, state_t *pstate);
fsmret_t fsm_free(fsm_t *pfsm);
fsmret_t fsm_set_state(fsm_t *pfsm, unsigned int stkey);
unsigned int fsm_get_current_state(const fsm_t *pfsm);
fsmret_t fsm_queue_event(fsm_t *pfsm, unsigned int evtkey,
                         void *pdata, size_t size, unsigned int prio);
fsmret_t fsm_dequeue_event(fsm_t *pfsm);
size_t fsm_get_queued_events(const fsm_t *pfsm);
fsmret_t fsm_process_event(fsm_t *pfsm, unsigned int evtkey, void *pdata);
fsmret_t fsm_validate(const fsm_t *pfsm);
void fsm_export_to_dot(const fsm_t *pfsm, FILE *fp);
void fsm_print_states(const fsm_t *pfsm, FILE *fp);
void fsm_mark_reachable_states(fsm_t *pfsm);
void fsm_remove_unreachable_state(fsm_t *pfsm, const state_t *pstate);
void fsm_minimize(fsm_t *pfsm);
#endif    /* FSM_H */
