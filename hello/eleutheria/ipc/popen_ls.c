#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>    /* for waitpid() macros */

#define MAX_STR 100

int main(void)
{
    char str[MAX_STR];
    int ret;
    FILE *fp;

    /* Initiate pipe stream to ls(1) */
    fp = popen("ls", "r");
    if (fp == NULL) {
        perror("popen()");
        exit(EXIT_FAILURE);
    }

    /* Read from stream */
    while (fgets(str, MAX_STR, fp) != NULL)
        printf("%s", str);

    /* Close pipe stream */
    ret = pclose(fp);
    if  (ret == -1) {
        perror("pclose()");
        exit(EXIT_FAILURE);
    }
    else {
        /*
         * Use macros described under wait() to inspect the return value
         * of pclose() in order to determine success/failure of command
         * executed by popen().
         */

        if (WIFEXITED(ret)) {
            printf("Child exited, ret = %d\n", WEXITSTATUS(ret));
        } else if (WIFSIGNALED(ret)) {
            printf("Child killed, signal %d\n", WTERMSIG(ret));
        } else if (WIFSTOPPED(ret)) {
            printf("Child stopped, signal %d\n", WSTOPSIG(ret));
        }
        /* Not all implementations support this */
#ifdef WIFCONTINUED
        else if (WIFCONTINUED(ret)) {
            printf("Child continued\n");
        }
#endif
        /* Non standard case -- may never happen */
        else {
            printf("Unexpected return value, ret = 0x%x\n", ret);
        }
    }

    return EXIT_SUCCESS;
}
