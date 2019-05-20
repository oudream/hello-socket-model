/*
 * Compile with:
 * gcc matrixmul.c -o matrixmul -lpthread -Wall -W -Wextra -ansi -pedantic
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct matrix {
    size_t rows;
    size_t cols;
    int **data;
} matrix_t;

typedef struct matrix_index {
    size_t row;
    size_t col;
} matindex_t;

typedef enum {
    MM_OK,
    MM_ENOMEM,
    MM_EIO
} mmret_t;

matrix_t *mat1, *mat2, *mat3;

/* Function prototypes */
mmret_t matrix_alloc(matrix_t **mat, size_t rows, size_t cols);
void matrix_free(matrix_t **mat);
mmret_t matrix_read(const char *path, matrix_t **mat);
void matrix_print(const matrix_t *mat);
void *mulvect(void *arg);
void diep(const char *s);

int main(int argc, char *argv[])
{
    pthread_t *tid;
    matindex_t *v;
    size_t i, j, k, mdepth, numthreads;

    /* Check argument count */
    if (argc != 3) {
        fprintf(stderr, "Usage: %s matfile1 matfile2\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /*
     * Initialize `mdepth' variable
     *
     * We increase `mdepth' every time we succesfully call malloc().
     * That way we can keep track of the "allocation depth" and easily
     * free the memory whenever needed, e.g. if a fatal error occurs.
     */
    mdepth = 0;

    /* Read matrix data from files */
    if (matrix_read(argv[1], &mat1) != MM_OK)
        goto CLEANUP_AND_EXIT;
    mdepth++;

    if (matrix_read(argv[2], &mat2) != MM_OK)
        goto CLEANUP_AND_EXIT;
    mdepth++;

    /* Is the multiplication feasible by definition? */
    if (mat1->cols != mat2->rows) {
        fprintf(stderr, "Matrices' dimensions size must satisfy"
                        "(NxM)(MxK)=(NxK)\n");
        goto CLEANUP_AND_EXIT;
    }

    /* Allocate memory for the result */
    if (matrix_alloc(&mat3, mat1->rows, mat2->cols) != MM_OK)
        goto CLEANUP_AND_EXIT;
    mdepth++;

    /* How many threads do we need ? */
    numthreads = mat1->rows * mat2->cols;

    /* v[k] holds the (i, j) pair in the k-th computation */
    if ((v = malloc(numthreads * sizeof *v)) == NULL) {
        perror("malloc()");
        goto CLEANUP_AND_EXIT;
    }
    mdepth++;

    /* Allocate memory for the threads' ids */
    if ((tid = malloc(numthreads * sizeof *tid)) == NULL) {
        perror("malloc()");
        goto CLEANUP_AND_EXIT;
    }
    mdepth++;

    /* Create the threads */
    for (i = 0; i < mat1->rows; i++) {
        for (j = 0; j < mat2->cols; j++) {
            k = i*mat1->rows + j;
            v[k].row = i;
            v[k].col = j;
            if (pthread_create(&tid[k], NULL, mulvect, (void *)&v[k])) {
                perror("pthread_create()");
                goto CLEANUP_AND_EXIT;
            }
        }
    }

    /* Make sure all threads are done  */
    for (i = 0; i < numthreads; i++)
        if (pthread_join(tid[i], NULL)) {
            perror("pthread_join()");
            goto CLEANUP_AND_EXIT;
        }

    /* Print the result */
    matrix_print(mat3);

 CLEANUP_AND_EXIT:;
    switch(mdepth) {
    case 5: free(tid);
    case 4: free(v);
    case 3: matrix_free(&mat3);
    case 2: matrix_free(&mat2);
    case 1: matrix_free(&mat1);
    case 0:  ;    /* free nothing */
    }

    return EXIT_SUCCESS;
}

mmret_t matrix_alloc(matrix_t **mat, size_t rows, size_t cols)
{
    size_t i, j, mdepth = 0;

    *mat = malloc(sizeof **mat);
    if (*mat == NULL)
        goto CLEANUP_AND_RETURN;
    mdepth++;

    (*mat)->rows = rows;
    (*mat)->cols = cols;

    (*mat)->data = malloc(rows * sizeof(int *));
    if ((*mat)->data == NULL)
        goto CLEANUP_AND_RETURN;
    mdepth++;

    for (i = 0; i < rows; i++) {
        (*mat)->data[i] = malloc(cols * sizeof(int));
        if ((*mat)->data[i] == NULL) {
            if (i != 0)
                mdepth++;
            goto CLEANUP_AND_RETURN;
        }
    }

    return MM_OK;

 CLEANUP_AND_RETURN:;
    perror("malloc()");
    switch(mdepth) {
    case 3: for (j = 0; j < i; j++) free((*mat)->data[j]);
    case 2: free((*mat)->data);
    case 1: free(*mat);
    case 0:  ;    /* free nothing */
    }

    return MM_ENOMEM;
}

void matrix_free(matrix_t **mat)
{
    size_t i;

    for (i = 0; i < (*mat)->rows; i++)
        free((*mat)->data[i]);

    free((*mat)->data);
    free(*mat);
}

mmret_t matrix_read(const char *path, matrix_t **mat)
{
    FILE *fp;
    size_t i, j, rows, cols;

    /* Open file */
    if ((fp = fopen(path, "r")) == NULL) {
        fprintf(stderr, "Error opening file: %s\n", path);
        return MM_EIO;
    }

    /* Read matrix dimensions */
    fscanf(fp, "%u%u", &rows, &cols);

    /* Allocate memory for matrix */
    if (matrix_alloc(mat, rows, cols) == MM_ENOMEM) {
        fclose(fp);
        return MM_ENOMEM;
    }

    /* Read matrix elements */
    for (i = 0; i < (*mat)->rows; i++) {
        for (j = 0; j < (*mat)->cols; j++) {
            fscanf(fp, "%d", &(*mat)->data[i][j]);
        }
   }

    /* Close file */
    fclose(fp);

    return MM_OK;
}

void matrix_print(const matrix_t *mat)
{
    size_t i, j;

    for (i = 0; i < mat->rows; i++) {
        for (j = 0; j < mat->cols; j++) {
            printf("%d ", mat->data[i][j]);
        }
        printf("\n");
    }
}


void *mulvect(void *arg)
{
    size_t i, row, col;

   row = *((int *)arg + 0);
   col = *((int *)arg + 1);

   mat3->data[row][col] = 0;
   for (i = 0; i < mat1->cols; i++)
      mat3->data[row][col] += mat1->data[row][i] * mat2->data[i][col];

   pthread_exit(NULL);
}

void diep(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}
