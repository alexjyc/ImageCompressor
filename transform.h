/*
 * transform.h
 *
 * Assignment: Arith
 * Authors: Nick Doan [hdoan02], Alex Jeon [yjeon02]
 * Date: 03/08/2022
 * 
 * Interface for compression and decompression of images. Every function in 
 * this interface returns a newly allocated 2D array. Clients are expected
 * to deallocate each array that is returned from the interface and not to 
 * modify any cells in the output array. Finally, they can choose to represent
 * 2D array as UArray2 or UArray2b.
 */
#ifndef TRANSFORM_INCLUDED
#define TRANSFORM_INCLUDED

#include "pnm.h"
#include "a2methods.h"

#define T A2Methods_UArray2
#define T_Interface A2Methods_T

/*
 * Scaling factor of the image during compression. The output image has half 
 * the dimension as the input image. It is an unchecked error for clients to 
 * modify BLOCKSIZE.
 */
static const int BLOCKSIZE = 2;

/*
 * Pack length of each codeword. Each codeword is packed into 32-bits. It is 
 * an unchecked error for clients to modify CODE_LENGTH.
 */
static const int CODE_LENGTH = 32; 

/******************************* COMPRESSION **********************************/

/*
 * Transform_normalize
 *
 * Divide each value in an RGB image by a given denominator. The output pixels
 * are in range [0, 1] for all 3 channels.
 *
 * @param T image             - 2D array where each cell is represented by
 *                              Pnm_rgb  
 * @param T_Interface methods - A method suites to interact with T
 * @param unsigned denom      - Maximum value in the input image
 * @return T                  - Normalized 2D array
 *
 * @expect                    - It is an unchecked error to input an image 
 *                              where each cell is not represented by Pnm_rgb
 * @expect                    - It is an unchecked error to modify a cell 
 *                              in the output array
 * @expect                    - It is a checked runtime error to pass in 
 *                              a null image or methods. A denom value greater
 *                              than 65535 will raise an error
 */
extern T Transform_normalize(T image, T_Interface methods, unsigned denom);

/*
 * Transform_rgb_to_cv
 *
 * Convert normalized rgb values to cv representation. 
 *
 * @param T image             - 2D array containing normalized rgb values
 * @param T_Interface methods - A method suites to interact with T 
 * @return T                  - 2D array in cv representation
 *
 * @expect                    - It is an unchecked error to input a 2D array
 *                              that was not returned from Transform_normalize
 * @expect                    - It is an unchecked error to modify a cell 
 *                              in the output array
 * @expect                    - It is a checked runtime error to pass in 
 *                              a null image or methods
 */
extern T Transform_rgb_to_cv(T image, T_Interface methods);

/*
 * Transform_cv_to_dct
 *
 * Convert cv values to dct representation.
 *
 * @param T image             - 2D array containing cv values
 * @param T_Interface methods - A method suites to interact with T
 * @return T                  - 2D array in dct representation
 *
 * @expect                    - It is an unchecked error to input a 2D array
 *                              that was not returned from Transform_rgb_to_cv
 * @expect                    - It is an unchecked error to modify a cell 
 *                              in the output array 
 * @expect                    - It is a checked runtime error to pass in 
 *                              a null image or methods
 */
extern T Transform_cv_to_dct(T image, T_Interface methods);

/*
 * Transform_quantize_dct
 *
 * Quantize dct values.
 *
 * @param T image             - 2D array in cv representation
 * @param T_Interface methods - A method suites to interact with T
 * @return T                  - 2D array which contains quantized dct values
 *
 * @expect                    - It is an unchecked error to input a 2D array
 *                              that was not returned from Transform_cv_to_dct
 * @expect                    - It is an unchecked error to modify a cell in the
 *                              output array
 * @expect                    - It is a checked runtime error to pass in 
 *                              a null image or methods
 */
extern T Transform_quantize_dct(T image, T_Interface methods);

/*
 * Transform_dct_to_word
 *
 * Convert quantized dct to codeword.
 *
 * @param T image             - 2D array in quantized dct values
 * @param T_Interface methods - A method suites to interact with T
 * @return T                  - 2D array which contains codeword
 *
 * @expect                    - It is an unchecked error to input a 2D array
 *                              that was not returned from
 *                              Transform_quantize_dct
 * @expect                    - It is an unchecked error to modify a cell in
 *                              the output array
 * @expect                    - It is a checked runtime error to pass in 
 *                              a null image or methods
 */
extern T Transform_dct_to_word(T image, T_Interface methods);

/*************************** END COMPRESSION **********************************/


/***************************** DECOMPRESSION **********************************/

/*
 * Transform_cv_to_rgb
 *
 * Convert cv representation to rgb, where each pixel value is in range
 * [0, denom].
 *
 * @param T image             - 2D array in cv values
 * @param T_Interface methods - A method suites to interact with T 
 * @param unsigned denom      - The maximum pixel value of the output image
 * @return T                  - Uncompressed image in rgb representation
 *
 * @expect                    - It is unchecked error to input a 2D array
 *                              that was not returned from Transform_dct_to_cv
 * @expect                    - It is an unchecked error to modify a cell in 
 *                              the output array
 * @expect                    - It is a checked runtime error to pass in 
 *                              a null image or methods. A denom value greater
 *                              than 65535 will raise an error
 */
extern T Transform_cv_to_rgb(T image, T_Interface methods, unsigned denom);

/* 
 * Transform_dct_to_cv
 *
 * Convert unquantized dct representation to cv representation. 
 *
 * @param T image             - 2D array of unquantized dct values
 * @param T_Interface methods - A method suites to interact with T
 * @return T                  - 2D array which contains cv values
 *
 * @expect                    - It is unchecked error to input a 2D array
 *                              that was not returned from 
 *                              Transform_unquantize_dct
 * @expect                    - It is an unchecked error to modify a cell in 
 *                              the output array
 * @expect                    - It is a checked runtime error to pass in 
 *                              a null image or methods. 
 */
extern T Transform_dct_to_cv(T image, T_Interface methods);

/*
 * Transform_unquantize_dct
 *
 * Unquantize dct values extracted from codeword.
 *
 * @param T image             - 2D array of values extracted from codeword
 * @param T_Interface methods - A method suites to interact with T
 * @return T                  - 2D array where each fields extracted from 
 *                              codeword is unquantized
 *
 * @expect                    - It is unchecked error to input a 2D array
 *                              that was not returned from Transform_word_to_dct
 * @expect                    - It is an unchecked error to modify a cell in 
 *                              the output array
 * @expect                    - It is a checked runtime error to pass in 
 *                              a null image or methods. 
 */
extern T Transform_unquantize_dct(T image, T_Interface methods);

/* 
 * Transform_word_to_dct
 *
 * Extract dct representation from codeword. 
 *
 * @param T image
 * @param T_Interface methods - A method suites to interact with T
 * @return T                  - 2D array which contains dct values from codeword
 *
 * @expect                    - It is unchecked error to input a 2D array
 *                              does not contain uint64_t values of codeword
 * @expect                    - It is an unchecked error to modify a cell in 
 *                              the output array
 * @expect                    - It is a checked runtime error to pass in 
 *                              a null image or methods. 
 */
extern T Transform_word_to_dct(T image, T_Interface methods);

/*************************** END DECOMPRESSION ********************************/

#undef T
#undef T_Interface
#endif