/*
 * Compile with:
 * gcc charfreq.c -o charfreq -Wall -W -Wextra -ansi -pedantic
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    /*
     * When we partially initialize an array,
     * C automatically initializes the rest of it
     * to 0, NULL, etc, depending on the element type
     * of the array.
     * That said, the following is adequate:
     */
    unsigned int freq[26] = { 0 };    /* Frequencies for 'A' to 'Z' */
    unsigned int i, j, len, maxf;
    int c;
    FILE *fp;

    /* Check argument count */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s path\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Open file for reading */
    if ((fp = fopen(argv[1], "r")) == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    /* Count frequencies */
    while ((c = fgetc(fp)) != EOF) {
        c = toupper(c);
        if (c >= 'A' && c <= 'Z')
            freq[c - 'A']++;
    }

    /* Calculate size of array */
    len = sizeof freq / sizeof *freq;

    /* Get max frequency */
    maxf = freq[0];
    for (i = 1; i < len; i++)
        if (freq[i] > maxf)
            maxf = i;

    /* Print frequencies */
    i = maxf;
    for (i = freq[maxf]; i > 0; i--) {
        printf("%3u| ", i);
        for (j = 0; j < len; j++)
            if (freq[j] >= i)
                printf("*");
            else
                printf(" ");
        printf("\n");
    }

    /* Print letters */
    printf("     ");
    for (i = 0; i < len; i++)
        printf("%c", (char)('A' + i));
    printf("\n");


    /*
     * Close file
     * (since we opened the file only for read,
     * we assume that it is safe to not check against
     * the return value of fclose())
    */
    (void)fclose(fp);

    return EXIT_SUCCESS;
}
