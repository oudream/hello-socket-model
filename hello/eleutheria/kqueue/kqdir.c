/*
 * Compile with:
 * gcc kqdir.c -o kqdir -Wall -W -Wextra -ansi -pedantic
 *
 * The following program will monitor up to MAX_ENTRIES files
 * (in a directory supplied by the user), and will report any
 * delete, write or attributes' change operation.
*/

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>    /* for strerror () */
#include <unistd.h>
#include <sys/event.h>

#define MAX_ENTRIES 256

/* Function prototypes */
void diep(const char *s);

int main(int argc, char *argv[])
{
    struct kevent evlist[MAX_ENTRIES];    /* events we want to monitor */
    struct kevent chlist[MAX_ENTRIES];    /* events that were triggered */
    struct dirent *pdent;
    DIR *pdir;
    char fullpath[256];
    int fdlist[MAX_ENTRIES], cnt, i, error, kq, nev;

    /* Check argument count */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s directory\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Create a new kernel event queue */
    if ((kq = kqueue()) == -1)
        diep("kqueue()");

    /*
     * Open directory named by argv[1], associate a directory stream
     * with it and return a pointer to it.
     */
    if ((pdir = opendir(argv[1])) == NULL)
        diep("opendir()");

    /* Skip . and .. entries */
    cnt = 0;
    while((pdent = readdir(pdir)) != NULL && cnt++ < 2)
        ;    /* VOID */

    /*
     * Get all directory entries and for each one of them,
     * initialise a kevent structure.
    */
    cnt = 0;
    while((pdent = readdir(pdir)) != NULL) {
	/*
         * Check whether we have exceeded the max number of
         * entries that we can monitor.
         */
        if (cnt > MAX_ENTRIES - 1) {
            fprintf(stderr, "Max number of entries exceeded\n");
            goto CLEANUP_AND_EXIT;
        }

        /*
         * Check path length
         * don't forget +1 for the '\0'
         */
        if (strlen(argv[1] + strlen(pdent->d_name) + 2) > 256) {
            fprintf(stderr,"Max path length exceeded\n");
            goto CLEANUP_AND_EXIT;
        }
        strcpy(fullpath, argv[1]);
        strcat(fullpath, "/");
        strcat(fullpath, pdent->d_name);

	/* Open directory entry */
        if ((fdlist[cnt] = open(fullpath, O_RDONLY)) == -1) {
            perror("open()");
            goto CLEANUP_AND_EXIT;
        }

        /* Initialise kevent structure */
	EV_SET(&chlist[cnt], fdlist[cnt], EVFILT_VNODE,
               EV_ADD | EV_ENABLE | EV_ONESHOT,
               NOTE_DELETE | NOTE_EXTEND | NOTE_WRITE | NOTE_ATTRIB,
               0, 0);

        cnt++;
    }

    /* Loop forever */
    for (;;) {
        nev = kevent(kq, chlist, cnt, evlist, cnt, NULL);
        if (nev == -1)
            perror("kevent()");

        else if (nev > 0) {
            for (i = 0; i < nev; i++) {
                if (evlist[i].flags & EV_ERROR) {
                    fprintf(stderr, "EV_ERROR: %s\n", strerror(evlist[i].data));
                    goto CLEANUP_AND_EXIT;
                }

                if (evlist[i].fflags & NOTE_DELETE)
                    printf("fd: %d Deleted\n", evlist[i].ident);

                else if (evlist[i].fflags & NOTE_EXTEND
                         || evlist[i].fflags & NOTE_WRITE)
                    printf("fd: %d Modified\n", evlist[i].ident);

                else if (evlist[i].fflags & NOTE_ATTRIB)
                    printf("fd: %d Attributes modified\n", evlist[i].ident);
            }
        }
    }

    /* Clean up file descriptors, directory stream,  kqueue */
 CLEANUP_AND_EXIT:;
    error = EXIT_SUCCESS;
    for (i = 0; i < cnt; i++)
        if (close(fdlist[i]) == -1) {
            error = EXIT_FAILURE;
            perror("close");
        }

    if ((closedir(pdir) == -1) || (close(kq) == -1)) {
        error = EXIT_FAILURE;
        perror("close");
    }

    return error;
}

void diep(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}
