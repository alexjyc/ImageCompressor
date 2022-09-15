#include "io.h"
#include "bitpack.h"
#include "formulas.h"
#include "assert.h"

#define T A2Methods_UArray2
#define T_Interface A2Methods_T

const unsigned BYTE_WIDTH = 8;
const char *HEADER = "COMP40 Compressed image format 2\n%u %u";
const char DELIMITER = '\n';

typedef struct Metadata {
    FILE *fp;
    int code_length;
} *Metadata;

static void apply_trim(int i, int j, T image, void *ptr, void *cl)
{
    (void) image;
    assert(cl != NULL && ptr != NULL);

    Pnm_ppm input = cl;
    const struct A2Methods_T *methods = input->methods;
    Pnm_rgb output_pixel = ptr;
    *output_pixel = *(Pnm_rgb) methods->at(input->pixels, i, j);
}

Pnm_ppm IO_read_plain_image(FILE *fp, T_Interface methods)
{
    assert(fp != NULL && methods != NULL);

    Pnm_ppm image = Pnm_ppmread(fp, methods); 
    unsigned width = Formulas_get_even(image->width);
    unsigned height = Formulas_get_even(image->height);
    assert(width <= image->width && height <= image->height);

    /* Create a new array and copy pixels over if the dimension is reduced */
    if (width < image->width || height < image->height) {
        assert(methods->new != NULL);
        assert(methods->map_default != NULL);
        assert(methods->free != NULL);

        A2Methods_UArray2 prev = image->pixels;
        A2Methods_UArray2 new = methods->new(width, height, 
                                             sizeof(struct Pnm_rgb));
        methods->map_default(new, apply_trim, image);
        image->pixels = new;
        image->width = width;
        image->height = height;

        /* Free the previous 2D array */
        methods->free(&prev);
    }

    return image;
}

static void apply_write_binary(void *ptr, void *cl)
{
    assert(ptr != NULL && cl != NULL);
    Metadata data = cl;

    int high_byte = data->code_length - BYTE_WIDTH;
    uint32_t word = *(uint64_t *) ptr;
    for (int lsb = high_byte; lsb >= 0; lsb = lsb - BYTE_WIDTH) {
        uint8_t field = Bitpack_getu(word, BYTE_WIDTH, lsb);
        putchar((char) field);
    }
}

void IO_write_binary(FILE *fp, T image, T_Interface methods, int blocksize,
                     int code_length)
{
    assert(fp != NULL);
    assert(methods != NULL);
    assert(methods->width != NULL && methods->height != NULL);
    assert(methods->small_map_default != NULL);

    int width = methods->width(image) * blocksize;
    int height = methods->height(image) * blocksize;
    fprintf(fp, HEADER, width, height);
    fprintf(fp, "%c", DELIMITER);
    
    struct Metadata data = {.fp = NULL, code_length = code_length};
    methods->small_map_default(image, apply_write_binary, &data);
}

static void apply_read_binary(void *ptr, void *cl)
{
    assert(ptr != NULL && cl != NULL);

    Metadata data = cl;
    FILE *fp = data->fp;
    int high_byte = data->code_length - BYTE_WIDTH;
    uint64_t *word_p = ptr;

    uint64_t word = 0;
    for (int lsb = high_byte; lsb >= 0; lsb = lsb - BYTE_WIDTH) {
        int byte = getc(fp);
        assert(feof(fp) == 0 && byte != EOF);
        word = Bitpack_newu(word, BYTE_WIDTH, lsb, (uint64_t) byte);
    }

    *word_p = word;
}

T IO_read_binary(FILE *fp, T_Interface methods, int blocksize, int code_length)
{
    assert(fp != NULL);
    assert(methods != NULL);
    assert(methods->width != NULL && methods->height != NULL);
    assert(methods->new != NULL && methods->small_map_default != NULL);

    unsigned width = 0, height = 0;
    int read = fscanf(fp, HEADER, &width, &height);

    assert(read == 2);
    int c = getc(fp);
    assert(c == DELIMITER);

    width = width / blocksize;
    height = height / blocksize;
    A2Methods_UArray2 word = methods->new(width, height, sizeof(uint64_t));

    struct Metadata data = {.fp = fp, .code_length = code_length};
    methods->small_map_default(word, apply_read_binary, &data);

    return word;
}

#undef T
#undef T_Interface