/*
 * Compile with:
 * gcc disklabel.c -o disklabel -Wall -W -Wextra -ansi -pedantic
 *
 * Each disk on a system may contain a disk label which provides
 * detailed information about the geometry of the disk and the
 * partitions into which the disk is divided.
 *
 * A copy of the in-core label for a disk can be obtained with the
 * DIOCGDINFO ioctl(2); this works with a file descriptor for a block or
 * character (``raw'') device for any partition of the disk.

 * For more information consult disklabel(5) man page.
*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/disklabel.h>
#include <sys/ioctl.h>

int main(int argc, char *argv[])
{
    struct disklabel dklbl;
    int fd;

    /* Check argument count */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s /dev/file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Open device file */
    if ((fd = open(argv[1], O_RDONLY)) == -1) {
        perror("open()");
        exit(EXIT_FAILURE);
    }

    /* Get disklabel by calling a disk-specific ioctl */
    if (ioctl(fd, DIOCGDINFO, &dklbl) == -1) {
        perror("ioctl()");
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("Disk: %s\n", dklbl.d_typename);

    return EXIT_SUCCESS;
}
