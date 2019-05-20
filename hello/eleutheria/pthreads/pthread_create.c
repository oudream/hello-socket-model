/*
 * Compile with:
 * gcc pthread_create.c -o pthread_create -lpthread -Wall -W -Wextra -ansi -pedantic
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 20

/* Function prototypes */
void *threadfun(void *arg);
void diep(const char *s);

int main(void)
{
    pthread_t tid[NUM_THREADS];
    int cnt[NUM_THREADS], i;

    for (i = 0; i < NUM_THREADS; i++) {
        cnt[i] = i;
        printf("Creating thread: %d\n", i);
        if (pthread_create(&tid[i], NULL, threadfun, (void *)&cnt[i]))
            diep("pthread_create() error\n");
    }

    /* Make sure all threads are done */
    for (i = 0; i < NUM_THREADS; i++)
        if (pthread_join(tid[i], NULL))
            diep("pthread_join");

    return EXIT_SUCCESS;
}

void *threadfun(void *arg)
{
    printf("Hello! I am thread: %d\n", *(int *) arg);
    pthread_exit(NULL);
}

void diep(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}
