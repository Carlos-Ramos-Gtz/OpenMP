#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#define NUM_THREADS 5

int main()
{
    omp_set_num_threads(NUM_THREADS);
    FILE* image, * outputImage, * lecturas;
    image = fopen("sample3.bmp", "rb");          //Imagen original a transformar 
    outputImage = fopen("img_dd.bmp", "wb");    //Imagen transformada
    long ancho;
    long alto;
    long space;
    unsigned char r, g, b;               //Pixel RGB
    unsigned char xx[54];

    for (int i = 0; i < 54; i++) {
        xx[i] = fgetc(image);
        fputc(xx[i], outputImage);   //Copia cabecera a nueva imagen
    }

    ancho = (long)xx[21] * 16777216 + (long)xx[20] * 65536 + (long)xx[19] * 256 + (long)xx[18];
    alto = (long)xx[25] * 16777216 + (long)xx[24] * 65536 + (long)xx[23] * 256 + (long)xx[22];
    space = alto * ancho * 3; //Precálculo del tamaño de la imagen rgb

    printf("alto img %li\n", alto);  //2580
    printf("ancho img %li\n", ancho);//1800

    unsigned char* arr_in = (unsigned char*)malloc(space * sizeof(unsigned char));

    int j = 0;
    while (!feof(image)) {
        *(arr_in + j) = fgetc(image);
        j++;
    }
    printf("Ciclos: %d\n", j); //Tamaño total de imagen
    printf("Space:  %d\n", space); //Tamaño total precalculado
    
    /* ------ Transformación a escala de grises ------*/
    int count = 0;
#pragma omp schedule(guided)
    {
            #pragma omp for
            for (int i = 0; i < space; i += 3) {
                b = *(arr_in + i);
                g = *(arr_in + i + 1);
                r = *(arr_in + i + 2);
                unsigned char pixel = 0.21 * r + 0.72 * g + 0.07 * b;
                *(arr_in + i) = pixel;
                *(arr_in + i + 1) = pixel;
                *(arr_in + i + 2) = pixel;
                count += 3;
                if (count == ancho * 3) {
                    count = 0;
                }
            }
    }
    for (int i = 0; i < space; i++) {
        fputc(*(arr_in + i), outputImage);
    }

    fclose(image);
    fclose(outputImage);
    return 0;
}