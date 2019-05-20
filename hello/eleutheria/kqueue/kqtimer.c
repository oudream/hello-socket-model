/*
 * Compile with:
 * gcc kqtimer.c -o kqtimer -Wall -W -Wextra -ansi -pedantic
 *
 * The following code will setup a timer that will trigger a
 * kevent every 5 seconds. Once it does, the process will fork
 * and the child will execute the date(1) command.
 */

#include <sys/event.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>   /* for strerror() */
#include <unistd.h>

/* Function prototypes */
void diep(const char *s);

int main(void)
{
    struct kevent change;    /* event we want to monitor */
    struct kevent event;     /* event that was triggered */
    pid_t pid;
    int kq, nev;

    /* Create a new kernel event queue */
    if ((kq = kqueue()) == -1)
        diep("kqueue()");

    /* Initialise kevent structure */
    EV_SET(&change, 1, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, 5000, 0);

    /* Loop forever */
    for (;;) {
        nev = kevent(kq, &change, 1, &event, 1, NULL);

        if (nev < 0)
            diep("kevent()");

        else if (nev > 0) {
            if (event.flags & EV_ERROR) {   /* report any error */
                fprintf(stderr, "EV_ERROR: %s\n", strerror(event.data));
                exit(EXIT_FAILURE);
            }

            if ((pid = fork()) < 0)         /* fork error */
                diep("fork()");

            else if (pid == 0)              /* child */
                if (execlp("date", "date", (char *)0) < 0)
                    diep("execlp()");
        }
    }

    /* Close kqueue */
    if (close(kq) == -1)
        diep("close()");

    return EXIT_SUCCESS;
}

void diep(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}
