#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#define NUM_THREADS 4
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
    printf("%d", extra);

    

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
   
    
    int count = 0;
    int index = 0;
    int limit = space/4;
    int height = alto/4;
//#pragma omp schedule (guided)
#pragma omp parallel sections num_threads(4) shared (my_arr,arr_in)
    {
         #pragma sections
         {
             
           #pragma section 
            {
            /* ------ Transformaci�n a escala de grises (1) ------*/
            for (int i = 0; i < limit; i += 3) {
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
             /* ------ Transformaci�n a espejo (1) ------*/
             for (int i = 0; i < height ; i++) {
                for (int j = 0; j < ancho - 1; j++) {
                    out_arr[(i * ancho) + j].b = my_arr[(i * ancho) + ancho - j].b;
                    out_arr[(i * ancho) + j].g = my_arr[(i * ancho) + ancho - j].g;
                    out_arr[(i * ancho) + j].r = my_arr[(i * ancho) + ancho - j].r;

                }
            }

              /* ------ Transformaci�n a Blur (1) ------*/
             for (int i = 0; i < height; i++) {
                 for (int j = 0; j < ancho; j++) {
                     padded_image[((i + padding_size) * (ancho + padding_size)) + j + padding] = out_arr[(i * ancho) + j];
                 }
             }

             double suma_b = 0;


             for (int i = 0; i < height; i++) {
                 for (int j = 0; j < ancho; j++) {
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

             
             
          }
            #pragma section 
           {
            /* ------ Transformaci�n a escala de grises (2) ------*/
            for (int i = limit; i < (limit*2); i += 3) {
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
             /* ------ Transformaci�n a espejo (2) ------*/
            for (int i = (height); i < height*2; i++) {
                for (int j = 0; j < ancho - 1; j++) {
                    out_arr[(i * ancho) + j].b = my_arr[(i * ancho) + ancho - j].b;
                    out_arr[(i * ancho) + j].g = my_arr[(i * ancho) + ancho - j].g;
                    out_arr[(i * ancho) + j].r = my_arr[(i * ancho) + ancho - j].r;

                }
            }
            
            /* ------ Transformaci�n a Blur (2) ------*/
            for (int i = height; i < 2 * height; i++) {
                for (int j = 0; j < ancho; j++) {
                    padded_image[((i + padding_size) * (ancho + padding_size)) + j + padding] = out_arr[(i * ancho) + j];
                }
            }

            double suma_b = 0;


            for (int i = height; i < 2 * height; i++) {
                for (int j = 0; j < ancho; j++) {
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

        }
           #pragma section 
            {
            /* ------ Transformaci�n a escala de grises (3) ------*/
            for (int i = (limit*2); i < limit*3; i += 3) {
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
            /* ------ Transformaci�n a espejo (3) ------*/
            for (int i =(height*2); i < height*3 ; i++) {
                for (int j = 0; j < ancho - 1; j++) {
                    out_arr[(i * ancho) + j].b = my_arr[(i * ancho) + ancho - j].b;
                    out_arr[(i * ancho) + j].g = my_arr[(i * ancho) + ancho - j].g;
                    out_arr[(i * ancho) + j].r = my_arr[(i * ancho) + ancho - j].r;

                }
            }

            /* ------ Transformaci�n a Blur (3) ------*/
            for (int i = 2 * height; i < 3 * height; i++) {
                for (int j = 0; j < ancho; j++) {
                    padded_image[((i + padding_size) * (ancho + padding_size)) + j + padding] = out_arr[(i * ancho) + j];
                }
            }

            double suma_b = 0;


            for (int i = 2 * height; i < 3 * height; i++) {
                for (int j = 0; j < ancho; j++) {
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
            
         }

            #pragma section
            {
            /* ------ Transformaci�n a escala de grises (4) ------*/  
            for (int i = (limit*3); i < space; i += 3) {
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
            /* ------ Transformaci�n a espejo (4) ------*/
            for (int i = (height*3); i < alto - 1 ; i++) {
                for (int j = 0; j < ancho - 1; j++) {
                    out_arr[(i * ancho) + j].b = my_arr[(i * ancho) + ancho - j].b;
                    out_arr[(i * ancho) + j].g = my_arr[(i * ancho) + ancho - j].g;
                    out_arr[(i * ancho) + j].r = my_arr[(i * ancho) + ancho - j].r;

                }
            }
            
            /* ------ Transformaci�n a Blur (4) ------*/
            for (int i = 3 * height; i < 4 * height; i++) {
                for (int j = 0; j < ancho; j++) {
                    padded_image[((i + padding_size) * (ancho + padding_size)) + j + padding] = out_arr[(i * ancho) + j];
                }
            }

            double suma_b = 0;


            for (int i = 3 * height; i < 4 * height; i++) {
                for (int j = 0; j < ancho; j++) {
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
         }
         
 }//scheduled (guided)
 }

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
        "img_sec_f1.bmp",
        "img_sec_f2.bmp",
        "img_sec_f3.bmp",
        "img_sec_f4.bmp",
        "img_sec_f5.bmp",
        "img_sec_f6.bmp",
        "img_sec_f7.bmp",
        "img_sec_f8.bmp",
        "img_sec_f9.bmp",
        "img_sec_f10.bmp"
    };
    #pragma omp parallel sections num_threads(4) shared (my_arr,arr_in)
    {
         #pragma sections
         {
         	printf("Imagen %d\n", 1);
        	processImages(nameOfImages[0], nameOfNewImages[0]);
         }
         #pragma sections
         {
         	printf("Imagen %d\n", 2);
        	processImages(nameOfImages[1], nameOfNewImages[1]);
         }
         #pragma sections
         {
         	printf("Imagen %d\n", 3);
        	processImages(nameOfImages[2], nameOfNewImages[2]);
         }
         #pragma sections
         {
         	printf("Imagen %d\n", 4);
        	processImages(nameOfImages[3], nameOfNewImages[3]);
         }
         #pragma sections
         {
         	printf("Imagen %d\n", 5);
        	processImages(nameOfImages[4], nameOfNewImages[4]);
         }
         #pragma sections
         {
         	printf("Imagen %d\n", 6);
        	processImages(nameOfImages[5], nameOfNewImages[5]);
         }
         #pragma sections
         {
         	printf("Imagen %d\n", 7);
        	processImages(nameOfImages[6], nameOfNewImages[6]);
         }
         #pragma sections
         {
         	printf("Imagen %d\n", 8);
        	processImages(nameOfImages[7], nameOfNewImages[7]);
         }
         #pragma sections
         {
         	printf("Imagen %d\n", 9);
        	processImages(nameOfImages[8], nameOfNewImages[8]);
         }
         #pragma sections
         {
         	printf("Imagen %d\n", 10);
        	processImages(nameOfImages[9], nameOfNewImages[9]);
         }
     }
    return 0;
}
