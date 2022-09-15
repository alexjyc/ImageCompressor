/*
 * formulas.c
 *
 * Assignment: Arith
 * Authors: Nick Doan [hdoan02], Alex Jeon [ycheon02]
 * Date: 03/08/2021
 *
 * Helper mathematical formulas to apply compression and decompression.
 * Implementation sets the expected range after a specific formula is applied.
 */
#include <math.h>
#include "formulas.h"
#include "assert.h"

/*
 * Minimum and maximum value y can take. The value y is calculate from rgb
 * values and from the inverse calculation from a, b, c, and d.
 */
const float MIN_Y = 0.0, MAX_Y = 1.0;

/*
 * Minimum and maximum value a can take. The value a is calculated as the 
 * average of 4 y's values, where each y value ranges between [0, 1].
 */
const float MIN_A = 0.0, MAX_A = 1.0;

/*
 * Minimum and maximum value b, c, and d values can take. b, c, and d a values
 * are calculated from 4 y's values.
 */
const float MIN_BCD = -0.5, MAX_BCD = 0.5;

/*
 * Minimum and maximum pb and pr values. Pb abd pr values are calculated from
 * normalized rgb values.
 */
const float MIN_PBR = -0.5, MAX_PBR = 0.5;

/*
 * Formulas_get_even
 *
 * "Trim" an input number if it's odd.
 *
 * @param unsigned dimension - Input dimension to check
 * @return unsigned          - If dimension is odd, return dimension - 1, 
 *                             otherwise return dimenion
 */
unsigned Formulas_get_even(unsigned dimension)
{
    return dimension % 2 == 0 ? dimension : dimension - 1;
}

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
float Formulas_set_range(float pixel, float lower, float upper)
{
    if (pixel < lower) {
        return lower;
    } else if (pixel > upper) {
        return upper;
    } else {
        return pixel;
    }
}

/*
 * Formulas_average
 *
 * Calculate the average of the given input value.
 *
 * @param float *input, length  - Input value and number of input value 
 * @return float                - The average of the input value
 *
 * @expect                      - An error is raised if input is NULL
 */
float Formulas_average(float *input, float length)
{
    assert(input != NULL);
    float sum = 0.0;
    for (int i = 0; i < length; i++) {
        sum += *input++;
    }
    
    return sum / length;
}

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
signed Formulas_quantize(float pixel, float denominator, float upper)
{
    float scaled = roundf((pixel / denominator) * upper);
    return (signed) scaled;
}

/*
 * Formulas_inverse_quantize
 *
 * Apply inverse quantization. If the input is unsigned, the output has range
 * [-upper, upper]. If the input value is unsigned, the output has range
 * [0, upper]. 
 * 
 * @param float pixel       - Input value to be quantized
 * @param float denominator - The upper bound of the pixel after quantization.
 *                            Dividing pixel by denominator gets the values 
 *                            in range [0, 1] 
 * @param float upper       - The upper bound of the pixel before quantization
 * @return signed scaled    - Scaled signed integer
 */
float Formulas_inverse_quantize(float pixel, float denominator, float upper)
{
    float inversed = (pixel / upper) * denominator;
    return inversed;
}

/*
 * Formulas_normalize
 * 
 * Transform a pixel in range [0, 1].
 *
 * @param unsigned pixel       - Input value to be normalized
 * @param unsigned denominator - The maximum value a pixel can have
 * @returrn float              - Normalized pixel
 */
float Formulas_normalize(unsigned pixel, unsigned denominator)
{
    float normed = (float) pixel / (float) denominator;
    return Formulas_set_range(normed, 0.0, 1.0);
}

/*
 * Formulas_calculate_y
 *
 * Convert rgb to y value. Output y value has range [0, 1].
 *
 * @param float r, g, b
 * @return float        - y component in cv
 *
 * @expect              - Expect rgb to have been normalize
 */
float Formulas_calculate_y(float r, float g, float b)
{
    float y = (0.299 * r) + (0.587 * g) + (0.114 * b);
    return Formulas_set_range(y, MIN_Y, MAX_Y);
}

/*
 * Formulas_calculate_pb
 *
 * The value of pb is a real number in range [-0.5, 0.5].
 * 
 * @param float r, g, b
 * @return float        - pb component in cv
 *
 * @expect              - Expect rgb to have been normalize
 */
float Formulas_calculate_pb(float r, float g, float b)
{
    float pb = (-0.168736 * r) - (0.331264 * g) + (0.5 * b);
    return Formulas_set_range(pb, MIN_PBR, MAX_PBR);
}

/*
 * Formulas_calculate_pr
 *
 * Convert rgb to pr value. pr value range between [-0.5, 0.5].
 * 
 * @param float r, g, b
 * @return float        - pr component in cv
 *
 * @expect              - Expect rgb to have been normalize
 */
float Formulas_calculate_pr(float r, float g, float b)
{
    float pr = (0.5 * r) - (0.418688 * g) - (0.081312 * b);
    return Formulas_set_range(pr, MIN_PBR, MAX_PBR);
}

/*
 * Formulas_calculate_inverse_r
 *
 * Calculate r value from y, pb, and pr. 
 *
 * @param float y, pb, pr
 * @return float          - Red value
 */
float Formulas_calculate_inverse_r(float y, float pb, float pr)
{
    return (1.0 * y) + (0.0 * pb) + (1.402 * pr);
}

/*
 * Formulas_calculate_inverse_g
 *
 * Calculate g value from y, pb, and pr. 
 *
 * @param float y, pb, pr
 * @return float          - Green value
 */
float Formulas_calculate_inverse_g(float y, float pb, float pr)
{
    return (1.0 * y) - (0.344136 * pb) - (0.714136 * pr);
}

/*
 * Formulas_calculate_inverse_b
 *
 * Calculate b value from y, pb, and pr. 
 *
 * @param float y, pb, pr
 * @return float          - Blue value
 */
float Formulas_calculate_inverse_b(float y, float pb, float pr)
{
    return (1.0 * y) + (1.772 * pb) + (0.0 * pr);
}

/*
 * Formulas_calculate_a
 *
 * Calculate a value from 4 y components.
 *
 * @param float y_1, y_2, y_3, y_4
 * @return float a 
 */
float Formulas_calculate_a(float y_1, float y_2, float y_3, float y_4)
{
    float input[] = {y_1, y_2, y_3, y_4};
    float a = Formulas_average(input, sizeof(input) / sizeof(input[0]));
    return Formulas_set_range(a, MIN_A, MAX_A);
}

/*
 * Formulas_calculate_b
 *
 * Calculate a value from 4 y components.
 *
 * @param float y_1, y_2, y_3, y_4
 * @return float b 
 */
float Formulas_calculate_b(float y_1, float y_2, float y_3, float y_4)
{
    float b = (y_4 + y_3 - y_2 - y_1) / 4.0;
    return Formulas_set_range(b, MIN_BCD, MAX_BCD);
}

/*
 * Formulas_calculate_c
 *
 * Calculate c value from 4 y components.
 *
 * @param float y_1, y_2, y_3, y_4
 * @return float c 
 */
float Formulas_calculate_c(float y_1, float y_2, float y_3, float y_4)
{
    float c = (y_4 - y_3 + y_2 - y_1) / 4.0;
    return Formulas_set_range(c, MIN_BCD, MAX_BCD);
}

/*
 * Formulas_calculate_d
 *
 * Calculate d value from 4 y components.
 *
 * @param float y_1, y_2, y_3, y_4
 * @return float d 
 */
float Formulas_calculate_d(float y_1, float y_2, float y_3, float y_4)
{
    float d = (y_4 - y_3 - y_2 + y_1) / 4.0;
    return Formulas_set_range(d, MIN_BCD, MAX_BCD);
}

/*
 * Formulas_calculate_y1
 *
 * Calculate y1 value from a, b, c, d components.
 *
 * @param float a, b, c, d
 * @return float y1 
 *
 * @expect                 - a, b, c, d has not been quantized
 */
float Formulas_calculate_y1(float a, float b, float c, float d)
{
    float y1 = a - b - c + d;
    return Formulas_set_range(y1, MIN_Y, MAX_Y);
}

/*
 * Formulas_calculate_y2
 *
 * Calculate y2 value from a, b, c, d components.
 *
 * @param float a, b, c, d
 * @return float y2 
 *
 * @expect                 - a, b, c, d has not been quantized
 */
float Formulas_calculate_y2(float a, float b, float c, float d)
{
    float y2 = a - b + c - d;
    return Formulas_set_range(y2, MIN_Y, MAX_Y);
}

/*
 * Formulas_calculate_y3
 *
 * Calculate y3 value from a, b, c, d components.
 *
 * @param float a, b, c, d
 * @return float y3
 *
 * @expect                 - a, b, c, d has not been quantized
 */
float Formulas_calculate_y3(float a, float b, float c, float d)
{
    float y3 = a + b - c - d;
    return Formulas_set_range(y3, MIN_Y, MAX_Y);
}

/*
 * Formulas_calculate_y4
 *
 * Calculate y4 value from a, b, c, d components.
 *
 * @param float a, b, c, d
 * @return float y4 
 *
 * @expect                 - a, b, c, d has not been quantized
 */
float Formulas_calculate_y4(float a, float b, float c, float d)
{
    float y4 = a + b + c + d;
    return Formulas_set_range(y4, MIN_Y, MAX_Y);
}
