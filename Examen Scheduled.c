#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#define NUM_THREADS 150
#define NUMBER_OF_IMAGES 10
#define MAX_NAME_IMAGES 15

typedef struct my_pixel {
    unsigned char r, g, b;
}PIX;

void processImages(char nameOfImage[MAX_NAME_IMAGES], char nameOfNewImage[MAX_NAME_IMAGES])
{
    FILE* image, * outputImage, * lecturas;
    image = fopen(nameOfImage, "rb"); 
    outputImage = fopen(nameOfNewImage, "wb");    //Imagen transformada
    long ancho;
    long alto;
    long space;
    int extra = 0;
    unsigned char r, g, b;               //Pixel
    double t1, t2, tiempo;
    t1 = omp_get_wtime();
    
    int kernel_size = 21;
    //printf("Insert kernel size: ");
    //scanf("%d", &kernel_size);
    double kernel[kernel_size * kernel_size];
    
    for (int i = 0; i < kernel_size; i++) {
        for (int j = 0; j < kernel_size; j++) { 
            kernel[(i * kernel_size) + j] = 1.0 / (double)(kernel_size * kernel_size);
        }
    }
    int padding_size = kernel_size - 1;
    int padding = padding_size / 2;

    unsigned char xx[54];
    for (int i = 0; i < 54; i++) {
        xx[i] = fgetc(image);
        fputc(xx[i], outputImage);   //Copia cabecera a nueva imagen
    }
    
    ancho = (long)xx[21] * 16777216 + (long)xx[20] * 65536 + (long)xx[19] * 256 + (long)xx[18];
    alto = (long)xx[25] * 16777216 + (long)xx[24] * 65536 + (long)xx[23] * 256 + (long)xx[22];
    while (((ancho * 3) + extra) % 4 != 0) {
        extra++;
    }

    space = ((ancho * 3) + extra) * alto;

    unsigned char* arr_in = (unsigned char*)malloc(space * sizeof(unsigned char));
    unsigned char* arr_out = (unsigned char*)malloc(space * sizeof(unsigned char));
    PIX* my_arr = (PIX*)malloc(ancho * alto * sizeof(PIX));
    PIX* out_arr = (PIX*)malloc(ancho * alto * sizeof(PIX));
    PIX* padded_image = (PIX*)malloc((ancho + padding_size) * (alto + padding_size) * sizeof(PIX));
    printf("extra: %d\n", extra);

    

    int j = 0;
    PIX pixel_null;
    pixel_null.b = 0;
    pixel_null.g = 0;
    pixel_null.r = 0;

    printf("largo img %li \n", alto);
    printf("ancho img %li \n", ancho);
    
    for (int i = 0; i < space; i++) {
        *(arr_in + i) = fgetc(image);
        *(arr_out + i) = *(arr_in + i);
        j++;
    }
    printf("Ciclos: %d \n", j);
   // printf("Space:  %d \n", space);
   
    /* ------ Transformaci�n a escala de grises ------*/
    int count = 0;
    int index = 0;

#pragma omp scheduled (guided)
    {
            #pragma omp for
            for (int i = 0; i < space; i += 3) {
                b = *(arr_in + i);
                g = *(arr_in + i + 1);
                r = *(arr_in + i + 2);
                unsigned char pixel = 0.21 * r + 0.72 * g + 0.07 * b;
                my_arr[index].b = pixel;
                my_arr[index].g = pixel;
                my_arr[index].r = pixel;
                index++;
                count += 3;
                if (count == ancho * 3) {
                    i += extra;
                    count = 0;
                }
            }

     /* ------ Transformaci�n a espejo ------*/

            #pragma omp for collapse(2)
            for (int i = 0; i < alto - 1; i++) {
                for (int j = 0; j < ancho - 1; j++) {
                    out_arr[(i * ancho) + j].b = my_arr[(i * ancho) + ancho - j].b;
                    out_arr[(i * ancho) + j].g = my_arr[(i * ancho) + ancho - j].g;
                    out_arr[(i * ancho) + j].r = my_arr[(i * ancho) + ancho - j].r;

                }
            }
   // }//scheduled (guided)


     /* ----------- Efecto Blur ----------- */
            #pragma omp for collapse(2)
            for (int i = 0; i < alto + padding_size; i++) {
                for (int j = 0; j < ancho + padding_size; j++) {
                    padded_image[(i * (ancho + padding_size)) + j] = pixel_null;
                }
            }

            #pragma omp for collapse(2) 
            for (int i = 0; i < alto; i++) {
                for (int j = 0; j < ancho; j++) {
                    padded_image[((i + padding) * (ancho + padding_size)) + j + padding] = out_arr[(i * ancho) + j];
                }
            }

            double suma_b = 0;

            
            for (int i = 0; i < alto; i++) {
                for (int j = 0; j < ancho; j++) {
                    #pragma omp for collapse(2)
                    for (int k = 0; k < kernel_size; k++) {
                        for (int l = 0; l < kernel_size; l++) {
                            suma_b += (double)(padded_image[((i + k) * (ancho + padding_size)) + j + l].b) * kernel[(k * kernel_size) + l];
                        }
                    }
                    out_arr[(i * ancho) + j].b = (unsigned char)(suma_b);
                    out_arr[(i * ancho) + j].g = (unsigned char)(suma_b);
                    out_arr[(i * ancho) + j].r = (unsigned char)(suma_b);
                    suma_b = 0;
                }
            }
    }//scheduled (guided)

    index = 0;
    count = 0;
    for (int i = 0; i < space; i += 3) {
        *(arr_out + i) = out_arr[index].b;
        *(arr_out + i + 1) = out_arr[index].g;
        *(arr_out + i + 2) = out_arr[index].r;
        index++;
        count += 3;
        if (count == ancho * 3) {
            i += extra;
            count = 0;
        }

    }
    for (int i = 0; i < space; i++) {
        fputc(*(arr_out + i), outputImage);
    }

    fclose(image);
    fclose(outputImage);
    t2 = omp_get_wtime();
    tiempo = t2 - t1;
    printf("tomo (%lf) segundos \n", tiempo);
}

int main()
{
    omp_set_num_threads(NUM_THREADS);
    
    int currentImage = 0;
    char nameOfImages[NUMBER_OF_IMAGES][MAX_NAME_IMAGES] = {
        "Bmp/f1.bmp",
        "Bmp/f2.bmp",
        "Bmp/f3.bmp",
        "Bmp/f4.bmp",
        "Bmp/f5.bmp",
        "Bmp/f6.bmp",
        "Bmp/f7.bmp",
        "Bmp/f8.bmp",
        "Bmp/f9.bmp",
        "Bmp/f10.bmp"
    };
    char nameOfNewImages[NUMBER_OF_IMAGES][MAX_NAME_IMAGES] = {
        "img_sch_f1.bmp",
        "img_sch_f2.bmp",
        "img_sch_f3.bmp",
        "img_sch_f4.bmp",
        "img_sch_f5.bmp",
        "img_sch_f6.bmp",
        "img_sch_f7.bmp",
        "img_sch_f8.bmp",
        "img_sch_f9.bmp",
        "img_sch_f10.bmp"
    };
    #pragma omp scheduled (guided)
    {
        #pragma omp for
    	for(currentImage = 0; currentImage < NUMBER_OF_IMAGES; currentImage++)
	    {
	        printf("Imagen %d\n", currentImage+1);
	        processImages(nameOfImages[currentImage], nameOfNewImages[currentImage]);
	    }
    }
    
    return 0;
}
