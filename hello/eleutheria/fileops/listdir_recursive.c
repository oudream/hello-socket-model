/*
 * Compile with:
 * gcc listdir_recursive.c -o listdir_recursive -Wall -W -Wextra -ansi -pedantic
 */

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Since listdir() uses a static variable to keep track of the call depth,
 * it is not safe to use it in a multi threading environment. If this is the
 * case, then you need to pass 'dirdepth' as an argument to listdir().
 */
int listdir(const char *path)
{
    struct dirent *pdent;
    DIR *pdir;
    char *newpath = NULL;
    static unsigned int dirdepth = 0;
    unsigned int i;

    /*
     * Open directory named by path, associate a directory stream
     * with it and return a pointer to it
    */
    if ((pdir = opendir(path)) == NULL) {
        perror("opendir()");
        return -1;
    }

    /* Get all directory entries */
    while((pdent = readdir(pdir)) != NULL) {
        /* Indent according to the depth we are */
        for (i = 0; i < dirdepth; i++)
            printf("  ");

        /* Print current entry, or [entry] if it's a directory */
        if (pdent->d_type == DT_DIR)
            printf("[%s]\n", pdent->d_name);
        else
            printf("%s\n", pdent->d_name);

        /* Is it a directory ? If yes, list it */
        if (pdent->d_type == DT_DIR
            && strcmp(pdent->d_name, ".")
            && strcmp(pdent->d_name, "..")) {
            dirdepth++;

            /* Allocate memory for new path (don't forget +1 for the '\0') */
            if ((newpath = malloc(strlen(path) + strlen(pdent->d_name) + 2)) == NULL) {
                perror("malloc()");
                return -1;
            }

            /* Construct new path */
            strcpy(newpath, path);
            strcat(newpath, "/");
            strcat(newpath, pdent->d_name);

            /* To iterate is human, to recurse, divine */
            if (listdir(newpath) == -1) {
                closedir(pdir);
                free(newpath);
                return -1;
            }
        }
    }

    closedir(pdir);
    if (newpath != NULL)
        free(newpath);

    dirdepth--;
    return 1;
}

int main(int argc, char *argv[])
{
    /* Check argument count */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s directory\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    (void)listdir(argv[1]);

    return EXIT_SUCCESS;
}
