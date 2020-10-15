#include <stdio.h>
#include <omp.h>

FILE* MatrixFileJammed;


int main(int argc, char const* argv[]) {
    float mat[100][100];
    float C[100][100];
    float num = 1.0;
    double t1, t2, tiempo;
    int n = 100, alpha = 2;
    int j, i;
    t1 = omp_get_wtime();
    MatrixFileJammed = fopen("MatrixFileJammed.txt", "w");

    //llenar matrices.
    for (int i = 0; i < 100; i+=2) {
        for (int j = 0; j < 100; j++) {
            mat[i][j] = j;
            mat[i + 1][j] = j;
            C[i][j] = 0;
            C[i + 1][j] = 0;
        }
    }

    //multiplicar matrices
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 100; j++) {
            for (int k = 0; k < 100; k++) {
                C[i][j] += mat[i][k] * mat[k][j];
            }
        }
    }

    //multiplicar resultado (C) por alpha=2
    for (int i = 0; i < 100; i+=2) {
        for (int j = 0; j < 100; j++) {
            C[i][j] *= alpha;
            C[i+1][j] *= alpha;
        }
    }

    //imprimir en archivo
    for (i = 0; i < 100; i++)
        for (j = 0; j < 100; j+=2) {
            //printf("%f , ", C[i][j]);
            fprintf(MatrixFileJammed, "%f ", C[i][j]);
            fprintf(MatrixFileJammed, "%f ", C[i][j+1]);
            if (j * 1 == 98) {
                //  printf("\n");
                fprintf(MatrixFileJammed, "\n", C[i][j]);
            }
        }
    t2 = omp_get_wtime();
    tiempo = t2 - t1;
    printf("tomo (%lf) segundos \n", tiempo);
    fclose(MatrixFileJammed);
}