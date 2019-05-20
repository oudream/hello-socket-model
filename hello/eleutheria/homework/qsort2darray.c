#include <stdio.h>
#include <stdlib.h>

/* Function prototypes */
void printarray(int a[3][3]);
int mycmp(const void *x, const void *y);

int main(void)
{
    int i;
    int a[3][3] = { { 1, 10, 3},
                    { 4,  0, 6},
                    {70, -3, 9}};

    printf("UNSORTED\n");
    printarray(a);

    /* Sort array on a per-row basis */
    for (i = 0; i < 3; i++)
        qsort(a[i], 3, sizeof(int), mycmp);

    printf("SORTED\n");
    printarray(a);

    return EXIT_SUCCESS;
}

void printarray(int a[3][3])
{
    int i, j;

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++)
            printf("%4d", a[i][j]);
        printf("\n");
    }
}

int mycmp(const void *x, const void *y)
{
    if (*(const int *)x > *(const int *)y)
        return 1;
    else if (*(const int *)x == *(const int *)y)
        return 0;
    else
        return -1;
}
