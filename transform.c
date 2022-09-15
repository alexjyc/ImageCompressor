/*
 * transform.c
 *
 * Assignment: Arith
 * Authors: Nick Doan [hdoan02], Alex Jeon [yjeon02]
 * Date: 03/07/2022
 *
 * Implementation of transformation and decompression. The code has 2 sections:
 * COMPRESSION and DECOMPRESSION steps. For each step, an apply function is
 * followed by the corresponding public function.
 *
 * The implementation always creates a new 2D array for each step. This is 
 * to avoid pointers management.
 */
#include "transform.h"
#include "formulas.h"
#include "arith40.h"
#include "bitpack.h"
#include "assert.h"
#include "mem.h"

#define T A2Methods_UArray2
#define T_Interface A2Methods_T

const unsigned A_WIDTH = 9, BCD_WIDTH = 5, PBR_WIDTH = 4; 
const unsigned A_LSB = 23, B_LSB = 18, C_LSB = 13, D_LSB = 8;
const unsigned PB_LSB = 4, PR_LSB = 0;

const float A_RANGE = 511.0, BCD_DENOM = 0.3, BCD_RANGE = 15.0;

/*
 * struct Closure
 * 
 * Closure to be passed in apply function. To make the closure compatible with
 * all steps of the compression, denominator field is added even though it is 
 * not always used.
 *
 * @field A2Methods_UArray2 image - 2D array containing pixels to be extracted
 *                                  from during transformations
 * @field A2Methods_T methods     - Pointer to a struct containing methods to
 *                                  interact with image
 * @field unsigned denominator    - The denominator of an image; sets to 0 if
 *                                  unused
 */
typedef struct Closure {
    A2Methods_UArray2 image;
    A2Methods_T methods;
    unsigned denominator;
} *Closure;

/*
 * struct CVideo
 *
 * Represents the cv component of a pixel in a 2D array.
 *
 * @field float y  - Ranges between [0, 1]
 * @field float pb - Ranges between [-0.5, 0.5]
 * @field float pr - Ranges between [-0.5, 0.5]
 */
typedef struct CVideo {
    float y, pb, pr;
} *CVideo;

/*
 * struct Normalized_rgb
 *
 * Represents the scaled rgb value of a pixel in a 2D array. Normalization is 
 * defined as the division between a value and the denominator.
 *
 * @field float red, green, blue - Ranges between [0, 1]
 */
typedef struct Normalized_rgb {
    float red, green, blue;
} *Normalized_rgb;

/*
 * struct DCT
 *
 * Represents the component needed to pack a 2x2 block in a 2D image for 
 * compression.
 *
 * @field pb      - Takes the average of BLOCKSIZE x BLOCKSIZE pb values. The
 *                  average ranges between [-0.5, 0.5] 
 * @field pr      - Takes the average of BLOCKSIZE x BLOCKSIZE pr values. The
 *                  average ranges between [-0.5, 0.5]
 * @field a       - Ranges between [0, 1]
 * @field b, c, d - Ranges between [-0.5, 0.5] 
 */
typedef struct DCT {
    float pb, pr, a, b, c, d;
} *DCT;


/*
 * struct Word_component
 *
 * Represents the fields to be bit pack into a word. 
 *
 * @field pb, pb  - Computed from Arith40_index_of_chroma(x)
 * @field a       - 9 bits unsigned integer between [0, 511]
 * @field b, c, d - 5 bits signed integer between [-15, 15]
 */
typedef struct Word_component {
    uint64_t pb, pr, a;
    int64_t b, c, d;
} *Word_component;

/*
 * check_map_param
 *
 * This function is called for every apply function to check if ptr and cl are
 * non-null. cl is expected to be of type Closure.
 *
 * @param void *ptr - Current cell in the 2D array
 * @param void *cl  - Pointer to struct Closure
 *
 * @expect          - Null ptr or cl raises an error. Null cl->image or
 *                    cl->methods will also raise an error
 */
static void check_map_param(void *ptr, void *cl)
{
    assert(ptr != NULL && cl != NULL);
    Closure closure = cl;
    assert(closure->image != NULL && closure->methods != NULL);
}

/*
 * check_interface
 *
 * Ensures that function pointers for the interface to use A2Methods_UArray2
 * is valid. This function is called for every public function in transform.h.
 *
 * @param T_Interface methods - Struct pointers of type A2Methods_T. Users can
 *                              pass function pointers to work with UArray2
 *                              or UArray2_b
 *
 * @expect                    - An error is raised if methods is null. In
 *                              addition, if the function pointers new; at; 
 *                              width; height; or map_default is null, an error
 *                              is raised
 */
static void check_interface(T_Interface methods)
{
    assert(methods != NULL);
    assert(methods->new != NULL);
    assert(methods->at != NULL);
    assert(methods->width != NULL && methods->height != NULL);
    assert(methods->map_default != NULL);
}

/*
 * get_pixel
 *
 * Helper function to get all the pixels in 2 x 2 block. This is used in
 * Transform_cv_to_block and Transform_block_to_cv.
 *
 * @param T image             - Image to extract 2 x 2 pixels from
 * @param T_Interface methods - Struct pointers of type A2Methods_T 
 * @param CVideo *arr         - An array to store pointer to each cell
 * @param int i               - Starting col of the current block
 * @param int j               - Starting row of the current block
 *
 * @expect                    - An error is raised if methods, methods->at, or
 *                              arr is null
 */
static void get_pixel(T image, T_Interface methods, CVideo *arr, int i, int j)
{
    assert(methods != NULL);
    assert(methods->at != NULL);
    assert(arr != NULL);
    arr[0] = methods->at(image, i, j);
    arr[1] = methods->at(image, i + 1, j);
    arr[2] = methods->at(image, i, j + 1);
    arr[3] = methods->at(image, i + 1, j + 1);
}

/******************************* COMPRESSION **********************************/

/*
 * apply_normalization
 *
 * Apply function to normalize every rgb pixel value in an image. This function
 * is used in Transform_normalize. 
 *
 * @param int i     - Index to the current column
 * @param int j     - Index to the current row
 * @param T image
 * @param void *ptr - Pointer to the current cell in the map operation
 * @param void *cl  - Pointer to struct Closure. Function caller is expected
 *                    to set the denominator
 *
 * @expect          - See check_map_param for assertions on ptr and cl
 */
static void apply_normalization(int i, int j, T image, void *ptr, void *cl)
{
    (void) image;
    check_map_param(ptr, cl);

    Closure closure = cl;
    unsigned denominator = closure->denominator;
    /* Normalized pixel */
    Normalized_rgb output = ptr;
    /* Input pixel */
    Pnm_rgb input = closure->methods->at(closure->image, i, j);

    /* Divide rgb by denominator */
    output->red = Formulas_normalize(input->red, denominator);
    output->green = Formulas_normalize(input->green, denominator);
    output->blue = Formulas_normalize(input->blue, denominator);
}

/*
 * Transform_normalization
 *
 * Divide each rgb value in the image by a given denominator.
 *
 * @param T image              - 2D array where each cell is represented by 
 *                               Pnm_rgb
 * @param T_Interface methods  - Struct pointers of type A2Methods_T
 * @param unsigned denom       - Denominator for normalization
 * @return T normed            - 2D array where each cell is represented by
 *                               struct Normalized_rgb
 *
 * @expect                     - See check_interface for assertions on methods
 */
T Transform_normalize(T image, T_Interface methods, unsigned denom)
{
    check_interface(methods);

    /* Normalized image is represented as structs of floating rgb */
    int width = methods->width(image), height = methods->height(image);
    int size = sizeof(struct Normalized_rgb);
    T normed = methods->new(width, height, size);

    struct Closure cl = {
        .image = image, .methods = methods, .denominator = denom
    };
    methods->map_default(normed, apply_normalization, &cl);

    return normed;
}

/*
 * apply_rgb2cv
 *
 * Apply function to change every rgb pixel value in an image to cv. This
 * function is used in Transform_rgb_to_cv. 
 *
 * @param int i     - Index to the current column
 * @param int j     - Index to the current row
 * @param T image
 * @param void *ptr - Pointer to the current cell in the map operation
 * @param void *cl  - Pointer to struct Closure. Function caller is expected
 *                    to set the denominator
 *
 * @expect          - See check_map_param for assertions on ptr and cl
 */
static void apply_rgb2cv(int i, int j, T image, void *ptr, void *cl)
{
    (void) image;
    check_map_param(ptr, cl);

    Closure closure = cl;
    CVideo cv = ptr;
    Normalized_rgb rgb = closure->methods->at(closure->image, i, j);

    /* Convert normalized rgb to cv components */
    float r = rgb->red, g = rgb->green, b = rgb->blue;
    cv->y = Formulas_calculate_y(r, g, b);
    cv->pb = Formulas_calculate_pb(r, g, b);
    cv->pr = Formulas_calculate_pr(r, g, b);
}

/*
 * Transform_rgb_to_cv
 *
 * Convert an image from rgb representation to cv representation. 
 *
 * @param T image              - Normalized RGB array where each cell is 
 *                               represented by Normalized_rgb
 * @param T_Interface methods  - Struct pointers of type A2Methods_T
 * @return T cv                - 2D array where each cell is represented by
 *                               struct CVideo
 *
 * @expect                     - See check_interface for assertions on methods
 */
T Transform_rgb_to_cv(T image, T_Interface methods)
{
    check_interface(methods);

    /* In cv representation, an image has components y, pb, pr */
    int width = methods->width(image), height = methods->height(image);
    T cv = methods->new(width, height, sizeof(struct CVideo));

    struct Closure cl = {.image = image, .methods = methods, .denominator = 0};
    methods->map_default(cv, apply_rgb2cv, &cl);

    return cv;
}

/*
 * apply_cv2dct
 *
 * Apply function to change every cv value in an image to DCT representation.
 * This function is used in Transform_cv_to_block. 
 *
 * @param int i     - Index to the current column
 * @param int j     - Index to the current row
 * @param T image
 * @param void *ptr - Pointer to the current cell in the map operation
 * @param void *cl  - Pointer to struct Closure. Function caller is expected
 *                    to set the denominator
 *
 * @expect          - See check_map_param for assertions on ptr and cl
 */
static void apply_cv2dct(int i, int j, T image, void *ptr, void *cl)
{
    (void) image;
    check_map_param(ptr, cl);

    Closure closure = cl;
    DCT block = ptr;

    /* Column and row index to closure->image */
    int col = i * BLOCKSIZE, row = j * BLOCKSIZE;
    CVideo pixels[BLOCKSIZE * BLOCKSIZE];
    get_pixel(closure->image, closure->methods, pixels, col, row);

    float pb[] = {pixels[0]->pb, pixels[1]->pb, pixels[2]->pb, pixels[3]->pb};
    float pr[] = {pixels[0]->pr, pixels[1]->pr, pixels[2]->pr, pixels[3]->pr};
    float y_1 = pixels[0]->y;
    float y_2 = pixels[1]->y;
    float y_3 = pixels[2]->y;
    float y_4 = pixels[3]->y;

    block->pb = Formulas_average(pb, sizeof(pb) / sizeof(pb[0]));
    block->pr = Formulas_average(pr, sizeof(pr) / sizeof(pr[0]));
    block->a = Formulas_calculate_a(y_1, y_2, y_3, y_4);
    block->b = Formulas_calculate_b(y_1, y_2, y_3, y_4);
    block->c = Formulas_calculate_c(y_1, y_2, y_3, y_4);
    block->d = Formulas_calculate_d(y_1, y_2, y_3, y_4);
}

/*
 * Transform_cv_to_block
 *
 * Map through an image in cv representation and convert each cell to a, b, c,
 * d, pb, and pr value. These values are used for in compression. 
 * The resulting array has half the dimension.
 *
 * @param T image              - 2D array where each cell is represented by 
 *                               CVideo
 * @param T_Interface methods  - Struct pointers of type A2Methods_T
 * @return T block             - 2D array where each cell is represented by
 *                               struct DCT
 * 
 * @expect                     - See check_interface for assertions on methods
 */
T Transform_cv_to_dct(T image, T_Interface methods)
{
    check_interface(methods);

    int width = methods->width(image) / BLOCKSIZE;
    int height = methods->height(image) / BLOCKSIZE;
    T dct = methods->new(width, height, sizeof(struct DCT));
    
    struct Closure cl = {.image = image, .methods = methods, .denominator = 0};
    methods->map_default(dct, apply_cv2dct, &cl);

    return dct;
}

/*
 * apply_quantize_dct
 *
 * Apply function to quantize a, b, c, d, pb, and pr into a given range. This
 * function is used in Transform_quantize_block.
 *
 * @param int i     - Index to the current column
 * @param int j     - Index to the current row
 * @param T image
 * @param void *ptr - Pointer to the current cell in the map operation
 * @param void *cl  - Pointer to struct Closure. Function caller is expected
 *                    to set the denominator
 *
 * @expect          - See check_map_param for assertions on ptr and cl
 */
static void apply_quantize_dct(int i, int j, T image, void *ptr, void *cl)
{
    (void) image;
    check_map_param(ptr, cl);

    Closure closure = cl;
    Word_component word = ptr;
    DCT block = closure->methods->at(closure->image, i, j);

    /* Enforce b, c, and d into range [-BCD_DENOM, BCD_DENOM] */
    float b = Formulas_set_range(block->b, -1.0 * BCD_DENOM,
                                 BCD_DENOM);
    float c = Formulas_set_range(block->c, -1.0 * BCD_DENOM,
                                 BCD_DENOM);
    float d = Formulas_set_range(block->d, -1.0 * BCD_DENOM,
                                 BCD_DENOM);

    /* Quantize a into range [0, A_RANGE] */
    word->a = Formulas_quantize(block->a, 1.0, A_RANGE);
    /* Quantize b, c, and d into range [-BCD_RANGE, BCD_RANGE] */
    word->b = Formulas_quantize(b, BCD_DENOM, BCD_RANGE);
    word->c = Formulas_quantize(c, BCD_DENOM, BCD_RANGE);
    word->d = Formulas_quantize(d, BCD_DENOM, BCD_RANGE);
    word->pb = Arith40_index_of_chroma(block->pb);
    word->pr = Arith40_index_of_chroma(block->pr);
}

/*
 * Transform_quantize_dct
 *
 * Map through a 2D array containing pixels in DCT and apply quantization to
 * the fields.
 *
 * @param T image              - 2D array where each cell is represented by
 *                               struct DCT
 * @param T_Interface methods  - Struct pointer of type A2Methods_T
 * @return T block             - 2D array where each cell is represented by
 *                               struct Word_component
 *
 * @expect                     - See check_interface for assertions on methods
 */
T Transform_quantize_dct(T image, T_Interface methods)
{
    check_interface(methods);

    int width = methods->width(image), height = methods->height(image);
    T quantized = methods->new(width, height, sizeof(struct Word_component));

    struct Closure cl = {.image = image, .methods = methods, .denominator = 0};
    methods->map_default(quantized, apply_quantize_dct, &cl);

    return quantized;
}

/*
 * apply_dct2word
 *
 * Apply function to pack quantized field into a uint64_t word. This function
 * is used in Transform_dct_to_word.
 *
 * @param int i     - Index to the current column
 * @param int j     - Index to the current row
 * @param T image
 * @param void *ptr - Pointer to the current cell in the map operation
 * @param void *cl  - Pointer to struct Closure. Function caller is expected
 *                    to set the denominator
 *
 * @expect          - See check_map_param for assertions on ptr and cl
 */
static void apply_dct2word(int i, int j, T image, void *ptr, void *cl)
{
    (void) image;
    check_map_param(ptr, cl);

    Closure closure = cl;
    uint64_t *word_p = ptr;
    Word_component component = closure->methods->at(closure->image, i, j);

    uint64_t word = 0;
    word = Bitpack_newu(word, A_WIDTH, A_LSB, component->a);
    word = Bitpack_news(word, BCD_WIDTH, B_LSB, component->b);
    word = Bitpack_news(word, BCD_WIDTH, C_LSB, component->c);
    word = Bitpack_news(word, BCD_WIDTH, D_LSB, component->d);
    word = Bitpack_newu(word, PBR_WIDTH, PB_LSB, component->pb);
    word = Bitpack_newu(word, PBR_WIDTH, PR_LSB, component->pr);

    *word_p = word;
}

/*
 * Transform_dct_to_word
 *
 * Pack each quantized DCT fields into a uint64_t word. 
 *
 * @param T image             - 2D array where each cell is represented by 
 *                              struct Word_component
 * @param T_Interface methods - Struct pointer of type A2Methods_T
 * @return T codeword         - 2D array where each cell is represented by 
 *                              uint64_t
 * 
 * @expect                    - See check_interface for assertions on methods
 */
T Transform_dct_to_word(T image, T_Interface methods)
{
    check_interface(methods);

    int width = methods->width(image), height = methods->height(image);
    T codeword = methods->new(width, height, sizeof(uint64_t));

    struct Closure cl = {.image = image, .methods = methods, .denominator = 0};
    methods->map_default(codeword, apply_dct2word, &cl);

    return codeword;
}

/*************************** END COMPRESSION **********************************/

/***************************** DECOMPRESSION **********************************/

/*
 * apply_cv2rgb
 *
 * Apply function to convert cv representation to rgb representation. This
 * function is used in Transform_cv_to_rgb.
 *
 * @param int i     - Index to the current column
 * @param int j     - Index to the current row
 * @param T image 
 * @param void *ptr - Pointer to the current cell in the map operation
 * @param void *cl  - Pointer to struct Closure
 *
 * @expect          - See check_map_param for assertions on ptr and cl
 */
static void apply_cv2rgb(int i, int j, T image, void *ptr, void *cl)
{
    (void) image;
    check_map_param(ptr, cl);

    Closure closure = cl;
    Pnm_rgb rgb = ptr;
    CVideo cv = closure->methods->at(closure->image, i, j);

    unsigned denom = closure->denominator;
    float y = cv->y, pb = cv->pb, pr = cv->pr;
    float r = Formulas_calculate_inverse_r(y, pb, pr);
    float g = Formulas_calculate_inverse_g(y, pb, pr);
    float b = Formulas_calculate_inverse_b(y, pb, pr);
    
    /* Enforce rgb into range [0, 1] */
    r = Formulas_set_range(r, 0.0, 1.0);
    g = Formulas_set_range(g, 0.0, 1.0);
    b = Formulas_set_range(b, 0.0, 1.0);

    /* Quantize rgb into range [0, denom] */
    rgb->red = (unsigned) Formulas_quantize(r, 1.0, denom);
    rgb->green = (unsigned) Formulas_quantize(g, 1.0, denom);
    rgb->blue = (unsigned) Formulas_quantize(b, 1.0, denom);
}

/*
 * Transform_cv_to_rgb
 *
 * Map through an image in cv representation in rgb representation.
 *
 * @param T image             - 2D array where each cell is represented by 
 *                              struct DCT
 * @param T_Interface methods - Struct pointer of type A2Methods_T
 * @param T rgb               - 2D array where each cell is representeed by 
 *                              struct Pnm_rgb
 *
 * @expect                    - See check_interface for assertions on methods
 */
T Transform_cv_to_rgb(T image, T_Interface methods, unsigned denom)
{
    check_interface(methods);

    int width = methods->width(image), height = methods->height(image);
    int size = sizeof(struct Pnm_rgb);
    T rgb = methods->new(width, height, size);

    struct Closure cl = {
        .image = image, 
        .methods = methods, 
        .denominator = denom
    };
    methods->map_default(rgb, apply_cv2rgb, &cl);

    return rgb;
}

/*
 * apply_dct2cv
 *
 * Apply function to convert DCT to cv representation. This
 * function is used in Transform_dct_to_cv.
 *
 * @param int i     - Index to the current column
 * @param int j     - Index to the current row
 * @param T image 
 * @param void *ptr - Pointer to the current cell in the map operation
 * @param void *cl  - Pointer to struct Closure
 *
 * @expect          - See check_map_param for assertions on ptr and cl
 */
static void apply_dct2cv(int i, int j, T image, void *ptr, void *cl)
{
    (void) image;
    check_map_param(ptr, cl);

    Closure closure = cl;
    DCT block = ptr;

    int num_cell = BLOCKSIZE * BLOCKSIZE;
    int col = i * BLOCKSIZE, row = j * BLOCKSIZE;
    CVideo pixels[num_cell];
    get_pixel(closure->image, closure->methods, pixels, col, row);

    float a = block->a, b = block->b, c = block->c, d = block->d;
    float *y = CALLOC(num_cell, sizeof(float));
    y[0] = Formulas_calculate_y1(a, b, c, d);
    y[1] = Formulas_calculate_y2(a, b, c, d);
    y[2] = Formulas_calculate_y3(a, b, c, d);
    y[3] = Formulas_calculate_y4(a, b, c, d);

    for (int i = 0; i < num_cell; i++) {
        struct CVideo cv = {
            .y = y[i],
            .pb = block->pb,
            .pr = block->pr
        };
        *(pixels[i]) = cv;
    }

	FREE(y);
}

/*
 * Transform_dct_to_cv
 *
 * Map through an image in DCT to cv representation.
 *
 * @param T image             - 2D array where each cell is represented by 
 *                              struct DCT
 * @param T_Interface methods - Struct pointer of type A2Methods_T
 * @param T rgb               - 2D array where each cell is representeed by 
 *                              struct Pnm_rgb
 *
 * @expect                    - See check_interface for assertions on methods
 */
T Transform_dct_to_cv(T image, T_Interface methods)
{
    check_interface(methods);

    /* A 2D component array contains half the dimension as the original image */
    int width = methods->width(image) * BLOCKSIZE; 
    int height = methods->height(image) * BLOCKSIZE;
    T cv = methods->new(width, height, sizeof(struct CVideo));

    struct Closure cl = {.image = cv, .methods = methods, .denominator = 0};
    methods->map_default(image, apply_dct2cv, &cl);

    return cv;
}

/*
 * apply_unquantize_dct
 *
 * Apply function to unquantize a, b, c, d, pb, and pr into a given range.
 * This function is used in Transform_unquantize_quantize.
 *
 * @param int i     - Index to the current column
 * @param int j     - Index to the current row
 * @param T image 
 * @param void *ptr - Pointer to the current cell in the map operation
 * @param void *cl  - Pointer to struct Closure
 *
 * @expect          - See check_map_param for assertions on ptr and cl
 */
static void apply_unquantize_dct(int i, int j, T image, void *ptr, void *cl)
{
    (void) image;
    check_map_param(ptr, cl);

    Closure closure = cl;
    DCT block = ptr;
    Word_component word = closure->methods->at(closure->image, i, j);

    /* Enforce a into range [0, 1] */
    block->a = Formulas_inverse_quantize(word->a, 1.0, A_RANGE);
    /* Enforce b, c, and d into range [-0.3, 0.3] */
    block->b = Formulas_inverse_quantize(word->b, BCD_DENOM, BCD_RANGE);
    block->c = Formulas_inverse_quantize(word->c, BCD_DENOM, BCD_RANGE);
    block->d = Formulas_inverse_quantize(word->d, BCD_DENOM, BCD_RANGE);
    block->pb = Arith40_chroma_of_index(word->pb);
    block->pr = Arith40_chroma_of_index(word->pr);
}

/*
 * Transform_unquantize_dct
 *
 * Map through a 2D array containing quantized DCT pixels and unquantize
 * the fields.
 *
 * @param T image             - 2D array where each cell is represented by 
 *                              struct DCT
 * @param T_Interface methods - Struct pointer of type A2Methods_T
 * @param T rgb               - 2D array where each cell is representeed by 
 *                              struct Pnm_rgb
 *
 * @expect                    - See check_interface for assertions on methods
 */
T Transform_unquantize_dct(T image, T_Interface methods)
{
    check_interface(methods);

    int width = methods->width(image), height = methods->height(image);
    T block = methods->new(width, height, sizeof(struct DCT));

    struct Closure cl = {.image = image, .methods = methods, .denominator = 0};
    methods->map_default(block, apply_unquantize_dct, &cl);

    return block;
}

/*
 * apply_word2dct
 *
 * Apply function to convert word to quantized DCT representation. This
 * function is used in Transform_word_to_dct.
 *
 * @param int i     - Index to the current column
 * @param int j     - Index to the current row
 * @param T image 
 * @param void *ptr - Pointer to the current cell in the map operation
 * @param void *cl  - Pointer to struct Closure
 *
 * @expect          - See check_map_param for assertions on ptr and cl
 */
static void apply_word2dct(int i, int j, T image, void *ptr, void *cl)
{
    (void) image;
    check_map_param(ptr, cl);

    Closure closure = cl;
    uint64_t word = *(uint64_t *) closure->methods->at(closure->image, i, j);
    Word_component codeword = ptr;

    codeword->a = Bitpack_getu(word, A_WIDTH, A_LSB);
    codeword->b = Bitpack_gets(word, BCD_WIDTH, B_LSB);
    codeword->c = Bitpack_gets(word, BCD_WIDTH, C_LSB);
    codeword->d = Bitpack_gets(word, BCD_WIDTH, D_LSB);
    codeword->pb = Bitpack_getu(word, PBR_WIDTH, PB_LSB);
    codeword->pr = Bitpack_getu(word, PBR_WIDTH, PR_LSB);
}

/*
 * Transform_word_to_dct
 *
 * Map through an image in word representation and convert to quantized DCT 
 * representation.
 *
 * @param T image             - 2D array where each cell is represented by 
 *                              struct as uint64_t
 * @param T_Interface methods - Struct pointer of type A2Methods_T
 * @return T dct              - 2D array where each cell is represented by 
 *                              struct Word_component
 *
 * @expect                    - See check_interface for assertions on methods
 */
T Transform_word_to_dct(T image, T_Interface methods)
{
    check_interface(methods);

    int width = methods->width(image), height = methods->height(image);
    T dct = methods->new(width, height, sizeof(struct Word_component));

    struct Closure cl = {.image = image, .methods = methods, .denominator = 0};
    methods->map_default(dct, apply_word2dct, &cl); 

    return dct;
}

/*************************** END DECOMPRESSION ********************************/

#undef T
#undef T_Interface
