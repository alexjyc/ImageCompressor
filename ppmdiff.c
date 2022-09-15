#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "assert.h"
#include "pnm.h"
#include "a2methods.h"
#include "a2blocked.h"

double power(unsigned denominator_1, unsigned denominator_2, unsigned num1, 
            unsigned num2);
void run(A2Methods_T methods, Pnm_ppm image_1, Pnm_ppm image_2);

int main(int argc, char *argv[])
{
    assert(argc == 3);
    
    A2Methods_T methods = uarray2_methods_blocked;
    int count_stdin = 0;
    FILE *fp_1,*fp_2;

    if (strcmp(argv[1], "-") == 0) {
        fp_1 = stdin;
        count_stdin += 1;
    } else {
        fp_1 = fopen(argv[1], "r");
    }

    if (strcmp(argv[2], "-") == 0) {
        fp_2 = stdin;
        count_stdin += 1;
    } else {
        fp_2 = fopen(argv[2], "r");
    }

    assert(count_stdin < 2 && fp_1 != NULL && fp_2 != NULL); 

    Pnm_ppm image_1 = Pnm_ppmread(fp_1, methods);
    Pnm_ppm image_2 = Pnm_ppmread(fp_2, methods); 
    run(methods, image_1, image_2);
    fclose(fp_1);
    fclose(fp_2);
    Pnm_ppmfree(&image_1);
    Pnm_ppmfree(&image_2);
    return EXIT_SUCCESS;
}

void run(A2Methods_T methods, Pnm_ppm image_1, Pnm_ppm image_2)
{
    (void) methods;
    int width_1 = image_1->width;
    int height_1 = image_1->height;
    int width_2 = image_2->width;
    int height_2 = image_2->height;

    printf("Original: (%d, %d)\n", width_1, height_1);
    printf("Decompressed: (%d, %d)\n", width_2, height_2);
    if (abs(width_1 - width_2) > 1 && abs(height_1 - height_2) > 1) {
        fprintf(stderr, "Dimension differs by more than 1. RMSE: %f\n", 1.0);
        return; 
    }

    int out_width = width_1, out_height = height_1;
    if (width_1 > width_2) {
        out_width = width_2;
    } 

    if (height_1 > height_2) {
        out_height = height_2;
    }

    A2Methods_UArray2 array1 = image_1->pixels, array2 = image_2->pixels;
    unsigned denom1 = image_1->denominator, denom2 = image_2->denominator;
    double rmse = 0;

    for (int i = 0; i < out_width; i++) {
        for (int j = 0; j < out_height; j++) {
            Pnm_rgb rgb1 = methods->at(array1, i, j);
            Pnm_rgb rgb2 = methods->at(array2, i, j);

            rmse = rmse + 
                   power(denom1, denom2, rgb1->red, rgb2->red) +
                   power(denom1, denom2, rgb1->green, rgb2->green) +
                   power(denom1, denom2, rgb1->blue, rgb2->blue);
        }
    }

    rmse = sqrt(rmse / (double)(3 * out_width * out_height));
    printf("RMSE: %0.4lf\n", rmse);
}

double power(unsigned denominator1, unsigned denominator2, unsigned num1, 
            unsigned num2) {

    double num = (double) num1 / (double) denominator1 -
                 (double) num2 / (double) denominator2;
    return num * num;
}