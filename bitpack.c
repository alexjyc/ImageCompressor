/*
 * bitpack.c
 *
 * Assignment: Arith
 * Authors: Nick Doan [hdoan02], Alex Jeon [yjeon02]
 * Date: 03/09/2022
 *
 * Implementation of bitpacking on 64-bits integer. Supported functionalities
 * include width test, get, and update.
 */
#include "assert.h"
#include "bitpack.h" 

const unsigned MAX_WIDTH = 64;
Except_T Bitpack_Overflow = {"Overflow packing bits"};

/*
 * in_range
 *
 * Checks if input width is less than or equal to MAX_WIDTH
 *
 * @param unsigned width - Input width
 * @return bool          - True if width is less than or equal to max width,
 *                         otherwise false
 */
static bool in_range(unsigned width)
{
    return width <= MAX_WIDTH;
}

/*
 * shift_leftu
 *
 * Multiply an unsigned integer by 2^width by left shifting.
 *
 * @param uint64_t n     - Number to be shifted
 * @param unsigned width - How many time n is shifted left
 * @return shifted       - n x 2^width
 *
 * @expect               - 0 is returned if width is greater than or equal to
 *                         MAX_WIDTH
 */
static uint64_t shift_leftu(uint64_t n, unsigned width)
{
    uint64_t shifted = width < MAX_WIDTH ? n << width : 0;
    return shifted;
}

/*
 * shift_rightu
 *
 * Dividing an unsigned integer by 2^width by right shifting.
 *
 * @param uint64_t n     - Number to be shifted
 * @param unsigned width - How many time n is shifted right
 * @return shifted       - n / 2^width
 *
 * @expect               - 0 is returned if width is greater than or equal to
 *                         MAX_WIDTH
 */
static uint64_t shift_rightu(uint64_t n, unsigned width)
{
    uint64_t shifted = width < MAX_WIDTH ? n >> width : 0;
    return shifted;
}

/*
 * shift_lefts
 *
 * Multiply an signed integer by 2^width by left shifting.
 *
 * @param uint64_t n     - Number to be shifted
 * @param unsigned width - How many time n is shifted left
 * @return shifted       - n x 2^width
 *
 * @expect               - 0 is returned if width is greater than or equal to
 *                         MAX_WIDTH
 */
static int64_t shift_lefts(int64_t n, unsigned width)
{
    int64_t shifted = width < MAX_WIDTH ? n << width : 0;
    return shifted;
}

/*
 * shift_rights
 *
 * Dividing an signed integer by 2^width by left shifting. 
 *
 * @param uint64_t n     - Number to be shifted
 * @param unsigned width - How many time n is shifted right
 * @return shifted       - n / 2^width
 *
 * @expect               - 0 is returned if width is greater than or equal to
 *                         MAX_WIDTH
 * @expect               - When shifting a signed number right, the highest
 *                         order bit is copied over
 */
static int64_t shift_rights(int64_t n, unsigned width)
{
    int64_t shifted = width < MAX_WIDTH ? n >> width : 0;
    return shifted;
}

/*
 * Bitpack_fitsu
 *
 * Checks if an unsigned number can fit in a given width bits.
 *
 * @param uint64_t n     - Input number to be checked 
 * @param unsigned width - Number of bits
 * @return bool          - True if the number fits in the given width, otherwise
 *                         false
 *
 * @expect               - An error is raised if width is greater than MAX_WIDTH
 */
bool Bitpack_fitsu(uint64_t n, unsigned width)
{
    assert(in_range(width));

    if (width == MAX_WIDTH) {
        return true;
    } else if (width == 0) {
        return false;
    } else {
        uint64_t upper = shift_leftu((uint64_t) 1, width);
        return n < upper;
    }

    assert(0);
}

/*
 * Bitpack_fitss
 *
 * Checks if a signed number can fit in a given width bits.
 * 
 * @param uint64_t n      - Input number to be checked
 * @param unsigned width  - Number of bits
 * @return bool           - True if the given number fits in the given width, 
 *                          otherwise false
 *
 * @expect                - An error is raised if width is greater than 
 *                          MAX_WIDTH
 */
bool Bitpack_fitss(int64_t n, unsigned width)
{
    assert(in_range(width));

    if (width == MAX_WIDTH) {
        return true;
    } else if (width == 0) {
        return false;
    } else {
        /* The range is [-2^(width - 1), 2^(width - 1) - 1] */
        int64_t lower = shift_lefts((int64_t) -1, width - 1);
        int64_t upper = shift_lefts((int64_t) 1, width - 1);
        return (n >= lower && n < upper);
    }

    assert(0);
}

/*
 * Bitpack_getu
 *
 * Get a field of width-bits starting at bit lsb from an unsigned number.
 *
 * @param uint64_t word   - A word containing the field to be extracted from
 * @param unsigned width  - Number of bits to be extracted
 * @param unsigned lsb    - Which bits to start extracting from
 * @return uint64_t field - The extracted field is shifted to the start of the
 *                          unsigned integer
 *
 * @expect                - An error is raised if width is greater than 
 *                          MAX_WIDTH or width + lsb is greater than MAX_WIDTH
 */ 
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
    assert(in_range(width) && in_range(width + lsb));

    uint64_t field = 0;
    if (width > 0) {
        uint64_t mask = ~field; 
        unsigned shift = MAX_WIDTH - width, offset = lsb;
        mask = shift_rightu(mask, shift);
        mask = shift_leftu(mask, offset);

        field = mask & word;
        field = shift_rightu(field, offset);
    }

    return field;
}

/*
 * Bitpack_gets
 *
 * Get a field of width-bits starting at bit lsb from an unsigned number. The
 * returned field is in signed representation.
 *
 * @param uint64_t word  - A word containing to be extracted from
 * @param unsigned width - Number of bits to be extracted
 * @param unsigned lsb   - Which bits to start extracting from
 * @return int64_t field - The extracted field is shifted to the start of a
 *                         signed integer. If the field is 1111, then the 
 *                         output will be -1
 *
 * @expect               - An error is raised if width or width + lsb is greater
 *                         than MAX_WIDTH
 */ 
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
    assert(in_range(width) && in_range(width + lsb));

    int64_t field = 0;
    if (width > 0) {
        unsigned shift = MAX_WIDTH - width;
        field = Bitpack_getu(word, width, lsb);
        field = shift_lefts(field, shift);
        field = shift_rights(field, shift);
    }

    return field;
}

/*
 * Bitpack_newu
 *
 * Update an unsigned number at lsb-bit with a new value of width bits.
 *
 * @param uint64_t word  - A word containing to be extracted from
 * @param unsigned width - Number of bits to be extracted
 * @param unsigned lsb   - Which bits to start extracting from
 * @param uint64_t v     - Input number to be checked
 * @return uint64_t up   - An unsigned number that has been updated
 *
 * @expect               - An error is raised if width or width + lsb is
 *                         greater than MAX_WIDTH
 * @expect               - Bitpack_Overflow is raised if the value does not 
 *                         fit in width bits
 */ 
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb, uint64_t v)
{
    assert(in_range(width) && in_range(width + lsb));
    if (!Bitpack_fitsu(v, width)) {
        RAISE(Bitpack_Overflow);
    }

    uint64_t mask = 0;
    mask = ~0;
    /* 1's in the low bit */
    uint64_t low = shift_rightu((uint64_t) mask, MAX_WIDTH - lsb);
    /* 1's in the high bit */
    uint64_t high = shift_leftu((uint64_t) mask, width + lsb);
    uint64_t field = low | high; 
    uint64_t updated =  (word & field) | shift_leftu(v, lsb);

    return updated;
}

/*
 * Bitpack_news
 *
 * Update an unsigned number at lsb-bit with a new value of width bits.
 *
 * @param uint64_t word     - A word containing the field to be updated
 * @param unsigned width    - The number of bits in the value to be inserted
 * @param unsigned lsb      - Which bits to start extracting from
 * @param int64_t v         - Input number to be checked
 * @return uint64_t updated - An unsigned number that has been updated
 *
 * @expect                  - An error is raised if width or width + lsb is
 *                            greater than MAX_WIDTH
 * @expect                  - Bitpack_Overflow is raised if the value does not 
 *                            fit in width bits
 */ 
uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb, int64_t v)
{
    assert(in_range(width) && in_range(width + lsb));
    if (!Bitpack_fitss(v, width)) {
        RAISE(Bitpack_Overflow);
    }

    uint64_t field = Bitpack_getu(v, width, 0);
    uint64_t updated = Bitpack_newu(word, width, lsb, field);

    return updated;
}