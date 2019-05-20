/*
 * Compile with:
 * gcc pthread_join.c -o pthread_join -lpthread -Wall -W -Wextra -ansi -pedantic
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 20

/* function prototypes */
void *threadfun(void *arg);
void diep(const char *s);

int main(void)
{
    pthread_t tid;
    int cnt[NUM_THREADS], i;

    for (i = 0; i < NUM_THREADS; i++) {
        cnt[i] = i;
        printf("Creating thread: %d\n", i);

      if (pthread_create(&tid, NULL, threadfun, (void *)&cnt[i]))
          diep("pthread_create");

      if (pthread_join(tid, NULL))
          diep("pthread_join()");
    }

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
