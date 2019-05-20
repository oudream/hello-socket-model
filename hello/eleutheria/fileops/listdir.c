/* 
 * Compile with:
 * gcc listdir.c -o listdir -Wall -W -Wextra -ansi -pedantic
 */

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    struct dirent *pdent;
    DIR *pdir;

    /* Check argument count */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s directory\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /*
     * Open directory named by argv[1], associate a directory stream
     * with it and return a pointer to it
     */
    if ((pdir = opendir(argv[1])) == NULL) {
        perror("opendir()");
        exit(EXIT_FAILURE);
    }

    /* Get all directory entries */
    while((pdent = readdir(pdir)) != NULL)
        printf("%s\n", pdent->d_name);

    closedir(pdir);
    return EXIT_SUCCESS;
}
