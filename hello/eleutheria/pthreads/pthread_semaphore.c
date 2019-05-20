/*
 * Compile with:
 * gcc pthread_semaphore.c -o pthread_semaphore -lpthread -Wall -W -Wextra -ansi -pedantic
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_THREADS 5

sem_t mutex;
int myglobal = 0;		/* global shared variable */

/* Function prototypes */
void *threadfun(void *arg);
void diep(const char *s);

int main(void)
{
    pthread_t tid[NUM_THREADS];
    int i;

    /* Initialize the semaphore */
    if (sem_init(&mutex, 0, 1))
        diep("sem_init");

    /* Create the threads */
    for (i = 0; i < NUM_THREADS; i++)
        if (pthread_create(&tid[i], NULL, threadfun, NULL))
            diep("pthread_create");

    /* Make sure all threads are done */
    for (i = 0; i < NUM_THREADS; i++)
        if (pthread_join(tid[i], NULL))
            diep("pthread_join");

    printf("myglobal = %d\n", myglobal);

    return EXIT_SUCCESS;
}

void *threadfun(void *arg)
{
    int i, j;

    for (i = 0; i < 5; i++) {
        sem_wait(&mutex);		/* begin critical region */
        j = myglobal;
        j++;
        sleep(1);
        myglobal = j;
        sem_post(&mutex);		/* end critical region */
    }

    pthread_exit(NULL);
}

void diep(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}
