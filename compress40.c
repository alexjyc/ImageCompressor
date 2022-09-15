#include <stdlib.h>
#include "compress40.h"
#include "io.h"
#include "transform.h"
#include "assert.h"
#include "mem.h"
#include "pnm.h"
#include "a2methods.h"
#include "a2plain.h"

/*
 * Maximum pixel value of the output after decompression.
 */
const unsigned DENOMINATOR = 255;

/*
 * compress40
 *
 * Compress an input image from the given input stream and print to stdout 
 * as bytes in big Endian order.
 *
 * @param FILE *input - Input stream can be stdin or file input
 * 
 * @expect            - A2Method_T function pointers are not null;
 *                      specifically, methods->free
 * @expect            - Functions in the Transform module always return a new
 *                      2D array
 */
void compress40(FILE *input)
{
    A2Methods_T methods = uarray2_methods_plain;
    assert(methods != NULL);
    assert(methods->free != NULL);
    
    /* Read input image */
    Pnm_ppm pixmap = IO_read_plain_image(input, methods); 

    /* Normalize rgb */
    A2Methods_UArray2 image = pixmap->pixels;
    unsigned denominator = pixmap->denominator;
    A2Methods_UArray2 rgb = Transform_normalize(image, methods, denominator);

    /* Convert RGB to CV */
    A2Methods_UArray2 cv = Transform_rgb_to_cv(rgb, methods);
    methods->free(&rgb);

    /* Each 2 x 2 block is packed as a DCT component */
    A2Methods_UArray2 dct = Transform_cv_to_dct(cv, methods);
    methods->free(&cv);

    /* Quantize DCT for bitpacking */
    A2Methods_UArray2 quantized = Transform_quantize_dct(dct, methods);
    methods->free(&dct);

    /* Each cell contains a codedword represented by 64 bits integer */
    A2Methods_UArray2 word = Transform_dct_to_word(quantized, methods);
    methods->free(&quantized);

    IO_write_binary(stdout, word, methods, BLOCKSIZE, CODE_LENGTH);
    methods->free(&word);
    Pnm_ppmfree(&pixmap);
}

/*
 * decompress40
 *
 * Decompress an image from the given input stream. The decompressed image 
 * is printed to stdout in binary.
 *
 * @param FILE *input - Input stream can be stdin or file input; specifically
 *                      and methods->new, methods->free, methods->width, and
 *                      methods->height
 *
 * @expect            - A2Methods_T function pointers are not null
 */
void decompress40(FILE *input)
{
    A2Methods_T methods = uarray2_methods_plain;
    assert(methods != NULL);

    /* Codeword stored in 2D array is represented by 64 bits integer */
    A2Methods_UArray2 word = IO_read_binary(input, methods, BLOCKSIZE,
                                            CODE_LENGTH);

    /* Extract quantized field from codeword */
    A2Methods_UArray2 dct = Transform_word_to_dct(word, methods);
    methods->free(&word);

    /* Reverse quantization of field */
    A2Methods_UArray2 unquantized = Transform_unquantize_dct(dct, methods);
    methods->free(&dct);

    /* Convert DCT into cv representation */
    A2Methods_UArray2 cv = Transform_dct_to_cv(unquantized, methods);
    methods->free(&unquantized);

    /* Convert cv representation into rgb representation */
    A2Methods_UArray2 rgb = Transform_cv_to_rgb(cv, methods, DENOMINATOR);
    methods->free(&cv);

    Pnm_ppm pixmap;
    NEW(pixmap);

    pixmap->width = methods->width(rgb);
    pixmap->height = methods->height(rgb);
    pixmap->denominator = DENOMINATOR;
    pixmap->methods = methods;
    pixmap->pixels = rgb;

    Pnm_ppmwrite(stdout, pixmap);
    Pnm_ppmfree(&pixmap);
}