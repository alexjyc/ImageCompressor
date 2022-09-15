/*
 * formulas.h
 *
 * Assignment: Arith
 * Authors: Nick Doan [hdoan02], Alex Jeon [ycheon02]
 * Date: 03/09/2021
 *
 * Mathematical formulas for compression and decompression.
 */
#ifndef FORMULAS_INCLUDED
#define FORMULAS_INCLUDED

/*
 * Formulas_get_even
 *
 * "Trim" an input number if it's odd.
 *
 * @param unsigned dimension - Input dimension to check
 * @return unsigned          - An even number
 */
extern unsigned Formulas_get_even(unsigned dimension);

/*
 * Formulas_set_range
 *
 * Enforce an input value into a given lower and upper bound.
 *
 * @param float pixel - Input value to be set into a given range
 * @param float lower - The lower bound of the range
 * @param float upper - The upper bound of the range
 * @return float      - If input value is greater than the upper bound, return
 *                      upper. If input value is smaller than the lower bound,
 *                      return lower. Otherwise, pixel is returned
 */
extern float Formulas_set_range(float pixel, float lower, float upper);

/*
 * Formulas_average
 *
 * Calculate the average of the given input value.
 *
 * @param float *input, length  - Input value and number of input value 
 * @return float                - The average of the input value
 *
 * @expect                      - An error is raised if input is null
 */
extern float Formulas_average(float *input, float length);

/*
 * Formulas_quantize
 *
 * Quantize a value into a given range. If input value is signed, the output
 * has range [-upper, upper]. If the input value is unsigned, the output has
 * range [0, upper]. 
 * 
 * @param float pixel       - Input value to be quantized
 * @param float denominator - The upper bound of the pixel before quantization.
 *                            Dividing pixel by denominator gets the values 
 *                            in range [0, 1] 
 * @param float upper       - The upper bound of the pixel after quantization
 * @return signed scaled    - Quantized signed integer
 */
signed Formulas_quantize(float pixel, float denominator, float upper);
float Formulas_inverse_quantize(float pixel, float denominator, float upper);
extern float Formulas_normalize(unsigned pixel, unsigned denominator);

extern float Formulas_calculate_y(float r, float g, float b);
extern float Formulas_calculate_pb(float r, float g, float b);
extern float Formulas_calculate_pr(float r, float g, float b);

extern float Formulas_calculate_inverse_r(float y, float pb, float pr);
extern float Formulas_calculate_inverse_g(float y, float pb, float pr);
extern float Formulas_calculate_inverse_b(float y, float pb, float pr);

extern float Formulas_calculate_a(float y_1, float y_2, float y_3, float y_4);
extern float Formulas_calculate_b(float y_1, float y_2, float y_3, float y_4);
extern float Formulas_calculate_c(float y_1, float y_2, float y_3, float y_4);
extern float Formulas_calculate_d(float y_1, float y_2, float y_3, float y_4);

extern float Formulas_calculate_y1(float a, float b, float c, float d);
extern float Formulas_calculate_y2(float a, float b, float c, float d);
extern float Formulas_calculate_y3(float a, float b, float c, float d);
extern float Formulas_calculate_y4(float a, float b, float c, float d);

#endif