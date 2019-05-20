/*
 * Compile with:
 * gcc mmap.c -o mmap -Wall -W -Wextra -ansi -pedantic
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

/* Function prototypes */
void diep(const char *s);

int main(int argc, char *argv[])
{
    const char *message = "this shall be written to file\n";
    char *map;
    int fd;

    /* Check argument count */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Open file */
    if ((fd = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600)) == -1)
        diep("open()");

    /* Stretch the file size to the size of the (mmapped) array of chars */
    if (lseek(fd, strlen(message) - 1, SEEK_SET) == -1) {
        close(fd);
        diep("lseek()");
    }

    /*
     * Something needs to be written at the end of the file to
     * have the file actually have the new size.
     */
    if (write(fd, "", 1) != 1) {
        close(fd);
        diep("write()");
    }

    /* mmap() the file */
    if ((map = (char *)mmap(0, strlen(message),
                            PROT_WRITE,
                            MAP_SHARED, fd, 0)) == MAP_FAILED) {
        close(fd);
        diep("mmap()");
    }

    /* Copy message to mmapped() memory  */
    memcpy(map, message, strlen(message));

    /* Free the mmaped() memory */
    if (munmap(map, strlen(message)) == -1) {
        close(fd);
        exit(EXIT_FAILURE);
    }

    /* Close file */
    close(fd);

    return EXIT_SUCCESS;
}

void diep(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}
