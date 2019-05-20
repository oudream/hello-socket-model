/*
 * Compile with:
 * gcc fork.c -o fork -Wall -W -Wextra -ansi -pedantic
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    pid_t pid;

    if ((pid = fork()) < 0) {  /* fork error */
        perror("fork()");
        exit(EXIT_FAILURE);
    }

    else if (pid > 0)          /* parent process */
        printf("Parent's pid: %d\n", getpid());

    else                       /* child process (pid = 0) */
        printf("Child's pid: %d\n", getpid());

    /*
     * Whatever goes here, will be executed from the
     * parent AND the child process as well.
     */

    return EXIT_SUCCESS;
}
