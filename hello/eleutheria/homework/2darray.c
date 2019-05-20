#include <stdio.h>
#include <stdlib.h>

/* Incorrect  */
void foo1(int **mat, int rows, int cols)
{
    int i, j;

    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++)
            printf("%d ", mat[i][j]);
        printf("\n");
    }
}

/* Correct */
void foo2(int **mat, int rows, int cols)
{
    int i, j;
    int **aux;

    aux = malloc(rows * sizeof(int *));
    if (aux == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < rows; i++) {
        aux[i] = (int *)mat + cols * i;
        for (j = 0; j < cols; j++)
            printf("%d ", aux[i][j]);
        printf("\n");
    }

    free(aux);
}

/* Correct */
void foo3(int **mat, int rows, int cols)
{
    int i, j;

    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++)
            printf("%d ", *((int *)mat + cols * i) + j);
        printf("\n");
    }
}

int main(void)
{
    int mat[4][3] = { { 1,  2,  3},
                      { 4,  5,  6},
                      { 7,  8,  9},
                      {10, 11, 12} };

    foo3((int **)&mat[0][0], 4, 3);
    foo2((int **)&mat[0][0], 4, 3);
    /* foo1((int **)&mat[0][0], 4, 3); */

    return EXIT_SUCCESS;
}
