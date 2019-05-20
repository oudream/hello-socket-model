/*
 * WARNING: I got spontaneous hangups with the following code in NetBSD 4.99.20
 *
 * Compile with:
 * gcc readwd.c -o readwd -Wall -W -Wextra -ansi -pedantic
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dev/ata/atareg.h>
#include <sys/ataio.h>
#include <sys/ioctl.h>
#include <sys/param.h>

int main(int argc, char *argv[])
{
    int fd;
    unsigned int i;
    struct atareq req;
    union {
        unsigned char inbuf[DEV_BSIZE];
        struct ataparams inqbuf;
    } inbuf;
    u_int16_t *p;

    /* Check argument count */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s /dev/file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Open device file descriptor */
    if ((fd = open(argv[1], O_RDONLY)) == -1) {
        perror("open()");
        exit(EXIT_FAILURE);
    }

    /* Construct an ata request */
    memset(&req, 0, sizeof req);
    req.flags = ATACMD_READ;
    req.command = WDCC_IDENTIFY;
    req.databuf = (caddr_t) &inbuf;
    req.datalen = sizeof inbuf;
    req.timeout = 1000;    /* 1 sec */

    /* Make the ioctl call */
    if (ioctl(fd, ATAIOCCOMMAND, &req) == -1) {
        perror("ioctl()");
        exit(EXIT_FAILURE);
    }

    /* Handle ata request return status */
    switch (req.retsts) {
    case ATACMD_OK:
        break;
    case ATACMD_TIMEOUT:
        fprintf(stderr, "ata request timed out\n");
        exit(EXIT_FAILURE);
    case ATACMD_DF:
        fprintf(stderr, "ata device returned a device fault\n");
        exit(EXIT_FAILURE);
    case ATACMD_ERROR:
        fprintf(stderr, "ata device returned error code: %0x\n", req.error);
        exit(EXIT_FAILURE);
    default:
        fprintf(stderr, "unknown ata request return status: %d\n", req.retsts);
        exit(EXIT_FAILURE);
    }

    /*
     * Magic for little endian archs
     * FIXME: add #ifdef condition for little endian archs
     */
    for (i = 0; i < sizeof inbuf.inqbuf.atap_model; i+=2) {
        p = (u_int16_t *) (inbuf.inqbuf.atap_model + i);
        *p = ntohs(*p);
    }

    /* Print the model (trim spaces when printing) */
    for (i = 0; i < sizeof inbuf.inqbuf.atap_model; i++)
        if (inbuf.inqbuf.atap_model[i] != ' ')
            printf("%c", inbuf.inqbuf.atap_model[i]);
    printf("\n");

    /* Close file descriptor */
    close(fd);

    return EXIT_SUCCESS;
}
