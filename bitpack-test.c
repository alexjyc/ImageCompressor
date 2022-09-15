#include <stdio.h>
#include <stdlib.h>
#include "utest.h"
#include "bitpack.h"

void printbytes(void *p, unsigned int len)
{
	unsigned int i;
	unsigned char *cp = (unsigned char *)p; 

	for (i = 0; i < len; i++) {
		printf("%02X", *cp++);
	}
}

UTEST(BitPack, UnsignedEmptyWidthNotFit)
{
    uint64_t x = 1;
    unsigned width = 0;
    EXPECT_FALSE(Bitpack_fitsu(x, width));
}

UTEST(BitPack, UnsignedEmptyWidthFit)
{
    uint64_t x = 0;
    unsigned width = 0;
    EXPECT_FALSE(Bitpack_fitsu(x, width));
}

UTEST(BitPack, UnsignedMaxWidthFit)
{
    uint64_t x = UINT64_MAX;
    unsigned width = 64;
    EXPECT_TRUE(Bitpack_fitsu(x, width));
}

UTEST(BitPack, UnsignedMaxNotFit)
{
    uint64_t x = UINT64_MAX;
    unsigned width = 63;
    EXPECT_FALSE(Bitpack_fitsu(x, width));
}

UTEST(BitPack, UnsignedAtBoundFit)
{
    uint64_t x = 15;
    unsigned width = 4;
    EXPECT_TRUE(Bitpack_fitsu(x, width));
}

UTEST(BitPack, UnsignedLargeFit)
{
    uint64_t x = 1;
    x = (x << 48);
    unsigned width = 50;
    EXPECT_TRUE(Bitpack_fitsu(x, width));
}

UTEST(BitPack, SignedMaxWidthFit)
{
    int64_t x = 0;
    unsigned width = 64;
    EXPECT_TRUE(Bitpack_fitss(x, width));
}

UTEST(BitPack, SignedEmptyWidthNotFit)
{
    int64_t x = 0;
    unsigned width = 0;
    EXPECT_FALSE(Bitpack_fitss(x, width));
}

UTEST(BitPack, SignedINT64_MaxFit)
{
    int64_t x = INT64_MAX;
    unsigned width = 64;
    EXPECT_TRUE(Bitpack_fitss(x, width));
}

UTEST(BitPack, SignedINT64_MinFit)
{
    int64_t x = INT64_MIN;
    unsigned width = 64;
    EXPECT_TRUE(Bitpack_fitss(x, width));
}

UTEST(BitPack, SignedAtBoundFit)
{
    int64_t upper = 7;
    unsigned width = 4;
    EXPECT_TRUE(Bitpack_fitss(upper, width));

    int64_t lower = -8;
    EXPECT_TRUE(Bitpack_fitss(lower, width));
}

UTEST(BitPack, SignedLargeFit)
{
    int64_t x = 1000;
    unsigned width = 11;
    EXPECT_TRUE(Bitpack_fitss(x, width));
}

UTEST(BitPack, SignedLargeNotFit)
{
    int64_t x = 1000;
    unsigned width = 10;
    EXPECT_FALSE(Bitpack_fitss(x, width));
}

UTEST(BitPack, UnsignedGetWidthGreaterThan64RaiseError)
{
    TRY
        Bitpack_getu(0, 65, 0);
        EXPECT_TRUE(0);
    ELSE
        printf("Exception: Get width greater than 64 raise error\n");
        EXPECT_TRUE(1);
    END_TRY;
}

UTEST(BitPack, UnsignedGetWidthAndLSBGreaterThan64RaiseError)
{
    TRY
        Bitpack_getu(0, 0, 65);
        EXPECT_TRUE(0);
    ELSE
        printf("Exception: Get width + lsb greater than 64 raise error\n");
        EXPECT_TRUE(1);
    END_TRY;
}

UTEST(BitPack, UnsignedGetUINT64_MAX)
{
    uint64_t x = UINT64_MAX;
    unsigned width = 64;
    unsigned lsb = 0;
    uint64_t out = Bitpack_getu(x, width, 0);
    EXPECT_EQ(x, out);
}

UTEST(BitPack, UnsignedGetEmptyWidthReturn0)
{
    uint64_t gt = 0;
    unsigned width = 0;
    unsigned lsb = 0;
    uint64_t out = Bitpack_getu(gt, width, lsb);
    EXPECT_EQ(gt, out);
}

UTEST(BitPack, UnsignedGetTest)
{
    EXPECT_EQ(Bitpack_getu(0x3f4, 6, 2), (uint64_t) 61);
    EXPECT_EQ(Bitpack_getu(2730, 6, 4), (uint64_t) 42);
}

UTEST(BitPack, UnsignedGetOddBits)
{
    /* 0000 1001 0100 */
    uint64_t gt = 37;
    /* 1110 1001 0100 */
    uint64_t input = 3732;
    unsigned width = 7;
    unsigned lsb = 2;
    uint64_t out = Bitpack_getu(input, width, lsb);
    EXPECT_EQ(gt, out);

    input = input << 30;
    lsb = 32;
    out = Bitpack_getu(input, width, lsb);
    EXPECT_EQ(gt, out);
}

UTEST(BitPack, UnsignedGetEvenBits)
{
    /* 1110 1001 0100 */
    uint64_t input = 3732;
    /*  110 100       */
    uint64_t gt = 52;
    unsigned width = 6;
    unsigned lsb = 5;
    uint64_t out = Bitpack_getu(input, width, lsb);
    EXPECT_EQ(gt, out);

    input = input << 45;
    lsb = 50;
    out = Bitpack_getu(input, width, lsb);
    EXPECT_EQ(gt, out); 

    EXPECT_EQ(Bitpack_getu(0x3f4, 6, 2), (uint64_t) 61);
}

UTEST(BitPack, SignedGetWidthGreaterThan64RaiseError)
{
    TRY
        Bitpack_gets(0, 65, 0);
        EXPECT_TRUE(0);
    ELSE
        printf("Exception: Get width greater than 64 raise error\n");
        EXPECT_TRUE(1);
    END_TRY;
}

UTEST(BitPack, SignedGetWidthAndLSBGreaterThan64RaiseError)
{
    TRY
        Bitpack_gets(0, 0, 65);
        EXPECT_TRUE(0);
    ELSE
        printf("Exception: Get width and lssb greater than 64 raise error\n");
        EXPECT_TRUE(1);
    END_TRY;
}

UTEST(BitPack, SignedGetINT64_MAX)
{
    int64_t gt = INT64_MAX;
    unsigned width = 64;
    unsigned lsb = 0;
    printf("%ld\n", Bitpack_gets(gt, width, lsb));
    EXPECT_EQ(Bitpack_gets(gt, width, lsb), gt);
}

UTEST(BitPack, SignedGetINT64_MIN)
{
    int64_t gt = INT64_MIN;
    unsigned width = 64;
    unsigned lsb = 0;
    EXPECT_EQ(Bitpack_gets(gt, width, lsb), gt);
}

UTEST(BitPack, SignedGetPositiveNumber)
{
    /* 1001 1111 0101 */
    int64_t x = 2549;
    int64_t gt = 5;
    unsigned width = 4;
    unsigned lsb = 0;
    EXPECT_EQ(Bitpack_gets(x, width, lsb), gt);

    int64_t y = x << 50;
    lsb = 50;
    EXPECT_EQ(Bitpack_gets(y, width, lsb), gt);

    gt = 31;
    width = 7;
    lsb = 4;
    EXPECT_EQ(Bitpack_gets(x, width, lsb), gt);
}

UTEST(BitPack, SignedGetNegative)
{
    int64_t gt = -1;
    int64_t flipped = 0;
    flipped = ~0;
    unsigned width = 64;
    unsigned lsb = 0;
    EXPECT_EQ(Bitpack_gets(flipped, width, lsb), gt);

    /* 1001 1111 0101 */
    int64_t x = 2549;
    gt = -3;
    width = 3;
    lsb = 0;
    EXPECT_EQ(Bitpack_gets(x, width, lsb), gt);

    width = 6;
    lsb = 2;
    EXPECT_EQ(Bitpack_gets(x, width, lsb), gt);

    EXPECT_EQ(Bitpack_gets(0x3f4, 6, 2), (int64_t) -3);
}

UTEST(BitPack, SignedGetEmptyWidthReturn0)
{
    int64_t gt = 0;
    EXPECT_EQ(Bitpack_gets(~gt, 0, 0), gt);
}

UTEST(BitPack, UnsignedNewWidthGreaterThan64RaiseError)
{
    TRY
        Bitpack_newu(0, 65, 0, 0);
        EXPECT_TRUE(0);
    ELSE
        printf("Exception: New width greater than 64 raise error\n");
        EXPECT_TRUE(1);
    END_TRY;
}

UTEST(BitPack, UnsignedNewWidthAndLSBGreaterThan64RaiseError)
{
    TRY
        Bitpack_newu(0, 0, 0, 80);
        EXPECT_TRUE(0);
    ELSE
        printf("Exception: New width and LSB greater than 64 raise error\n");
        EXPECT_TRUE(1);
    END_TRY;
}

UTEST(BitPack, UnsignedNewOverflowRaiseError)
{
    TRY
        uint64_t v = 8;
        unsigned width = 3;
        unsigned lsb = 0;
        Bitpack_newu(0, width, lsb, v);
        EXPECT_TRUE(0);
    EXCEPT(Bitpack_Overflow)
        printf("Exception: New overflow raised error\n");
        EXPECT_TRUE(1);
    END_TRY;
}

UTEST(BitPack, UnsignedNewUINT64_MAX)
{
    uint64_t max = UINT64_MAX;
    uint64_t x = 0;
    unsigned width = 64;
    unsigned lsb = 0;
    EXPECT_EQ(max, Bitpack_newu(x, width, lsb, max));
}

UTEST(BitPack, UnsignedNewUINT64_MIN)
{
    uint64_t gt = 0;
    uint64_t x = 0;
    x = ~x;
    unsigned width = 64;
    unsigned lsb = 0;
    EXPECT_EQ(gt, Bitpack_newu(x, width, lsb, gt));
}

UTEST(BitPack, UnsignedNewHighBit)
{
    uint64_t gt = (uint64_t) 1 << 63;
    uint64_t x = 0;
    unsigned width = 1;
    unsigned lsb = 63;
    EXPECT_EQ(gt, Bitpack_newu(x, width, lsb, 1));
}

UTEST(BitPack, UnsignedNewLowBit)
{
    uint64_t gt = 1;
    uint64_t x = 0;
    unsigned width = 1;
    unsigned lsb = 0;
    EXPECT_EQ(gt, Bitpack_newu(x, width, lsb, 1));
}

UTEST(BitPack, UnsignedSetEvenBit)
{
    /* 1011 0011 0010 */
    uint64_t x = 2866;
    /*   10 0101      */
    uint64_t v = 37;
    /* 1010 0101 0010 */ 
    uint64_t gt = 2642;
    unsigned lsb = 4;
    unsigned width = 6;
    EXPECT_EQ(gt, Bitpack_newu(x, width, lsb, v));

    /*   11 1010      */
    v = 58;
    /* 1011 1010 0010 */
    gt = 2978;
    EXPECT_EQ(gt, Bitpack_newu(x, width, lsb, v));
}

UTEST(BitPack, UnsignedSetOddBit)
{
    /* 1111 0101 1001 */
    uint64_t x = 3929;
    /* 0000 111       */
    uint64_t v = 7;
    /* 0000 1111 1001 */
    uint64_t gt = 249;
    unsigned lsb = 5;
    unsigned width = 7;
    EXPECT_EQ(gt, Bitpack_newu(x, width, lsb, v));

    /*       100 0000 */
    v = 64;
    /* 1111 0100 0000 */
    gt = 3904;
    lsb = 0;
    EXPECT_EQ(gt, Bitpack_newu(x, width, lsb, v));
}

UTEST(BitPack, UnsignedNewMisc)
{
    EXPECT_EQ((uint64_t) 698, Bitpack_newu(682, 3, 3, 7));
    EXPECT_EQ((uint64_t) 5878, Bitpack_newu(6070, 6, 4, 47));
    EXPECT_EQ((uint64_t) 4886, Bitpack_newu(6006, 8, 5, 152));
}

UTEST(BitPack, SignedNewMisc)
{
    EXPECT_EQ((int64_t) Bitpack_news(-128, 2, 2, -1), (int64_t) -116);
}