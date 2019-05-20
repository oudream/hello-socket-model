/*
 * Compile with:
 * gcc tree.c -o tree -Wall -W -Wextra -ansi -pedantic
 */

#include <stdio.h>
#include <stdlib.h>

#define LINES 10
#define COLS (2 * LINES)

int main(void)
{
    unsigned int i, j;

    for (i = 0; i < LINES; i++) {
        for (j = 0; j < COLS; j++) {
            if ((j < (COLS/2-i)) || (j > (COLS/2+i)))
                printf(" ");
            else
                printf("*");
        }
        printf("\n");
    }

    return EXIT_SUCCESS;
}
