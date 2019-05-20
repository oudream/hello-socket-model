/*
 * Compile with:
 * gcc hexdump.c -o hexdump -Wall -W -Wextra -ansi -pedantic
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>    /* for memset() */
#include <getopt.h>    /* FIXME: make it portable */

/* Function prototypes */
void diep(const char *s);
void dieu(const char *pname);

#define BUFSIZE 20    /* Must be dividable by 2 */

int main(int argc, char *argv[])
{
    unsigned char buf[BUFSIZE + 1];    /* +1 for the '\0' */
    FILE *fp;
    int caps, cnt, i, j, len, opt, skip;
    int readbytes, totalbytes;
    char *fpath;

    /* Parse arguments */
    caps = 0;
    len = -1;
    skip = 0;
    fpath = NULL;
    while ((opt = getopt(argc, argv, "Cn:s:f:")) != -1) {
        switch (opt) {
        case 'C':
            caps = 1;
            break;
        case 'n':
            len = atoi(optarg);
            break;
        case 's':
            skip = atoi(optarg);
            break;
        case 'f':
            fpath = optarg;
            break;
        default:    /* '?' */
            dieu(argv[0]);
        }
    }

    /* optind shows to the argv[] index of the first non-option element */
    if (optind < argc) {
        fprintf(stderr, "non-option argv[]-elements: ");
        while (optind < argc)
            fprintf(stderr, "%s ", argv[optind++]);
        fprintf(stderr, "\n");
        exit(EXIT_FAILURE);
    }

    if (fpath == NULL)
        dieu(argv[0]);

    /* Open file */
    if ((fp = fopen(fpath, "r")) == NULL)
        diep("open");

    /* Skip `skip' bytes */
    fseek(fp, skip, SEEK_SET);

    /* Loop */
    cnt = 0;
    totalbytes = 0;
    while(!feof(fp) && (totalbytes < len || len == -1)) {
        /* Initialize buffer */
        memset(buf, 0, BUFSIZE);

        /* Read `BUFSIZE' elements of 1 byte long from file */
        readbytes = fread(buf, 1, BUFSIZE, fp);

        /* Print buffer */
        printf(caps == 1 ? "%08lX " : "%08lx ", (unsigned long)cnt * BUFSIZE + skip);
        for (i = 0;
             i < readbytes && (totalbytes < len || len == -1);
             i++, totalbytes++) {
            printf(caps == 1 ? "%02X " : "%02x ", buf[i]);
            if (i == (BUFSIZE / 2) - 1)
                printf(" ");
        }

        /* Fill in the blanks */
        for (j = 0; j < (BUFSIZE - i); j++) {
            printf("   ");
            if (j == (BUFSIZE/2) - 1)
                printf(" ");
        }

        /*
         * Print the output characters in the default character set.
         * Nonprinting characters are displayed as a single `.'
         */
        printf(" |");
        for (j = 0; j < i; j++) {
            if (buf[j] >= 32 && buf[j] <= 127)
                printf("%c", buf[j]);
            else
                printf(".");
        }
        printf("|");

        /* New line */
        printf("\n");

        cnt++;
    }

    /* Close device file */
    (void)fclose(fp);

    return EXIT_SUCCESS;
}

void diep(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}

void dieu(const char *pname)
{
    fprintf(stderr, "Usage: %s [-C] [-n length] [-s skip] -f file\n", pname);
    exit(EXIT_FAILURE);
}
