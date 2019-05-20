/*
 * Compile with:
 * gcc scanfprob.c -o scanfprob -Wall -W -Wextra -ansi -pedantic
 */

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    char buffer[100];
    int a, rv;

    /*
     * When scanf is attempting to convert numbers, any non-numeric characters
     * it encounters terminate the conversion and are left in the input stream.
     * Therefore, unexpected non-numeric input ``jams'' scanf again and again.
     * E.g. Try to type a char and see what happens with the following code:
     *
     * do {
     *     printf("Input: ");
     *     scanf("%d", &a);
     * } while (a != -1);
     *
     * See also: http://c-faq.com/stdio/scanfjam.html
     */

    /* First approach: check against the return value of scanf(3)  */
    do {
        printf("Input: ");
        rv = scanf("%d", &a);
        if (rv == 0)
            scanf("%[^\n]");
    } while (a != -1);

    /* Second approach: combine fgets(3) with sscanf(3)  */
    getchar();    /* Trim '\n' from previous */
    do {
        printf("Input: ");
        fgets(buffer, sizeof buffer, stdin);
        rv = sscanf(buffer, "%d", &a);
    } while (rv == 0 || a != -1);

    return EXIT_SUCCESS;
}
